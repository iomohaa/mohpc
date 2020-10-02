#include <MOHPC/Network/Event.h>
#include <MOHPC/Network/Client/ClientGame.h>
#include <MOHPC/Network/Client/Server.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Log.h>
#include <GameSpy/gcdkey/gcdkeyc.h>

static constexpr char* MOHPC_LOG_NAMESPACE = "server_cmd";

using namespace MOHPC;
using namespace Network;
using namespace Log;

//== Get version before challenge
MOHPC::Network::EngineServer::VerBeforeChallengeRequest::VerBeforeChallengeRequest(ConnectionParams&& inData)
	: data(std::move(inData))
{
}

MOHPC::str MOHPC::Network::EngineServer::VerBeforeChallengeRequest::generateRequest()
{
	return "getinfo";
}

bool MOHPC::Network::EngineServer::VerBeforeChallengeRequest::supportsEvent(const char* name)
{
	return !stricmp(name, "infoResponse");
}

IRequestPtr MOHPC::Network::EngineServer::VerBeforeChallengeRequest::handleResponse(const char* name, TokenParser& parser)
{
	const char* token = parser.GetLine(true);
	ReadOnlyInfo info(token);

	str value = info.ValueForKey("serverType");
	const uint32_t serverType = atoi(value.c_str());

	value = info.ValueForKey("protocol");
	const uint32_t protocolVersionNumber = atoi(value.c_str());

	value = info.ValueForKey("gamever");

	const protocolVersion_e protocolVersion = getProtocolEnumVersion(protocolVersionNumber);
	if (protocolVersion == protocolVersion_e::bad)
	{
		// Don't connect to servers with unsupported protocol
		MOHPC_LOG(Error, "unsupported protocol version: %d", protocolVersionNumber);
		return nullptr;
	}

	MOHPC_LOG(Log, "server type %d protocol version %d game version \"%s\"", serverType, protocolVersionNumber, value.c_str());
	return makeShared<ChallengeRequest>(protocolType_c(serverType, protocolVersion), std::move(data));
}

//== Challenge
Network::EngineServer::ChallengeRequest::ChallengeRequest(const protocolType_c& proto, ConnectionParams&& inData)
	: data(std::move(inData))
	, protocol(proto)
	, numRetries(0)
{

}

str Network::EngineServer::ChallengeRequest::generateRequest()
{
	return "getchallenge";
}

bool Network::EngineServer::ChallengeRequest::supportsEvent(const char* name)
{
	return !stricmp(name, "getKey") || !stricmp(name, "challengeResponse");
}

IRequestPtr MOHPC::Network::EngineServer::ChallengeRequest::handleResponse(const char* name, TokenParser& parser)
{
	if (!stricmp(name, "getKey"))
	{
		const char* challenge = parser.GetLine(false);
		MOHPC_LOG(Verbose, "forwarding request for getKey: %s", name);
		return makeShared<AuthorizeRequest>(protocol, std::move(data), challenge);
	}

	int32_t challengeResponse = parser.GetInteger(false);
	MOHPC_LOG(Verbose, "challenge %d", challengeResponse);
	return makeShared<ConnectRequest>(protocol, std::move(data), challengeResponse);
}

size_t MOHPC::Network::EngineServer::ChallengeRequest::overrideTimeoutTime(bool& overriden)
{
	overriden = true;
	return 3000;
}

MOHPC::IRequestPtr EngineServer::ChallengeRequest::timedOut()
{
	if (numRetries > 5)
	{
		return IRequestBase::timedOut();
	}

	numRetries++;
	return shared_from_this();
}

size_t EngineServer::ChallengeRequest::deferredTime()
{
	return data.settings->getDeferredChallengeTime();
}

//== Authorize
Network::EngineServer::AuthorizeRequest::AuthorizeRequest(const protocolType_c& proto, ConnectionParams&& inData, const char* inChallenge)
	: data(std::move(inData))
	, protocol(proto)
	, challenge(inChallenge)
	, numRetries(0)
{
}

bool Network::EngineServer::AuthorizeRequest::supportsEvent(const char* name)
{
	return !stricmp(name, "getKey") || !stricmp(name, "challengeResponse");
}

str Network::EngineServer::AuthorizeRequest::generateRequest()
{
	char outResponse[76];

	// Get the key
	gcd_compute_response((char*)"", (char*)challenge.c_str(), outResponse, CDResponseMethod_NEWAUTH);

	ILog::get().log(logType_e::Verbose, "network", "will send authorization \"%s\"", outResponse);

	return "authorizeThis " + str(outResponse);
}

IRequestPtr MOHPC::Network::EngineServer::AuthorizeRequest::handleResponse(const char* name, TokenParser& parser)
{
	if (!stricmp(name, "challengeResponse"))
	{
		int32_t challenge = parser.GetInteger(false);
		MOHPC_LOG(Verbose, "got challenge %d", challenge);
		return makeShared<ConnectRequest>(protocol, std::move(data), challenge);
	}

	return nullptr;
}

size_t MOHPC::Network::EngineServer::AuthorizeRequest::overrideTimeoutTime(bool& overriden)
{
	overriden = true;
	// 5000 is the timeout time for the server to authorize
	return 5000;
}

MOHPC::IRequestPtr MOHPC::Network::EngineServer::AuthorizeRequest::timedOut()
{
	if (numRetries > 0) {
		return IEngineRequest::timedOut();
	}

	numRetries++;
	return shared_from_this();
}

//== Connect
Network::EngineServer::ConnectRequest::ConnectRequest(const protocolType_c& proto, ConnectionParams&& inData, uint32_t inChallenge)
	: data(std::move(inData))
	, protocol(proto)
	, challenge(inChallenge)
	, numRetries(0)
{
	// a port between 20000 and 65535
	qport = (rand() % 45536) + 20000;
}

str Network::EngineServer::ConnectRequest::generateRequest()
{
	str connectArgs = "connect";

	Info info;

	// Fill in important info

	// Append the challenge
	info.SetValueForKey("challenge", str::printf("%i", challenge));
	// Send the client version and the protocol
	info.SetValueForKey("version", CLIENT_VERSION);
	info.SetValueForKey("protocol", str::printf("%i", protocol.getProtocolVersion()));

	// In case it's connecting to a breakthrough server
	if (protocol.getServerType() == 2) {
		info.SetValueForKey("clientType", "Breakthrough");
	}

	info.SetValueForKey("qport", str::printf("%i", qport));

	// Set user info
	data.info->fillInfoString(info);

	// Send user info string
	connectArgs.reserve(info.GetInfoLength() + 2);
	connectArgs += " \"";
	connectArgs += info.GetString();
	connectArgs += '"';

	return connectArgs;
}

bool Network::EngineServer::ConnectRequest::shouldCompressRequest(size_t& offset)
{
	offset = 13;
	return true;
}

bool Network::EngineServer::ConnectRequest::supportsEvent(const char* name)
{
	return !str::icmp(name, "connectResponse") || !str::icmp(name, "droperror") || !str::icmp(name, "print");
}

IRequestPtr MOHPC::Network::EngineServer::ConnectRequest::handleResponse(const char* name, TokenParser& parser)
{
	if (!str::icmp(name, "droperror"))
	{
		const char* error = parser.GetLine(true);
		data.response(0, 0, protocolType_c(), data.info, error);
		return nullptr;
	}
	else if(str::icmp(name, "connectResponse"))
	{
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

	MOHPC_LOG(Verbose, "connection succeeded");
	data.response(qport, challenge, protocol, std::move(data.info), nullptr);
	return nullptr;
}

size_t MOHPC::Network::EngineServer::ConnectRequest::overrideTimeoutTime(bool& overriden)
{
	overriden = true;
	return 3000;
}

MOHPC::IRequestPtr MOHPC::Network::EngineServer::ConnectRequest::timedOut()
{
	if (numRetries > 5) {
		return IEngineRequest::timedOut();
	}

	numRetries++;
	return shared_from_this();
}

size_t EngineServer::ConnectRequest::deferredTime()
{
	return data.settings->getDeferredConnectTime();
}

//== GetStatus
Network::EngineServer::StatusRequest::StatusRequest(Callbacks::Response&& inResponse)
	: response(inResponse)
{

}

str Network::EngineServer::StatusRequest::generateRequest()
{
	return "getstatus";
}

bool Network::EngineServer::StatusRequest::supportsEvent(const char* name)
{
	return !stricmp(name, "statusResponse");
}

IRequestPtr MOHPC::Network::EngineServer::StatusRequest::handleResponse(const char* name, TokenParser& parser)
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
Network::EngineServer::InfoRequest::InfoRequest(Callbacks::Response&& inResponse)
	: response(inResponse)
{

}

str Network::EngineServer::InfoRequest::generateRequest()
{
	return "getinfo";
}

bool Network::EngineServer::InfoRequest::supportsEvent(const char* name)
{
	return !stricmp(name, "infoResponse");
}

IRequestPtr MOHPC::Network::EngineServer::InfoRequest::handleResponse(const char* name, TokenParser& parser)
{
	const char* token = parser.GetLine(true);
	ReadOnlyInfo info(token);
	response(&info);
	return nullptr;
}
