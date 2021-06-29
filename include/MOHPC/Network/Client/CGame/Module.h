#pragma once

#include <cstdint>
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

#include "Objective.h"
#include "Prediction.h"
#include "Scoreboard.h"
#include "Snapshot.h"
#include "Trace.h"
#include "Vote.h"

#include "GameType.h"

#include <type_traits>

#include <morfuse/Container/Container.h>

namespace MOHPC
{
class MSG;
class StringMessage;
class TokenParser;
class ReadOnlyInfo;

namespace Network
{
	class entityState_t;
	class ServerConnection;
	struct gameState_t;
	class Pmove;

	namespace CGame
	{
	namespace DMFlags
	{
		/**
		 * Values for dmflags
		 */
		/** Players don't drop health on death. */
		static constexpr unsigned int DF_NO_HEALTH				= (1 << 0);
		/** Players don't drop powerups on death. */
		static constexpr unsigned int DF_NO_POWERUPS			= (1 << 1);
		/** Whether or not weapons in the level stays available on player pick up. */
		static constexpr unsigned int DF_WEAPONS_STAY			= (1 << 2);
		/** Prevent falling damage. */
		static constexpr unsigned int DF_NO_FALLING				= (1 << 3);
		/** This flag doesn't seem to be used at all. */
		static constexpr unsigned int DF_INSTANT_ITEMS			= (1 << 4);
		/** TriggerChangeLevel won't switch level. */
		static constexpr unsigned int DF_SAME_LEVEL				= (1 << 5);
		/** Prevent players from having an armor. */
		static constexpr unsigned int DF_NO_ARMOR				= (1 << 11);
		/** MOH:AA: Infinite clip ammo. MOH:SH/MOH:BT: Infinite magazines. */
		static constexpr unsigned int DF_INFINITE_AMMO			= (1 << 14);
		/** This should prevent footstep sounds to play. */
		static constexpr unsigned int DF_NO_FOOTSTEPS			= (1 << 17);

		/**
		 * protocol version >= 15
		 * (SH)
		 */

		/** Allow leaning while in movement. */
		static constexpr unsigned int DF_ALLOW_LEAN				= (1 << 18);
		/** Specify that G43 is replaced with Kar98. */
		static constexpr unsigned int DF_OLD_SNIPERRIFLE		= (1 << 19);

		/**
		 * protocol version >= 17
		 * (BT)
		 */

		/** Axis use a shotgun rather than kar98 mortar. */
		static constexpr unsigned int DF_GERMAN_SHOTGUN			= (1 << 20);
		/** Allow landmine to be used on AA maps. */
		static constexpr unsigned int DF_ALLOW_OLDMAP_MINES		= (1 << 21);

		/**
		 * [BT]
		 * Weapon type filtering
		 */
		/** Disallow the usage of rifles. */
		static constexpr unsigned int DF_BAN_WEAP_RIFLE			= (1 << 22);
		/** Disallow the usage of rifles. */
		static constexpr unsigned int DF_BAN_WEAP_SNIPER		= (1 << 23);
		/** Disallow the usage of snipers. */
		static constexpr unsigned int DF_BAN_WEAP_SMG			= (1 << 24);
		/** Disallow the usage of sub-machine guns. */
		static constexpr unsigned int DF_BAN_WEAP_MG			= (1 << 25);
		/** Disallow the usage of machine guns. */
		static constexpr unsigned int DF_BAN_WEAP_HEAVY			= (1 << 26);
		/** Disallow the usage of shotgun. */
		static constexpr unsigned int DF_BAN_WEAP_SHOTGUN		= (1 << 27);
		/** Disallow the usage of landmine. */
		static constexpr unsigned int DF_BAN_WEAP_LANDMINE		= (1 << 28);
	}

	static constexpr size_t NUM_TEAM_OBJECTIVES = 5;

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

	enum class hudMessage_e : unsigned char
	{
		Yellow = 1,
		/** White message, shown top center with death messages. */
		ChatWhite,
		/** White message, shown top left, below compass. */
		White,
		/** Red message, shown top center with death messages. */
		ChatRed,
		/** (protocol version 15) Green message, shown top center with death messages. */
		ChatGreen,
		/** Maximum number of valid message type that can be sent by the server. */
		Max,
		/** Message printed in console instead of the HUD. */
		Console,
	};

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

		//=== Hud drawing functions

		/** HUD Alignment. */
		enum class horizontalAlign_e : unsigned char { left, center, right };
		enum class verticalAlign_e : unsigned char { top, center, bottom };

		/**
		 * Set the shader to HUD index.
		 *
		 * @param	index	HUD index.
		 * @param	name	HUD name.
		 */
		struct HudDraw_Shader : public HandlerNotifyBase<void(uint8_t index, const char* name)> {};
		/**
		 * Set the alignment for HUD.
		 *
		 * @param	index				HUD index.
		 * @param	horizontalAlign		Horizontal alignment on screen.
		 * @param	verticalAlign		Vertical alignment on screen.
		 */
		struct HudDraw_Align : public HandlerNotifyBase<void(uint8_t index, horizontalAlign_e horizontalAlign, verticalAlign_e verticalAlign)> {};

		/**
		 * Set the HUD rect.
		 *
		 * @param	index	HUD index.
		 * @param	x		X position on screen.
		 * @param	y		Y position on screen.
		 * @param	width	Width of the element.
		 * @param	height	Height of the element.
		 */
		struct HudDraw_Rect : public HandlerNotifyBase<void(uint8_t index, uint16_t x, uint16_t y, uint16_t width, uint16_t height)> {};

		/**
		 * Specify if the HUD is virtual screen (rect relative to 640x480).
		 *
		 * @param	index			HUD index.
		 * @param	virtualScreen	True if the HUD is virtual screen.
		 */
		struct HudDraw_VirtualScreen : public HandlerNotifyBase<void(uint8_t index, bool virtualScreen)> {};

		/**
		 * Specify the color of the HUD.
		 *
		 * @param	index	HUD index.
		 * @param	color	[[0...1]...] RGB color of the HUD.
		 */
		struct HudDraw_Color : public HandlerNotifyBase<void(uint8_t index, const Vector& color)> {};

		/**
		 * Specify the alpha of the HUD.
		 *
		 * @param	index	HUD index.
		 * @arap	alpha	[0...1] Alpha.
		 */
		struct HudDraw_Alpha : public HandlerNotifyBase<void(uint8_t index, float alpha)> {};

		/**
		 * Set the string of the HUD element.
		 *
		 * @param	index	HUD index.
		 * @param	string	string value.
		 */
		struct HudDraw_String : public HandlerNotifyBase<void(uint8_t index, const char* string)> {};

		/**
		 * Set the font to use for the HUD (only if it has a string).
		 *
		 * @param	index		HUD index.
		 * @param	fontName	Name of the font.
		 */
		struct HudDraw_Font : public HandlerNotifyBase<void(uint8_t index, const char* fontName)> {};

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
		 * Called to print a message on console.
		 *
		 * @param	type	Type of the message (see hudMessage_e).
		 * @param	text	Text to print.
		 */
		struct Print : public HandlerNotifyBase<void(hudMessage_e type, const char* text)> {};

		/**
		 * Called to print a message that is displayed on HUD, yellow color.
		 *
		 * @param	text	Text to print.
		 */
		struct HudPrint : public HandlerNotifyBase<void(const char* text)> {};

		/**
		 * Called when the client received a command.
		 *
		 * @param	command	The command.
		 * @param	parser	Used to parse the command arguments.
		 */
		struct ServerCommand : public HandlerNotifyBase<void(const char* command, TokenParser& parser)> {};

		/**
		 * Called from server after score has been parsed.
		 *
		 * @param	scores	Score data.
		 */
		struct ServerCommand_Scores : public HandlerNotifyBase<void(const Scoreboard& scores)> {};

		/**
		 * Called from server for statistics.
		 *
		 * @param	stats	Statistics.
		 */
		struct ServerCommand_Stats : public HandlerNotifyBase<void(const stats_t& stats)> {};

		/**
		 * Called to display a stopwatch.
		 *
		 * @param	startTime	Start time of the stopwatch.
		 * @param	endTime		End time of the stopwatch.
		 */
		struct ServerCommand_Stopwatch : public HandlerNotifyBase<void(uint64_t startTime, uint64_t endTime)> {};

		/**
		 * The server notify clients when it experiences lags, such as hitches.
		 */
		struct ServerCommand_ServerLag : public HandlerNotifyBase<void()> {};

		/**
		 * Called from server to make the client process a console command.
		 *
		 * @param	tokenized	Arguments of stufftext (command to exec on console).
		 * @note	Better not allow the server to exec any command. Should not even process it at all.
		 */
		struct ServerCommand_Stufftext : public HandlerNotifyBase<void(TokenParser& tokenized)> {};
	}

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

	struct clientInfo_t
	{
	public:
		str name;
		teamType_e team;
		PropertyObject properties;

	public:
		clientInfo_t();

		/** Name of the client. */
		MOHPC_NET_EXPORTS const char* getName() const;

		/** Client's current team. */
		MOHPC_NET_EXPORTS teamType_e getTeam() const;

		/** List of misc client properties. */
		MOHPC_NET_EXPORTS const PropertyObject& getProperties() const;
	};

	/**
	 * Parsed server info data.
	 */
	class cgsInfo
	{
	public:
		cgsInfo();

		/** Return the server time at which the match has started. */
		MOHPC_NET_EXPORTS uint64_t getMatchStartTime() const;

		/** Return the server time at which the match has ended. */
		MOHPC_NET_EXPORTS uint64_t getMatchEndTime() const;

		/** Return the server time at which the level has started. */
		MOHPC_NET_EXPORTS uint64_t getLevelStartTime() const;

		/** Get the last lag time. */
		MOHPC_NET_EXPORTS uint64_t getServerLagTime() const;

		/** Return the current game type. */
		MOHPC_NET_EXPORTS gameType_e getGameType() const;

		/** Return current DF_ flags. */
		MOHPC_NET_EXPORTS uint32_t getDeathmatchFlags() const;

		/** 
		 * Return true if dmflags contain one or more of the specified flags.
		 *
		 * @param	flags	Flags to look for.
		 * @return	true	if one of the following flags are valid.
		 */
		MOHPC_NET_EXPORTS bool hasAnyDMFlags(uint32_t flags) const;

		/**
		 * Return true if dmflags contain the specified flags.
		 *
		 * @param	flags	Flags to look for.
		 * @return	true	if all of the flags are valid.
		 */
		MOHPC_NET_EXPORTS bool hasAllDMFlags(uint32_t flags) const;

		/**
		 * User version of hasAnyDMFlags that split each flags by arguments.
		 * Checks if any of the specified flags matches.
		 */
		template<typename...Args>
		bool hasAnyDMFlagsArgs(Args... args)
		{
			const uint32_t flags = (args | ...);
			return hasAnyDMFlags(flags);
		}

		/**
		 * User version of hasAnyDMFlags that split each flags by arguments.
		 * Checks if all of the specified flags matches.
		 */
		template<typename...Args>
		bool hasAllDMFlagsArgs(Args... args)
		{
			const uint32_t flags = (args | ...);
			return hasAllDMFlags(flags);
		}

		/** Return teamFlags (doesn't seem to be used). */
		MOHPC_NET_EXPORTS uint32_t getTeamFlags() const;

		/** Return the number of maximum clients that can be connected in the current server. */
		MOHPC_NET_EXPORTS uint32_t getMaxClients() const;

		/** Return the frag limit before the match ends. */
		MOHPC_NET_EXPORTS int32_t getFragLimit() const;

		/** Returns the time limit before the match ends. */
		MOHPC_NET_EXPORTS int32_t getTimeLimit() const;

		/** Return the server type the client is connected on. */
		MOHPC_NET_EXPORTS serverType_e getServerType() const;

		/** Return whether or not voting is allowed. */
		MOHPC_NET_EXPORTS bool isVotingAllowed() const;

		/** Return the map checksum on the server. Useful to compare against the checksum of the map on the client. */
		MOHPC_NET_EXPORTS uint32_t getMapChecksum() const;

		/** Return the current map name. */
		MOHPC_NET_EXPORTS const char* getMapName() const;
		MOHPC_NET_EXPORTS const str& getMapNameStr() const;

		/** Return the path to the map name. */
		MOHPC_NET_EXPORTS const char* getMapFilename() const;
		MOHPC_NET_EXPORTS const str& getMapFilenameStr() const;

		/** Return the allied text NUM_TEAM_OBJECTIVES is the max. */
		MOHPC_NET_EXPORTS const char* getAlliedText(size_t index) const;

		/** Return the axis text. NUM_TEAM_OBJECTIVES is the max. */
		MOHPC_NET_EXPORTS const char* getAxisText(size_t index);

		/** Return the scoreboard pic shader. */
		MOHPC_NET_EXPORTS const char* getScoreboardPic() const;

		/** Return the scoreboard pic shader when game is over. */
		MOHPC_NET_EXPORTS const char* getScoreboardPicOver() const;

	public:
		uint64_t matchStartTime;
		uint64_t matchEndTme;
		uint64_t levelStartTime;
		uint64_t serverLagTime;
		str mapName;
		str mapFilename;
		str alliedText[NUM_TEAM_OBJECTIVES];
		str axisText[NUM_TEAM_OBJECTIVES];
		str scoreboardPic;
		str scoreboardPicOver;
		uint32_t dmFlags;
		uint32_t teamFlags;
		uint32_t maxClients;
		uint32_t mapChecksum;
		int32_t fragLimit;
		int32_t timeLimit;
		serverType_e serverType;
		gameType_e gameType;
		bool allowVote;
	};

	class ClientInfoList
	{
	public:
		/** Get a client info in the interval of [0, MAX_CLIENTS]. */
		MOHPC_NET_EXPORTS const clientInfo_t& get(uint32_t clientNum) const;
		const clientInfo_t& set(const ReadOnlyInfo& info, uint32_t clientNum);

	private:
		clientInfo_t clientInfo[MAX_CLIENTS];
	};

	struct Imports
	{
	public:
		const ServerSnapshotManager& snapshotManager;
		const ClientTime& clientTime;
		const UserInput& userInput;
		const ICommandSequence& commandSequence;
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
		virtual ~ModuleBase() = default;

		void setProtocol(protocolType_c protocol);
		void setImports(const Imports& imports);

		void init(uintptr_t serverMessageSequence, rsequence_t serverCommandSequence);

		/** Tick function for CGame module. */
		void tick(uint64_t deltaTime, uint64_t currentTime, uint64_t serverTime);

		/** Return the handler list. */
		MOHPC_NET_EXPORTS HandlerList& handlers();
		MOHPC_NET_EXPORTS const HandlerList& handlers() const;

		/** Return the alpha interpolation between the current frame and the next frame. */
		MOHPC_NET_EXPORTS float getFrameInterpolation() const;
		
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

		/** Get imports list. */
		const ClientImports& getImports() const;

		virtual Pmove& getMove() = 0;

		/** Setup move between versions. */
		virtual void setupMove(Pmove& pmove) = 0;

		/** Used to parse CG messages between different versions. */
		virtual void handleCGMessage(MSG& msg, uint8_t msgType) = 0;

	private:
		void parseServerInfo(const char* cs);
		void conditionalReflectClient(const clientInfo_t& client);
	
	private:
		/**
		 * Server commands
		 */
		void SCmd_Print(TokenParser& args);
		void SCmd_HudPrint(TokenParser& args);
		void SCmd_Scores(TokenParser& args);
		void SCmd_Stats(TokenParser& args);
		void SCmd_Stopwatch(TokenParser& args);
		void SCmd_ServerLag(TokenParser& args);
		void SCmd_Stufftext(TokenParser& args);
		void SCmd_PrintDeathMsg(TokenParser& args);

	protected:
		ClientImports imports;

	private:
		uint64_t svTime;
		HandlerList handlerList;
		uint32_t physicsTime;
		float frameInterpolation;
		float cameraFov;
		Vector predictedError;
		Vector cameraAngles;
		Vector cameraOrigin;
		playerState_t predictedPlayerState;
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
		Pmove& getMove() override;
		void setupMove(Pmove& pmove) override;

	private:
		effects_e getEffectId(uint32_t effectId);

	private:
		Pmove_ver6 pmove;
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
		Pmove& getMove() override;
		void setupMove(Pmove& pmove) override;

	private:
		effects_e getEffectId(uint32_t effectId);

	private:
		Pmove_ver15 pmove;
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
