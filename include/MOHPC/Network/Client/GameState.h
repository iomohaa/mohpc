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
class CommandManager;
class TokenParser;

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
		/** Construct a game state without a protocol. */
		ServerGameState();
		/** Construct a game state for remote replication. */
		ServerGameState(protocolType_c protocol, ClientTime* clientTimePtr);

		/** Return the handler list specific to the server game state. */
		MOHPC_NET_EXPORTS HandlerList& handlers();
		MOHPC_NET_EXPORTS const HandlerList& handlers() const;

		/** Return the underlying game state. */
		MOHPC_NET_EXPORTS gameState_t& get();
		MOHPC_NET_EXPORTS const gameState_t& get() const;

		/**
		 * Parse the game state.
		 *
		 * @param msg The message to use for reading.
		 * @param serverCommands A pointer to the command sequence to set the new server command sequence.
		 * @return True if it's a different map.
		 */
		MOHPC_NET_EXPORTS bool parseGameState(MSG& msg, ICommandSequence* serverCommands);

		/**
		 * Reload the game state by parsing the config string.
		 *
		 * @return True if it's a different map.
		 */
		MOHPC_NET_EXPORTS bool reloadGameState();

		/** Return the server id of the game state (represents an ID of the current map). */
		MOHPC_NET_EXPORTS uint32_t getServerId() const;

		/** Return the client num in the server for the local client.*/
		MOHPC_NET_EXPORTS uint32_t getClientNum() const;

		/** Return the map checksum. */
		MOHPC_NET_EXPORTS uint32_t getChecksumFeed() const;

		void RegisterCommands(CommandManager& commandManager);

	private:
		void notifyAllConfigStringChanges() const;
		void modifyConfigString(csNum_t num, const char* string);
		void ConfigstringCommand(TokenParser& tokenized);

	private:
		const Parsing::IGameState* gameStateParser;
		ClientTime* clientTime;
		HandlerList handlerList;
		gameState_t gameState;
		uint32_t serverId;
		uint32_t clientNum;
		uint32_t checksumFeed;
	};
}
}