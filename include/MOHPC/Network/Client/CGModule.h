#pragma once

#include <cstdint>
#include "../Configstring.h"
#include "../InfoTypes.h"
#include "../Types.h"
#include "../pm/bg_public.h"
#include "../../Utilities/HandlerList.h"
#include "../../Utilities/PropertyMap.h"
#include "../../Utilities/Function.h"
#include "../../Vector.h"
#include "Imports.h"
#include "Vote.h"
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

	class Pmove;

	namespace Network
	{
		class ClientGameConnection;
		struct gameState_t;

		static constexpr size_t MAX_ACTIVE_SNAPSHOTS = 2;
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
		MOHPC_EXPORTS const char* getEffectName(effects_e effect);

		/**
		 * List of valid game types.
		 */
		enum class gameType_e : unsigned char
		{
			SinglePlayer = 0,
			FreeForAll,
			TeamDeathmatch,
			RoundBasedMatch,
			Objective,
			TugOfWar,
			Liberation
		};

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
			 * @param	type	Explosion type.
			 */
			struct MakeExplosionEffect : public HandlerNotifyBase<void(const Vector& origin, effects_e type)> {};

			/**
			 * Called to spawn an effect.
			 *
			 * @param	origin	Location of the effect.
			 * @param	normal	Explosion's normal direction.
			 * @param	type	Type of the effect.
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
			 * @param	ucmd		Input to replay.
			 * @param	ps			Player state where to apply movement.
			 * @param	frameTime	delta time between last cmd time and playerState command time. Usually client's frametime.
			 */
			struct ReplayMove : public HandlerNotifyBase<void(const usercmd_t& ucmd, playerState_t& ps, float frameTime)> {};

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

			/**
			 * Called when the vote options has been received and parsed.
			 *
			 * @param options Vote options.
			 */
			struct ReceivedVoteOptions : public HandlerNotifyBase<void (const VoteOptions& options)> {};

			/**
			 * Called when the vote options has been received and parsed.
			 *
			 * @param options Vote options.
			 */
			struct VoteModified : public HandlerNotifyBase<void(const voteInfo_t& voteInfo)> {};
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

		public:
			EntityInfo();

			/** Get the current entity state. */
			MOHPC_EXPORTS const entityState_t& getCurrentState() const;
			/** Get the next entity state from the next incoming snap. */
			MOHPC_EXPORTS const entityState_t& getNextState() const;
			/** Get the last updated time (snapshot time). */
			MOHPC_EXPORTS uint32_t getSnapshotTime() const;
			/** Whether or not this entity info exists in world. */
			MOHPC_EXPORTS bool isValid() const;
			/** Whether or not this entity info should interpolate. */
			MOHPC_EXPORTS bool isInterpolating() const;
			MOHPC_EXPORTS bool hasTeleported() const;
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

			MOHPC_EXPORTS float getDensity() const;
			MOHPC_EXPORTS float getSpeed() const;
			MOHPC_EXPORTS uint32_t getSpeedVariation() const;
			MOHPC_EXPORTS uint32_t getSlant() const;
			MOHPC_EXPORTS float getLength() const;
			MOHPC_EXPORTS float getMinimumDistance() const;
			MOHPC_EXPORTS float getWidth() const;
			MOHPC_EXPORTS uint32_t getNumShaders() const;
			MOHPC_EXPORTS const char* getShader(uint8_t index) const;
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

			/** Return the server type the client is connected on. */
			MOHPC_EXPORTS serverType_e getServerType() const;

			/** Return whether or not voting is allowed. */
			MOHPC_EXPORTS bool isVotingAllowed() const;

			/** Return the map checksum on the server. Useful to compare against the checksum of the map on the client. */
			MOHPC_EXPORTS uint32_t getMapChecksum() const;

			/** Return the current map name. */
			MOHPC_EXPORTS const char* getMapName() const;
			MOHPC_EXPORTS const str& getMapNameStr() const;

			/** Return the path to the map name. */
			MOHPC_EXPORTS const char* getMapFilename() const;
			MOHPC_EXPORTS const str& getMapFilenameStr() const;

			/** Return the allied text NUM_TEAM_OBJECTIVES is the max. */
			MOHPC_EXPORTS const char* getAlliedText(size_t index) const;

			/** Return the axis text. NUM_TEAM_OBJECTIVES is the max. */
			MOHPC_EXPORTS const char* getAxisText(size_t index);

			/** Return the scoreboard pic shader. */
			MOHPC_EXPORTS const char* getScoreboardPic() const;

			/** Return the scoreboard pic shader when game is over. */
			MOHPC_EXPORTS const char* getScoreboardPicOver() const;

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
			voteInfo_t voteInfo;
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

		/**
		 * Client settings data structure.
		 */
		struct MOHPC_EXPORTS clientSettings_t
		{
		public:
			clientSettings_t();

			/**
			 * Update pmove sub-ticking.
			 * @param value Values are in range through [8, 33]
			 */
			void setPmoveMsec(uint32_t value);

			/** Return pmove sub-ticking milliseconds. */
			uint32_t getPmoveMsec() const;

			/**
			 * Set whether or not pmove sub-ticking should be using a fixed time.
			 *
			 * @param value true for fixed sub-ticking.
			 */
			void setPmoveFixed(bool value);

			/** Return whether or not pmove sub-ticking is a fixed time. */
			bool isPmoveFixed() const;

			/** Disable local prediction on client. */
			void disablePrediction();

			/** Enable local prediction on client. */
			void enablePrediction();

			/** Return whether or not the prediction is disabled. */
			bool isPredictionDisabled() const;

		private:
			uint32_t pmove_msec;
			bool pmove_fixed : 1;
			bool forceDisablePrediction : 1;
		};

		/**
		 * Base CG module, contains most implementations.
		 */
		class CGameModuleBase
		{
		public:
			class HandlerListCGame : public HandlerList
			{
			public:
				FunctionList<CGameHandlers::MakeBulletTracer> makeBulletTracerHandler;
				FunctionList<CGameHandlers::MakeBubbleTrail> makeBubbleTrailHandler;
				FunctionList<CGameHandlers::Impact> impactHandler;
				FunctionList<CGameHandlers::MeleeImpact> meleeImpactHandler;
				FunctionList<CGameHandlers::MakeExplosionEffect> makeExplosionEffectHandler;
				FunctionList<CGameHandlers::MakeEffect> makeEffectHandler;
				FunctionList<CGameHandlers::SpawnDebris> spawnDebrisHandler;
				FunctionList<CGameHandlers::HudDraw_Shader> huddrawShaderHandler;
				FunctionList<CGameHandlers::HudDraw_Align> huddrawAlignHandler;
				FunctionList<CGameHandlers::HudDraw_Rect> huddrawRectHandler;
				FunctionList<CGameHandlers::HudDraw_VirtualScreen> huddrawVSHandler;
				FunctionList<CGameHandlers::HudDraw_Color> huddrawColorHandler;
				FunctionList<CGameHandlers::HudDraw_Alpha> huddrawAlphaHandler;
				FunctionList<CGameHandlers::HudDraw_String> huddrawStringHandler;
				FunctionList<CGameHandlers::HudDraw_Font> huddrawFontHandler;
				FunctionList<CGameHandlers::HitNotify> hitNotifyHandler;
				FunctionList<CGameHandlers::KillNotify> killNotifyHandler;
				FunctionList<CGameHandlers::VoiceMessage> voiceMessageHandler;
				FunctionList<CGameHandlers::EntityAdded> entityAddedHandler;
				FunctionList<CGameHandlers::EntityRemoved> entityRemovedHandler;
				FunctionList<CGameHandlers::EntityModified> entityModifiedHandler;
				FunctionList<CGameHandlers::ReplayMove> replayCmdHandler;
				FunctionList<CGameHandlers::Print> printHandler;
				FunctionList<CGameHandlers::ServerCommand> scmdHandler;
				FunctionList<CGameHandlers::HudPrint> hudPrintHandler;
				FunctionList<CGameHandlers::ServerCommand_Scores> scmdScoresHandler;
				FunctionList<CGameHandlers::ServerCommand_Stats> scmdStatsHandler;
				FunctionList<CGameHandlers::ServerCommand_Stopwatch> scmdStopwatchHandler;
				FunctionList<CGameHandlers::ServerCommand_ServerLag> scmdServerLagHandler;
				FunctionList<CGameHandlers::ServerCommand_Stufftext> scmdStufftextHandler;
				FunctionList<CGameHandlers::ReceivedVoteOptions> receivedVoteOptionsHandler;
				FunctionList<CGameHandlers::VoteModified> voteModifiedHandler;
			};

		public:
			CGameModuleBase(const ClientImports& inImports);
			virtual ~CGameModuleBase() = default;

			virtual void init(uintptr_t serverMessageSequence, uintptr_t serverCommandSequence);

			/** Tick function for CGame module. */
			virtual void tick(uint64_t deltaTime, uint64_t currentTime, uint64_t serverTime);

			/** Return the handler list. */
			MOHPC_EXPORTS HandlerListCGame& getHandlerList();

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
			 * Predict the player state movement.
			 * It is not called in the tick() function. It gives a chance for the caller
			 * to immediately predict player state after the input has been processed.
			 */
			MOHPC_EXPORTS void predictPlayerState();

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
			MOHPC_EXPORTS uint32_t pointContents(CollisionWorld& cm, const Vector& point, uintptr_t passEntityNum) const;

			/** Return server rain settings. */
			MOHPC_EXPORTS const rain_t& getRain() const;

			/** Return server environment settings. */
			MOHPC_EXPORTS const environment_t& getEnvironment() const;

			/** Return the server info. */
			MOHPC_EXPORTS const cgsInfo& getServerInfo() const;

			/** Get an objective in the interval of [0, MAX_OBJECTIVES]. */
			MOHPC_EXPORTS const objective_t& getObjective(uint32_t objNum) const;

			/** Get a client info in the interval of [0, MAX_CLIENTS]. */
			MOHPC_EXPORTS const clientInfo_t& getClientInfo(uint32_t clientNum) const;

			/** Return the client settings. */
			MOHPC_EXPORTS clientSettings_t& getSettings();

			/** CG message notification. */
			void parseCGMessage(MSG& msg);

			/** Notified when a configString has been modified. */
			void configStringModified(uint16_t num, const char* newString);

		protected:
			/** Get the list of handlers. */
			const HandlerListCGame& handlers() const;

			/** Get imports list. */
			const ClientImports& getImports() const;

			virtual Pmove& getMove() = 0;

			/** Used to parse CG messages between different versions. */
			virtual void handleCGMessage(MSG& msg, uint8_t msgType) = 0;

			/** Setup move between versions. */
			virtual void setupMove(Pmove& pmove) = 0;

			/** Normalize some playerState fields so they all match between versions. */
			virtual void normalizePlayerState(playerState_t& ps) = 0;

			/** Parse fog/environment info between versions. */
			virtual void parseFogInfo(const char* s, environment_t& env) = 0;

			virtual serverType_e parseServerType(const char* version, size_t len) = 0;

		private:
			void parseServerInfo(const char* cs);
			void conditionalReflectClient(const clientInfo_t& client);

			/**
			 * Environment
			 */
			void parseSkyInfo(const char* cs, environment_t& env);

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
			 * Player state calculation
			 */
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
			bool replayAllCommands();
			bool tryReplayCommand(Pmove& pmove, const playerState_t& oldPlayerState, const usercmd_t& latestCmd, uintptr_t cmdNum);
			bool replayMove(Pmove& pmove, usercmd_t& cmd);
			void extendMove(Pmove& pmove, uint32_t msec);
			void physicsNoclip(Pmove& pmove, float frametime);
			//====

			/**
			 * Vote
			 */
			 void voteModified();
			 void parseVoteOptions(const char* options, size_t len);
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
			void SCmd_PrintDeathMsg(TokenParser& args);

			void SCmd_VoteOptions_StartReadFromServer(TokenParser& args);
			void SCmd_VoteOptions_ContinueReadFromServer(TokenParser& args);
			void SCmd_VoteOptions_FinishReadFromServer(TokenParser& args);

		protected:
			ClientImports imports;

		private:
			uint64_t svTime;
			TraceFunction traceFunction;
			PointContentsFunction pointContentsFunction;
			HandlerListCGame handlerList;
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
			CollisionWorld boxHull;
			clientSettings_t settings;
			cgsInfo cgs;
			environment_t environment;
			rain_t rain;
			VoteOptions voteOptions;
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
		};

		/**
		 * CG Module for protocol version 6.
		 * => MOH:AA ver 1.00.
		 */
		class CGameModule6 : public CGameModuleBase
		{
		public:
			CGameModule6(const ClientImports& inImports);

		protected:
			void handleCGMessage(MSG& msg, uint8_t msgType) override;
			void normalizePlayerState(playerState_t& ps) override;
			void parseFogInfo(const char* s, environment_t& env) override;
			Pmove& getMove() override;
			void setupMove(Pmove& pmove) override;
			serverType_e parseServerType(const char* version, size_t len) override;

		private:
			effects_e getEffectId(uint32_t effectId);

		private:
			Pmove_ver6 pmove;
		};

		/**
		 * CG Module for protocol version 15.
		 * => MOH ver 2.00 and above (since SH).
		 */
		class CGameModule15 : public CGameModuleBase
		{
		public:
			CGameModule15(const ClientImports& inImports);

		protected:
			void handleCGMessage(MSG& msg, uint8_t msgType) override;
			void normalizePlayerState(playerState_t& ps) override;
			void setupMove(Pmove& pmove) override;
			void parseFogInfo(const char* s, environment_t& env) override;
			Pmove& getMove() override;
			serverType_e parseServerType(const char* version, size_t len) override;

		private:
			effects_e getEffectId(uint32_t effectId);

		private:
			Pmove_ver15 pmove;
		};

		class CommonMessageHandler
		{
		public:
			CommonMessageHandler(MSG& inMsg, const CGameModuleBase::HandlerListCGame& inHandlerList);

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
			const CGameModuleBase::HandlerListCGame& handlerList;
		};

		namespace CGError
		{
			class Base : public NetworkException {};

			/**
			 * The next snap time is backward of the client time.
			 */
			class NextSnapTimeWentBackward : public Base
			{
			public:
				NextSnapTimeWentBackward(uint64_t inPrevTime, uint64_t inTime);

				/** Return the client time. */
				MOHPC_EXPORTS uint64_t getClientTime() const;
				/** Return the time of the snap. */
				MOHPC_EXPORTS uint64_t getSnapTime() const;

			private:
				uint64_t oldTime;
				uint64_t time;
			};

			/**
			 * The next snapshot number is lower than the latest snapshot number.
			 */
			class SnapNumWentBackward : public Base
			{
			public:
				SnapNumWentBackward(uintptr_t newNum, uintptr_t latestNum);

				/** Return the new snapshot number. */
				MOHPC_EXPORTS uintptr_t getNewNum() const;
				/** Return the latest snapshot number. */
				MOHPC_EXPORTS uintptr_t getLatestNum() const;

			private:
				uintptr_t newNum;
				uintptr_t latestNum;
			};

			/**
			 * Null snapshot when processing snapshots.
			 */
			class NullSnapshot : public Base {};
		}
	}
}
