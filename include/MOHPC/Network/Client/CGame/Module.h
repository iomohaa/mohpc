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
#include "Hud.h"
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

#include <morfuse/Container/Container.h>

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
	/**
	 * This is the list of effects.
	 *
	 * To get the full model path, use getEffectName().
	 */
	enum class effects_e
	{
		barrel_oil_leak_big = 0,
		barrel_oil_leak_medium,
		barrel_oil_leak_small,
		barrel_oil_leak_splat,
		barrel_water_leak_big,
		barrel_water_leak_medium,
		barrel_water_leak_small,
		barrel_water_leak_splat,
		bazookaexp_base,
		bazookaexp_dirt,
		bazookaexp_snow,
		bazookaexp_stone,
		bh_carpet_hard,
		bh_carpet_lite,
		bh_dirt_hard,
		bh_dirt_lite,
		bh_foliage_hard,
		bh_foliage_lite,
		bh_glass_hard,
		bh_glass_lite,
		bh_grass_hard,
		bh_grass_lite,
		bh_human_uniform_hard,
		bh_human_uniform_lite,
		bh_metal_hard,
		bh_metal_lite,
		bh_mud_hard,
		bh_mud_lite,
		bh_paper_hard,
		bh_paper_lite,
		bh_sand_hard,
		bh_sand_lite,
		bh_snow_hard,
		bh_snow_lite,
		bh_stone_hard,
		bh_stone_lite,
		bh_water_hard,
		bh_water_lite,
		bh_wood_hard,
		bh_wood_lite,
		fs_dirt,
		fs_grass,
		fs_heavy_dust,
		fs_light_dust,
		fs_mud,
		fs_puddle,
		fs_sand,
		fs_snow,
		fx_fence_wood,
		grenexp_base,
		grenexp_carpet,
		grenexp_dirt,
		grenexp_foliage,
		grenexp_grass,
		grenexp_gravel,
		grenexp_metal,
		grenexp_mud,
		grenexp_paper,
		grenexp_sand,
		grenexp_snow,
		grenexp_stone,
		grenexp_water,
		grenexp_wood,
		heavyshellexp_base,
		heavyshellexp_dirt,
		heavyshellexp_snow,
		heavyshellexp_stone,
		tankexp_base,
		tankexp_dirt,
		tankexp_snow,
		tankexp_stone,
		water_ripple_moving,
		water_ripple_still,
		water_trail_bubble,
		max
	};

	/**
	 * Return the model path from an effect.
	 *
	 * @param effect The client game module effect.
	 * @return the model path, i.e "/models/fx/grenexp_mud.tik".
	 */
	MOHPC_NET_EXPORTS const char* getEffectName(effects_e effect);

	struct EntityInfo;
	struct stats_t;
	class Scoreboard;

	namespace Handlers
	{
		//=== FX functions

		/**
		 * Callback for creating bullet tracers.
		 *
		 * @param	barrel			tag_barrel where the bullet has been emitted from.
		 * @param	start			Start of the bullet tracer.
		 * @param	end				Where the bullet tracer should end.
		 * @param	numBullets		Number of bullets that have been fired at once (could be fired from a shotgun).
		 * @param	iLarge			Whether or not it's a large bullet.
		 * @param	tracerVisible	Specify if this tracer is visible.
		 * @param	bulletSize		The length of the bullet.
		 */
		struct MakeBulletTracer : public HandlerNotifyBase<void(const Vector& barrel, const Vector& start, const Vector& end, uint32_t numBullets, uint32_t iLarge, uint32_t numTracersVisible, float bulletSize)> {};

		/**
		 * Underwater bubble trail.
		 *
		 * @param	start		Trail's start.
		 * @param	end			Trail's end.
		 * @param	iLarge		If it's a large trail.
		 * @param	bulletSize	Length of the trail.
		 */
		struct MakeBubbleTrail : public HandlerNotifyBase<void(const Vector& start, const Vector& end, uint32_t iLarge, float bulletSize)> {};

		/**
		 * Called on impact.
		 *
		 * @param	origin	Position for melee impact.
		 * @param	normal	Direction of the impact.
		 * @param	large	If it's a large impact.
		 */
		struct Impact : public HandlerNotifyBase<void(const Vector& origin, const Vector& normal, uint32_t large)> {};

		/**
		 * Called on melee impact.
		 *
		 * @param	start	Start position for melee impact.
		 * @param	end		End position for melee impact.
		 */
		struct MeleeImpact : public HandlerNotifyBase<void(const Vector& start, const Vector& end)> {};

		/**
		 * Called when an explosion occurs.
		 *
		 * @param	origin	Location of the explosion.
		 * @param	type	Explosion type.
		 */
		struct MakeExplosionEffect : public HandlerNotifyBase<void(const Vector& origin, const char* modelName)> {};

		/**
		 * Called to spawn an effect.
		 *
		 * @param	origin	Location of the effect.
		 * @param	normal	Explosion's normal direction.
		 * @param	type	Type of the effect.
		 */
		struct MakeEffect : public HandlerNotifyBase<void(const Vector& origin, const Vector& normal, const char* modelName)> {};

		enum class debrisType_e : unsigned char { crate, window };
		/**
		 * Spawn a debris of the specified type.
		 *
		 * @param	debrisType	Type of the debris.
		 * @param	origin		Location where to spawn the debris.
		 * @param	numDebris	Number of debris to spawn.
		 */
		struct SpawnDebris : public HandlerNotifyBase<void(debrisType_e debrisType, const Vector& origin, uint32_t numDebris)> {};

		//=== Deathmatch functions
		/** Called to notify the client of an enemy hit in DM game. */
		struct HitNotify : public HandlerNotifyBase<void()> {};

		/** Called to notify the client of an enemy kill in DM game. */
		struct KillNotify : public HandlerNotifyBase<void()> {};

		/**
		 * Called to play a DM voice message.
		 *
		 * @param	origin		Origin of the message
		 * @param	local		True to play the sound locally. False if it should be spatialized.
		 * @param	clientNum	Player responsible for the voice.
		 * @param	soundName	Sound to play.
		 */
		struct VoiceMessage : public HandlerNotifyBase<void(const Vector& origin, bool local, uint8_t clientNum, const char* soundName)> {};

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
		const UserInput& userInput;
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
			FunctionList<Handlers::MakeBulletTracer> makeBulletTracerHandler;
			FunctionList<Handlers::MakeBubbleTrail> makeBubbleTrailHandler;
			FunctionList<Handlers::Impact> impactHandler;
			FunctionList<Handlers::MeleeImpact> meleeImpactHandler;
			FunctionList<Handlers::MakeExplosionEffect> makeExplosionEffectHandler;
			FunctionList<Handlers::MakeEffect> makeEffectHandler;
			FunctionList<Handlers::SpawnDebris> spawnDebrisHandler;
			FunctionList<Handlers::HudDraw_Shader> huddrawShaderHandler;
			FunctionList<Handlers::HudDraw_Align> huddrawAlignHandler;
			FunctionList<Handlers::HudDraw_Rect> huddrawRectHandler;
			FunctionList<Handlers::HudDraw_VirtualScreen> huddrawVSHandler;
			FunctionList<Handlers::HudDraw_Color> huddrawColorHandler;
			FunctionList<Handlers::HudDraw_Alpha> huddrawAlphaHandler;
			FunctionList<Handlers::HudDraw_String> huddrawStringHandler;
			FunctionList<Handlers::HudDraw_Font> huddrawFontHandler;
			FunctionList<Handlers::HitNotify> hitNotifyHandler;
			FunctionList<Handlers::KillNotify> killNotifyHandler;
			FunctionList<Handlers::VoiceMessage> voiceMessageHandler;
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
		void tick(uint64_t deltaTime, uint64_t currentTime, uint64_t serverTime);

		/** Return the handler list. */
		MOHPC_NET_EXPORTS HandlerList& handlers();
		MOHPC_NET_EXPORTS const HandlerList& handlers() const;
		
		/** Get the current client time. */
		MOHPC_NET_EXPORTS uint64_t getTime() const;

		MOHPC_NET_EXPORTS Prediction& getPrediction();
		MOHPC_NET_EXPORTS const Prediction& getPrediction() const;

		/** Return a reference to the snapshot manager/processor. */
		MOHPC_NET_EXPORTS SnapshotProcessor& getSnapshotProcessor();
		MOHPC_NET_EXPORTS const SnapshotProcessor& getSnapshotProcessor() const;

		/** Return a reference to the vote manager. */
		MOHPC_NET_EXPORTS VoteManager& getVoteManager();
		MOHPC_NET_EXPORTS const VoteManager& getVoteManager() const;

		/** Return a reference to the class that manages trace and collision. */
		MOHPC_NET_EXPORTS const TraceManager& getTraceManager() const;

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

		/** Used to parse CG messages between different versions. */
		virtual void handleCGMessage(MSG& msg, uint8_t msgType) = 0;

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
		uint64_t svTime;
		HandlerList handlerList;
		ServerGameState* gameState;
		fnHandle_t configStringHandler;
		Prediction prediction;
		const Parsing::IEnvironment* environmentParse;
		const Parsing::IGameState* gameStateParse;
		cgsInfo cgs;
		environment_t environment;
		rain_t rain;
		VoteManager voteManager;
		ObjectiveManager objectiveManager;
		ClientInfoList clientInfoList;
		SnapshotProcessor processedSnapshots;
		TraceManager traceManager;
		CommandManager serverCommandManager;
	};

	/**
	 * CG Module for protocol version 6.
	 * => MOH:AA ver 1.00.
	 */
	class CGameModule6 : public ModuleBase
	{
	public:
		CGameModule6();

	protected:
		void handleCGMessage(MSG& msg, uint8_t msgType) override;

	private:
		effects_e getEffectId(uint32_t effectId);
	};

	/**
	 * CG Module for protocol version 15.
	 * => MOH ver 2.00 and above (since SH).
	 */
	class CGameModule15 : public ModuleBase
	{
	public:
		CGameModule15();

	protected:
		void handleCGMessage(MSG& msg, uint8_t msgType) override;

	private:
		effects_e getEffectId(uint32_t effectId);
	};
	
	class CommonMessageHandler
	{
	public:
		CommonMessageHandler(MSG& inMsg, const ModuleBase::HandlerList& inHandlerList);

		void impactMelee();
		void debrisCrate();
		void debrisWindow();
		void huddrawShader();
		void huddrawAlign();
		void huddrawRect();
		void huddrawVirtualScreen();
		void huddrawColor();
		void huddrawAlpha();
		void huddrawString();
		void huddrawFont();
		void notifyHit();
		void notifyKill();
		void playSoundEntity();

	private:
		MSG& msg;
		const ModuleBase::HandlerList& handlerList;
	};

	class ModuleInstancier : public IProtocolClassInstancier<ModuleBase>
	{
	public:
		ModuleBase* createInstance() const override = 0;
	};

	namespace CGError
	{
		class Base : public NetworkException {};
	}
	}
}
}
