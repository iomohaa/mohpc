#include <MOHPC/Network/ClientCommands.h>
#include <MOHPC/Network/Event.h>
#include <MOHPC/Network/Client.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Log.h>
#include <GameSpy/gcdkey/gcdkeyc.h>

using namespace MOHPC;
using namespace Network;
using namespace Log;

static constexpr char* MOHPC_LOG_NAMESPACE = "network_cmd";

CommandList Network::clientEvents;

IClientConnectionlessHandler::IClientConnectionlessHandler(const char* inEventName)
	: IConnectionlessCommand(Network::clientEvents, inEventName)
{
}

class GetKeyEvent : public IClientConnectionlessHandler
{
private:
	const char* cdkey;
	
	struct kstruct_t {
		uint32_t field0;
		uint32_t field1;
		uint32_t field2;
		uint32_t field3;
		uint32_t field4;
		uint32_t field5;

		kstruct_t()
			: field0(0x67452301)
			, field1(0xEFCDAB89)
			, field2(0x98BADCFE)
			, field3(0x10325476)
			, field4(0)
			, field5(0)
		{}
	};

public:
	GetKeyEvent()
		: IClientConnectionlessHandler("getKey")
	{
		cdkey = "";
	}

	virtual bool doRun(MSG& msg, TokenParser& TokenParser, Event& ev) override
	{
		char outResponse[76];

		// Get the key
		const char* key = TokenParser.GetToken(false);

		gcd_compute_response((char*)cdkey, (char*)key, outResponse, CDResponseMethod_NEWAUTH);

		ev.AddString(outResponse);

		return true;
	}
};
IClientConnectionlessHandler& Network::getKeyEvent = GetKeyEvent();

class ChallengeResponseEvent : public IClientConnectionlessHandler
{
public:
	ChallengeResponseEvent()
		: IClientConnectionlessHandler("challengeResponse")
	{
	}

	virtual bool doRun(MSG& msg, TokenParser& TokenParser, Event& ev) override
	{
		int challengeResponse = TokenParser.GetInteger(false);
		ev.AddInteger(challengeResponse);
		return true;
	}
};
IClientConnectionlessHandler& Network::challengeResponseEvent = ChallengeResponseEvent();

class ConnectResponseEvent : public IClientConnectionlessHandler
{
public:
	ConnectResponseEvent()
		: IClientConnectionlessHandler("connectResponse")
	{
	}

	virtual bool doRun(MSG& msg, TokenParser& TokenParser, Event& ev) override
	{
		return true;
	}
};
IClientConnectionlessHandler& Network::connectResponseEvent = ConnectResponseEvent();

class DropErrorEvent : public IClientConnectionlessHandler
{
public:
	DropErrorEvent()
		: IClientConnectionlessHandler("droperror")
	{
	}

	virtual bool doRun(MSG& msg, TokenParser& TokenParser, Event& ev) override
	{
		const char* message = TokenParser.GetLine(false);
		ev.AddString(message);
		return true;
	}
};
IClientConnectionlessHandler& Network::dropErrorEvent = DropErrorEvent();

