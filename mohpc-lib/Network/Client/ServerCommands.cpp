#include <MOHPC/Network/Client/ServerConnection.h>
#include <MOHPC/Network/Client/Server.h>
#include <MOHPC/Network/Client/Protocol.h>
#include <MOHPC/Network/Version.h>
#include <MOHPC/Utility/Info.h>
#include <MOHPC/Utility/TokenParser.h>
#include <MOHPC/Utility/ClassList.h>
#include <MOHPC/Common/Log.h>
#include <GameSpy/gcdkey/gcdkeyc.h>

#include <random>

static constexpr char MOHPC_LOG_NAMESPACE[] = "server_cmd";

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace Log;

EngineServer::IEngineRequest::IEngineRequest(Callbacks::ServerTimeout&& inTimeoutCallback)
	: timeoutCallback(inTimeoutCallback)
{
}

//== Get version before challenge
EngineServer::VerBeforeChallengeRequest::VerBeforeChallengeRequest(ConnectionParams&& inData)
	: data(std::move(inData))
{
}

str EngineServer::VerBeforeChallengeRequest::generateRequest()
{
	return "getinfo";
}

bool EngineServer::VerBeforeChallengeRequest::supportsEvent(const char* name) const
{
	return !str::icmp(name, "infoResponse");
}

IRequestPtr EngineServer::VerBeforeChallengeRequest::handleResponse(const char* name, TokenParser& parser)
{
	const char* token = parser.GetLine(true);
	ReadOnlyInfo info(token);

	const serverType_e serverType = serverType_e(info.IntValueForKey("serverType"));
	const uint32_t protocolVersionNumber = info.IntValueForKey("protocol");

	const protocolVersion_e protocolVersion = getProtocolEnumVersion(protocolVersionNumber);
	if (protocolVersion == protocolVersion_e::bad)
	{
		// Don't connect to servers with unsupported protocol
		MOHPC_LOG(Error, "unsupported protocol version: %d", protocolVersionNumber);
		return nullptr;
	}

	size_t gameVersionLen;
	const char* gameVersion = info.ValueForKey("gamever", gameVersionLen);

	MOHPC_LOG(Info, "server type %d protocol version %d game version \"%.*s\"", serverType, protocolVersionNumber, gameVersionLen, gameVersion);
	return makeShared<ChallengeRequest>(protocolType_c(serverType, protocolVersion), std::move(data), std::move(timeoutCallback));
}

//== Challenge
EngineServer::ChallengeRequest::ChallengeRequest(const protocolType_c& proto, ConnectionParams&& inData, Callbacks::ServerTimeout&& inTimeoutCallback)
	: IEngineRequest(std::move(inTimeoutCallback))
	, data(std::move(inData))
	, protocol(proto)
	, numRetries(0)
{

}

str EngineServer::ChallengeRequest::generateRequest()
{
	return "getchallenge";
}

bool EngineServer::ChallengeRequest::supportsEvent(const char* name) const
{
	return !str::icmp(name, "getKey") || !str::icmp(name, "challengeResponse");
}

IRequestPtr EngineServer::ChallengeRequest::handleResponse(const char* name, TokenParser& parser)
{
	if (!str::icmp(name, "getKey"))
	{
		const char* challenge = parser.GetLine(false);
		MOHPC_LOG(Debug, "forwarding request for getKey: %s", name);
		return makeShared<AuthorizeRequest>(protocol, std::move(data), challenge, std::move(timeoutCallback));
	}

	int32_t challengeResponse = parser.GetInteger(false);
	MOHPC_LOG(Debug, "challenge %d", challengeResponse);
	return makeShared<ConnectRequest>(protocol, std::move(data), challengeResponse, std::move(timeoutCallback));
}

uint64_t EngineServer::ChallengeRequest::overrideTimeoutTime(bool& overriden)
{
	overriden = true;
	return 3000;
}

IRequestPtr EngineServer::ChallengeRequest::timedOut()
{
	if (numRetries > 5)
	{
		return IEngineRequest::timedOut();
	}

	numRetries++;
	return shared_from_this();
}

uint64_t EngineServer::ChallengeRequest::deferredTime()
{
	return data.settings ? data.settings->getDeferredChallengeTime() : 100;
}

//== Authorize
EngineServer::AuthorizeRequest::AuthorizeRequest(const protocolType_c& proto, ConnectionParams&& inData, const char* inChallenge, Callbacks::ServerTimeout&& inTimeoutCallback)
	: IEngineRequest(std::move(inTimeoutCallback))
	, data(std::move(inData))
	, protocol(proto)
	, challenge(inChallenge)
	, numRetries(0)
{
}

bool EngineServer::AuthorizeRequest::supportsEvent(const char* name) const
{
	return !str::icmp(name, "getKey") || !str::icmp(name, "challengeResponse");
}

str EngineServer::AuthorizeRequest::generateRequest()
{
	const char* cdKey = data.settings ? data.settings->getCDKey() : "";

	char outResponse[76];

	// Get the key
	gcd_compute_response((char*)cdKey, (char*)challenge.c_str(), outResponse, CDResponseMethod_NEWAUTH);

	ILog::get().log(logType_e::Debug, "network", "will send authorization \"%s\"", outResponse);

	return "authorizeThis " + str(outResponse);
}

IRequestPtr EngineServer::AuthorizeRequest::handleResponse(const char* name, TokenParser& parser)
{
	if (!str::icmp(name, "challengeResponse"))
	{
		int32_t challenge = parser.GetInteger(false);
		MOHPC_LOG(Debug, "got challenge %d", challenge);
		return makeShared<ConnectRequest>(protocol, std::move(data), challenge, std::move(timeoutCallback));
	}

	return nullptr;
}

uint64_t EngineServer::AuthorizeRequest::overrideTimeoutTime(bool& overriden)
{
	overriden = true;
	// 5000 is the timeout time for the server to authorize
	// but adding 500ms more to let the server receive the response
	return 5500;
}

IRequestPtr EngineServer::AuthorizeRequest::timedOut()
{
	if (numRetries > 2) {
		return IEngineRequest::timedOut();
	}

	numRetries++;
	return shared_from_this();
}

//== Connect
EngineServer::ConnectRequest::ConnectRequest(const protocolType_c& proto, ConnectionParams&& inData, uint32_t inChallenge, Callbacks::ServerTimeout&& inTimeoutCallback)
	: IEngineRequest(std::move(inTimeoutCallback))
	, data(std::move(inData))
	, protocol(proto)
	, challenge(inChallenge)
	, numRetries(0)
{
	qport = data.settings ? data.settings->getQport() : 0;
	if(!qport)
	{
		std::random_device dev;
		std::mt19937 gen;
		std::uniform_int_distribution<uint16_t> dist((1 << 15) + (1 << 14), (1 << 16) - 1);
		// choose a random port
		qport = dist(gen);
	}
}

str EngineServer::ConnectRequest::generateRequest()
{
	//========================
	// fill in important info
	//========================

	Info info;
	// append the challenge
	info.SetValueForKey("challenge", str::printf("%i", challenge));
	// send the client version and the protocol
	const char* version = data.settings ? data.settings->getVersion() : NETWORK_VERSION;
	info.SetValueForKey("version", version);

	for(const IClientProtocol* proto = IClientProtocol::getHead(); proto; proto = proto->getNext())
	{
		if (proto->getServerProtocol() == protocol.getProtocolVersionNumber())
		{
			info.SetValueForKey("protocol", str::printf("%i", proto->getBestCompatibleProtocol()));
			const char* clientType = proto->getClientType();
			if (clientType)
			{
				// connecting to spearhead/breakthrough
				info.SetValueForKey("clientType", clientType);
			}

			break;
		}
	}
	// write the translated port
	info.SetValueForKey("qport", str::printf("%i", qport));

	// fill user settings into an info instance
	ClientInfoHelper::fillInfoString(*data.info, info);

	// send user info string
	str connectArgs;
	// 9: --> "connect " <--
	// 2: quotes
	connectArgs.reserve(info.GetInfoLength() + 9 + 2);
	connectArgs += "connect ";
	connectArgs += " \"";
	connectArgs += info.GetString();
	connectArgs += '"';

	return connectArgs;
}

bool EngineServer::ConnectRequest::shouldCompressRequest(size_t& offset)
{
	// after --> "connect " <--
	offset = 8;
	return true;
}

bool EngineServer::ConnectRequest::supportsEvent(const char* name) const
{
	return !str::icmp(name, "connectResponse") || !str::icmp(name, "droperror") || !str::icmp(name, "print") || !str::icmp(name, "serverfull");
}

IRequestPtr EngineServer::ConnectRequest::handleResponse(const char* name, TokenParser& parser)
{
	if (!str::icmp(name, "droperror"))
	{
		const char* error = parser.GetLine(true);
		data.response(0, 0, protocolType_c(), data.info, error);
		return nullptr;
	}
	else if(str::icmp(name, "connectResponse"))
	{
		if (!str::icmp(name, "serverfull"))
		{
			// Received other than connect response after multiple time, so don't connect again
			MOHPC_LOG(Error, "server is full");
			return nullptr;
		}

		const char* args = parser.GetLine(true);

		if(numRetries < 5)
		{
			MOHPC_LOG(Error, "not a connect response, received %s: \"%s\"", name, args);

			numRetries++;
			return shared_from_this();
		}
		else
		{
			// Received other than connect response after multiple time, so don't connect again
			MOHPC_LOG(Error, "assuming connection failed: %s: \"%s\"", name, args);
			return nullptr;
		}
	}

	MOHPC_LOG(Debug, "connection succeeded");
	data.response(qport, challenge, protocol, std::move(data.info), nullptr);
	return nullptr;
}

uint64_t EngineServer::ConnectRequest::overrideTimeoutTime(bool& overriden)
{
	overriden = true;
	return 3000;
}

IRequestPtr EngineServer::ConnectRequest::timedOut()
{
	if (numRetries > 5) {
		return IEngineRequest::timedOut();
	}

	numRetries++;
	return shared_from_this();
}

uint64_t EngineServer::ConnectRequest::deferredTime()
{
	return data.settings ? data.settings->getDeferredConnectTime() : 100;
}

//== GetStatus
EngineServer::StatusRequest::StatusRequest(Callbacks::Response&& inResponse)
	: response(inResponse)
{

}

str EngineServer::StatusRequest::generateRequest()
{
	return "getstatus";
}

bool EngineServer::StatusRequest::supportsEvent(const char* name) const
{
	return !str::icmp(name, "statusResponse");
}

IRequestPtr EngineServer::StatusRequest::handleResponse(const char* name, TokenParser& parser)
{
	const char* token = parser.GetLine(true);
	if (*token)
	{
		ReadOnlyInfo info(token);
		response(&info);
	}
	else
	{
		// Timed out
		response(nullptr);
	}

	return nullptr;
}

//== GetInfo
EngineServer::InfoRequest::InfoRequest(Callbacks::Response&& inResponse)
	: response(inResponse)
{
}

str EngineServer::InfoRequest::generateRequest()
{
	return "getinfo";
}

bool EngineServer::InfoRequest::supportsEvent(const char* name) const
{
	return !str::icmp(name, "infoResponse");
}

IRequestPtr EngineServer::InfoRequest::handleResponse(const char* name, TokenParser& parser)
{
	const char* token = parser.GetLine(true);
	ReadOnlyInfo info(token);
	response(&info);
	return nullptr;
}
