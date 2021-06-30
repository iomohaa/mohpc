#pragma once

#include <cstdint>
#include <functional>

#include "UserInfo.h"
#include "../Types/Reliable.h"

namespace MOHPC
{
	class TokenParser;
	class MSG;
	class StringMessage;
	namespace Network
	{
		struct gameState_t;
		class usercmd_t;
		class ClientTime;
		class UserInput;
		class ServerGameState;
		class ServerSnapshotManager;

		namespace Parsing
		{
			class IString;
		};

		/**
		 * Various imports for anything wanting to use ClientConnection APIs.
		 */
		class ClientImports
		{
		public:
			std::function<uint32_t()> getClientNum;
			std::function<const ServerSnapshotManager& ()> getSnapshotManager;
			std::function<const ClientTime& ()> getClientTime;
			std::function<const UserInput&()> getUserInput;
			std::function<bool(uintptr_t cmdNum, usercmd_t& outCmd)> getUserCmd;
			std::function<bool(rsequence_t serverCommandNumber, TokenParser& tokenized)> getServerCommand;
			std::function<ServerGameState& ()> getGameState;
			std::function<void(const char* cmd)> addReliableCommand;
			std::function<const UserInfoPtr&()> getUserInfo;

		public:
			const Parsing::IString* stringParser;
		};
	}
}