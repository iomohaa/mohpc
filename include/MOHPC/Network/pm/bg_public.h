/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// bg_public.h -- definitions shared by both the server game and client game modules

// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#pragma once

#include "../../Math.h"
#include "../InfoTypes.h"
#include "../../Utilities/Function.h"

namespace MOHPC
{
	static constexpr unsigned int DEFAULT_GRAVITY = 800;
	static constexpr unsigned int SCORE_NOT_PRESENT = -9999;	// for the CS_SCORES[12] when only one player is present

	static constexpr unsigned int MINS_Z = 0;
	static constexpr unsigned int MINS_X = -15;
	static constexpr unsigned int MINS_Y = -15;
	static constexpr unsigned int MAXS_X = 15;
	static constexpr unsigned int MAXS_Y = 15;
	static constexpr unsigned int MAXS_Z = 96;

	static constexpr unsigned int DEAD_MINS_Z			= 32;
	static constexpr unsigned int CROUCH_MAXS_Z			= 49;
	static constexpr unsigned int DEFAULT_VIEWHEIGHT	= 82;
	static constexpr unsigned int CROUCH_RUN_VIEWHEIGHT	= 64;
	static constexpr unsigned int JUMP_START_VIEWHEIGHT	= 52;
	static constexpr unsigned int CROUCH_VIEWHEIGHT		= 48;
	static constexpr unsigned int PRONE_VIEWHEIGHT		= 16;
	static constexpr unsigned int DEAD_VIEWHEIGHT		= 8;

	/*
	===================================================================================

	PMOVE MODULE

	The pmove code takes a player_state_t and a usercmd_t and generates a new player_state_t
	and some other output data.  Used for local prediction on the client game and true
	movement on the server game.
	===================================================================================
	*/

	static constexpr unsigned int MAX_CLIP_PLANES			= 5;
	// can't walk on very steep slopes
	static constexpr float MIN_WALK_NORMAL					= 0.7f;

	static constexpr unsigned int STEPSIZE					= 18;

	static constexpr float WATER_TURBO_SPEED				= 1.35f;
	static constexpr unsigned int WATER_TURBO_TIME			= 1200;
	static constexpr unsigned int MINIMUM_RUNNING_TIME		= 800;
	static constexpr unsigned int MINIMUM_WATER_FOR_TURBO	= 90;

	static constexpr float OVERCLIP							= 1.001f;

	typedef enum {
		EV_NONE,

		EV_FALL_SHORT,
		EV_FALL_MEDIUM,
		EV_FALL_FAR,
		EV_FALL_FATAL,
		EV_TERMINAL_VELOCITY,

		EV_WATER_TOUCH,   // foot touches
		EV_WATER_LEAVE,   // foot leaves
		EV_WATER_UNDER,   // head touches
		EV_WATER_CLEAR,   // head leaves

		EV_LAST_PREDICTED      // just a marker point

		// events generated by non-players or never predicted
	} entity_event_t;

	// moveposflags
	static constexpr unsigned int MPF_POSITION_STANDING		= (1 << 0);
	static constexpr unsigned int MPF_POSITION_CROUCHING	= (1 << 1);
	static constexpr unsigned int MPF_POSITION_PRONE		= (1 << 2);
	static constexpr unsigned int MPF_POSITION_OFFGROUND	= (1 << 3);

	static constexpr unsigned int MPF_MOVEMENT_WALKING		= (1 << 4);
	static constexpr unsigned int MPF_MOVEMENT_RUNNING		= (1 << 5);
	static constexpr unsigned int MPF_MOVEMENT_FALLING		= (1 << 6);

	static constexpr unsigned int MAXTOUCH = 32;

	// nothing blocking
	static constexpr unsigned int MOVERESULT_NONE		= 0;
	// move blocked, but player turned to avoid it
	static constexpr unsigned int MOVERESULT_TURNED		= 1;
	// move blocked by slope or wall
	static constexpr unsigned int MOVERESULT_BLOCKED	= 2;
	// player ran into wall
	static constexpr unsigned int MOVERESULT_HITWALL	= 3;

	static constexpr unsigned int SOLID_BMODEL = 0xffffff;

	class playerState_t;

	using TraceFunction = Function<void(trace_t* results, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, uintptr_t passEntityNum, uintptr_t contentMask, bool capsule, bool traceDeep)>;
	using PointContentsFunction = Function<uint32_t(const Vector& point, uintptr_t passEntityNum)>;

	void stubTrace(trace_t* results, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, uintptr_t passEntityNum, uintptr_t contentMask, bool capsule, bool traceDeep);
	int stubPointContents(const Vector& point, uintptr_t passEntityNum);

	struct pmove_t
	{
		// state (in / out)
		playerState_t* ps;

		// command (in)
		usercmd_t cmd;
		// collide against these types of surfaces
		int tracemask;
		// if set, diagnostic output will be printed
		int debugLevel;
		// if the game is setup for no footsteps by the server
		bool noFootsteps;
		bool canLean;

		int framecount;

		// results (out)
		int numtouch;
		int touchents[MAXTOUCH];

		// indicates whether 2the player's movement was blocked and how
		int moveresult;
		bool stepped;

		// events predicted on client side
		int pmoveEvent;
		// bounding box size
		Vector mins, maxs;

		int watertype;
		int waterlevel;

		float xyspeed;

		// for fixed msec Pmove
		int pmove_fixed;
		int pmove_msec;

		// callbacks to test the world
		// these will be different functions during game and cgame
		TraceFunction trace;
		PointContentsFunction pointcontents;

	public:
		pmove_t();
	};

	// all of the locals will be zeroed before each
	// pmove, just to make damn sure we don't have
	// any differences when running on client or server
	struct pml_t
	{
		Vector forward, left, up;
		Vector flat_forward, flat_left, flat_up;
		float frametime;

		int msec;

		bool walking;
		bool groundPlane;
		trace_t groundTrace;

		float impactSpeed;

		Vector previous_origin;
		Vector previous_velocity;
		int previous_waterlevel;

	public:
		pml_t();
	};

	// FIXME: Make it an abstract interface for versions
	class Pmove
	{
	public:
		pmove_t pm;
		pml_t pml;
		int c_pmove;

	public:
		MOHPC_EXPORTS Pmove();

		MOHPC_EXPORTS pmove_t& get();

		// if a full pmove isn't done on the client, you can just update the angles
		void PM_GetMove(float* pfForward, float* pfRight);
		static void PM_UpdateViewAngles(playerState_t* ps, const usercmd_t* cmd);
		void move_GroundTrace();
		MOHPC_EXPORTS void move();
		void moveAdjustAngleSettings(Vector& vViewAngles, Vector& vAngles, playerState_t* pPlayerState, entityState_t* pEntState);
		void moveAdjustAngleSettings_Client(Vector& vViewAngles, Vector& vAngles, playerState_t* pPlayerState, entityState_t* pEntState);

	private:
		void PM_AirMove();
		bool PM_FeetOnGround(const Vector& pos);
		bool PM_FindBestFallPos(const Vector& pos, Vector& bestdir);
		void PM_CheckFeet(const Vector& vWishdir);
		void PM_WalkMove();
		void PM_DeadMove();
		void PM_NoclipMove();
		void PM_CrashLand();
		int PM_CorrectAllSolid();
		void PM_GroundTrace();
		void PM_SetWaterLevel();
		void PM_CheckDuck();
		void PM_Footsteps();
		void PM_WaterEvents();
		void PM_DropTimers();
		void moveSingle();
		void moveAdjustViewAngleSettings_OnLadder(Vector& vViewAngles, Vector& vAngles, playerState_t* pPlayerState, entityState_t* pEntState);
		bool PM_SlideMove(bool gravity);
		void PM_StepSlideMove(bool gravity);
		void PM_AddEvent(int newEvent);
		void PM_ClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce);
		void PM_AddTouchEnt(int entityNum);
		void PM_Friction(void);
		void PM_Accelerate(const Vector& wishdir, float wishspeed, float accel);
		float PM_CmdScale(usercmd_t* cmd);
		void PM_CheckTerminalVelocity();
	};

	//===================================================================================

	// means of death
	typedef enum {
		MOD_NONE,
		MOD_SUICIDE,
		MOD_CRUSH,
		MOD_CRUSH_EVERY_FRAME,
		MOD_TELEFRAG,
		MOD_LAVA,
		MOD_SLIME,
		MOD_FALLING,
		MOD_LAST_SELF_INFLICTED,
		MOD_EXPLOSION,
		MOD_EXPLODEWALL,
		MOD_ELECTRIC,
		MOD_ELECTRICWATER,
		MOD_THROWNOBJECT,
		MOD_GRENADE,
		MOD_BEAM,
		MOD_ROCKET,
		MOD_IMPACT,
		MOD_BULLET,
		MOD_FAST_BULLET,
		MOD_VEHICLE,
		MOD_FIRE,
		MOD_FLASHBANG,
		MOD_ON_FIRE,
		MOD_GIB,
		MOD_IMPALE,
		MOD_BASH,
		MOD_SHOTGUN,
		MOD_TOTAL_NUMBER
	} meansOfDeath_t;

	//static constexpr unsigned int DM_FLAG = (flag)(g_gametype->integer && ((int)dmflags->integer & (flag)));

	/*
	// content masks
	static constexpr unsigned int MASK_ALL = (-1);
	static constexpr unsigned int MASK_SOLID = (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_BBOX);
	static constexpr unsigned int MASK_USABLE = (CONTENTS_SOLID | CONTENTS_BODY);
	static constexpr unsigned int MASK_PLAYERSOLID = (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY | CONTENTS_TRIGGER | CONTENTS_FENCE | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_NOTTEAM2);
	static constexpr unsigned int MASK_DEADSOLID = (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_CORPSE | CONTENTS_NOTTEAM2 | CONTENTS_FENCE);
	static constexpr unsigned int MASK_MONSTERSOLID = (CONTENTS_SOLID | CONTENTS_MONSTERCLIP | CONTENTS_BODY);
	static constexpr unsigned int MASK_WATER = (CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME);
	static constexpr unsigned int MASK_OPAQUE = (CONTENTS_SOLID | CONTENTS_SLIME | CONTENTS_LAVA);
	static constexpr unsigned int MASK_SHOT = (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_BODY | CONTENTS_FENCE | CONTENTS_WEAPONCLIP | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_WATER | CONTENTS_NOTTEAM1 | CONTENTS_NOTTEAM2);
	static constexpr unsigned int MASK_PROJECTILE = (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_TRIGGER | CONTENTS_FENCE | CONTENTS_WEAPONCLIP | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_BBOX);
	static constexpr unsigned int MASK_MELEE = (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_TRIGGER | CONTENTS_WEAPONCLIP | CONTENTS_FENCE | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_BBOX | CONTENTS_NOTTEAM1);
	static constexpr unsigned int MASK_PATHSOLID = (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_BODY | CONTENTS_FENCE | CONTENTS_UNKNOWN2 | CONTENTS_BBOX | CONTENTS_MONSTERCLIP);
	static constexpr unsigned int MASK_CAMERASOLID = (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY | MASK_WATER);
	static constexpr unsigned int MASK_BEAM = (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_PLAYERCLIP | CONTENTS_BODY | CONTENTS_FENCE);
	static constexpr unsigned int MASK_LADDER = (CONTENTS_SOLID | CONTENTS_LADDER | CONTENTS_TRIGGER | CONTENTS_PLAYERCLIP | CONTENTS_BODY | CONTENTS_FENCE);
	static constexpr unsigned int MASK_AUTOCALCLIFE = (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_FENCE);
	static constexpr unsigned int MASK_EXPLOSION = (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_WEAPONCLIP);
	static constexpr unsigned int MASK_SOUND = (CONTENTS_SOLID | CONTENTS_TRANSLUCENT);
	static constexpr unsigned int MASK_VEHICLE = (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_VEHICLECLIP | CONTENTS_FENCE);
	static constexpr unsigned int MASK_CLICK = (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_UNKNOWN3 | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_BBOX);
	static constexpr unsigned int MASK_CANSEE = (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_WEAPONCLIP | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_BBOX);
	static constexpr unsigned int MASK_ITEM = (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_PLAYERCLIP | CONTENTS_FENCE);
	static constexpr unsigned int MASK_TRANSITION = (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_UNKNOWN2 | CONTENTS_NOTTEAM1 | CONTENTS_WEAPONCLIP);
	static constexpr unsigned int MASK_TARGETPATH = (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_MONSTERCLIP | CONTENTS_FENCE | CONTENTS_UNKNOWN2 | CONTENTS_BBOX);
	static constexpr unsigned int MASK_LINE = (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_BBOX);
	static constexpr unsigned int MASK_VEHICLETURRET = (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_FENCE | CONTENTS_VEHICLECLIP | CONTENTS_MONSTERCLIP | CONTENTS_PLAYERCLIP);
	*/

	void BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, bool snap );
	void BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, bool snap );
};
