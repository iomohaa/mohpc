#include <MOHPC/Network/Server/ServerHost.h>
#include <MOHPC/Network/Socket.h>
#include <MOHPC/Network/Types.h>
#include <MOHPC/Network/Channel.h>
#include <MOHPC/Network/Encoding.h>
#include <MOHPC/Network/Configstring.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Log.h>

#include <chrono>

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
	, reliableCommands{0}
	, serverCommands{0}
{
	clientSequence = 0;
	numCommands = 0;

	for (size_t i = 0; i < MAX_RELIABLE_COMMANDS; ++i)
	{
		reliableCommandList[i] = &reliableCommands[i * MAX_STRING_CHARS];
		serverCommandList[i] = &serverCommands[i * MAX_STRING_CHARS];
	}

	encoding = Encoding::create(challengeNum, (const char**)serverCommandList, (const char**)reliableCommandList);
}

const NetAddr& ClientData::getAddress() const
{
	return *source;
}

uint16_t ClientData::getQPort() const
{
	return qport;
}

Encoding& ClientData::getEncoding() const
{
	return *encoding;
}

uint32_t ClientData::newSequence()
{
	return ++sequenceNum;
}

MOHPC_OBJECT_DEFINITION(ServerHost);

ServerHost::ServerHost(const NetworkManagerPtr& networkManager)
	: ITickableNetwork(networkManager)
{
	NetAddr4 bindAddress;
	bindAddress.port = 12203;

	serverSocket = ISocketFactory::get()->createUdp(&bindAddress);
	conChan = ConnectionlessChan::create(serverSocket);

	serverId = rand();
}

ClientData& ServerHost::createClient(const NetAddrPtr& from, uint16_t qport, uint32_t challengeNum)
{
	ClientDataPtr data = ClientData::create(serverSocket, from, qport, challengeNum);
	clientList.AddObject(data);

	return *data;
}

ClientData* ServerHost::findClient(const NetAddr& from, uint16_t qport) const
{
	const size_t numClients = clientList.NumObjects();
	for (size_t i = 1; i <= numClients; ++i)
	{
		const ClientDataPtr& clientData = clientList.ObjectAt(i);
		if (clientData->getAddress() == from && clientData->getQPort() == qport)
		{
			return clientData.get();
		}
	}

	return nullptr;
}

Challenge& ServerHost::createChallenge(const NetAddrPtr& from)
{
	Challenge* challenge = new (challenges) Challenge(from, std::chrono::steady_clock::now(), rand());
	return *challenge;
}

uintptr_t ServerHost::getChallenge(const NetAddr& from) const
{
	const size_t numChallenges = challenges.NumObjects();
	for (size_t i = 1; i <= numChallenges; ++i)
	{
		const Challenge& challenge = challenges.ObjectAt(i);
		if (challenge.getSourceAddress() == from)
		{
			return i;
		}
	}

	return 0;
}

void ServerHost::tick(uint64_t deltaTime, uint64_t currentTime)
{
	for (size_t i = challenges.NumObjects(); i > 0; i--)
	{
		Challenge& challenge = challenges.ObjectAt(i);
		if (challenge.hasElapsed(std::chrono::milliseconds(10000)))
		{
			// remove the challenge if it has expired
			challenges.RemoveObjectAt(i);
		}
	}

	if (serverSocket->dataAvailable())
	{
		processRequests();
	}

	sendStateToClients();
}

void ServerHost::connectionLessReply(const NetAddr& target, const char* reply)
{
	uint8_t transmission[MAX_UDP_DATA_SIZE];
	FixedDataMessageStream clientStream(transmission, MAX_UDP_DATA_SIZE);

	MSG clientMessage(clientStream, msgMode_e::Writing);
	clientMessage.SetCodec(MessageCodecs::OOB);
	clientMessage.WriteInteger(-1);
	clientMessage.WriteByte(0);
	clientMessage.WriteString(reply);

	clientStream = FixedDataMessageStream(transmission, clientStream.GetPosition());
	conChan->transmit(target, clientStream);
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
		if (!str::icmp(command, "getchallenge"))
		{
			Challenge* challenge;

			const uintptr_t challengeIndex = getChallenge(*from);
			if (challengeIndex)
			{
				// a challenge already exist for this ip
				challenge = &challenges.ObjectAt(challengeIndex);
			}
			else
			{
				// create the challenge for the address
				challenge = &createChallenge(from);
			}

			// send the client challenge
			connectionLessReply(*from, str::printf("challengeResponse %d", challenge->getChallenge()));
		}
		else if (!str::icmp(command, "connect "))
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

			const Challenge& challenge = challenges.ObjectAt(challengeIndex);
			if (challenge.getChallenge() != challengeNum)
			{
				// bad challenge
				return;
			}

			challenges.RemoveObjectAt(challengeIndex);

			uint16_t qport = info.IntValueForKey("qport");

			// success, now create the client
			ClientData& client = createClient(from, qport, challengeNum);

			// inform the client that it's connected
			connectionLessReply(*from, "connectResponse");
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

			conChan->transmit(*from, clientStream);

			return;
		}

		client->numCommands++;
		client->clientSequence = sequenceNum;
		processClient(*client, sequenceNum, stream, msg);
	}
}

void ServerHost::processClient(ClientData& client, uint32_t sequenceNum, IMessageStream& stream, MSG& msg)
{
	Encoding& encoding = client.getEncoding();

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
	Encoding& encoding = client.getEncoding();

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
	clientMessage.WriteUShort(CS_SERVERINFO);
	clientMessage.WriteString("\\mapname\\dm/mohdm6\\version\\Medal of Honor Allied Assault 1.11 win-x86 Mar  5 2002\\protocol\\8");

	clientMessage.WriteByte((uint8_t)svc_ops_e::Configstring);
	clientMessage.WriteUShort(CS_SYSTEMINFO);
	clientMessage.WriteString(str::printf("\\test\\value\\sv_serverid\\%.03d", serverId).c_str());

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
	encoding.encode(encodedStream, encodedStream);

	encodedStream.Seek(0, IMessageStream::SeekPos::Begin);

	serverSocket->send(client.getAddress(), transmission, len);
}
