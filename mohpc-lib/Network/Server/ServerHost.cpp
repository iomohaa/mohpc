#include <MOHPC/Network/Server/ServerHost.h>
#include <MOHPC/Network/Remote/Socket.h>
#include <MOHPC/Network/Remote/Channel.h>
#include <MOHPC/Network/Remote/Encoding.h>
#include <MOHPC/Network/Remote/Ops.h>
#include <MOHPC/Network/Configstring.h>
#include <MOHPC/Network/Remote/IPRemoteIdentifier.h>
#include <MOHPC/Network/Remote/UDPMessageDispatcher.h>
#include <MOHPC/Utility/Misc/MSG/Codec.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>
#include <MOHPC/Utility/Info.h>
#include <MOHPC/Utility/TokenParser.h>
#include <MOHPC/Common/Log.h>

#include <chrono>
#include <random>

using namespace MOHPC;
using namespace MOHPC::Network;

MOHPC_OBJECT_DEFINITION(Challenge);

Challenge::Challenge(const NetAddrPtr& inFrom, clientNetTime inTime, uint32_t inChallenge)
	: time(inTime)
	, challenge(inChallenge)
	, from(inFrom)
{
}


bool Challenge::hasElapsed(std::chrono::milliseconds elapsedTime) const
{
	clientNetTime currentTime = std::chrono::steady_clock::now();
	return time >= currentTime + elapsedTime;
}

uint32_t Challenge::getChallenge() const
{
	return challenge;
}

const NetAddr& Challenge::getSourceAddress() const
{
	return *from;
}

MOHPC_OBJECT_DEFINITION(ClientData);

ClientData::ClientData(const IUdpSocketPtr& inSocket, const NetAddrPtr& from, uint16_t inQport, uint32_t inChallenge)
	: socket(inSocket)
	, source(from)
	, challengeNum(inChallenge)
	, sequenceNum(0)
	, qport(inQport)
	, encoder(inChallenge, reliableCommands)
	, decoder(inChallenge, serverCommands)
{
	clientSequence = 0;
	numCommands = 0;

	// FIXME: NULL
	//encoding = Encoding::create(challengeNum, (const char**)serverCommandList, (const char**)reliableCommandList);
}

const NetAddrPtr& ClientData::getAddress() const
{
	return source;
}

uint16_t ClientData::getQPort() const
{
	return qport;
}

XOREncoding& ClientData::getEncoder()
{
	return encoder;
}

XOREncoding& ClientData::getDecoder()
{
	return decoder;
}

uint32_t ClientData::newSequence()
{
	return ++sequenceNum;
}

MOHPC_OBJECT_DEFINITION(ServerHost);

ServerHost::ServerHost()
{
	NetAddr4 bindAddress;
	bindAddress.port = 12203;

	serverSocket = ISocketFactory::get()->createUdp(&bindAddress);
	conChan = ConnectionlessChan::create(UDPCommunicator::create(serverSocket));

	serverId = rand();
}

ClientData& ServerHost::createClient(const NetAddrPtr& from, uint16_t qport, uint32_t challengeNum)
{
	ClientDataPtr data = ClientData::create(serverSocket, from, qport, challengeNum);
	clientList.push_back(data);

	return *data;
}

ClientData* ServerHost::findClient(const NetAddr& from, uint16_t qport) const
{
	for(auto clientDataPtr = clientList.begin(); clientDataPtr != clientList.end(); ++clientDataPtr)
	{
		ClientData* clientData = clientDataPtr->get();
		if (*clientData->getAddress() == from && clientData->getQPort() == qport)
		{
			return clientData;
		}
	}

	return nullptr;
}

Challenge& ServerHost::createChallenge(const NetAddrPtr& from)
{
	std::random_device rd;

	return challenges.emplace_back(from, std::chrono::steady_clock::now(), rd());
}

uintptr_t ServerHost::getChallenge(const NetAddr& from) const
{
	for (auto challenge = challenges.begin(); challenge != challenges.end(); ++challenge)
	{
		if (challenge->getSourceAddress() == from)
		{
			return challenge - challenges.begin();
		}
	}

	return 0;
}

void ServerHost::tick(deltaTime_t deltaTime, tickTime_t currentTime)
{
	for (auto challenge = challenges.begin(); challenge != challenges.end(); ++challenge)
	{
		if (challenge->hasElapsed(std::chrono::milliseconds(10000)))
		{
			// remove the challenge if it has expired
			challenge = challenges.erase(challenge);
		}
	}

	if (serverSocket->dataCount())
	{
		processRequests();
	}

	sendStateToClients();
}

void ServerHost::connectionLessReply(const NetAddrPtr& target, const char* reply)
{
	uint8_t transmission[MAX_UDP_DATA_SIZE];
	FixedDataMessageStream clientStream(transmission, MAX_UDP_DATA_SIZE);

	MSG clientMessage(clientStream, msgMode_e::Writing);
	clientMessage.SetCodec(MessageCodecs::OOB);
	clientMessage.WriteInteger(-1);
	clientMessage.WriteByte(0);
	clientMessage.WriteString(reply);

	clientStream = FixedDataMessageStream(transmission, clientStream.GetPosition());

	IPRemoteIdentifier id(target);
	conChan->transmit(id, clientStream);
}

void ServerHost::processRequests()
{
	uint8_t data[MAX_UDP_DATA_SIZE];

	NetAddrPtr from;
	const size_t len = serverSocket->receive((void*)data, sizeof(data), from);
	FixedDataMessageStream stream(data, len);

	MSG msg(stream, msgMode_e::Reading);
	msg.SetCodec(MessageCodecs::OOB);

	const uint32_t sequenceNum = msg.ReadInteger();

	using namespace std::chrono;
	clientNetTime requestTime = steady_clock::now();

	if (sequenceNum == -1)
	{
		// connectionless command
		const netsrc_e dirByte = (netsrc_e)msg.ReadByte();
		if (dirByte != netsrc_e::Server) {
			return;
		}

		StringMessage command = msg.ReadString();
		if (!strHelpers::icmp(command.c_str(), "getchallenge"))
		{
			Challenge* challenge;

			const uintptr_t challengeIndex = getChallenge(*from);
			if (challengeIndex)
			{
				// a challenge already exist for this ip
				challenge = &challenges.at(challengeIndex);
			}
			else
			{
				// create the challenge for the address
				challenge = &createChallenge(from);
			}

			// send the client challenge
			connectionLessReply(from, ("challengeResponse " + std::to_string(challenge->getChallenge())).c_str());
		}
		else if (!strHelpers::icmp(command.c_str(), "connect "))
		{
			FixedDataMessageStream compressedStream(data + 13, len - 13);

			uint8_t uncompressed[MAX_UDP_DATA_SIZE];
			FixedDataMessageStream uncompressedStream(uncompressed, sizeof(uncompressed));

			CompressedMessage compressedMessage(compressedStream, uncompressedStream);
			compressedMessage.Decompress(0, sizeof(uncompressed));

			uncompressedStream = FixedDataMessageStream(uncompressed, uncompressedStream.GetPosition());

			MSG connectMessage(uncompressedStream, msgMode_e::Reading);
			connectMessage.SetCodec(MessageCodecs::OOB);

			// read the user connect arguments
			StringMessage arguments = connectMessage.ReadString();

			// get the user info (without quotes)
			const char* userInfo = arguments + 1;
			ReadOnlyInfo info(userInfo, strlen(userInfo) - 1);

			const uint32_t challengeNum = info.IntValueForKey("challenge");
			// find the challenge by the source address
			const uintptr_t challengeIndex = getChallenge(*from);
			if (!challengeIndex)
			{
				// don't continue further
				return;
			}

			const Challenge& challenge = challenges.at(challengeIndex);
			if (challenge.getChallenge() != challengeNum)
			{
				// bad challenge
				return;
			}

			challenges.erase(challenges.begin() + challengeIndex);

			uint16_t qport = info.IntValueForKey("qport");

			// success, now create the client
			ClientData& client = createClient(from, qport, challengeNum);

			// inform the client that it's connected
			connectionLessReply(from, "connectResponse");
		}
	}
	else
	{
		// in-game packet
		const uint32_t qport = msg.ReadUShort();

		ClientData* client = findClient(*from, qport);
		if (!client)
		{
			// Not connected
			uint8_t transmission[MAX_UDP_DATA_SIZE];
			FixedDataMessageStream clientStream(transmission, MAX_UDP_DATA_SIZE);

			MSG clientMessage(clientStream, msgMode_e::Writing);
			clientMessage.SetCodec(MessageCodecs::OOB);
			clientMessage.WriteInteger(-1);
			clientMessage.WriteByte(0);
			clientMessage.WriteString("disconnect");

			IPRemoteIdentifier id(from);
			conChan->transmit(id, clientStream);

			return;
		}

		client->numCommands++;
		client->clientSequence = sequenceNum;
		processClient(*client, sequenceNum, stream, msg);
	}
}

void ServerHost::processClient(ClientData& client, uint32_t sequenceNum, IMessageStream& stream, MSG& msg)
{
	IEndec& encoding = client.getEncoder();

	if (stream.GetPosition() != stream.GetLength() || msg.GetBitPosition() / 8 < stream.GetLength())
	{
		msg.SetCodec(MessageCodecs::Bit);

		uint32_t clientSvId = msg.ReadUInteger();
		uint32_t msgAck = msg.ReadUInteger();
		uint32_t relAck = msg.ReadUInteger();

		if (clientSvId != serverId)
		{
			sendGameStateToClient(client);
		}
	}
}

void ServerHost::sendStateToClients()
{
	for (const ClientDataPtr& client : clientList)
	{
		if (!client->numCommands)
		{
			// don't send anything yet until client send something
			continue;
		}

		sendClientMessage(*client);
	}
}

void ServerHost::sendClientMessage(ClientData& client)
{
}

void ServerHost::sendGameStateToClient(ClientData& client)
{
	XOREncoding& encoding = client.getEncoder();

	const uint32_t sequenceNum = client.newSequence();

	uint8_t transmission[MAX_UDP_DATA_SIZE];
	FixedDataMessageStream clientStream(transmission, MAX_UDP_DATA_SIZE);

	MSG clientMessage(clientStream, msgMode_e::Writing);
	clientMessage.SetCodec(MessageCodecs::OOB);
	clientMessage.WriteUInteger(sequenceNum);

	clientMessage.SetCodec(MessageCodecs::Bit);
	clientMessage.WriteUInteger(0);

	clientMessage.WriteByte((uint8_t)svc_ops_e::Gamestate);
	clientMessage.WriteUInteger(0);

	clientMessage.WriteByte((uint8_t)svc_ops_e::Configstring);
	clientMessage.WriteUShort(CS::SERVERINFO);
	clientMessage.WriteString("\\mapname\\dm/mohdm6\\version\\Medal of Honor Allied Assault 1.11 win-x86 Mar  5 2002\\protocol\\8");

	clientMessage.WriteByte((uint8_t)svc_ops_e::Configstring);
	clientMessage.WriteUShort(CS::SYSTEMINFO);
	clientMessage.WriteString("\\test\\value\\sv_serverid\\" + std::to_string(serverId));

	clientMessage.WriteByte((uint8_t)svc_ops_e::Configstring);
	clientMessage.WriteUShort(11);
	clientMessage.WriteString("mohaa-base-1");

	clientMessage.WriteByte((uint8_t)svc_ops_e::Eof);
	// clients
	clientMessage.WriteUInteger(0);
	// checksum
	clientMessage.WriteUInteger(0);

	// end of sv messages
	clientMessage.WriteByte((uint8_t)svc_ops_e::Eof);

	clientMessage.Flush();

	const size_t len = clientStream.GetPosition();

	FixedDataMessageStream encodedStream(transmission, len);
	encodedStream.Seek(8, IMessageStream::SeekPos::Begin);

	encoding.setMessageAcknowledge(0);
	encoding.setReliableAcknowledge(0);
	encoding.setSecretKey(sequenceNum);
	encoding.convert(encodedStream, encodedStream);

	encodedStream.Seek(0, IMessageStream::SeekPos::Begin);

	serverSocket->send(client.getAddress(), transmission, len);
}
