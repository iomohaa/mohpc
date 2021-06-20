#pragma once

#include <cstdint>
#include <functional>

#include "UserInfo.h"
#include "ProtocolParsing.h"
#include "../Reliable.h"

namespace MOHPC
{
	class TokenParser;
	class MSG;
	class StringMessage;

	namespace Network
	{
		struct gameState_t;
		class ClientTime;
		class UserInput;

		/**
		 * Various imports for anything wanting to use ClientConnection APIs.
		 */
		class ClientImports
		{
		public:
			std::function<uint32_t()> getClientNum;
			std::function<uintptr_t()> getCurrentSnapshotNumber;
			std::function<bool(uintptr_t snapshotNum, SnapshotInfo& outSnapshot)> getSnapshot;
			std::function<const ClientTime& ()> getClientTime;
			std::function<const UserInput&()> getUserInput;
			std::function<bool(uintptr_t cmdNum, usercmd_t& outCmd)> getUserCmd;
			std::function<bool(rsequence_t serverCommandNumber, TokenParser& tokenized)> getServerCommand;
			std::function<IGameState& ()> getGameState;
			std::function<StringMessage(MSG& msg)> readStringMessage;
			std::function<void(const char* cmd)> addReliableCommand;
			std::function<const ClientInfoPtr&()> getUserInfo;
		};
	}
}