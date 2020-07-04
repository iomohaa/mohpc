#include <MOHPC/Network/Server.h>
#include <MOHPC/Network/ClientGame.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Log.h>

using namespace MOHPC;
using namespace Network;

#define MOHPC_LOG_NAMESPACE "gs_server"

MOHPC::Network::GSServer::GSServer(NetworkManager* inManager, const netadr_t& adr)
	: IServer(inManager, adr)
{
	socket = ISocketFactory::get()->createUdp(addressType_e::IPv4);
}

void MOHPC::Network::GSServer::query(Callbacks::Query&& response, Callbacks::ServerTimeout&& timeoutResult)
{
	GamespyUDPRequestParam param(socket, getAddress());
	handler.sendRequest(makeShared<Request_Query>(std::move(response), std::move(timeoutResult)), std::move(param), 10000);
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

MOHPC::Network::IServer::IServer(NetworkManager* inManager, const netadr_t& adr)
	: ITickableNetwork(inManager)
	, address(adr)
{
}

const netadr_t& IServer::getAddress() const
{
	return address;
}

MOHPC::Network::LANServer::LANServer(NetworkManager* inManager, const netadr_t& inAddress, char* inInfo, size_t infoSize)
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

void MOHPC::Network::LANServer::query(Callbacks::Query&& response, Callbacks::ServerTimeout&& timeoutResult)
{
	// No need to sent any request with the info
	response(info);
}

MOHPC_OBJECT_DEFINITION(EngineServer);

EngineServer::EngineServer(NetworkManager* inManager, const netadr_t& inAddress)
	: ITickableNetwork(inManager)
	, socket(ISocketFactory::get()->createUdp(addressType_e::IPv4))
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

void EngineServer::connect(ClientInfo&& clientInfo, Callbacks::Connect&& result, Callbacks::ServerTimeout&& timeoutResult)
{
	using namespace std::placeholders;

	ConnectionParams connData;
	connData.response = std::bind(&EngineServer::onConnect, this, std::move(result), _1, _2, _3, _4, _5);
	connData.info = std::move(clientInfo);

	sendRequest(makeShared<VerBeforeChallengeRequest>(std::move(connData)), std::move(timeoutResult));

	MOHPC_LOG(Verbose, "connection request sent");
	//protocolType_c proto(0, protocolVersion_e::ver211);
	//sendRequest(to, makeShared<ChallengeRequest>(proto, std::move(connData)));
}

void EngineServer::getStatus(Callbacks::Response&& result, Callbacks::ServerTimeout&& timeoutResult)
{
	sendRequest(makeShared<StatusRequest>(std::move(result)), std::move(timeoutResult));
}

void EngineServer::getInfo(Callbacks::Response&& result, Callbacks::ServerTimeout&& timeoutResult)
{
	sendRequest(makeShared<InfoRequest>(std::move(result)), std::move(timeoutResult));
}

void EngineServer::sendRequest(IEngineRequestPtr&& req, Callbacks::ServerTimeout&& timeoutResult)
{
	req->timeoutCallback = std::move(timeoutResult);

	GamespyUDPRequestParam param(socket, address);
	handler.sendRequest(std::move(req), std::move(param), 10000);
}

void EngineServer::onConnect(const Callbacks::Connect result, uint16_t qport, uint32_t challengeResponse, const protocolType_c& protoType, ClientInfo&& cInfo, const char* errorMessage)
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
			std::move(cInfo)
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

ClientInfo::ClientInfo()
	: snaps(20)
	, rate(5000)
{
}

void MOHPC::Network::ClientInfo::setRate(uint32_t inRate)
{
	rate = inRate;
}

uint32_t MOHPC::Network::ClientInfo::getRate() const
{
	return rate;
}

void MOHPC::Network::ClientInfo::setSnaps(uint32_t inSnaps)
{
	snaps = inSnaps;
}

uint32_t MOHPC::Network::ClientInfo::getSnaps() const
{
	return snaps;
}

void ClientInfo::setName(const char* newName)
{
	name = newName;
}

const char* ClientInfo::getName() const
{
	return name.c_str();
}

void ClientInfo::setPlayerAlliedModel(const char* newModel)
{
	properties.SetPropertyValue("dm_playermodel", newModel);
}

const char* ClientInfo::getPlayerAlliedModel() const
{
	return properties.GetPropertyRawValue("dm_playermodel");
}

void ClientInfo::setPlayerGermanModel(const char* newModel)
{
	properties.SetPropertyValue("dm_playergermanmodel", newModel);
}

const char* ClientInfo::getPlayerGermanModel() const
{
	return properties.GetPropertyRawValue("dm_playergermanmodel");
}

void MOHPC::Network::ClientInfo::setUserKeyValue(const char* key, const char* value)
{
	properties.SetPropertyValue(key, value);
}

const char* MOHPC::Network::ClientInfo::getUserKeyValue(const char* key) const
{
	return properties.GetPropertyRawValue(key);
}

void ClientInfo::fillInfoString(Info& info) const
{
	// Build mandatory variables
	info.SetValueForKey("rate", str::printf("%i", rate));
	info.SetValueForKey("snaps", str::printf("%i", snaps));
	info.SetValueForKey("name", name.c_str());

	// Build miscellaneous values
	for (PropertyMapIterator it = properties.GetIterator(); it; ++it)
	{
		info.SetValueForKey(
			it.key().GetFullPropertyName(),
			it.value()
		);
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
	msg.WriteByte(2);

	// Generate the request string
	str reqstr = generateRequest();

	size_t startOffset = output.GetPosition();

	size_t compressionOffset;
	if (shouldCompressRequest(compressionOffset))
	{
		compressionOffset -= startOffset;

		// New compression stream
		std::vector<uint8_t> compressedData(8192, 0);
		FixedDataMessageStream compressedStream(compressedData.data(), compressedData.size());

		// Copy existing data into the new stream
		compressedStream.Seek(0, IMessageStream::SeekPos::Begin);
		compressedStream.Write(reqstr.c_str(), compressionOffset);

		std::vector<uint8_t> uncompressedData(8192, 0);
		FixedDataMessageStream uncompressedStream(uncompressedData.data(), uncompressedData.size());

		// Copy existing data into the new stream
		uncompressedStream.Seek(0, IMessageStream::SeekPos::Begin);
		uncompressedStream.Write(reqstr.c_str(), reqstr.length());

		// Compress data after the connect offset
		const size_t compressionLength = reqstr.length() - compressionOffset;
		CompressedMessage compression(uncompressedStream, compressedStream);
		compression.Compress(compressionOffset, compressionLength);

		msg.WriteData(compressedData.data(), compressedStream.GetPosition());
	}
	else
	{
		// Write the request string
		StringMessage string = reqstr;
		msg.WriteString(string);
	}
}

IRequestPtr MOHPC::Network::EngineServer::IEngineRequest::process(RequestData& data)
{
	IRequestPtr newRequest;

	const netadr_t& from = data.getParam<GamespyUDPRequestParam>().getLastIp();

	// Set in OOB mode to read one single byte each call
	MSG msg(data.stream, msgMode_e::Reading);
	msg.SetCodec(MessageCodecs::OOB);

	uint32_t marker = msg.ReadUInteger();
	uint8_t dirByte = msg.ReadByte();

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
				Warning, "got wrong reply \"%s\" from %d.%d.%d.%d:%d",
				command, from.ip[0], from.ip[1], from.ip[2], from.ip[3], from.port
			);
		}
	}
	else
	{
		// Empty response
		MOHPC_LOG(
			Warning, "empty response from %d.%d.%d.%d:%d",
			from.ip[0], from.ip[1], from.ip[2], from.ip[3], from.port
		);
	}

	return newRequest;
}
