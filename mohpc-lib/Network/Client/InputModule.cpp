#include <MOHPC/Network/Client/InputModule.h>
#include <MOHPC/Network/Client/Time.h>
#include <MOHPC/Network/Types/Protocol.h>
#include <MOHPC/Network/Parsing/Input.h>
#include <MOHPC/Common/Log.h>

using namespace MOHPC;
using namespace MOHPC::Network;

constexpr char MOHPC_LOG_NAMESPACE[] = "client_input_mod";

static constexpr unsigned long MAX_PACKET_USERCMDS = 32;

MOHPC_OBJECT_DEFINITION(UserGameInputModule);
UserGameInputModule::UserGameInputModule(const protocolType_c& protocolType)
{
	const uint32_t protocol = protocolType.getProtocolVersionNumber();

	userMoveWriter = Parsing::IUserMove::get(protocol);
}

UserGameInputModule::~UserGameInputModule()
{
}

const UserInput& UserGameInputModule::getUserInput() const
{
	return input;
}

UserInput& UserGameInputModule::getUserInput()
{
	return input;
}

UserGameInputModule::HandlerList& UserGameInputModule::getHandlerList()
{
	return handlerList;
}

uint8_t UserGameInputModule::getNumCommandsToWrite(uint32_t index)
{
	const uint32_t cmdNumber = input.getCurrentCmdNumber();
	if (cmdNumber > index + MAX_PACKET_USERCMDS)
	{
		MOHPC_LOG(Warn, "MAX_PACKET_USERCMDS");
		return MAX_PACKET_USERCMDS;
	}

	return cmdNumber - index;
}

void UserGameInputModule::process(const ClientTime& time)
{
	// only create commands if the client is completely ready
	usercmd_t& newCmd = input.createCommand(time_cast<tickTime_t>(time.getSimulatedRemoteTime()));
	usereyes_t& newEyes = input.createEyes();

	// all movement will happen through the event notification
	// the callee is responsible for filling the user input
	getHandlerList().userInputHandler.broadcast(newCmd, newEyes);
}

bool UserGameInputModule::canWrite() const
{
	return true;
}

void UserGameInputModule::write(MSG& msg, uint32_t key, bool deltaMove)
{
	// gather command count
	const uint8_t count = getNumCommandsToWrite(lastCmdNum);

	// write user commands
	userMoveWriter->writeMovement(
		msg,
		oldeyes,
		input.getEyeInfo(),
		[this](size_t i) -> const usercmd_t& { return input.getCommandFromLast(i); },
		count,
		key,
		deltaMove
	);

	// save values for later
	lastCmdNum = input.getCurrentCmdNumber();
	oldeyes = input.getEyeInfo();
}

