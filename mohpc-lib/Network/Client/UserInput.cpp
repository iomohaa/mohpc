#include <MOHPC/Network/Client/UserInput.h>

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

void UserInput::createCommand(uint64_t currentTime, uint64_t remoteTime, usercmd_t*& outCmd, usereyes_t*& outEyes)
{
	++cmdNumber;
	const uint32_t cmdNum = cmdNumber % CMD_BACKUP;

	usercmd_t& cmd = cmds[cmdNum];
	cmd = usercmd_t((uint32_t)remoteTime);
	eyeinfo = usereyes_t();

	outCmd = &cmd;
	outEyes = &eyeinfo;
}

uint32_t UserInput::getCurrentCmdNumber() const
{
	return cmdNumber;
}

const usercmd_t& UserInput::getCommand(size_t index) const
{
	return cmds[index % CMD_BACKUP];
}

const usercmd_t& UserInput::getCommandFromLast(size_t index) const
{
	const size_t elem = cmdNumber - index;
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
	return cmds[cmdNumber % CMD_BACKUP];
}

usercmd_t& UserInput::getLastCommand()
{
	return cmds[cmdNumber % CMD_BACKUP];
}

bool UserInput::getUserCmd(uintptr_t cmdNum, usercmd_t& outCmd) const
{
	// the usercmd has been overwritten in the wrapping
	// buffer because it is too far out of date
	const uint32_t cmdNumber = getCurrentCmdNumber();
	if (cmdNum + CMD_BACKUP < cmdNumber) {
		return false;
	}

	outCmd = getCommand(cmdNum);
	return true;
}
