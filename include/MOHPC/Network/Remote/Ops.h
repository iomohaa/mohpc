#pragma once

#include "../Exception.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	enum class netsrc_e : unsigned char {
		Client = 1,
		Server
	};

	enum class svc_ops_e : unsigned char
	{
		/** Bad operation. */
		Bad,
		/** Operation that should be ignored. */
		Nop,
		/** Gamestate parsing. */
		Gamestate,
		/** [short] [string] only in gamestate messages. */
		Configstring,
		/** [short] [entity] Baseline entity parsing (only in gamestate messages). */
		Baseline,
		/** [string] server command to execute on client. */
		ServerCommand,
		/** [short] download block. */
		Download,
		/** Snapshot parsing. */
		Snapshot,
		/** [string] Centerprint command. */
		Centerprint,
		/** [short] [short] [string] Locationprint command. */
		Locprint,
		/** [byte6] Message for client game module. */
		CGameMessage,
		/** Indicate the end of message (no more commands). */
		Eof
	};

	enum clc_ops_e : unsigned char
	{
		/** Bad operation. */
		Bad,
		Nop,
		/** [usercmd_t] Movement. */
		Move,
		/** [usercmd_t] Movement without delta. */
		MoveNoDelta,
		/** [string] Command to execute on server. */
		ClientCommand,
		/** Indicate the end of client message. */
		eof
	};

	namespace OpsErrors
	{
		class Base : public Network::NetworkException {};

		/**
		 * Invalid command while parsing game state.
		 */
		class BadCommandByteException : public Base
		{
		public:
			BadCommandByteException(uint8_t inCmdNum);

			MOHPC_NET_EXPORTS uint8_t getLength() const;
			MOHPC_NET_EXPORTS str what() const override;

		private:
			uint8_t cmdNum;
		};
	}
}
}