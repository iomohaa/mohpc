#pragma once

#include "../Global.h"
#include "../Vector.h"
#include <stdint.h>

namespace MOHPC
{
	class MOHPC_EXPORTS usercmd_t
	{
	public:
		uint32_t serverTime;
		uint8_t msec;
		uint16_t buttons;
		uint16_t angles[3];
		int8_t forwardmove, rightmove, upmove;

	public:
		usercmd_t();
	};

	class MOHPC_EXPORTS usereyes_t
	{
	public:
		int8_t ofs[3];
		float angles[2];

	public:
		usereyes_t();
	};

	static constexpr size_t GENTITYNUM_BITS = 10; // don't need to send any more
	static constexpr size_t	MAX_GENTITIES = (1 << GENTITYNUM_BITS);

	class MOHPC_EXPORTS playerState_t
	{
	public:
		static constexpr size_t MAX_STATS = 32;
		static constexpr size_t MAX_ACTIVEITEMS = 8;
		static constexpr size_t MAX_AMMO = 16;
		static constexpr size_t MAX_AMMO_AMOUNT = 16;
		static constexpr size_t MAX_MAX_AMMO_AMOUNT = 16;

	public:
		// cmd->serverTime of last executed command
		uint32_t	commandTime;
		uint8_t		pm_type;
		// for view bobbing and footstep generation
		uint8_t		bobCycle;
		// ducked, jump_held, etc
		uint16_t	pm_flags;
		uint16_t	pm_time;

		Vector		origin;
		Vector		velocity;

		uint16_t	gravity;
		uint16_t	speed;
		// add to command angles to get view direction
		// changed by spawns, rotating objects, and teleporters
		uint16_t	delta_angles[3];

		// ENTITYNUM_NONE = in air
		uint16_t	groundEntityNum;

		bool		walking;
		bool		groundPlane;
		uint8_t		feetfalling;
		uint32_t	radarInfo;
		Vector		falldir;
		//trace_t		groundTrace;

		// ranges from 0 to MAX_CLIENTS-1
		uint8_t		clientNum;

		// for fixed views
		Vector		viewangles;
		uint8_t		viewheight;

		float		fLeanAngle;
		uint8_t		iViewModelAnim;
		uint8_t		iViewModelAnimChanged;
		uint8_t		bVoted; // used in SH/BT

		uint16_t	stats[MAX_STATS];
		uint16_t	activeItems[MAX_ACTIVEITEMS];
		uint16_t	ammo_name_index[MAX_AMMO];
		uint16_t	ammo_amount[MAX_AMMO_AMOUNT];
		uint16_t	max_ammo_amount[MAX_MAX_AMMO_AMOUNT];

		uint8_t		current_music_mood;
		uint8_t		fallback_music_mood;
		float		music_volume;
		float		music_volume_fade_time;
		uint8_t		reverb_type;
		float		reverb_level;
		float		blend[4];
		float		fov;
		Vector		camera_origin, camera_angles;
		Vector		camera_offset, camera_posofs, damage_angles;
		float		camera_time;
		uint16_t	camera_flags;

	public:
		playerState_t();
	};

	struct MOHPC_EXPORTS trajectory_t {
		int trTime;
		Vector trDelta;

		trajectory_t();
	};

	struct MOHPC_EXPORTS frameInfo_t
	{
		int index;
		float time;
		float weight;

		frameInfo_t();
	};

	class MOHPC_EXPORTS entityState_t
	{
	public:
		static constexpr size_t MAX_FRAMEINFOS = 16;
		static constexpr size_t NUM_BONE_CONTROLLERS = 5;
		static constexpr size_t ENTITYNUM_NONE = (MAX_GENTITIES - 1);
		static constexpr size_t ENTITYNUM_WORLD = (MAX_GENTITIES - 2);
		static constexpr size_t ENTITYNUM_MAX_NORMAL = (MAX_GENTITIES - 2);

	public:
		// entity index
		uint16_t number;
		// entityType_t
		uint8_t eType;
		uint16_t eFlags;

		trajectory_t pos;

		Vector	netorigin;
		Vector	origin;
		Vector	origin2;

		Vector	netangles;
		Vector	angles;

		uint32_t constantLight;	// r + (g<<8) + (b<<16) + (intensity<<24)
		uint32_t loopSound;		// constantly loop this sound
		float loopSoundVolume;
		float loopSoundMinDist;
		float loopSoundMaxDist;
		float loopSoundPitch;
		uint8_t loopSoundFlags;

		uint16_t parent;
		uint16_t tag_num;

		bool	attach_use_angles;
		Vector	attach_offset;

		uint16_t beam_entnum;

		uint16_t modelindex;
		uint16_t usageIndex;
		uint16_t skinNum;
		uint16_t wasframe;
		frameInfo_t frameInfo[MAX_FRAMEINFOS];
		float actionWeight;

		uint8_t bone_tag[NUM_BONE_CONTROLLERS];
		Vector	bone_angles[NUM_BONE_CONTROLLERS];
		// not sent over
		quat_t	bone_quat[NUM_BONE_CONTROLLERS];
		uint8_t	surfaces[32];

		// 0 to (MAX_CLIENTS - 1), for players and corpses
		uint8_t clientNum;
		// -1 = in air
		uint16_t groundEntityNum;

		// for client side prediction, trap_linkentity sets this properly
		uint32_t solid;

		float	scale;
		float	alpha;
		int		renderfx;
		float	shader_data[2];
		float	shader_time;
		quat_t	quat;
		Vector	eyeVector;

	public:
		entityState_t();
	};
}