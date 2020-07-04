#pragma once

#include "../Global.h"
#include "../Vector.h"
#include <stdint.h>
#include "../Collision/Collision.h"

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
		usePistol = 1,
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
		drop
	};

	static const unsigned int BUTTON_ATTACK = (1 << 0);
	static const unsigned int BUTTON_MELEE = (1 << 1);
	static const unsigned int BUTTON_RUN = (1 << 2);
	static const unsigned int BUTTON_USE = (1 << 3);
	static const unsigned int BUTTON_LEANLEFT = (1 << 4);
	static const unsigned int BUTTON_LEANRIGHT = (1 << 5);

	class MOHPC_EXPORTS usercmd_t
	{
	public:
		uint32_t serverTime;
		struct {
			union {
				struct {
					// Button flags
					struct {
						uint8_t attack		: 1;
						uint8_t melee		: 1;
						uint8_t run			: 1;
						uint8_t use			: 1;
						uint8_t leanleft	: 1;
						uint8_t leanright	: 1;
					} button;
				} fields;

				struct {
					uint8_t moveflags : 6;
					uint8_t weaponFlags : 5;
				} bitflags;

				// The whole buttons flags
				uint16_t flags;
			};
		} buttons;
		uint16_t angles[3];
		int8_t forwardmove, rightmove, upmove;

	public:
		usercmd_t();
		usercmd_t(uint32_t inServerTime);

		/** Sends a weapon command. */
		void setWeaponCommand(weaponCommand_e weaponCommand);

		/** Converts and set angles. */
		void setAngles(float pitch, float yaw, float roll);

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
	};

	class MOHPC_EXPORTS usereyes_t
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

		/** Set the eyes angles. */
		void setAngle(float pitch, float yaw);
	};

	static constexpr size_t GENTITYNUM_BITS = 10; // don't need to send any more
	static constexpr size_t	MAX_GENTITIES = (1 << GENTITYNUM_BITS);

	static constexpr size_t ENTITYNUM_NONE = (MAX_GENTITIES - 1);
	static constexpr size_t ENTITYNUM_WORLD = (MAX_GENTITIES - 2);
	static constexpr size_t ENTITYNUM_MAX_NORMAL = (MAX_GENTITIES - 2);


	/*
	// plane_t structure
	// !!! if this is changed, it must be changed in asm code too !!!
	typedef struct cplane_s {
		vec3_t normal;
		float dist;
		uint8_t type;			// for fast side tests: 0,1,2 = axial, 3 = nonaxial
		uint8_t signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
	} cplane_t;

	// a trace is returned when a box is swept through the world
	struct trace_t  {
		bool	allsolid;	// if true, plane is not valid
		bool	startsolid;	// if true, the initial point was in a solid area
		float		fraction;	// time completed, 1.0 = didn't hit anything
		Vector		endpos;		// final position
		cplane_t	plane;		// surface normal at impact, transformed to world space
		int			surfaceFlags;	// surface hit
		int			shaderNum;
		int			contents;	// contents on other side of surface hit
		int			entityNum;	// entity the contacted surface is a part of

		int			location;
	};
	*/

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

	// Those flags was deleted in SH/BT
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

	//
	// Camera Flags
	//
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

	enum class playerstat_e : unsigned char {
		STAT_HEALTH,
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
		PM_NORMAL,
		/** Ladder climbing. */
		PM_CLIMBWALL,
		/** Noclip movement. */
		PM_NOCLIP,
		/** no acceleration or turning, but free falling */
		PM_DEAD,
	};

	class MOHPC_EXPORTS playerState_t
	{
	public:
		static constexpr size_t MAX_STATS = 32;
		static constexpr size_t MAX_ACTIVEITEMS = 8;
		static constexpr size_t MAX_AMMO = 16;
		static constexpr size_t MAX_AMMO_AMOUNT = 16;
		static constexpr size_t MAX_MAX_AMMO_AMOUNT = 16;

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
		uint32_t radarInfo;
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
	};

	struct MOHPC_EXPORTS trajectory_t {
		Vector trDelta;
		int trTime;

		trajectory_t();
	};

	struct MOHPC_EXPORTS frameInfo_t
	{
		int index;
		float time;
		float weight;

		frameInfo_t();
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

	class MOHPC_EXPORTS entityState_t
	{
	public:
		static constexpr unsigned long MAX_FRAMEINFOS = 16;
		static constexpr unsigned long NUM_BONE_CONTROLLERS = 5;

	public:
		trajectory_t pos;
		Vector netorigin;
		Vector origin;
		Vector origin2;
		Vector netangles;
		Vector angles;
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
	};

	struct sound_t
	{
		/** The entity that the sound is playing on. */
		const entityState_t* entity;

		/** The name of the sound (retrieved from configstrings). */
		const char* soundName;

		/** The sound origin if it's spatialized. */
		Vector origin;

		/** Sound's volume. */
		float volume;

		/** The minimum distance the sound will play at full volume. */
		float minDist;

		/** The maximum distance before the sound stops playing. */
		float maxDist;

		/** The pitch of the sound. */
		float pitch;

		/** The channel the sound is playing on. */
		uint8_t channel;

		/** Whether or not the sound has stopped playing. */
		bool hasStopped : 1;

		/** If the whole sound is not into memory. */
		bool isStreamed : 1;

		/** If the sound is spatialized (check origin).*/
		bool isSpatialized : 1;

		/** Volume is assigned if yes. */
		bool hasVolume : 1;

		/** minDist is assigned if yes. */
		bool hasDist : 1;

		/** pitch is assigned if yes. */
		bool hasPitch : 1;

	public:
		sound_t();
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
		};
	}
}