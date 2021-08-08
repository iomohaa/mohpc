#pragma once

#include "../Types/UserInput.h"
#include "../../Utility/WarpArray.h"

namespace MOHPC
{
namespace Network
{
	class ClientTime;

	class UserInput
	{
	public:
		UserInput();

		MOHPC_NET_EXPORTS usercmd_t& createCommand(tickTime_t time);
		MOHPC_NET_EXPORTS usereyes_t& createEyes();

		void reset();
		uint32_t getCurrentCmdNumber() const;
		size_t getNumCommands() const;
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
		StaticWarpArray<usercmd_t, 256> cmds;
	};
}
}
