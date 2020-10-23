#include <MOHPC/Network/Client/Server.h>
#include <MOHPC/Network/Client/ClientGame.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Log.h>

using namespace MOHPC;
using namespace Network;

#define MOHPC_LOG_NAMESPACE "gs_server"

MOHPC::Network::GSServer::GSServer(const NetworkManagerPtr& inManager, const netadr_t& adr)
	: IServer(inManager, adr)
{
	socket = ISocketFactory::get()->createUdp(addressType_e::IPv4);
}

void MOHPC::Network::GSServer::query(Callbacks::Query&& response, Callbacks::ServerTimeout&& timeoutResult, size_t timeoutTime)
{
	GamespyUDPRequestParam param(socket, getAddress());
	handler.sendRequest(makeShared<Request_Query>(std::move(response), std::move(timeoutResult)), std::move(param), timeoutTime);
}

void MOHPC::Network::GSServer::tick(uint64_t deltaTime, uint64_t currentTime)
{
	// Avoid deletion while the request was pending
	IServerPtr This = shared_from_this();

	handler.handle();
}

MOHPC::Network::GSServer::Request_Query::Request_Query(Callbacks::Query&& inResponse, Callbacks::ServerTimeout&& timeoutResult)
	: response(std::move(inResponse))
	, timeout(std::move(timeoutResult))
{
}

const char* MOHPC::Network::GSServer::Request_Query::generateQuery()
{
	return "status";
}

MOHPC::SharedPtr<MOHPC::IRequestBase> MOHPC::Network::GSServer::Request_Query::process(RequestData& data)
{
	const size_t len = data.stream.GetLength();

	char* dataStr = new char[len + 1];
	data.stream.Read(dataStr, len);
	dataStr[len] = 0;

	infoStr += dataStr;

	delete[] dataStr;

	const char* endStr = infoStr.c_str() + infoStr.length();
	const char* finalPos;
	if ((finalPos = strstr(infoStr.c_str(), "\\final\\\\queryid\\")))
	{
		// Don't bother parsing the status without a valid callback
		if (response)
		{
			ReadOnlyInfo info(infoStr.c_str(), finalPos - infoStr.c_str());
			/*
			for (InfoIterator it = info.createConstIterator(); it; ++it)
			{
				const char* key = it.key();
				const char* value = it.value();
				MOHPC_LOG(VeryVerbose, "%s: %s", key, value);
			}
			*/

			response(info);
		}

		return nullptr;
	}

	return shared_from_this();
}

MOHPC::SharedPtr<MOHPC::IRequestBase> MOHPC::Network::GSServer::Request_Query::timedOut()
{
	if(timeout) timeout();
	return nullptr;
}

MOHPC::Network::IServer::IServer(const NetworkManagerPtr& inManager, const netadr_t& adr)
	: ITickableNetwork(inManager)
	, address(adr)
{
}

const netadr_t& IServer::getAddress() const
{
	return address;
}

MOHPC::Network::LANServer::LANServer(const NetworkManagerPtr& inManager, const netadr_t& inAddress, char* inInfo, size_t infoSize)
	: IServer(inManager, inAddress)
	, info(ReadOnlyInfo(inInfo, infoSize))
	, dataStr(inInfo)
{
}

MOHPC::Network::LANServer::~LANServer()
{
	delete[] dataStr;
}

void MOHPC::Network::LANServer::tick(uint64_t deltaTime, uint64_t currentTime)
{
}

void MOHPC::Network::LANServer::query(Callbacks::Query&& response, Callbacks::ServerTimeout&& timeoutResult, size_t timeoutTime)
{
	// No need to sent any request with the info
	response(info);
}

MOHPC_OBJECT_DEFINITION(EngineServer);

EngineServer::EngineServer(const NetworkManagerPtr& inManager, const netadr_t& inAddress, const IUdpSocketPtr& existingSocket)
	: ITickableNetwork(inManager)
	, socket(existingSocket ? existingSocket : ISocketFactory::get()->createUdp(addressType_e::IPv4))
	, address(inAddress)
{
}

EngineServer::~EngineServer()
{
}

static uintptr_t getCurrentTime()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void EngineServer::tick(uint64_t deltaTime, uint64_t currentTime)
{
	EngineServerPtr ThisPtr = shared_from_this();
	handler.handle();
}

void EngineServer::connect(const ClientInfoPtr& clientInfo, const ConnectSettingsPtr& connectSettings, Callbacks::Connect&& result, Callbacks::ServerTimeout&& timeoutResult)
{
	using namespace std::placeholders;

	ConnectionParams connData;
	connData.response = std::bind(&EngineServer::onConnect, this, std::move(result), _1, _2, _3, _4, _5);
	connData.info = std::move(clientInfo);
	connData.settings = std::move(connectSettings);
	if (!connData.settings) connData.settings = ConnectSettings::create();

	sendRequest(makeShared<VerBeforeChallengeRequest>(std::move(connData)), std::move(timeoutResult));

	MOHPC_LOG(Verbose, "connection request sent");
	//protocolType_c proto(0, protocolVersion_e::ver211);
	//sendRequest(to, makeShared<ChallengeRequest>(proto, std::move(connData)));
}

void EngineServer::getStatus(Callbacks::Response&& result, Callbacks::ServerTimeout&& timeoutResult, size_t timeoutTime)
{
	sendRequest(makeShared<StatusRequest>(std::move(result)), std::move(timeoutResult), timeoutTime);
}

void EngineServer::getInfo(Callbacks::Response&& result, Callbacks::ServerTimeout&& timeoutResult, size_t timeoutTime)
{
	sendRequest(makeShared<InfoRequest>(std::move(result)), std::move(timeoutResult), timeoutTime);
}

void EngineServer::sendRequest(IEngineRequestPtr&& req, Callbacks::ServerTimeout&& timeoutResult, size_t timeoutTime)
{
	req->timeoutCallback = std::move(timeoutResult);

	GamespyUDPRequestParam param(socket, address);
	handler.sendRequest(std::move(req), std::move(param), timeoutTime);
}

void EngineServer::onConnect(const Callbacks::Connect result, uint16_t qport, uint32_t challengeResponse, const protocolType_c& protoType, const ClientInfoPtr& cInfo, const char* errorMessage)
{
	if (!errorMessage)
	{
		// Create a net channel
		INetchanPtr newChannel = makeShared<Netchan>(socket, address, qport);
		// And pass it to the new client game connection
		// Using CreatePtr to use the deleter from the library itself
		ClientGameConnectionPtr connection = ClientGameConnection::create(
			getManager(),
			newChannel,
			address,
			challengeResponse,
			protoType,
			cInfo
		);
		
		// Init client time
		connection->initTime(getCurrentTime());
		// Return the newly created client game connection
		result(connection, nullptr);
	}
	else
	{
		// Send the error message
		result(nullptr, errorMessage);
	}
}

EngineServer::EmbeddedRequest::EmbeddedRequest(const IRequestPtr& inRequest, Callbacks::ServerTimeout&& inTimedout)
	: request(inRequest)
	, timedout(std::move(inTimedout))
{

}

IRequestPtr MOHPC::Network::EngineServer::IEngineRequest::timedOut()
{
	if(timeoutCallback) timeoutCallback();
	return nullptr;
}

void MOHPC::Network::EngineServer::IEngineRequest::generateOutput(IMessageStream& output)
{
	// Set in OOB mode to read one single byte each call
	MSG msg(output, msgMode_e::Writing);
	msg.SetCodec(MessageCodecs::OOB);

	msg.WriteUInteger(-1);
	msg.WriteByte((uint8_t)netsrc_e::Server);

	// Generate the request string
	const str reqstr = generateRequest();
	const size_t startOffset = output.GetPosition();

	size_t compressionOffset;
	if (shouldCompressRequest(compressionOffset))
	{
		DynamicDataMessageStream compressedStream;
		DynamicDataMessageStream uncompressedStream;

		// copy existing data into the new stream
		compressedStream.reserve(MAX_INFO_STRING);
		compressedStream.Seek(0, IMessageStream::SeekPos::Begin);
		// write the uncompressed data first into the stream
		compressedStream.Write(reqstr.c_str(), compressionOffset);

		// copy existing data into the new stream
		uncompressedStream.reserve(MAX_INFO_STRING);
		uncompressedStream.Seek(0, IMessageStream::SeekPos::Begin);
		uncompressedStream.Write(reqstr.c_str() + compressionOffset, reqstr.length() - compressionOffset);

		// Compress data after the connect offset
		const size_t compressionLength = reqstr.length() - compressionOffset;
		CompressedMessage compression(uncompressedStream, compressedStream);
		compression.Compress(0, compressionLength);

		msg.WriteData(compressedStream.getStorage(), compressedStream.GetLength());
	}
	else
	{
		// Write the request string
		const StringMessage string(reqstr);
		msg.WriteString(string);
	}
}

IRequestPtr MOHPC::Network::EngineServer::IEngineRequest::process(RequestData& data)
{
	const netadr_t& from = data.getParam<GamespyUDPRequestParam>().getLastIp();

	// Set in OOB mode to read one single byte each call
	MSG msg(data.stream, msgMode_e::Reading);
	msg.SetCodec(MessageCodecs::OOB);

	const uint32_t marker = msg.ReadUInteger();
	if (marker != -1)
	{
		// must be a connectionless packet
		MOHPC_LOG(Log, "Received a sequenced packet (%d)", marker);
		return nullptr;
	}

	const netsrc_e dirByte = (netsrc_e)msg.ReadByte();
	if (dirByte != netsrc_e::Client)
	{
		MOHPC_LOG(Log, "Wrong direction for connectionLess packet (must be targeted at client, got %d)", dirByte);
		return nullptr;
	}

	IRequestPtr newRequest;

	// Read request string from remote
	StringMessage arg = msg.ReadString();
	const size_t len = strlen(arg);
	if (len > 0)
	{
		TokenParser parser;
		parser.Parse(arg, strlen(arg) + 1);

		const char* command = parser.GetToken(false);
		if (supportsEvent(command)) {
			newRequest = handleResponse(command, parser);
		}
		else
		{
			// Unexpected response
			MOHPC_LOG(
				Log, "unexpected reply \"%s\" from %d.%d.%d.%d:%d",
				command, from.ip[0], from.ip[1], from.ip[2], from.ip[3], from.port
			);
		}
	}
	else
	{
		// Empty response
		MOHPC_LOG(
			Log, "empty response from %d.%d.%d.%d:%d",
			from.ip[0], from.ip[1], from.ip[2], from.ip[3], from.port
		);
	}

	return newRequest;
}

MOHPC_OBJECT_DEFINITION(ConnectSettings);

ConnectSettings::ConnectSettings()
	: version(CLIENT_VERSION)
	, cdKey("")
	, deferredChallengeTime(100)
	, deferredConnectTime(100)
	, qport(0)
{
}

void MOHPC::Network::ConnectSettings::setDeferredChallengeTime(size_t newTime)
{
	deferredChallengeTime = newTime;
}

const char* ConnectSettings::getCDKey() const
{
	return cdKey;
}

void ConnectSettings::setCDKey(const char* value)
{
	cdKey = value ? value : "";
}

const char* ConnectSettings::getVersion() const
{
	return version;
}

void ConnectSettings::setVersion(const char* value)
{
	version = value && *value ? value : CLIENT_VERSION;
}

uint16_t ConnectSettings::getQport() const
{
	return qport;
}

void ConnectSettings::setQport(uint16_t newValue)
{
	qport = newValue;
}

size_t ConnectSettings::getDeferredChallengeTime() const
{
	return deferredChallengeTime;
}

void ConnectSettings::setDeferredConnectTime(size_t newTime)
{
	deferredConnectTime = newTime;
}

size_t ConnectSettings::getDeferredConnectTime() const
{
	return deferredConnectTime;
}
