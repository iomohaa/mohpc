#pragma once

#include "../Types/UserInput.h"

namespace MOHPC
{
namespace Network
{
	static constexpr unsigned long CMD_BACKUP = (1 << 8);
	static constexpr unsigned long CMD_MASK = CMD_BACKUP - 1;

	class UserInput
	{
	public:
		UserInput();

		void reset();
		void createCommand(uint64_t currentTime, uint64_t remoteTime, usercmd_t*& outCmd, usereyes_t*& outEyes);
		uint32_t getCurrentCmdNumber() const;
		const usercmd_t& getCommand(size_t index) const;
		const usercmd_t& getCommandFromLast(size_t index) const;
		const usercmd_t& getLastCommand() const;
		usercmd_t& getLastCommand();
		const usereyes_t& getEyeInfo() const;
		usereyes_t& getEyeInfo();
		bool getUserCmd(uintptr_t cmdNum, usercmd_t& outCmd) const;

	private:
		uint32_t cmdNumber;
		usereyes_t eyeinfo;
		usercmd_t cmds[CMD_BACKUP];
	};
}
}