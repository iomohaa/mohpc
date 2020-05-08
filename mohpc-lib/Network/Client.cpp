#include <MOHPC/Managers/NetworkManager.h>
#include <MOHPC/Network/Client.h>
#include <MOHPC/Network/ClientGame.h>
#include <MOHPC/Network/ClientCommands.h>
#include <MOHPC/Network/Event.h>
#include <MOHPC/Network/SerializableTypes.h>
#include <MOHPC/Network/Channel.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Misc/MSG/Serializable.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Log.h>
#include <GameSpy/gcdkey/gcdkeyc.h>

#include <chrono>

using namespace MOHPC;
using namespace Network;

static constexpr char* MOHPC_LOG_NAMESPACE = "network_client";

ClientInstance::ClientInstance(const IClientPtr& inClient)
	: client(inClient)
{
}

ClientInstance::~ClientInstance()
{
	/*
	// Delete all responses
	const size_t num = clientEvents.getNumCommands();
	for (size_t i = 0; i < num; ++i)
	{
		Container<ClientResponse> responseList = std::move(responses[i]);
		for (size_t j = 0; j < responseList.NumObjects(); ++j) {
			delete responseList[j];
		}
	}
	*/

	//delete[] responses;
}

/*
void ClientInstance::AcquireChallenge()
{
	std::vector<uint8_t> data(256);
	FixedDataMessageStream stream(data.data(), data.size());
	MSG msg(stream, msgMode_e::Writing);

	// Write connectionless header
	ConnectionlessHeader(msg);

	StringMessage arg = "getchallenge";
	msg.SerializeString(arg);

	socket->send(data.data(), stream.GetPosition() - 1);
}

void ClientInstance::Connect(const netadr_t& to, int32_t challengeResponse, uint16_t qport)
{
	std::vector<uint8_t> rawData(8192, 0);
	FixedDataMessageStream rawStream(rawData.data(), rawData.size());
	MSG msg(rawStream, msgMode_e::Writing);

	// Write connectionless header
	ConnectionlessHeader(msg);

	str connectArgs = "connect";

	const size_t offset = rawStream.GetPosition() + connectArgs.length() + 1;

	// Set user info
	Info info;
	info.SetValueForKey("protocol", str::printf("%i", 17)); // 8 = mohaa
	info.SetValueForKey("clientType", "Breakthrough"); // 8 = mohaa
	info.SetValueForKey("qport", str::printf("%i", qport));
	info.SetValueForKey("challenge", str::printf("%i", challengeResponse));
	info.SetValueForKey("name", "");

	// Send user info string
	connectArgs.reserve(info.GetInfoLength() + 2);
	connectArgs += " \"";
	connectArgs += info.GetString();
	connectArgs += '"';

	StringMessage connectString = connectArgs;
	msg.SerializeString(connectString);

	std::vector<uint8_t> compressedData(8192, 0);
	FixedDataMessageStream compressedStream(compressedData.data(), compressedData.size());

	// Copy existing data into the new stream
	compressedStream.Seek(0, IMessageStream::SeekPos::Begin);
	compressedStream.Write(rawData.data(), offset);

	// Compress data after the connect offset
	CompressNetStream(rawStream, compressedStream, offset, rawStream.GetPosition() - offset);

	socket->send(compressedData.data(), compressedStream.GetPosition());

	CreateNetworkChannel(to, challengeResponse);

	//AddCallbackForHandler(connectResponseEvent, new ConnectHandler(to, challengeResponse));
}
*/

void ClientInstance::tick(uint64_t deltaTime, uint64_t currentTime)
{
	processEvents();
}

void ClientInstance::processEvents()
{
	std::vector<uint8_t> data(65536);
	FixedDataMessageStream stream(data.data(), data.size());
	MSG msg(stream, msgMode_e::Reading);

	if (!client) {
		return;
	}

	IUdpSocket* socket = client->getNetchan()->getRawSocket();
	const bool hasData = socket->wait(50);

	//client->tick();

	if (hasData)
	{
		// Read pending data
		netadr_t from;
		socket->receive(data.data(), data.size(), from);

		try
		{
			// Pass data to client
			client->receive(from, msg);
		}
		catch (NetworkException& e)
		{
			MOHPC_LOG(Error, "Exception %s from client: \"%s\"", typeid(e).name(), e.what().c_str());
		}
	}
}

/*
void ClientInstance::HandleConnectionlessPackets(const netadr_t& from, MSG& msg)
{
	// Read header packet
	ConnectionlessPacket(msg);

	StringMessage arg;
	msg.SerializeString(arg);

	TokenParser parser;
	parser.Parse(arg, strlen(arg) + 1);

	const char* command = parser.GetToken(false);
	ProcessConnectionlessCommand(command, from, msg, parser);
}

void ClientInstance::ProcessConnectionlessCommand(const char* commandName, const netadr_t& from, MSG& msg, TokenParser& parser)
{
	for (IClientConnectionlessHandler* handler = clientEvents.getHeadAs<IClientConnectionlessHandler>();
		handler;
		handler = (IClientConnectionlessHandler*)handler->getNext()
		)
	{
		if (!str::icmp(handler->getEventName(), commandName))
		{
			Event ev;
			handler->doRun(msg, parser, ev);

			// Call listeners for this handler
			Container<ClientResponse> responseList = std::move(responses[handler->getId()]);
			for (size_t i = 0; i < responseList.NumObjects(); ++i)
			{
				ClientResponse& response = responseList[i];
				response(from, &ev);
			}
		}
	}
}
*/

/*
void ClientInstance::HandleKey(const netadr_t& from, const Event* ev)
{
	std::vector<uint8_t> rawData(8192, 0);
	FixedDataMessageStream rawStream(rawData.data(), rawData.size());
	MSG msg(rawStream, msgMode_e::Writing);

	// Write connectionless header
	ConnectionlessHeader(msg);

	// Write the command
	StringMessage arg = str::printf("authorizeThis %s", ev->GetString(1).c_str());
	msg.WriteString(arg);

	socket->send(rawData.data(), rawStream.GetPosition() - 1);
}

void ClientInstance::HandleChallenge(const netadr_t& from, const Event* ev, uint16_t qport)
{
	RemoveCallbacks(getKeyEvent);
	Connect(from, ev->GetInteger(1), qport);
}
*/

