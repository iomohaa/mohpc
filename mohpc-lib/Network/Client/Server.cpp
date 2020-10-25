#include <MOHPC/Network/Client/Server.h>
#include <MOHPC/Network/Client/ClientGame.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Log.h>

using namespace MOHPC;
using namespace MOHPC::Network;

#define MOHPC_LOG_NAMESPACE "gs_server"

GSServer::GSServer(const NetworkManagerPtr& inManager, const NetAddrPtr& adr)
	: IServer(inManager, adr)
{
	socket = ISocketFactory::get()->createUdp();
}

void GSServer::query(Callbacks::Query&& response, Callbacks::ServerTimeout&& timeoutResult, size_t timeoutTime)
{
	GamespyUDPRequestParam param(socket, getAddress());
	handler.sendRequest(makeShared<Request_Query>(std::move(response), std::move(timeoutResult)), std::move(param), timeoutTime);
}

void GSServer::tick(uint64_t deltaTime, uint64_t currentTime)
{
	// Avoid deletion while the request was pending
	IServerPtr This = shared_from_this();

	handler.handle();
}

GSServer::Request_Query::Request_Query(Callbacks::Query&& inResponse, Callbacks::ServerTimeout&& timeoutResult)
	: response(std::move(inResponse))
	, timeout(std::move(timeoutResult))
{
}

const char* GSServer::Request_Query::generateQuery()
{
	return "status";
}

SharedPtr<IRequestBase> GSServer::Request_Query::process(RequestData& data)
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

SharedPtr<IRequestBase> GSServer::Request_Query::timedOut()
{
	if(timeout) timeout();
	return nullptr;
}

IServer::IServer(const NetworkManagerPtr& inManager, const NetAddrPtr& adr)
	: ITickableNetwork(inManager)
	, address(adr)
{
}

const NetAddrPtr& IServer::getAddress() const
{
	return address;
}

LANServer::LANServer(const NetworkManagerPtr& inManager, const NetAddrPtr& inAddress, char* inInfo, size_t infoSize)
	: IServer(inManager, inAddress)
	, info(ReadOnlyInfo(inInfo, infoSize))
	, dataStr(inInfo)
{
}

LANServer::~LANServer()
{
	delete[] dataStr;
}

void LANServer::tick(uint64_t deltaTime, uint64_t currentTime)
{
}

void LANServer::query(Callbacks::Query&& response, Callbacks::ServerTimeout&& timeoutResult, size_t timeoutTime)
{
	// No need to sent any request with the info
	response(info);
}

MOHPC_OBJECT_DEFINITION(EngineServer);

EngineServer::EngineServer(const NetworkManagerPtr& inManager, const NetAddrPtr& inAddress, const IUdpSocketPtr& existingSocket)
	: ITickableNetwork(inManager)
	, socket(existingSocket ? existingSocket : ISocketFactory::get()->createUdp())
	, address(inAddress)
{
}

EngineServer::~EngineServer()
{
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

	sendRequest(makeShared<VerBeforeChallengeRequest>(std::move(connData)), std::move(timeoutResult));

	MOHPC_LOG(Verbose, "connection request sent");
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
		// create a net channel and use the same socket
		INetchanPtr newChannel = makeShared<Netchan>(socket, qport);
		// and pass it to the new client game connection
		// using a shared ptr, to be able to use the deleter from the library itself
		ClientGameConnectionPtr connection = ClientGameConnection::create(
			getManager(),
			newChannel,
			address,
			challengeResponse,
			protoType,
			cInfo
		);
		
		// init the client time with the current time
		connection->initTime(getCurrentTime());
		// Return the newly created client game connection
		result(connection, nullptr);
	}
	else
	{
		// notify about the error message
		result(nullptr, errorMessage);
	}
}

EngineServer::EmbeddedRequest::EmbeddedRequest(const IRequestPtr& inRequest, Callbacks::ServerTimeout&& inTimedout)
	: request(inRequest)
	, timedout(std::move(inTimedout))
{

}

IRequestPtr EngineServer::IEngineRequest::timedOut()
{
	if(timeoutCallback) timeoutCallback();
	return nullptr;
}

void EngineServer::IEngineRequest::generateOutput(IMessageStream& output)
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

IRequestPtr EngineServer::IEngineRequest::process(RequestData& data)
{
	const NetAddr& from = data.getParam<GamespyUDPRequestParam>().getLastIp();

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
				Log, "unexpected reply \"%s\" from %s:%d",
				command, from.asString().c_str(), from.port
			);
		}
	}
	else
	{
		// Empty response
		MOHPC_LOG(
			Log, "empty response from %s:%d",
			from.asString().c_str(), from.port
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

void ConnectSettings::setDeferredChallengeTime(size_t newTime)
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
