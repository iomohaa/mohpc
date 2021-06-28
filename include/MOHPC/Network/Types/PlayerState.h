#pragma once

#include "../NetGlobal.h"
#include "../../Common/Vector.h"
#include "../../Utility/Collision/Collision.h"
#include "Entity.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	// current ammo
	static constexpr unsigned int ITEM_AMMO		= 0;
	// current mainhand weapon
	static constexpr unsigned int ITEM_WEAPON	= 1;

	// pmove->pm_flags
	// Those flags correspond to SH/BT PlayerMove flags, which are normalized for AA

	// player is ducked
	static constexpr unsigned int PMF_DUCKED			= (1 << 0);
	// Prone view
	static constexpr unsigned int PMF_VIEW_PRONE		= (1 << 1);

	// Those flags were deleted in SH/BT, reason why other flags are shifted
	//
	// pm_time is time before rejump
	//static constexpr unsigned int PMF_TIME_LAND			= (1 << 2);
	// pm_time is an air-accelerate only time
	//static constexpr unsigned int PMF_TIME_KNOCKBACK	= (1 << 3);
	// pm_time is waterjump
	//static constexpr unsigned int PMF_TIME_WATERJUMP	= (1 << 4);
	//

	// The player is spectating
	static constexpr unsigned int PMF_SPECTATING		= (1 << 2);
	// pm_time is teleport
	static constexpr unsigned int PMF_TIME_TELEPORT		= (1 << 3);
	static constexpr unsigned int PMF_RESPAWNED			= (1 << 3);
	// no prediction
	static constexpr unsigned int PMF_NO_PREDICTION		= (1 << 4);
	// player cannot move or look around (freezeplayer)
	static constexpr unsigned int PMF_FROZEN			= (1 << 5);
	// intermission view
	static constexpr unsigned int PMF_INTERMISSION		= (1 << 6);

	// the following flag is required by the server and cannot be changed
	//
	//
	// use camera view instead of ps view
	static constexpr unsigned int PMF_CAMERA_VIEW		= (1 << 7);

	// player cannot move but can still look around (physics_off)
	static constexpr unsigned int PMF_NO_MOVE			= (1 << 8);
	// player has pulled up his legs so that he occupies only the top of his bounding box
	static constexpr unsigned int PMF_LEGS_LIFTED		= (1 << 9);
	static constexpr unsigned int PMF_VIEW_DUCK_RUN		= (1 << 9);
	// pm_time is stuckjump
	static constexpr unsigned int PMF_TIME_STUCKJUMP	= (1 << 10);
	static constexpr unsigned int PMF_VIEW_JUMP_START	= (1 << 10);
	// player is near an exit
	static constexpr unsigned int PMF_LEVELEXIT			= (1 << 11);
	// do not apply gravity to the player
	static constexpr unsigned int PMF_NO_GRAVITY		= (1 << 12);
	static constexpr unsigned int PMF_NO_HUD			= (1 << 13);
	static constexpr unsigned int PMF_UNKNOWN			= (1 << 14);

	//static constexpr unsigned int PMF_ALL_TIMES			= (PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_KNOCKBACK | PMF_TIME_TELEPORT | PMF_TIME_STUCKJUMP);
	static constexpr unsigned int PMF_ALL_TIMES				= (PMF_TIME_TELEPORT | PMF_TIME_STUCKJUMP);

	/**
	 * Camera Flags
	 */
	/** Absolute angles.*/
	static constexpr unsigned int CF_CAMERA_ANGLES_ABSOLUTE		= (1 << 0);
	/** Ignore pitch for camera_angles. */
	static constexpr unsigned int CF_CAMERA_ANGLES_IGNORE_PITCH = (1 << 1);
	/** Ignore yaw for camera_angles. */
	static constexpr unsigned int CF_CAMERA_ANGLES_IGNORE_YAW	= (1 << 2);
	/** Allow camera_offset to be set. */
	static constexpr unsigned int CF_CAMERA_ANGLES_ALLOWOFFSET	= (1 << 3);
	/** Indicate that this is a turret camera. */
	static constexpr unsigned int CF_CAMERA_ANGLES_TURRETMODE	= (1 << 4);
	/** this bit gets toggled everytime we do a hard camera cut */
	static constexpr unsigned int CF_CAMERA_CUT_BIT = (1 << 7);

	/**
	 * Cinematic stats flags.
	 */
	static constexpr unsigned int CINE_CINEMATIC				= (1 << 0);
	static constexpr unsigned int CINE_CAMERA					= (1 << 1);

	static constexpr unsigned int MAX_LETTERBOX_SIZE			= 0x7fff;
	static constexpr unsigned int STAT_DEAD_YAW					= 5;

	enum class playerstat_e : unsigned char {
		/** Player's current health. */
		Health,
		/** Max player health. */
		MaxHealth,

		/** Number of equipped weapon. */
		Weapons,
		/** Index to the name of the weapon currently equipped. */
		EquippedWeapon,
		/** Ammo remaining for filling the weapon. */
		Ammo,
		/** Max value for the weapon ammo. */
		MaxAmmo,
		/** Current ammo on clip. */
		ClipAmmo,
		/** Max clip size. */
		MaxClipAmmo,

		/** Whether or not the player is zooming. */
		InZoom,
		/** Whether or not the crosshair is drawn. */
		Crosshair,

		/** Last pain damage taken. */
		LastPain,

		Unused2,

		/** Value of bosshealth cvar. */
		BossHealth,

		/** Non-zero for cinematic. See CINE_* flags. */
		Cinematic,
		/** Whether or not to use blend values on playerState. */
		AddFade,
		/** Size of letterbox (see MAX_LETTERBOX_SIZE). */
		LetterBox,

		/** North of the world. */
		CompassNorth,
		/** Objective left direction on the compass. */
		ObjectiveLeft,
		/** Objective right direction on the compass. */
		ObjectiveRight,
		/** Objective center direction on the compass. */
		ObjectiveCenter,

		/** The player's current team. */
		Team,
		/** On FFA: Number of kills the player made. On TDM: Number of kills the team the player is in made. On objective: Number of team wins. */
		Kills,
		/** On FFA: Number of kills the player made. On TDM/Objective: Number of times the player's current team died.*/
		Deaths,
		Unused3,
		Unused4,
		/** The highest score in game. */
		HighestScore,

		/** Client number of the last attacker. -1 = no attacker. */
		AttackerClient,
		/** Client number of the client the player is aiming at. */
		InfoClient,
		/** Health of the info client. */
		InfoClientHealth,
		/** Direction of the damage. */
		DamageDir,

		LastStat,

		/**
		 * Old names for backward compatibility
		 */
		//
		STAT_HEALTH = Health,
		STAT_MAXHEALTH,
		STAT_WEAPONS,
		STAT_EQUIPPED_WEAPON,
		STAT_AMMO,
		STAT_MAXAMMO,
		STAT_CLIPAMMO,
		STAT_MAXCLIPAMMO,
		STAT_INZOOM,
		STAT_CROSSHAIR,
		STAT_LAST_PAIN,
		STAT_UNUSED_2,
		STAT_BOSSHEALTH,
		STAT_CINEMATIC,
		STAT_ADDFADE,
		STAT_LETTERBOX,
		STAT_COMPASSNORTH,
		STAT_OBJECTIVELEFT,
		STAT_OBJECTIVERIGHT,
		STAT_OBJECTIVECENTER,
		STAT_TEAM,
		STAT_KILLS,
		STAT_DEATHS,
		STAT_UNUSED_3,
		STAT_UNUSED_4,
		STAT_HIGHEST_SCORE,
		STAT_ATTACKERCLIENT,
		STAT_INFOCLIENT,
		STAT_INFOCLIENT_HEALTH,
		STAT_DAMAGEDIR,
		STAT_LAST_STAT
	};

	enum class pmType_e : unsigned char {
		/** Normal movement. */
		Normal,
		/** Ladder climbing. */
		ClimbWall,
		/** Noclip movement. */
		Noclip,
		/** no acceleration or turning, but free falling */
		Dead,
	};

	static constexpr unsigned int RADAR_PLAYER_FAR = 1;

	/**
	 * Radar info, packed into one 32-bit value.
	 */
	struct MOHPC_NET_EXPORTS radarInfo_t
	{
	public:
		static constexpr unsigned int BIT_CLIENT = (1 << 6) - 1;
		static constexpr unsigned int BIT_COORD = (1 << 7) - 1;
		static constexpr unsigned int BIT_YAW = (1 << 5) - 1;

	public:
		radarInfo_t();
		radarInfo_t(uint32_t value);
		radarInfo_t(uint8_t clientNum, int8_t x, int8_t y, int8_t yaw, int8_t flags);

		/** Get the clientnum targetted by this radar. */
		uint8_t clientNum() const;

		/** Return the compass-space horizontal value. Range is [-63, 63]. */
		int8_t x() const;

		/** Return the compass-space vertical value. Range is [-63, 63]. */
		int8_t y() const;

		/**
		 * Return the target packed yaw (5-bits value). Range is [0, 31].
		 * Divide 360 by the number of values to get the actual yaw.
		 */
		int8_t yaw() const;

		/** Flags of the radar. See RADAR_* for values. */
		int8_t flags() const;

		/** Return the raw radar info value. */
		uint32_t getRaw() const;

		/** Return the minimum coord value. */
		static int8_t getMinCoord();

		/** Return the maximum coord value. */
		static int8_t getMaxCoord();

		/** Return the coord precision for radar info. */
		static float getCoordPrecision();

		/** Return the yaw precision for radar info. */
		static float getYawPrecision();

	private:
		uint32_t radarValue;
	};

	/**
	 * Unpacked radar information.
	 */
	struct radarUnpacked_t
	{
		uint32_t clientNum;
		float x;
		float y;
		float yaw;
	};

	class MOHPC_NET_EXPORTS playerState_t
	{
	public:
		static constexpr unsigned long MAX_STATS = 32;
		static constexpr unsigned long MAX_ACTIVEITEMS = 8;
		static constexpr unsigned long MAX_AMMO = 16;
		static constexpr unsigned long MAX_AMMO_AMOUNT = 16;
		static constexpr unsigned long MAX_MAX_AMMO_AMOUNT = 16;

	public:
		Vector origin;
		Vector velocity;
		Vector falldir;
		// for fixed views
		Vector viewangles;
		Vector camera_origin, camera_angles;
		Vector camera_offset, camera_posofs, damage_angles;

		float fLeanAngle;
		float music_volume;
		float music_volume_fade_time;
		float reverb_level;
		float blend[4];
		float fov;
		float camera_time;

		// cmd->serverTime of last executed command
		uint32_t commandTime;
		radarInfo_t radarInfo;
		// last trace on ground
		trace_t groundTrace;

		// for view bobbing and footstep generation
		// ducked, jump_held, etc
		uint16_t pm_flags;
		uint16_t pm_time;
		uint16_t gravity;
		uint16_t speed;
		// add to command angles to get view direction
		// changed by spawns, rotating objects, and teleporters
		uint16_t delta_angles[3];

		// ENTITYNUM_NONE = in air
		entityNum_t groundEntityNum;
		uint16_t camera_flags;

		uint16_t stats[MAX_STATS];
		uint16_t activeItems[MAX_ACTIVEITEMS];
		uint16_t ammo_name_index[MAX_AMMO];
		uint16_t ammo_amount[MAX_AMMO_AMOUNT];
		uint16_t max_ammo_amount[MAX_MAX_AMMO_AMOUNT];

		pmType_e pm_type;
		uint8_t bobCycle;
		uint8_t feetfalling;
		// ranges from 0 to MAX_CLIENTS-1
		uint8_t clientNum;
		uint8_t viewheight;
		uint8_t iViewModelAnim;
		uint8_t iViewModelAnimChanged;
		uint8_t current_music_mood;
		uint8_t fallback_music_mood;
		uint8_t reverb_type;
		// used in SH/BT
		bool bVoted;
		bool walking;
		bool groundPlane;

	public:
		playerState_t();

		// Accessor for cross-dll access

		/** Elapsed playerState server time. */
		uint32_t getCommandTime() const;
		/** Movement type. */
		pmType_e getPlayerMoveType() const;
		/** Movement flags (see PMF_*). */
		uint16_t getPlayerMoveFlags() const;
		/** Time in PM code. */
		uint16_t getPlayerMoveTime() const;
		/** Bob movement cycle. */
		uint8_t getBobCycle() const;

		/** The origin of the playerState. */
		const Vector& getOrigin() const;
		/** The movement velocity. */
		const Vector& getVelocity() const;

		/** Current gravity. (Can be different across playerState in rare cases)*/
		uint16_t getGravity() const;
		/** Current speed. Often changes when crouching, jumping... */
		uint16_t getSpeed() const;
		/** Delta angles. Set on spawning and on firing weapons with recoil. */
		void getDeltaAngles(uint16_t angles[3]) const;

		/** The current entity the player is landing at. */
		entityNum_t getGroundEntityNum() const;

		/** Whether or not the player is walking or in air. */
		bool isWalking() const;
		/** True if the player is on a valid plane. */
		bool isGroundPlane() const;
		/** Set when the player is landing on a ledge so that he slides. */
		uint8_t getFeetFalling() const;
		/** The fall direction of the player. */
		const Vector& getFalldir() const;
		/** The last valid trace_t on ground. */
		const trace_t& getGroundTrace() const;

		/** Player cient number. */
		uint8_t getClientNum() const;

		/** Current view angles of the player. */
		const Vector& getViewAngles() const;
		/** The view height. */
		uint8_t getViewHeight() const;

		/** The angles at which the player is leaning. */
		float getLeanAngles() const;
		/** Return the current view model animation number. */
		uint8_t getViewModelAnim() const;
		/** Value that changes when the current viewmodelanim is the same but is repeating. */
		uint8_t getViewModelAnimChanges() const;

		/** Return the value for the specified stats at index. */
		uint16_t getStats(playerstat_e statIndex) const;
		/** Return the index of the active item. */
		uint16_t getActiveItems(uint32_t index) const;
		/** Ammo name in CS_* at the specified index. */
		uint16_t getAmmoNameIndex(uint32_t index) const;
		/** Ammo amount at index. */
		uint16_t getAmmoAmount(uint32_t index) const;
		/** Max ammo amount at index. */
		uint16_t getMaxAmmoAmount(uint32_t index) const;

		/** Mood of the current music. */
		uint8_t getCurrentMusicMood() const;
		/** Mood to fallback on. */
		uint8_t getFallbackMusicMood() const;
		/** Volume of the current music. */
		float getMusicVolume() const;
		/** Time to fade to 0. */
		float getMusicVolumeFadeTime() const;
		/** Type of the reverb to use. */
		uint8_t getReverbType() const;
		/** The level of reverb to use. */
		float getReverbLevel() const;
		/** View target blending (RGBA colors). */
		void getBlend(float outBlend[4]) const;
		/** Field-of-View the player was assigned (usually 80). */
		float getFov() const;

		/** The following values are valid when pm_flags is set to PMF_CAMERA_VIEW. */
		//
		/** The current camera origin. */
		const Vector& getCameraOrigin() const;
		/** The current camera angles. */
		const Vector& getCameraAngles() const;
		/** Time elapsed in camera mode. */
		float getCameraTime() const;
		/** Get the camera offset to add to origin (relative location). */
		const Vector& getCameraOffset() const;
		/** Get the position offset of camera to add to origin. */
		const Vector& getCameraPositionOffset() const;
		/** Camera flags (see CF_* values). */
		uint16_t getCameraFlags() const;
		//

		/** Additive angles when damaged. */
		const Vector& getDamageAngles() const;

		/** Information on the player compass in an SH/BT server. */
		radarInfo_t getRadarInfo() const;

		/** Whether or not the player has voted in an SH/BT server. */
		bool hasVoted() const;
	};
}
}