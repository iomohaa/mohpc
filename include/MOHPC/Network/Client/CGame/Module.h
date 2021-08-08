#pragma once

#include "../../Configstring.h"
#include "../../Exception.h"
#include "../../pm/bg_public.h"
#include "../../../Utility/HandlerList.h"
#include "../../../Utility/PropertyMap.h"
#include "../../../Utility/Function.h"
#include "../../../Utility/CommandManager.h"
#include "../../../Common/Vector.h"
#include "../../ProtocolSingleton.h"
#include "../../Types/Environment.h"
#include "../../Parsing/Environment.h"
#include "../../Parsing/GameState.h"
#include "../Imports.h"

#include "ClientInfo.h"
#include "Effect.h"
#include "GameplayNotify.h"
#include "Hud.h"
#include "MessageParse.h"
#include "Objective.h"
#include "Prediction.h"
#include "Scoreboard.h"
#include "ServerInfo.h"
#include "Snapshot.h"
#include "Trace.h"
#include "Vote.h"

#include "GameType.h"

#include <type_traits>
#include <cstdint>
#include <chrono>

namespace MOHPC
{
class MSG;
class StringMessage;
class TokenParser;
class ReadOnlyInfo;

namespace Network
{
	class Pmove;

	namespace CGame
	{
	struct EntityInfo;
	struct stats_t;
	class Scoreboard;

	namespace Handlers
	{
		/**
		 * Called when the client received a command.
		 *
		 * @param	command	The command.
		 * @param	parser	Used to parse the command arguments.
		 */
		struct ServerCommand : public HandlerNotifyBase<void(const char* command, TokenParser& parser)> {};

		/**
		 * Called from server for statistics.
		 *
		 * @param	stats	Statistics.
		 */
		struct ServerCommand_Stats : public HandlerNotifyBase<void(const stats_t& stats)> {};

		/**
		 * Called from server to make the client process a console command.
		 *
		 * @param	tokenized	Arguments of stufftext (command to exec on console).
		 * @note	Better not allow the server to exec any command. Should not even process it at all.
		 */
		struct ServerCommand_Stufftext : public HandlerNotifyBase<void(TokenParser& tokenized)> {};
	}

	struct Imports
	{
	public:
		const ServerSnapshotManager& snapshotManager;
		const ClientTime& clientTime;
		const ICommandSequence& commandSequence;
		ServerGameState& gameState;
		const UserInfoPtr& userInfo;
	};

	struct rain_t
	{
	public:
		rain_t();

		MOHPC_NET_EXPORTS float getDensity() const;
		MOHPC_NET_EXPORTS float getSpeed() const;
		MOHPC_NET_EXPORTS uint32_t getSpeedVariation() const;
		MOHPC_NET_EXPORTS uint32_t getSlant() const;
		MOHPC_NET_EXPORTS float getLength() const;
		MOHPC_NET_EXPORTS float getMinimumDistance() const;
		MOHPC_NET_EXPORTS float getWidth() const;
		MOHPC_NET_EXPORTS uint32_t getNumShaders() const;
		MOHPC_NET_EXPORTS const char* getShader(uint8_t index) const;

	public:
		float density;
		float speed;
		float length;
		float minDist;
		float width;
		uint32_t speedVary;
		uint32_t slant;
		uint32_t numShaders;
		str shader[16];
	};

	struct stats_t
	{
		const char* preferredWeapon;
		uint32_t numObjectives;
		uint32_t numComplete;
		uint32_t numShotsFired;
		uint32_t numHits;
		uint32_t accuracy;
		uint32_t numHitsTaken;
		uint32_t numObjectsDestroyed;
		uint32_t numEnemysKilled;
		uint32_t headshots;
		uint32_t torsoShots;
		uint32_t leftLegShots;
		uint32_t rightLegShots;
		uint32_t groinShots;
		uint32_t leftArmShots;
		uint32_t rightArmShots;
		uint32_t gunneryEvaluation;
		uint32_t gotMedal;
		uint32_t success;
		uint32_t failed;
	};

	/**
	 * Base CG module, contains most implementations.
	 */
	class ModuleBase
	{
	public:
		class HandlerList
		{
		public:
			FunctionList<Handlers::Print> printHandler;
			FunctionList<Handlers::HudPrint> hudPrintHandler;
			FunctionList<Handlers::ServerCommand_Scores> scmdScoresHandler;
			FunctionList<Handlers::ServerCommand_Stats> scmdStatsHandler;
			FunctionList<Handlers::ServerCommand_Stopwatch> scmdStopwatchHandler;
			FunctionList<Handlers::ServerCommand_ServerLag> scmdServerLagHandler;
			FunctionList<Handlers::ServerCommand_Stufftext> scmdStufftextHandler;
		};

	public:
		ModuleBase();
		virtual ~ModuleBase();

		void setProtocol(protocolType_c protocol);
		void setImports(const Imports& imports);

		void init(uintptr_t serverMessageSequence, rsequence_t serverCommandSequence);

		/** Tick function for CGame module. */
		void tick(deltaTime_t deltaTime, tickTime_t currentTime, tickTime_t simulatedServerTime);

		/** Return the handler list. */
		MOHPC_NET_EXPORTS HandlerList& handlers();
		MOHPC_NET_EXPORTS const HandlerList& handlers() const;
		
		/** Get the current client time. */
		MOHPC_NET_EXPORTS tickTime_t getTime() const;

		MOHPC_NET_EXPORTS GameplayNotify& getGameplayNotify();
		MOHPC_NET_EXPORTS const GameplayNotify& getGameplayNotify() const;

		/** Return a reference to the snapshot manager/processor. */
		MOHPC_NET_EXPORTS SnapshotProcessor& getSnapshotProcessor();
		MOHPC_NET_EXPORTS const SnapshotProcessor& getSnapshotProcessor() const;

		/** Return a reference to the vote manager. */
		MOHPC_NET_EXPORTS VoteManager& getVoteManager();
		MOHPC_NET_EXPORTS const VoteManager& getVoteManager() const;

		/** Return server rain settings. */
		MOHPC_NET_EXPORTS const rain_t& getRain() const;

		/** Return server environment settings. */
		MOHPC_NET_EXPORTS const environment_t& getEnvironment() const;

		/** Return the server info. */
		MOHPC_NET_EXPORTS const cgsInfo& getServerInfo() const;

		/** Return the objective manager. */
		MOHPC_NET_EXPORTS const ObjectiveManager& getObjectiveManager() const;

		/** Return the list of clients. */
		MOHPC_NET_EXPORTS const ClientInfoList& getClientInfoList() const;

		/** CG message notification. */
		void parseCGMessage(MSG& msg);

		/** Notified when a configString has been modified. */
		void configStringModified(csNum_t num, const char* newString);

	private:
		void parseServerInfo(const char* cs);
	
	private:
		/**
		 * Server commands
		 */
		void SCmd_Print(TokenParser& args);
		void SCmd_HudPrint(TokenParser& args);
		void SCmd_Scores(TokenParser& args);
		void SCmd_Stopwatch(TokenParser& args);
		void SCmd_PrintDeathMsg(TokenParser& args);
		void SCmd_Stats(TokenParser& args);
		void SCmd_ServerLag(TokenParser& args);
		void SCmd_Stufftext(TokenParser& args);

	private:
		tickTime_t svTime;

		const Parsing::IEnvironment* environmentParse;
		const Parsing::IGameState* gameStateParse;
		const IMessageParser* messageParser;

		HandlerList handlerList;
		ServerGameState* gameState;
		fnHandle_t configStringHandler;
		GameplayNotify gameplayNotify;
		cgsInfo cgs;
		environment_t environment;
		rain_t rain;
		VoteManager voteManager;
		ObjectiveManager objectiveManager;
		ClientInfoList clientInfoList;
		SnapshotProcessor processedSnapshots;

		CommandTemplate<ModuleBase, &ModuleBase::SCmd_Print> printHandler;
		CommandTemplate<ModuleBase, &ModuleBase::SCmd_HudPrint> hudPrintHandler;
		CommandTemplate<ModuleBase, &ModuleBase::SCmd_Scores> scoresHandler;
		CommandTemplate<ModuleBase, &ModuleBase::SCmd_Stopwatch> stopwatchHandler;
		CommandTemplate<ModuleBase, &ModuleBase::SCmd_PrintDeathMsg> printDeathMsgHandler;
		CommandTemplate<ModuleBase, &ModuleBase::SCmd_Stats> statsHandler;
		CommandTemplate<ModuleBase, &ModuleBase::SCmd_ServerLag> serverLagHandler;
		CommandTemplate<ModuleBase, &ModuleBase::SCmd_Stufftext> stufftextHandler;

		CommandManager serverCommandManager;
	};

	namespace CGError
	{
		class Base : public NetworkException {};
	}
	}
}
}
