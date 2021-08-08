#include <MOHPC/Network/Client/Server.h>
#include <MOHPC/Network/Client/ServerConnection.h>
#include <MOHPC/Network/Version.h>
#include <MOHPC/Utility/Misc/MSG/Codec.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Info.h>
#include <MOHPC/Utility/TokenParser.h>
#include <MOHPC/Utility/MessageDispatcher.h>
#include <MOHPC/Common/Log.h>

#include <cassert>

using namespace MOHPC;
using namespace Network;

#define MOHPC_LOG_NAMESPACE "gs_server"

MOHPC_OBJECT_DEFINITION(GSServer);

GSServer::GSServer(const MessageDispatcherPtr& dispatcher, const ICommunicatorPtr& comm, const IRemoteIdentifierPtr& identifier)
	: IServer(identifier)
	, handler(dispatcher.get(), comm, identifier)
{
	socket = ISocketFactory::get()->createUdp();
}

void GSServer::query(Callbacks::Query&& response, Callbacks::ServerTimeout&& timeoutResult, size_t timeoutTime)
{
	handler.sendRequest(makeShared<Request_Query>(std::move(response), std::move(timeoutResult)), timeoutTime);
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

bool GSServer::Request_Query::isThisRequest(InputRequest& data) const
{
	char c;
	data.stream.Read(&c, 1);
	if (c == '\\')
	{
		// the response doesn't correspond to what is expected
		return true;
	}

	return false;
}

SharedPtr<IRequestBase> GSServer::Request_Query::process(InputRequest& data)
{
	const size_t len = data.stream.GetLength();

	char* dataStr = new char[len + 1];
	data.stream.Read(dataStr + 1, len);
	// put what has been read before
	dataStr[0] = '\\';
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
				MOHPC_LOG(Trace, "%s: %s", key, value);
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

IServer::IServer(const IRemoteIdentifierPtr& inIdentifier)
	: identifier(inIdentifier)
{
}

const IRemoteIdentifierPtr& IServer::getIdentifier() const
{
	return identifier;
}

MOHPC_OBJECT_DEFINITION(LANServer);

LANServer::LANServer(const IRemoteIdentifierPtr& inIdentifier, char* inInfo, size_t infoSize)
	: IServer(inIdentifier)
	, info(ReadOnlyInfo(inInfo, infoSize))
	, dataStr(inInfo)
{
}

LANServer::~LANServer()
{
	delete[] dataStr;
}

void LANServer::query(Callbacks::Query&& response, Callbacks::ServerTimeout&& timeoutResult, size_t timeoutTime)
{
	// No need to sent any request with the info
	response(info);
}

MOHPC_OBJECT_DEFINITION(EngineServer);

EngineServer::EngineServer(const MessageDispatcherPtr& dispatcher, const ICommunicatorPtr& comm, const IRemoteIdentifierPtr& remoteIdentifier)
	: handler(dispatcher.get(), comm, remoteIdentifier)
{
}

EngineServer::~EngineServer()
{
}

void EngineServer::connect(const UserInfoPtr& UserInfo, const ConnectSettingsPtr& connectSettings, Callbacks::Connect&& result, Callbacks::ServerTimeout&& timeoutResult)
{
	using namespace std::placeholders;

	ConnectionParams connData;
	connData.response = std::bind(&EngineServer::onConnect, this, std::move(result), _1, _2, _3, _4, _5);
	connData.info = std::move(UserInfo);
	connData.settings = std::move(connectSettings);

	sendRequest(makeShared<VerBeforeChallengeRequest>(std::move(connData)), std::move(timeoutResult));

	MOHPC_LOG(Debug, "connection request sent");
}

void EngineServer::getStatus(Callbacks::Response&& result, Callbacks::ServerTimeout&& timeoutResult, uint64_t timeoutTime)
{
	sendRequest(makeShared<StatusRequest>(std::move(result)), std::move(timeoutResult), timeoutTime);
}

void EngineServer::getInfo(Callbacks::Response&& result, Callbacks::ServerTimeout&& timeoutResult, uint64_t timeoutTime)
{
	sendRequest(makeShared<InfoRequest>(std::move(result)), std::move(timeoutResult), timeoutTime);
}

void EngineServer::sendRequest(IEngineRequestPtr&& req, Callbacks::ServerTimeout&& timeoutResult, uint64_t timeoutTime)
{
	req->timeoutCallback = std::move(timeoutResult);
	handler.sendRequest(std::move(req), timeoutTime);
}

void EngineServer::onConnect(const Callbacks::Connect result, uint16_t qport, uint32_t challengeResponse, const protocolType_c& protoType, const UserInfoPtr& cInfo, const char* errorMessage)
{
	if (!errorMessage)
	{
		// create a net channel and use the same socket
		INetchanPtr newChannel = makeShared<Netchan>(handler.getComm(), qport);
		// and pass it to the new client game connection
		// using a shared ptr, to be able to use the deleter from the library itself
		ServerConnectionPtr connection = ServerConnection::create(
			newChannel,
			handler.getRemoteId(),
			challengeResponse,
			protoType,
			cInfo
		);
	
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
		compressedStream.reserve(2000);
		compressedStream.Seek(0, IMessageStream::SeekPos::Begin);
		// write the uncompressed data first into the stream
		compressedStream.Write(reqstr.c_str(), compressionOffset);

		// copy existing data into the new stream
		uncompressedStream.reserve(2000);
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

bool EngineServer::IEngineRequest::isThisRequest(InputRequest& data) const
{
	MSG msg(data.stream, msgMode_e::Reading);
	msg.SetCodec(MessageCodecs::OOB);

	const uint32_t marker = msg.ReadUInteger();
	if (marker != -1)
	{
		// must be a connectionless packet
		return false;
	}

	const netsrc_e dirByte = (netsrc_e)msg.ReadByte();
	if (dirByte != netsrc_e::Client)
	{
		// this must be targeted at us, the client
		return false;
	}

	StringMessage arg = msg.ReadString();
	const size_t len = strlen(arg);
	if (len <= 0)
	{
		// invalid length
		return false;
	}

	TokenParser parser;
	parser.Parse(arg, strlen(arg) + 1);

	const char* command = parser.GetToken(false);
	// now return if the request supports the command
	return supportsEvent(command);
}

SharedPtr<IRequestBase> EngineServer::IEngineRequest::process(InputRequest& data)
{
	// Set in OOB mode to read one single byte each call
	MSG msg(data.stream, msgMode_e::Reading);
	msg.SetCodec(MessageCodecs::OOB);

	const uint32_t marker = msg.ReadUInteger();
	//assert(marker == -1);
	if (marker != -1)
	{
		// must be a connectionless packet
		MOHPC_LOG(Info, "Received a sequenced packet (%d)", marker);
		return nullptr;
	}

	const netsrc_e dirByte = (netsrc_e)msg.ReadByte();
	//assert(dirByte == netsrc_e::Client);
	if (dirByte != netsrc_e::Client)
	{
		MOHPC_LOG(Info, "Wrong direction for connectionLess packet (must be targeted at client, got %d)", dirByte);
		return nullptr;
	}

	IRequestPtr newRequest;

	// Read request string from remote
	StringMessage arg = msg.ReadString();
	const size_t len = strlen(arg);
	assert(len > 0);
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
				Info, "unexpected reply \"%s\" from %s",
				command, data.identifier->getString().c_str()
			);
		}
	}
	else
	{
		// Empty response
		MOHPC_LOG(
			Info, "empty response from %s",
			data.identifier->getString().c_str()
		);
	}

	return newRequest;
}

MOHPC_OBJECT_DEFINITION(ConnectSettings);

ConnectSettings::ConnectSettings()
	: version(NETWORK_VERSION)
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
	version = value && *value ? value : NETWORK_VERSION;
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

