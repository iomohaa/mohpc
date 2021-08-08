#include <MOHPC/Network/Client/UserInput.h>
#include <MOHPC/Network/Client/Time.h>

using namespace MOHPC;
using namespace MOHPC::Network;

UserInput::UserInput()
	: cmdNumber(0)
{
}

void UserInput::reset()
{
	cmdNumber = 0;
}

usercmd_t& UserInput::createCommand(tickTime_t time)
{
	using namespace std::chrono;

	usercmd_t& cmd = cmds.get(cmdNumber++);
	cmd = usercmd_t(time);

	return cmd;
}

usereyes_t& UserInput::createEyes()
{
	eyeinfo = usereyes_t();
	return eyeinfo;
}

uint32_t UserInput::getCurrentCmdNumber() const
{
	return cmdNumber;
}

size_t UserInput::getNumCommands() const
{
	return cmds.count();
}

const usercmd_t& UserInput::getCommand(size_t index) const
{
	return cmds.get(index);
}

const usercmd_t& UserInput::getCommandFromLast(size_t index) const
{
	const size_t elem = cmdNumber - (index + 1);
	return getCommand(elem);
}

const usereyes_t& UserInput::getEyeInfo() const
{
	return eyeinfo;
}

usereyes_t& UserInput::getEyeInfo()
{
	return eyeinfo;
}

const usercmd_t& UserInput::getLastCommand() const
{
	return cmds.get(cmdNumber);
}

usercmd_t& UserInput::getLastCommand()
{
	return cmds.get(cmdNumber);
}

bool UserInput::getUserCmd(uintptr_t cmdNum, usercmd_t& outCmd) const
{
	// the usercmd has been overwritten in the wrapping
	// buffer because it is too far out of date
	const uint32_t cmdNumber = getCurrentCmdNumber();
	if (cmdNum + cmds.count() < cmdNumber) {
		return false;
	}

	outCmd = getCommand(cmdNum);
	return true;
}
