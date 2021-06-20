#pragma once

#include "../NetGlobal.h"
#include "../ProtocolSingleton.h"
#include "../../Utility/ClassList.h"
#include "../../Utility/SharedPtr.h"
#include "../../Utility/HandlerList.h"
#include "../../Network/Configstring.h"
#include "../../Network/InfoTypes.h"
#include "../../Network/Types.h"
#include "../../Utility/Misc/MSG/Serializable.h"

namespace MOHPC
{
class MSG;
class StringMessage;
class ReadOnlyInfo;

namespace Network
{
	class ClientSnapshot;
	class IGameState;
	class IClientGameProtocol;

	namespace GameStateHandlers
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
		struct GameStateParsed : public HandlerNotifyBase<void(const IGameState& gameState, bool differentLevel)> {};
	}

	class IGameState
	{
	private:
		struct HandlerList
		{
		public:
			FunctionList<GameStateHandlers::Configstring> configStringHandler;
			FunctionList<GameStateHandlers::GameStateParsed> gameStateParsedHandler;
		};

	public:
		IGameState();
		virtual ~IGameState();

		MOHPC_NET_EXPORTS HandlerList& getHandlerList();

		MOHPC_NET_EXPORTS const char* getConfigString(csNum_t num) const;
		virtual const char* getConfigStringChecked(csNum_t num) const = 0;
		virtual size_t getNumConfigStrings() const = 0;
		virtual void setConfigString(csNum_t num, const char* configString, size_t sz) = 0;
		virtual entityState_t& getBaseline(size_t num) = 0;
		virtual void reset() = 0;
		void postParse(bool differentServer);

		void notifyConfigStringChange(csNum_t num, const char* newString);
		void notifyAllConfigStringChanges();
		void configStringModified(csNum_t num, const char* newString, bool notify);
		virtual void systemInfoChanged();
		bool isDifferentServer(uint32_t id) const;

	public:
		HandlerList handlerList;
		uint32_t clientNum;
		uint32_t checksumFeed;
		uint32_t serverId;
		uint64_t serverDeltaTime;
		float serverDeltaTimeSeconds;
	};
	using IGameStatePtr = SharedPtr<IGameState>;
	using IGameStateInstancier = IProtocolClassInstancier<IGameState>;

	class IGameStateParser : public ISerializableMessage
	{
	public:
		IGameStateParser(const IClientGameProtocol& clientProto, IGameState& gameStateValue);

		size_t getCommandSequence() const;

	public:
		const IClientGameProtocol& clientProto;
		IGameState& gameState;

	protected:
		size_t commandSequence;
	};

	class IClientGameProtocol : public ClassList<IClientGameProtocol>
	{
	public:
		virtual ~IClientGameProtocol();

		virtual uint32_t getProtocolVersion() const = 0;

		virtual StringMessage readString(MSG& msg) const = 0;
		virtual void writeString(MSG& msg, const char* s) const = 0;
		virtual uint32_t hashKey(const char* string, size_t maxlen) const = 0;
		virtual entityNum_t readEntityNum(MSG& msg) const = 0;
		virtual void readDeltaPlayerstate(MSG& msg, const playerState_t* from, playerState_t* to) const = 0;
		virtual void readDeltaEntity(MSG& msg, const IGameState& gameState, const entityState_t* from, entityState_t* to, entityNum_t newNum) const = 0;
		virtual bool readNonPVSClient(radarInfo_t radarInfo, const ClientSnapshot& currentSnap, float radarRange, radarUnpacked_t& unpacked) const = 0;
		virtual IGameState* createGameState() const = 0;
		virtual void readGameState(MSG& msg, IGameState& gameState) const = 0;
	};

	static ReadOnlyInfo getGameStateSystemInfo(const IGameState& gameState);
	static ReadOnlyInfo getGameStateServerInfo(const IGameState& gameState);
}
}
