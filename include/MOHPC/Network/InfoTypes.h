#pragma once

#include "NetGlobal.h"
#include "../Common/Vector.h"
#include "../Utility/Collision/Collision.h"
#include <cstdint>

namespace MOHPC
{
	static constexpr size_t MAX_MAP_AREA_BYTES = 32;
	static constexpr size_t MAX_ENTITIES_IN_SNAPSHOT = 1024;
	static constexpr size_t MAX_SERVER_SOUNDS = 32;
	static constexpr size_t MAX_SERVER_SOUNDS_BITS = 6;

	static constexpr uintptr_t SNAPFLAG_RATE_DELAYED = 1;
	// snapshot used during connection and for zombies
	static constexpr uintptr_t SNAPFLAG_NOT_ACTIVE = 2;
	// toggled every map_restart so transitions can be detected
	static constexpr uintptr_t SNAPFLAG_SERVERCOUNT = 4;

	using entityNum_t = uint16_t;

// renderfx flags (entityState_t::renderfx)
// render flags are set by "renderEffects" event
// and sent to cgame trough entityState_t
static constexpr unsigned int RF_THIRD_PERSON		= (1<<0);   	// don't draw through eyes, only mirrors = (player bodies, chat sprites)
static constexpr unsigned int RF_FIRST_PERSON		= (1<<1);		// only draw through eyes = (view weapon, damage blood blob)
static constexpr unsigned int RF_DEPTHHACK			= (1<<2);		// hack the z-depth so that view weapons do not clip into walls
static constexpr unsigned int RF_VIEWLENSFLARE		= (1<<3);		// View dependent lensflare
static constexpr unsigned int RF_FRAMELERP			= (1<<4);		// interpolate between current and next state
static constexpr unsigned int RF_BEAM				= (1<<5);		// draw a beam between origin and origin2
static constexpr unsigned int RF_DONTDRAW			= (1<<7);		// don't draw this entity but send it over
static constexpr unsigned int RF_LENSFLARE			= (1<<8);		// add a lens flare to this
static constexpr unsigned int RF_EXTRALIGHT			= (1<<9);		// use good lighting on this entity
static constexpr unsigned int RF_DETAIL				= (1<<10);		// Culls a model based on the distance away from you
static constexpr unsigned int RF_SHADOW				= (1<<11);		// whether or not to draw a shadow
static constexpr unsigned int RF_PORTALSURFACE		= (1<<12);   	// don't draw, but use to set portal views
static constexpr unsigned int RF_SKYORIGIN   		= (1<<13);   	// don't draw, but use to set sky portal origin and coordinate system
static constexpr unsigned int RF_SKYENTITY   		= (1<<14);   	// this entity is only visible through a skyportal
static constexpr unsigned int RF_LIGHTOFFSET   		= (1<<15);   	// this entity has a light offset
static constexpr unsigned int RF_CUSTOMSHADERPASS	= (1<<16);		// draw the custom shader on top of the base geometry
static constexpr unsigned int RF_MINLIGHT			= (1<<17);		// allways have some light = (viewmodel, some items)
static constexpr unsigned int RF_FULLBRIGHT			= (1<<18);		// allways have full lighting
static constexpr unsigned int RF_LIGHTING_ORIGIN	= (1<<19);		// use refEntity->lightingOrigin instead of refEntity->origin
																	// for lighting.  This allows entities to sink into the floor
																	// with their origin going solid, and allows all parts of a
																	// player to get the same lighting

static constexpr unsigned int RF_SHADOW_PLANE		= (1<<20);		// use refEntity->shadowPlane
static constexpr unsigned int RF_WRAP_FRAMES		= (1<<21);		// mod the model frames by the maxframes to allow continuous animation without needing to know the frame count
static constexpr unsigned int RF_ADDITIVE_DLIGHT	= (1<<22);		// this entity has an additive dynamic light
static constexpr unsigned int RF_LIGHTSTYLE_DLIGHT	= (1<<23);		// this entity has a dynamic light that uses a light style
static constexpr unsigned int RF_SHADOW_PRECISE		= (1<<24);		// this entity can have a precise shadow applied to it
static constexpr unsigned int RF_INVISIBLE			= (1<<25);		// This entity is invisible, and only negative lights will light it up
static constexpr unsigned int RF_VIEWMODEL			= (1<<26);		// This entity is invisible, and only negative lights will light it up
static constexpr unsigned int RF_PRECISESHADOW		= (1<<28);		// This entity is invisible, and only negative lights will light it up
static constexpr unsigned int RF_ALWAYSDRAW			= (1<<30);
//
// use this mask when propagating renderfx from one entity to another
//
#define  RF_FLAGS_NOT_INHERITED ( RF_LENSFLARE | RF_VIEWLENSFLARE | RF_BEAM | RF_EXTRALIGHT | RF_SKYORIGIN | RF_SHADOW | RF_SHADOW_PRECISE | RF_SHADOW_PLANE )


//#define RF_ADDICTIVEDYNAMICLIGHT ?????

#define RF_FORCENOLOD		1024

#define BEAM_LIGHTNING_EFFECT   (1<<0)
#define BEAM_USEMODEL           (1<<1)
#define BEAM_PERSIST_EFFECT     (1<<2)
#define BEAM_SPHERE_EFFECT      (1<<3)
#define BEAM_RANDOM_DELAY       (1<<4)
#define BEAM_TOGGLE             (1<<5)
#define BEAM_RANDOM_TOGGLEDELAY (1<<6)
#define BEAM_WAVE_EFFECT        (1<<7)
#define BEAM_USE_NOISE          (1<<8)
#define BEAM_PARENT             (1<<9)
#define BEAM_TILESHADER         (1<<10)
#define BEAM_OFFSET_ENDPOINTS   (1<<11)

	enum class weaponCommand_e : unsigned char
	{
		none,
		usePistol,
		useRifle,
		useSmg,
		useMg,
		useGrenade,
		useHeavy,
		useItem,
		useItem2,
		useItem3,
		useItem4,
		prevWeapon,
		nextWeapon,
		useLast,
		holster,
		drop,
		max
	};

	static const unsigned int BUTTON_ATTACK_PRIMARY = (1 << 0);
	static const unsigned int BUTTON_ATTACK_SECONDARY = (1 << 1);
	static const unsigned int BUTTON_RUN = (1 << 2);
	static const unsigned int BUTTON_USE = (1 << 3);
	static const unsigned int BUTTON_LEANLEFT = (1 << 4);
	static const unsigned int BUTTON_LEANRIGHT = (1 << 5);
	static const unsigned int WEAPONCOMMAND_MASK = ((unsigned int)weaponCommand_e::max) - 1;

	class playerState_t;

	class MOHPC_NET_EXPORTS usercmd_t
	{
	public:
		uint32_t serverTime;
		struct buttons_t {
			uint16_t flags;
		} buttons;
		uint16_t angles[3];
		int8_t forwardmove, rightmove, upmove;

	public:
		usercmd_t();
		usercmd_t(uint32_t inServerTime);

		/** Get the user angles. */
		void getAngles(uint16_t& pitch, uint16_t& yaw, uint16_t& roll);

		/** Get the forward value. Range [-128, 127]. */
		int8_t getForwardValue() const;
		/** Get the right value. Range [-128, 127]. */
		int8_t getRightValue() const;
		/** Get the up value. Range [-128, 127]. */
		int8_t getUpValue() const;

		/** Sends a weapon command. */
		void setWeaponCommand(weaponCommand_e weaponCommand);

		/** Converts and set angles. */
		void setAngles(float pitch, float yaw, float roll);

		/** Set angles relative to player's deltaAngles. */
		void setAnglesRelativeTo(const playerState_t& ps, float pitch, float yaw, float roll);

		/** Move forward by the specified value. Range [-128, 127]. */
		void moveForward(int8_t value);

		/** Move right by the specified value. Range [-128, 127]. */
		void moveRight(int8_t value);

		/** Move up by the specified value. Range [-128, 127]. */
		void moveUp(int8_t value);

		/** Jump. Same as calling moveUp with a value of 127. */
		void jump();

		/** Crouch. Same as calling moveUp with a value of -128. */
		void crouch();

		/** Return buttons that are held. */
		buttons_t getButtons() const;

		void setButtonFlags(uint32_t flags);
		void removeButtonFlags(uint32_t flags);
	};

	class MOHPC_NET_EXPORTS usereyes_t
	{
	public:
		/** Pitch and yaw of eyes. */
		float angles[2];

		/** Position of eyes. */
		int8_t ofs[3];

	public:
		usereyes_t();

		/** Set the offset of eyes view. The offset is the head position starting from the player's origin (feet). */
		void setOffset(int8_t x, int8_t y, int8_t z);

		/** Return the view offset. */
		void getOffset(int8_t xyz[3]);

		/** Set the eyes angles. */
		void setAngles(float pitch, float yaw);

		/** Return the eyes angles. */
		void getAngles(float& pitch, float& yaw);
	};

	static constexpr size_t GENTITYNUM_BITS = 10; // don't need to send any more
	static constexpr size_t	MAX_GENTITIES = (1 << GENTITYNUM_BITS);

	static constexpr size_t ENTITYNUM_NONE = (MAX_GENTITIES - 1);
	static constexpr size_t ENTITYNUM_WORLD = (MAX_GENTITIES - 2);
	static constexpr size_t ENTITYNUM_MAX_NORMAL = (MAX_GENTITIES - 2);

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

	struct MOHPC_NET_EXPORTS trajectory_t
	{
	public:
		Vector trDelta;
		uint32_t trTime;

	public:
		trajectory_t();

		const Vector& getDelta() const;
		uint32_t getTime() const;
	};

	struct MOHPC_NET_EXPORTS frameInfo_t
	{
	public:
		uint32_t index;
		float time;
		float weight;

	public:
		frameInfo_t();

		uint32_t getIndex() const;
		float getTime() const;
		float getWeight() const;
	};

	static constexpr unsigned int EF_TELEPORT_BIT = (1 << 2);
	static constexpr unsigned int EF_CLIMBWALL = (1 << 4);
	/** This player entity has his weapon holstered. */
	static constexpr unsigned int EF_UNARMED = (1 << 5);
	static constexpr unsigned int EF_LINKANGLES = (1 << 6);
	/** This player entity is in allies team. */
	static constexpr unsigned int EF_ALLIES = (1 << 7);
	/** This player entity is in axis team. */
	static constexpr unsigned int EF_AXIS = (1 << 8);
	/** This player entity is actually dead. Don't draw a foe marker over players with EF_DEAD. */
	static constexpr unsigned int EF_DEAD = (1 << 9);

	enum class entityType_e : unsigned char {
		modelanim_skel,
		modelanim,
		vehicle,
		player,
		item,
		general,
		missile,
		mover,
		beam,
		multibeam,
		portal,
		event_only,
		rain,
		leaf,
		speaker,
		push_trigger,
		teleport_trigger,
		decal,
		emitter,
		rope,
		events,
		exec_commands
	};

	// flip the togglebit every time an animation
	// changes so a restart of the same anim can be detected
	static constexpr unsigned int ANIM_TOGGLEBIT = (1 << 9);
	static constexpr unsigned int ANIM_BLEND = (1 << 10);
	static constexpr unsigned int ANIM_NUMBITS = 11;

	// server side anim bits
	static constexpr unsigned int ANIM_SERVER_EXITCOMMANDS_PROCESSED = (1 << 12);

	static constexpr unsigned int ANIM_MASK = (~(ANIM_TOGGLEBIT | ANIM_BLEND | ANIM_SERVER_EXITCOMMANDS_PROCESSED));

	// if FRAME_EXPLICIT is set, don't auto animate
	static constexpr unsigned int FRAME_EXPLICIT = 512;
	static constexpr unsigned int FRAME_MASK = (~FRAME_EXPLICIT);

	//
	// Tag specific flags
	//
	static constexpr unsigned int TAG_NUMBITS = 10;                    // number of bits required to send over network
	static constexpr unsigned int TAG_MASK = ((1 << 10) - 1);

	// bone_tag values for players
	enum class playerBoneTag_e : unsigned char {
		head,
		torso,
		arms,
		pelvis,
		mouth
	};

	class MOHPC_NET_EXPORTS entityState_t
	{
	public:
		static constexpr unsigned long MAX_FRAMEINFOS = 16;
		static constexpr unsigned long NUM_BONE_CONTROLLERS = 5;

	public:
		trajectory_t pos;
		Vector netorigin;
		Vector origin2;
		Vector netangles;
		Vector attach_offset;
		Vector eyeVector;
		Vector bone_angles[NUM_BONE_CONTROLLERS];
		quat_t bone_quat[NUM_BONE_CONTROLLERS];

		uint32_t constantLight;	// r + (g<<8) + (b<<16) + (intensity<<24)
		uint32_t loopSound;		// constantly loop this sound

		// for client side prediction, trap_linkentity sets this properly
		uint32_t solid;
		// Render flags (see RF_*)
		uint32_t renderfx;

		float loopSoundVolume;
		float loopSoundMinDist;
		float loopSoundMaxDist;
		float loopSoundPitch;
		float actionWeight;
		float scale;
		float alpha;
		float shader_data[2];
		float shader_time;
		frameInfo_t frameInfo[MAX_FRAMEINFOS];

		// entity index
		entityNum_t number;
		uint16_t eFlags;

		uint16_t parent;
		uint16_t tag_num;

		uint16_t beam_entnum;

		uint16_t modelindex;
		uint16_t usageIndex;
		uint16_t skinNum;
		uint16_t wasframe;
		// -1 = in air
		uint16_t groundEntityNum;

		// entityType_t
		entityType_e eType;
		uint8_t loopSoundFlags;

		// 0 to (MAX_CLIENTS - 1), for players and corpses
		uint8_t clientNum;
		uint8_t bone_tag[NUM_BONE_CONTROLLERS];
		uint8_t	surfaces[32];
		bool attach_use_angles;

	public:
		entityState_t();

		/** Entity number. */
		entityNum_t getNumber() const;
		/** Type of the entity. */
		entityType_e getType() const;
		/** Flags of the entity (see EF_* values). */
		uint16_t getEntityFlags() const;

		/** Entity's trajectory (delta movement and time). */
		const trajectory_t& getTrajectory() const;

		/** Network origin. */
		const Vector& getNetOrigin() const;
		/** Network secondary origin. */
		const Vector& getAlternateOrigin() const;
		/** Network angles. */
		const Vector& getNetAngles() const;

		/** Entity's RGBA values from constant light. */
		uint32_t getConstantLight() const;
		/** Extract RGBA values from constant light. */
		void getConstantLight(uint8_t& red, uint8_t& green, uint8_t& blue, uint8_t& intensity);

		/** Whether or not entity is looping sound. */
		bool doesLoopsound() const;
		/** Looping sound volume. */
		float getLoopsoundVolume() const;
		/** Looping sound minimum distance. */
		float getLoopsoundMinDist() const;
		/** Looping sound maximum distance. */
		float getLoopsoundMaxDist() const;
		/** Looping sound flags. */
		uint32_t getLoopsoundFlags() const;

		/** Entity's parent if it has one. None = ENTITYNUM_NONE. */
		entityNum_t getParent() const;
		/** Tag num of the entity's parent (usually 0). */
		uint16_t getTagNum() const;

		/** Whether or not to also bind the entity's angles. */
		bool doesAttachUseAngle() const;
		/** Attachment offset (relative location). */
		const Vector& getAttachOffset() const;

		/** Beam number of the entity. */
		uint16_t getBeamEntityNum() const;

		/** Model index (starting from CS_MODELS)*/
		uint16_t getModelIndex() const;
		/** Usage index of the entity. */
		uint16_t getUsageIndex() const;
		/** Skin number. */
		uint16_t getSkinNum() const;
		/** This field doesn't seem to have a reason to exist. */
		uint16_t getWasFrame() const;
		/** Frame info at the specified index. */
		const frameInfo_t& getFrameInfo(uint8_t frameIndex) const;
		/** Get the torso action weight if it's a player. */
		float getActionWeight() const;

		/** Get bone number for index. */
		uint8_t getBoneTag(uint8_t boneIndex) const;
		/** Bone angles at index. */
		const Vector& getBoneAngles(uint8_t boneIndex);
		/** Surface flags index. */
		uint8_t getSurface(uint8_t surfaceIndex) const;

		/** Client number of the entity if it's a player. */
		uint8_t getClientNum() const;
		/** Number of the entity it is landing on. */
		uint16_t getGroundEntityNum() const;

		/** Contains bounding box values. SOLID_BMODEL for brushmodel collision. */
		uint32_t getSolid() const;

		/** Scale multiplier. */
		float getScale() const;
		/** Render alpha. */
		float getAlpha() const;
		/** Render flags of the entity (see RF_* values). */
		uint32_t getRenderFlags() const;
		/** Shader data. */
		float getShaderData(uint8_t shaderIndex) const;
		/** Shader time. */
		float getShaderTime() const;
	};

	entityState_t getNullEntityState();

	struct sound_t
	{
	public:
		const entityState_t* entity;
		const char* soundName;
		Vector origin;
		float volume;
		float minDist;
		float maxDist;
		float pitch;
		uint8_t channel;
		bool hasStopped : 1;
		bool isStreamed : 1;
		bool isSpatialized : 1;
		bool hasVolume : 1;
		bool hasDist : 1;
		bool hasPitch : 1;

	public:
		sound_t();

		/** The entity that the sound is playing on. */
		MOHPC_NET_EXPORTS const entityState_t* getEntityState() const;
		/** The name of the sound (retrieved from configstrings). */
		MOHPC_NET_EXPORTS const char* getName() const;
		/** The sound origin if it's spatialized. */
		MOHPC_NET_EXPORTS const Vector& getOrigin() const;

		/** Sound's volume. */
		MOHPC_NET_EXPORTS float getVolume() const;
		/** The minimum distance the sound will play at full volume. */
		MOHPC_NET_EXPORTS float getMinimumDistance() const;
		/** The maximum distance before the sound stops playing. */
		MOHPC_NET_EXPORTS float getMaximumDistance() const;
		/** The pitch of the sound. */
		MOHPC_NET_EXPORTS float getPitch();
		/** The channel the sound is playing on. */
		MOHPC_NET_EXPORTS uint8_t getChannel() const;

		/** Whether or not the sound has stopped playing. */
		MOHPC_NET_EXPORTS bool hasSoundStopped() const;
		/** If the whole sound is not into memory. */
		MOHPC_NET_EXPORTS bool isStreamedSound() const;
		/** If the sound is spatialized (check origin).*/
		MOHPC_NET_EXPORTS bool isSpatializedSound() const;

		/** Volume is assigned if yes. */
		MOHPC_NET_EXPORTS bool hasSoundVolume() const;
		/** minDist is assigned if yes. */
		MOHPC_NET_EXPORTS bool hasSoundDistance() const;
		/** pitch is assigned if yes. */
		MOHPC_NET_EXPORTS bool hasSoundPitch() const;
	};

	namespace Network
	{
		class SnapshotInfo
		{
		public:
			// all of the entities that need to be presented
			size_t numEntities;

			size_t numSounds;

			// text based server commands to execute when this
			size_t numServerCommands;
			// snapshot becomes current
			size_t serverCommandSequence;

			// SNAPFLAG_RATE_DELAYED, etc
			uint32_t snapFlags;
			uint32_t ping;

			// server time the message is valid for (in msec)
			uint32_t serverTime;

			// complete information about the current player at this time
			playerState_t ps;

			// portalarea visibility bits
			uint8_t areamask[MAX_MAP_AREA_BYTES];

			// at the time of this snapshot
			entityState_t entities[MAX_ENTITIES_IN_SNAPSHOT];

			// sounds
			sound_t sounds[MAX_SERVER_SOUNDS];

		public:
			SnapshotInfo();

			/** Flags of the snap (see SNAPFLAG_* values). */
			MOHPC_NET_EXPORTS uint32_t getSnapFlags() const;
			/** Ping of the client in snap. */
			MOHPC_NET_EXPORTS uint32_t getPing() const;

			/** Server time when the server sent the snap. */
			MOHPC_NET_EXPORTS uint32_t getServerTime() const;

			/** Area mask at index (for visibility). */
			MOHPC_NET_EXPORTS uint8_t getAreaMask(uint8_t index) const;

			/** Current playerState in snap. */
			MOHPC_NET_EXPORTS const playerState_t& getPlayerState() const;

			/** Number of entities in this snap. */
			MOHPC_NET_EXPORTS size_t getNumEntities() const;
			/** Entity at the specified index. */
			MOHPC_NET_EXPORTS const entityState_t& getEntityState(entityNum_t index) const;
			/** Search for an entity by a global entity number. */
			MOHPC_NET_EXPORTS const entityState_t* getEntityStateByNumber(entityNum_t entityNum) const;

			/** Number of server commands that was received in this snap. */
			MOHPC_NET_EXPORTS size_t getNumServerCommands();
			/** Start sequence number of the command in list. */
			MOHPC_NET_EXPORTS size_t getServerCommandSequence() const;

			/** Number of sounds in this snap*/
			MOHPC_NET_EXPORTS size_t getNumSounds() const;
			/** Sound at the specified index. */
			MOHPC_NET_EXPORTS const sound_t& getSound(uint8_t index) const;
		};
	}
}