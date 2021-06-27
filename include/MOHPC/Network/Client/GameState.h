#pragma once

#include "../../Utility/HandlerList.h"
#include "../Types/GameState.h"
#include "../Parsing/GameState.h"
#include "../Configstring.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
class MSG;

namespace Network
{
	class ClientTime;
	class ICommandSequence;
	class ServerGameState;

	namespace ClientHandlers
	{
		/**
		 * Called after the insertion/modification of a new configstring.
		 *
		 * @param	csNum			The configstring num.
		 * @param	configString	The string pointed at by the csNum.
		 */
		struct Configstring : public HandlerNotifyBase<void(csNum_t csNum, const char* configString)> {};

		/**
		 * Called the game state was parsed. Can be called multiple times.
		 * This is the callback to use for map change/loading.
		 *
		 * @param   gameState       New game state.
		 * @param   differentLevel  False if the client has to re-download game state for the current game.
		 *                          True = new game session (but doesn't necessarily mean that it is a different map file).
		 */
		struct GameStateParsed : public HandlerNotifyBase<void(const ServerGameState& gameState, bool differentLevel)> {};
	}

	class ServerGameState
	{
	private:
		struct HandlerList
		{
		public:
			FunctionList<ClientHandlers::GameStateParsed> gameStateParsedHandler;
			FunctionList<ClientHandlers::Configstring> configStringHandler;
		};

	public:
		ServerGameState(protocolType_c protocol);

		MOHPC_NET_EXPORTS HandlerList& getHandlers();
		MOHPC_NET_EXPORTS const HandlerList& getHandlers() const;
		MOHPC_NET_EXPORTS gameState_t& get();
		MOHPC_NET_EXPORTS const gameState_t& get() const;
		bool parseGameState(MSG& msg, ICommandSequence* serverCommands, ClientTime& clientTime);
		bool reloadGameState(ClientTime& clientTime);

	private:
		void notifyAllConfigStringChanges() const;

	private:
		gameState_t gameState;
		HandlerList handlers;
		const Parsing::IGameState* gameStateParser;

	public:
		uint32_t serverId;
		uint32_t clientNum;
		uint32_t checksumFeed;
	};
}
}