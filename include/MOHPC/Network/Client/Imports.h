#pragma once

#include <cstdint>
#include <functional>

#include "UserInfo.h"

namespace MOHPC
{
	class TokenParser;
	class MSG;
	class StringMessage;

	namespace Network
	{
		struct gameState_t;

		/**
		 * Various imports for anything wanting to use ClientConnection APIs.
		 */
		class ClientImports
		{
		public:
			std::function<uint32_t()> getClientNum;
			std::function<uintptr_t()> getCurrentSnapshotNumber;
			std::function<bool(uintptr_t snapshotNum, SnapshotInfo& outSnapshot)> getSnapshot;
			std::function<uint64_t()> getServerStartTime;
			std::function<uint64_t()> getServerTime;
			std::function<uint64_t()> getServerFrameFrequency;
			std::function<uintptr_t()> getCurrentCmdNumber;
			std::function<bool(uintptr_t cmdNum, usercmd_t& outCmd)> getUserCmd;
			std::function<bool(uintptr_t serverCommandNumber, TokenParser& tokenized)> getServerCommand;
			std::function<const gameState_t& ()> getGameState;
			std::function<StringMessage(MSG& msg)> readStringMessage;
			std::function<void(const char* cmd)> addReliableCommand;
			std::function<const ClientInfoPtr&()> getUserInfo;
		};
	}
}