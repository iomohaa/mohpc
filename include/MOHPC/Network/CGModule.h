#pragma once

#include <stdint.h>
#include "../Utilities/HandlerList.h"
#include "../Utilities/PropertyMap.h"
#include "../Utilities/Function.h"
#include "../Vector.h"
#include "Configstring.h"
#include "InfoTypes.h"
#include "pm/bg_public.h"
#include <type_traits>

namespace MOHPC
{
	class MSG;
	class StringMessage;
	class TokenParser;

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
		 * New flags since SH
		 */
		/** Allow leaning while in movement. */
		static constexpr unsigned int DF_ALLOW_LEAN				= (1 << 18);
		/** Specify that G43 is replaced with Kar98. */
		static constexpr unsigned int DF_OLD_SNIPERRIFLE		= (1 << 19);

		/**
		 * New flags since BT
		 */
		/** Axis use a shotgun rather than kar98 mortar. */
		static constexpr unsigned int DF_GERMAN_SHOTGUN			= (1 << 20);
		/** Allow landmine to be used on AA maps. */
		static constexpr unsigned int DF_ALLOW_OLDMAP_MINES		= (1 << 21);

		/**
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

	class Pmove;

	namespace Network
	{
		class ClientGameConnection;
		struct gameState_t;

		static constexpr size_t MAX_ACTIVE_SNAPSHOTS = 2;

		// Valid effects
		enum class effects_e : unsigned char
		{
			barrel_oil_leak_big,
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

		MOHPC_EXPORTS const char* getEffectName(effects_e effect);

		enum class gameType_e : unsigned char
		{
			SinglePlayer,
			FreeForAll,
			TeamDeathmatch,
			RoundBasedMatch,
			Objective
			// FIXME: add others
		};

		enum class hudMessage_e : unsigned char
		{
			Yellow = 1,
			ChatWhite,
			White,
			ChatRed
		};

		struct EntityInfo;
		struct stats_t;
		class Scoreboard;

		namespace CGameHandlers
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
			 * @param	type	(FIXME) Explosion type.
			 */
			struct MakeExplosionEffect : public HandlerNotifyBase<void(const Vector& origin, effects_e type)> {};

			/**
			 * Called to spawn an effect.
			 *
			 * @param	origin	Location of the effect.
			 * @param	normal	Explosion's normal direction.
			 * @param	type	(FIXME) Type of the effect.
			 */
			struct MakeEffect : public HandlerNotifyBase<void(const Vector& origin, const Vector& normal, effects_e type)> {};

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
			 * @param	string	String value.
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
			 * Called when an entity has just been added/spawned.
			 *
			 * @param	entity	The entity that was added.
			 */
			struct EntityAdded : public HandlerNotifyBase<void(const EntityInfo& entity)> {};

			/**
			 * Called when an entity has just been removed.
			 *
			 * @param	entity	The entity that was removed.
			 */
			struct EntityRemoved : public HandlerNotifyBase<void(const EntityInfo& entity)> {};

			/**
			 * Called when an entity has just been modified.
			 *
			 * @param	entity	The entity that was modified.
			 */
			struct EntityModified : public HandlerNotifyBase<void(const EntityInfo& entity)> {};

			/**
			 * Called each frame for replaying move that have not been executed yet on server.
			 *
			 * @param	ucmd	Input to replay.
			 * @param	ps		Player state where to apply movement.
			 * @param	msec	delta time between last cmd time and playerState command time. Usually client's frametime.
			 */
			struct ReplayMove : public HandlerNotifyBase<void(const usercmd_t& ucmd, playerState_t& ps, uint32_t msec)> {};

			/**
			 * Called to print a message on console.
			 *
			 * @param	type	Type of the message (see hudMessage_e).
			 * @param	text	Text to print.
			 */
			struct ServerCommand_Print : public HandlerNotifyBase<void(hudMessage_e type, const char* text)> {};

			/**
			 * Called to print a message that is displayed on HUD.
			 *
			 * @param	type	Type of the message (see hudMessage_e).
			 * @param	text	Text to print.
			 */
			struct ServerCommand_HudPrint : public HandlerNotifyBase<void(hudMessage_e type, const char* text)> {};

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

		struct EntityInfo
		{
		public:
			entityState_t currentState;
			entityState_t nextState;
			uint32_t snapshotTime;
			bool currentValid : 1;
			bool interpolate : 1;
			bool teleported : 1;
			bool notified : 1;

		public:
			EntityInfo();
		};

		struct rain_t
		{
			float density;
			float speed;
			float length;
			float minDist;
			float width;
			uint32_t speedVary;
			uint32_t slant;
			uint32_t numShaders;
			str shader[16];

		public:
			rain_t();
		};

		struct objective_t
		{
		public:
			// List of valid flags

			/** The objective shouldn't be drawn. */
			static constexpr unsigned int FLAGS_DONTDRAW = (1 << 0);

			/** The objective is currently in progress. */
			static constexpr unsigned int FLAGS_INPROGRESS = (1 << 1);

			/** The objective had been completed. */
			static constexpr unsigned int FLAGS_COMPLETED = (1 << 2);

		public:
			uint32_t flags;
			str text;
			Vector location;

		public:
			objective_t();

		public:
			/** Objective flags. See above. */
			MOHPC_EXPORTS uint32_t getFlags() const;

			/** Objective text. */
			MOHPC_EXPORTS const char* getText() const;

			/** Objective location. */
			MOHPC_EXPORTS const Vector& getLocation() const;
		};

		struct environment_t
		{
		public:
			Vector farplaneColor;
			Vector farplaneColorOverride;
			float farplaneDistance;
			float skyAlpha;
			float skyboxFarplane;
			float skyboxSpeed;
			float farplaneBias;
			float farclipOverride;
			bool farplaneCull : 1;
			bool skyPortal : 1;
			bool renderTerrain : 1;

		public:
			environment_t();

			/** If the farplane culls distant objects. */
			MOHPC_EXPORTS bool isFarplaneCulling() const;

			/** The max distance of the fog. */
			MOHPC_EXPORTS float getFarplane() const;

			/** Fog color. */
			MOHPC_EXPORTS const Vector& getFarplaneColor() const;

			/** SH/BT: Fog bias. */
			MOHPC_EXPORTS float getFarplaneBias() const;

			/** SH/BT: Farplane in the skybox. */
			MOHPC_EXPORTS float getSkyboxFarplane() const;

			/** SH/BT: Skybox movement speed. */
			MOHPC_EXPORTS float getSkyboxSpeed() const;

			/** SH/BT: Farclip override. */
			MOHPC_EXPORTS float getFarclipOverride() const;

			/** SH/BT: Colors for temporarily overriding fog. */
			MOHPC_EXPORTS const Vector& getFarplaneColorOverride() const;

			/** SH/BT: True if terrain should be rendered. */
			MOHPC_EXPORTS bool shouldRenderTerrain() const;

			/** Sky alpha. */
			MOHPC_EXPORTS float getSkyAlpha() const;

			/** True if this is a sky portal. */
			MOHPC_EXPORTS bool isSkyPortal() const;
		};

		enum class teamType_e : unsigned char {
			None,
			Spectator,
			FreeForAll,
			Allies,
			Axis
		};

		class Scoreboard
		{
		public:
			static constexpr size_t MAX_ENTRIES = 256;

			struct teamEntry_t
			{
				uint32_t teamNum;
				uint32_t numKills;
				uint32_t numDeaths;
				uint32_t ping;

			public:
				/** Team number. See teamType_e. */
				MOHPC_EXPORTS uint32_t getTeamNum() const;

				/** The number of kills (or score) the team has made. */
				MOHPC_EXPORTS uint32_t getNumKills() const;

				/** The number of deaths from this team. */
				MOHPC_EXPORTS uint32_t getNumDeaths() const;

				/** Team's ping (usually the average ping of players in team). */
				MOHPC_EXPORTS uint32_t getPing() const;
			};

			struct player_t
			{
				uint64_t timeStamp;
				uint32_t clientNum;
				uint32_t teamNum;
				uint32_t numKills;
				uint32_t numDeaths;
				uint32_t ping;
				bool alive;

			public:
				/** The client number. */
				MOHPC_EXPORTS uint32_t getClientNum() const;

				/** Team number. See teamType_e. */
				MOHPC_EXPORTS uint32_t getTeamNum() const;

				/** The number of kills made by the player. */
				MOHPC_EXPORTS uint32_t getNumKills() const;

				/** The number of time the player died. */
				MOHPC_EXPORTS uint32_t getNumDeaths() const;

				/** Time in milliseconds since the player is in the match. */
				MOHPC_EXPORTS uint64_t getTimeStamp() const;

				/** Player's ping. */
				MOHPC_EXPORTS uint32_t getPing() const;

				/** Whether or not the player is currently alive. */
				MOHPC_EXPORTS bool isAlive() const;
			};

		private:
			Container<teamEntry_t> teamEntries;
			Container<player_t> playerList;

		private:
			gameType_e gameType;

		public:
			Scoreboard(gameType_e inGameType);

			/** Return the number of team entries. */
			MOHPC_EXPORTS size_t getNumTeams() const;

			/** Return the team at the specified index. */
			MOHPC_EXPORTS const teamEntry_t& getTeam(size_t index);

			/** Return the team by the specified type. */
			MOHPC_EXPORTS const teamEntry_t* getTeamByType(teamType_e type);

			/** Return the number of players. */
			MOHPC_EXPORTS size_t getNumPlayers() const;

			/** Return player at the specified index. */
			MOHPC_EXPORTS const player_t& getPlayer(size_t index) const;

		// Parsing functions
		public:
			void parse(TokenParser& tokenizer);
			void parseTeamInfo(TokenParser& tokenizer);
			void parseTeamPlayer(uint32_t clientNum, TokenParser& tokenizer);
			void parseTeamEmpty(TokenParser& tokenizer);
			void parsePlayer(uint32_t clientNum, TokenParser& tokenizer);
			void parseEmpty(TokenParser& tokenizer);
			uint64_t parseTime(const char* timeStr);
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
			MOHPC_EXPORTS const char* getName() const;

			/** Client's current team. */
			MOHPC_EXPORTS teamType_e getTeam() const;

			/** List of misc client properties. */
			MOHPC_EXPORTS const PropertyObject& getProperties() const;
		};

		/**
		 * Parsed server info data.
		 */
		class cgsInfo
		{
		public:
			uint64_t matchStartTime;
			uint64_t matchEndTme;
			uint64_t levelStartTime;
			uint64_t serverLagTime;
			uint64_t voteTime;
			gameType_e gameType;
			uint32_t dmFlags;
			uint32_t teamFlags;
			uint32_t maxClients;
			int32_t fragLimit;
			int32_t timeLimit;
			uint32_t numVotesYes;
			uint32_t numVotesNo;
			uint32_t numUndecidedVotes;
			str mapName;
			str mapFilename;
			str alliedText[3];
			str axisText[3];
			str scoreboardPic;
			str scoreboardPicOver;
			str voteString;

		public:
			cgsInfo();

			/** Return the server time at which the match has started. */
			MOHPC_EXPORTS uint64_t getMatchStartTime() const;

			/** Return the server time at which the match has ended. */
			MOHPC_EXPORTS uint64_t getMatchEndTime() const;

			/** Return the server time at which the level has started. */
			MOHPC_EXPORTS uint64_t getLevelStartTime() const;

			/** Get the last lag time. */
			MOHPC_EXPORTS uint64_t getServerLagTime() const;

			/** Return the current game type. */
			MOHPC_EXPORTS gameType_e getGameType() const;

			/** Return current DF_ flags. */
			MOHPC_EXPORTS uint32_t getDeathmatchFlags() const;

			/** 
			 * Return true if dmflags contain one or more of the specified flags.
			 *
			 * @param	flags	Flags to look for.
			 * @return	true	if one of the following flags are valid.
			 */
			MOHPC_EXPORTS bool hasAnyDMFlags(uint32_t flags) const;

			/**
			 * Return true if dmflags contain the specified flags.
			 *
			 * @param	flags	Flags to look for.
			 * @return	true	if all of the flags are valid.
			 */
			MOHPC_EXPORTS bool hasAllDMFlags(uint32_t flags) const;

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
			MOHPC_EXPORTS uint32_t getTeamFlags() const;

			/** Return the number of maximum clients that can be connected in the current server. */
			MOHPC_EXPORTS uint32_t getMaxClients() const;

			/** Return the frag limit before the match ends. */
			MOHPC_EXPORTS int32_t getFragLimit() const;

			/** Returns the time limit before the match ends. */
			MOHPC_EXPORTS int32_t getTimeLimit() const;

			/** Return the current map name. */
			MOHPC_EXPORTS const char* getMapName() const;

			/** Return the path to the map name. */
			MOHPC_EXPORTS const char* getMapFilename() const;

			/** Return the allied text in the range of [0..2]*/
			MOHPC_EXPORTS const char* getAlliedText(size_t index) const;

			/** Return the axis text in the range of [0..2]*/
			MOHPC_EXPORTS const char* getAxisText(size_t index);

			/** Return the scoreboard pic shader. */
			MOHPC_EXPORTS const char* getScoreboardPic() const;

			/** Return the scoreboard pic shader when game is over. */
			MOHPC_EXPORTS const char* getScoreboardPicOver() const;

			/** Return the last time vote has started. */
			MOHPC_EXPORTS uint64_t getVoteTime() const;

			/** Return the number of players that voted yes. */
			MOHPC_EXPORTS uint32_t getNumVotesYes() const;

			/** Return the number of players that voted no. */
			MOHPC_EXPORTS uint32_t getNumVotesNo() const;

			/** Return the number of players that didn't vote. */
			MOHPC_EXPORTS uint32_t getNumVotesUndecided() const;

			/** Return the vote name/text. */
			MOHPC_EXPORTS const char* getVoteString() const;
		};

		/**
		 * Various imports for CGame.
		 */
		class CGameImports
		{
		public:
			std::function<uintptr_t()> getCurrentSnapshotNumber;
			std::function<bool(uintptr_t snapshotNum, SnapshotInfo& outSnapshot)> getSnapshot;
			std::function<uint64_t()> getServerStartTime;
			std::function<uint64_t()> getServerTime;
			std::function<uint64_t()> getServerFrameFrequency;
			std::function<uintptr_t()> getCurrentCmdNumber;
			std::function<bool(uintptr_t cmdNum, usercmd_t& outCmd)> getUserCmd;
			std::function<bool(uintptr_t serverCommandNumber, TokenParser& tokenized)> getServerCommand;
			std::function<const gameState_t&()> getGameState;
		};

		/**
		 * Base CG module, contains most implementations.
		 */
		class CGameModuleBase
		{
		private:
			class HandlerListCGame : public HandlerList
			{
			public:
				MOHPC_HANDLERLIST_DEFINITIONS();

				MOHPC_HANDLERLIST_NOTIFY0();
				MOHPC_HANDLERLIST_NOTIFY1(const EntityInfo&);
				MOHPC_HANDLERLIST_NOTIFY2(uint8_t, const char*);
				MOHPC_HANDLERLIST_NOTIFY1(const char*);
				MOHPC_HANDLERLIST_NOTIFY2(hudMessage_e, const char*);

				MOHPC_HANDLERLIST_HANDLER7(CGameHandlers::MakeBulletTracer, makeBulletTracerHandler, const Vector&, const Vector&, const Vector&, uint32_t, uint32_t, uint32_t, float);
				MOHPC_HANDLERLIST_HANDLER4(CGameHandlers::MakeBubbleTrail, makeBubbleTrailHandler, const Vector&, const Vector&, uint32_t, float);
				MOHPC_HANDLERLIST_HANDLER3(CGameHandlers::Impact, impactHandler, const Vector&, const Vector&, uint32_t);
				MOHPC_HANDLERLIST_HANDLER2(CGameHandlers::MeleeImpact, meleeImpactHandler, const Vector&, const Vector&);
				MOHPC_HANDLERLIST_HANDLER2(CGameHandlers::MakeExplosionEffect, makeExplosionEffectHandler, const Vector&, effects_e);
				MOHPC_HANDLERLIST_HANDLER3(CGameHandlers::MakeEffect, makeEffectHandler, const Vector&, const Vector&, effects_e);
				MOHPC_HANDLERLIST_HANDLER3(CGameHandlers::SpawnDebris, spawnDebrisHandler, CGameHandlers::debrisType_e, const Vector&, uint32_t);
				MOHPC_HANDLERLIST_HANDLER2_NODEF(CGameHandlers::HudDraw_Shader, huddrawShaderHandler, uint8_t, const char*);
				MOHPC_HANDLERLIST_HANDLER3(CGameHandlers::HudDraw_Align, huddrawAlignHandler, uint8_t, CGameHandlers::horizontalAlign_e, CGameHandlers::verticalAlign_e);
				MOHPC_HANDLERLIST_HANDLER5(CGameHandlers::HudDraw_Rect, huddrawRectHandler, uint8_t, uint16_t, uint16_t, uint16_t, uint16_t);
				MOHPC_HANDLERLIST_HANDLER2(CGameHandlers::HudDraw_VirtualScreen, huddrawVSHandler, uint8_t, bool);
				MOHPC_HANDLERLIST_HANDLER2(CGameHandlers::HudDraw_Color, huddrawColorHandler, uint8_t, const Vector&);
				MOHPC_HANDLERLIST_HANDLER2(CGameHandlers::HudDraw_Alpha, huddrawAlphaHandler, uint8_t, float);
				MOHPC_HANDLERLIST_HANDLER2_NODEF(CGameHandlers::HudDraw_String, huddrawStringHandler, uint8_t, const char*);
				MOHPC_HANDLERLIST_HANDLER2_NODEF(CGameHandlers::HudDraw_Font, huddrawFontHandler, uint8_t, const char*);
				MOHPC_HANDLERLIST_HANDLER0_NODEF(CGameHandlers::HitNotify, hitNotifyHandler);
				MOHPC_HANDLERLIST_HANDLER0_NODEF(CGameHandlers::KillNotify, killNotifyHandler);
				MOHPC_HANDLERLIST_HANDLER4(CGameHandlers::VoiceMessage, voiceMessageHandler, const Vector&, bool, uint8_t, const char*);
				MOHPC_HANDLERLIST_HANDLER1_NODEF(CGameHandlers::EntityAdded, entityAddedHandler, const EntityInfo&);
				MOHPC_HANDLERLIST_HANDLER1_NODEF(CGameHandlers::EntityRemoved, entityRemovedHandler, const EntityInfo&);
				MOHPC_HANDLERLIST_HANDLER1_NODEF(CGameHandlers::EntityModified, entityModifiedHandler, const EntityInfo&);
				MOHPC_HANDLERLIST_HANDLER3(CGameHandlers::ReplayMove, replayCmdHandler, const usercmd_t&, playerState_t&, uint32_t);
				MOHPC_HANDLERLIST_HANDLER2_NODEF(CGameHandlers::ServerCommand_Print, serverCommandPrintHandler, hudMessage_e, const char*);
				MOHPC_HANDLERLIST_HANDLER2_NODEF(CGameHandlers::ServerCommand_HudPrint, scmdHudPrintHandler, hudMessage_e, const char*);
				MOHPC_HANDLERLIST_HANDLER1(CGameHandlers::ServerCommand_Scores, scmdScoresHandler, const Scoreboard&);
				MOHPC_HANDLERLIST_HANDLER1_NODEF(CGameHandlers::ServerCommand_Stats, scmdStatsHandler, const stats_t&);
				MOHPC_HANDLERLIST_HANDLER2(CGameHandlers::ServerCommand_Stopwatch, scmdStopwatchHandler, uint64_t, uint64_t);
				MOHPC_HANDLERLIST_HANDLER0_NODEF(CGameHandlers::ServerCommand_ServerLag, scmdServerLagHandler);
				MOHPC_HANDLERLIST_HANDLER1(CGameHandlers::ServerCommand_Stufftext, scmdStufftextHandler, TokenParser&);
			};

		protected:
			CGameImports imports;

		private:
			uint64_t svTime;
			TraceFunction traceFunction;
			PointContentsFunction pointContentsFunction;
			uintptr_t processedSnapshotNum;
			uintptr_t latestSnapshotNum;
			uintptr_t latestCommandSequence;
			size_t numSolidEntities;
			size_t numTriggerEntities;
			uint32_t physicsTime;
			float frameInterpolation;
			float cameraFov;
			Vector predictedError;
			Vector cameraAngles;
			Vector cameraOrigin;
			playerState_t predictedPlayerState;
			HandlerListCGame handlerList;
			CollisionWorld boxHull;
			cgsInfo cgs;
			environment_t environment;
			rain_t rain;
			objective_t objectives[MAX_OBJECTIVES];
			clientInfo_t clientInfo[MAX_CLIENTS];
			SnapshotInfo oldSnap;
			SnapshotInfo* nextSnap;
			SnapshotInfo* snap;
			SnapshotInfo activeSnapshots[MAX_ACTIVE_SNAPSHOTS];
			EntityInfo clientEnts[MAX_GENTITIES];
			EntityInfo* solidEntities[MAX_ENTITIES_IN_SNAPSHOT];
			EntityInfo* triggerEntities[MAX_ENTITIES_IN_SNAPSHOT];
			bool nextFrameTeleport : 1;
			bool thisFrameTeleport : 1;
			bool validPPS : 1;
			bool nextFrameCameraCut : 1;
			bool forceDisablePrediction : 1;

		public:
			CGameModuleBase(const CGameImports& inImports);
			virtual ~CGameModuleBase() = default;

			/** Tick function for CGame module. */
			virtual void tick(uint64_t deltaTime, uint64_t currentTime, uint64_t serverTime);

			/**
			 * Set a function to be called when a specific game event occurs.
			 *
			 * See CGameHandlers above.
			 * @param	args...	Function
			 * @return	callback handle so it can be later unregistered.
			 */
			template<typename T>
			fnHandle_t setCallback(typename T::Type&& handler)
			{
				return handlerList.set<T>(std::forward<T::Type>(handler));
			}

			/**
			 * Unset a previously set callback.
			 *
			 * @param	handle	The returned handle when registering a callback.
			 */
			template<typename T>
			void unsetCallback(fnHandle_t handle)
			{
				handlerList.unset<T>(handle);
			}

			/** Return the alpha interpolation between the current frame and the next frame. */
			MOHPC_EXPORTS float getFrameInterpolation() const;
			
			/** Get the current client time. */
			MOHPC_EXPORTS uint64_t getTime() const;

			/** Get the entity with the specified number. */
			MOHPC_EXPORTS const EntityInfo* getEntity(entityNum_t num);

			/** Return the current snapshot. */
			MOHPC_EXPORTS SnapshotInfo* getCurrentSnapshot() const;

			/** Return the next snapshot. Useful for interpolations. */
			MOHPC_EXPORTS SnapshotInfo* getNextSnapshot() const;

			/** Return the predicted player state. */
			MOHPC_EXPORTS const playerState_t& getPredictedPlayerState() const;

			/**
			 * Set the function used to trace through the world.
			 *
			 * @param	inTraceFunction		Custom function to use for tracing
			 */
			MOHPC_EXPORTS void setTraceFunction(TraceFunction&& inTraceFunction);

			/**
			 * Set the function used to trace through the world.
			 *
			 * @param	inPointContentsFunction		Custom function to use getting content flags.
			 */
			MOHPC_EXPORTS void setPointContentsFunction(PointContentsFunction&& inPointContentsFunction);

			/**
			 * Trace through various entities. This function should be used in conjunction to a previous trace.
			 *
			 * @param	cm			Collision world to use.
			 * @param	start		Start trace.
			 * @param	mins		Bounding box of the trace.
			 * @param	maxs		Bounding box of the trace.
			 * @param	end			End trace.
			 * @param	skipNumber	Entity to ignore.
			 * @param	mask		Trace mask.
			 * @param	tr			Input/Output results.
			 */
			MOHPC_EXPORTS void clipMoveToEntities(CollisionWorld& cm, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, uint16_t skipNumber, uint32_t mask, bool cylinder, trace_t& tr);

			/**
			 * Perform a trace from start to the end, taking entities into account.
			 *
			 * @param	cm			Collision world to use.
			 * @param	start		Start trace.
			 * @param	mins		Bounding box of the trace.
			 * @param	maxs		Bounding box of the trace.
			 * @param	end			End trace.
			 * @param	skipNumber	Entity to ignore.
			 * @param	mask		Trace mask.
			 * @param	tr			Input/Output results.
			 */
			MOHPC_EXPORTS void trace(CollisionWorld& cm, trace_t& tr, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, uint16_t skipNumber, uint32_t mask, bool cylinder, bool cliptoentities);

			/**
			 * Get contents of point.
			 *
			 * @param	cm				Collision world to use.
			 * @param	point			Location to get contents from.
			 * @param	passEntityNum	Entity number to skip.
			 */
			MOHPC_EXPORTS uint32_t pointContents(CollisionWorld& cm, const Vector& point, uintptr_t passEntityNum);

			/** Return server rain settings. */
			MOHPC_EXPORTS const rain_t& getRain() const;

			/** Return server environment settings. */
			MOHPC_EXPORTS const environment_t& getEnvironment() const;

			/** Return the server info. */
			MOHPC_EXPORTS const cgsInfo& getServerInfo() const;

			/** Disable local prediction on client. */
			MOHPC_EXPORTS void disablePrediction();

			/** Enable local prediction on client. */
			MOHPC_EXPORTS void enablePrediction();

			/** Get an objective in the interval of [0, MAX_OBJECTIVES]. */
			MOHPC_EXPORTS const objective_t& getObjective(uint32_t objNum) const;

			/** Get a client info in the interval of [0, MAX_CLIENTS]. */
			MOHPC_EXPORTS const clientInfo_t& getClientInfo(uint32_t clientNum) const;

			/** CG message notification. */
			void parseCGMessage(MSG& msg);

			/** Notified when a configString has been modified. */
			void configStringModified(uint16_t num);

		protected:
			/** Get the list of handlers. */
			const HandlerListCGame& handlers() const;

			/** Get imports list. */
			const CGameImports& getImports() const;

			/** Used to parse CG messages between different versions. */
			virtual void handleCGMessage(MSG& msg, uint8_t msgType) = 0;

			/** Setup move between versions. */
			virtual void setupMove(Pmove& pmove);

			/** Normalize some playerState fields so they all match between versions. */
			virtual void normalizePlayerState(playerState_t& ps);

			/** Parse fog/environment info between versions. */
			virtual void parseFogInfo(const char* s, environment_t& env) = 0;

		private:
			void parseServerInfo(const char* cs);

			/**
			 * Snapshot parsing
			 */
			SnapshotInfo* readNextSnapshot();
			void processSnapshots();
			void setNextSnap(SnapshotInfo* newSnap);
			void setInitialSnapshot(SnapshotInfo* newSnap);
			//====

			/**
			 * Entity transitioning
			 */
			void transitionSnapshot(bool differentServer);
			void transitionEntity(EntityInfo& entInfo);
			void buildSolidList();
			//====

			/**
			 * Client entities
			 */
			void addPacketEntities();
			void addEntity(EntityInfo& entInfo);
			//====

			/**
			 * Player state calculation
			 */
			void predictPlayerState(uint64_t deltaTime);
			void interpolatePlayerState(bool grabAngles);
			void interpolatePlayerStateCamera();
			//====

			/**
			 * Server commands
			 */
			void executeNewServerCommands(uintptr_t serverCommandSequence, bool differentServer);
			void processServerCommand(TokenParser& tokenized);
			//====

			/**
			 * Player movement
			 */
			bool replayMove(Pmove& pmove);
			void extendMove(Pmove& pmove, uint32_t msec);
			//====
		
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
		};

		/**
		 * CG Module for protocol version 6.
		 * => MOH:AA ver 1.00.
		 */
		class CGameModule6 : public CGameModuleBase
		{
		public:
			CGameModule6(const CGameImports& inImports);

		protected:
			virtual void handleCGMessage(MSG& msg, uint8_t msgType) override;
			virtual void normalizePlayerState(playerState_t& ps) override;
			virtual void parseFogInfo(const char* s, environment_t& env) override;

		private:
			effects_e getEffectId(uint32_t effectId);
		};

		/**
		 * CG Module for protocol version 15.
		 * => MOH:SH ver 2.00.
		 */
		class CGameModule15 : public CGameModuleBase
		{
		public:
			CGameModule15(const CGameImports& inImports);

		protected:
			virtual void handleCGMessage(MSG& msg, uint8_t msgType) override;
			virtual void setupMove(Pmove& pmove) override;
			virtual void parseFogInfo(const char* s, environment_t& env) override;

		private:
			effects_e getEffectId(uint32_t effectId);
		};
	}
}
