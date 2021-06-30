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
// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate

#include <MOHPC/Network/pm/bg_public.h>
#include <MOHPC/Network/Types/Entity.h>
#include <MOHPC/Network/Types/PlayerState.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>

#include "bg_local.h"

using namespace MOHPC;
using namespace MOHPC::Network;

#define PITCH 0
#define YAW 1
#define ROLL 2

// movement parameters
float Network::pm_stopspeed = 50.0f;
float Network::pm_duckScale = 0.25f;
float Network::pm_swimScale = 1.0f;
float Network::pm_wadeScale = 0.70f;

float Network::pm_accelerate = 8.0f;
float Network::pm_airaccelerate = 1.0f;
float Network::pm_wateraccelerate = 8.0f;

float Network::pm_friction = 6.0f;
float Network::pm_waterfriction = 2.0f;
float pm_slipperyfriction = 0.25f;
float pm_strafespeed = 0.85f;
float pm_backspeed = 0.80f;
float Network::pm_flightfriction = 3.0f;
float PM_NOCLIPfriction = 5.0f;

pmove_t::pmove_t()
	: ps(nullptr)
	, tracemask(0)
	, debugLevel(0)
	, noFootsteps(false)
	, framecount(0)
	, numtouch(0)
	, touchents{ 0 }
	, moveresult(0)
	, stepped(false)
	, pmoveEvent(0)
	, mins(), maxs()
	, watertype(0)
	, waterlevel(0)
	, xyspeed(0)
	, pmove_fixed(0)
	, pmove_msec(0)
	, traceInterface(PmoveNoTrace)
{

}

pml_t::pml_t()
	: forward(), left(), up()
	, flat_forward(), flat_left(), flat_up()
	, frametime(0)
	, msec(0)
	, walking(false)
	, groundPlane(false)
	, impactSpeed(0)
	, previous_origin()
	, previous_velocity()
	, previous_waterlevel(0)
{
}

Pmove::Pmove()
	: c_pmove(0)
	, canLeanWhileMoving(false)
	, clearLeanOnExit(true)
{
}

Pmove::~Pmove()
{
}

pmove_t& Pmove::get()
{
	return pm;
}

/*
===============
PM_AddEvent

===============
*/
void Pmove::PM_AddEvent( int newEvent ) {

}

/*
===============
PM_AddTouchEnt
===============
*/
void Pmove::PM_AddTouchEnt( int entityNum ) {
	int		i;

	if ( entityNum == ENTITYNUM_WORLD ) {
		return;
	}

	if ( pm.numtouch == MAXTOUCH ) {
		return;
	}

	// see if it is already added
	for ( i = 0 ; i < pm.numtouch ; i++ ) {
		if ( pm.touchents[ i ] == entityNum ) {
			return;
		}
	}

	// add it
	pm.touchents[pm.numtouch] = entityNum;
	pm.numtouch++;
}

/*
===================
PM_StartTorsoAnim
===================
*/
static void PM_StartTorsoAnim( int anim ) {

}
static void PM_StartLegsAnim( int anim ) {

}

static void PM_ContinueLegsAnim( int anim ) {

}

static void PM_ContinueTorsoAnim( int anim ) {
	
}

static void PM_ForceLegsAnim( int anim ) {
	
}


/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void Pmove::PM_ClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce)
{
	float	backoff;
	float	dir_z;
	float	normal2[ 3 ];

	if( normal[ 2 ] >= pm_wadeScale )
	{
		if( in[ 0 ] == 0.0f && in[ 1 ] == 0.0f )
		{
			VectorClear( out );
			return;
		}

		normal2[ 0 ] = in[ 0 ] * DotProduct2D( in, normal );
		normal2[ 1 ] = in[ 1 ] * DotProduct2D( in, normal );
		normal2[ 2 ] = normal[ 2 ] * DotProduct2D( in, in );

		VectorNormalize( normal2 );

		dir_z = -normal2[ 2 ];

		out[ 0 ] = in[ 0 ];
		out[ 1 ] = in[ 1 ];
		out[ 2 ] = DotProduct2D( in, normal2 ) / dir_z;
	}
	else
	{
		backoff = DotProduct( in, normal );

		if( backoff < 0 )
			backoff *= overbounce;
		else
			backoff /= overbounce;

		out[ 0 ] = in[ 0 ] - normal[ 0 ] * backoff;
		out[ 1 ] = in[ 1 ] - normal[ 1 ] * backoff;
		out[ 2 ] = in[ 2 ] - normal[ 2 ] * backoff;
	}
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
void Pmove::PM_Friction( void ) {
	Vector	vec;
	float	*vel;
	float	speed, newspeed, control;
	float	drop;

	vel = pm.ps->velocity;

	VecCopy( vel, vec );
	if( pml.walking )
	{
		// ignore slope movement
		vec[ 2 ] = 0;
	}

	speed = VectorLength( vec );
	if( speed < 1 )
	{
		// allow sinking underwater
		vel[ 0 ] = 0;
		vel[ 1 ] = 0;

		return;
	}

	drop = 0;

	if( pml.walking )
	{
		control = ( speed < pm_stopspeed ) ? pm_stopspeed : speed;

		// if getting knocked back, no friction
		if( pml.groundTrace.surfaceFlags & SURF_SLICK )
		{
			drop += control * pm_slipperyfriction * pml.frametime;
		}
		else
		{
			drop += control * pm_friction * pml.frametime;
		}
	}

	// apply water friction even if just wading
	if( pm.waterlevel )
	{
		if( pm.watertype & CONTENTS_SLIME )
		{
			drop += speed * pm_waterfriction * 5 * pm.waterlevel * pml.frametime;
		}
		else
		{
			drop += speed * pm_waterfriction * pm.waterlevel * pml.frametime;
		}
	}

	// scale the velocity
	newspeed = speed - drop;
	if( newspeed < 0 )
	{
		newspeed = 0;
	}

	newspeed /= speed;

	vel[ 0 ] = vel[ 0 ] * newspeed;
	vel[ 1 ] = vel[ 1 ] * newspeed;
	vel[ 2 ] = vel[ 2 ] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
void Pmove::PM_Accelerate(const Vector& wishdir, float wishspeed, float accel)
{
	Vector		wishVelocity;
	Vector		pushDir;
	float		pushLen;
	float		canPush;

	VectorScale( wishdir, wishspeed, wishVelocity );
	VecSubtract( wishVelocity, pm.ps->velocity, pushDir );
	pushLen = VectorNormalize( pushDir );

	canPush = accel*pml.frametime*wishspeed;
	if (canPush > pushLen) {
		canPush = pushLen;
	}

	VectorMA( pm.ps->velocity, canPush, pushDir, pm.ps->velocity );
}



/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrtf(2) distortion in speed.
============
*/
float Pmove::PM_CmdScale( usercmd_t *cmd )
{
	float fmove, smove;
	PM_GetMove(&fmove, &smove);
	const float smovepos = abs(smove);

	int max = (int)abs(fmove);
	if (smovepos > max) {
		max = (int)smovepos;
	}

	const int upmove = abs(cmd->upmove);
	if (upmove > max) {
		max = upmove;
	}

	if (!max) {
		return 0;
	}

	const double total = sqrt(double(
			cmd->forwardmove * cmd->forwardmove
			+ cmd->rightmove * cmd->rightmove
			+ cmd->upmove * cmd->upmove
		));

	return float(double(pm.ps->speed * max) / (127.0 * total));
}

//============================================================================

/*
=============
PM_CheckTerminalVelocity
=============
*/
#define TERMINAL_VELOCITY 1200
void Pmove::PM_CheckTerminalVelocity()
{
	float oldspeed;
	float speed;

	//
	// how fast were we falling
	//
	oldspeed = -pml.previous_velocity[ 2 ];

	//
	// how fast are we falling
	//
	speed = -pm.ps->velocity[ 2 ];

	if( speed <= 0 )
	{
		return;
	}

	if( ( oldspeed <= TERMINAL_VELOCITY ) && ( speed > TERMINAL_VELOCITY ) )
	{
		pm.pmoveEvent = EV_TERMINAL_VELOCITY;
	}
}

/*
{
	int			i;
	Vector		wishvel;
	float		fmove, smove;
	Vector		wishdir;
	float		wishspeed;
	float		scale;
	usercmd_t	cmd;

	PM_Friction();

	fmove = pm.cmd.forwardmove;
	smove = pm.cmd.rightmove;

	cmd = pm.cmd;
	scale = PM_CmdScale( &cmd );

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[ 2 ] = 0;
	pml.right[ 2 ] = 0;
	VectorNormalize( pml.forward );
	VectorNormalize( pml.right );

	for( i = 0; i < 2; i++ ) {
		wishvel[ i ] = pml.forward[ i ] * fmove + pml.right[ i ] * smove;
	}
	wishvel[ 2 ] = 0;

	VecCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	// not on ground, so little effect on velocity
	PM_Accelerate( wishdir, wishspeed, pm_airaccelerate );

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if( pml.groundPlane ) {
		PM_ClipVelocity( pm.ps->velocity, pml.groundTrace.plane.normal,
			pm.ps->velocity, OVERCLIP );
	}

	PM_StepSlideMove( true );
}
*/

/*
===================
PM_GetMove

===================
*/
void Pmove::PM_GetMove
	(
	float *pfForward,
	float *pfRight
	)

{
	*pfForward = pm.cmd.forwardmove;
	if( *pfForward < 0 )
	{
		*pfForward *= pm_backspeed;
	}
	*pfRight = pm.cmd.rightmove * pm_strafespeed;
}

/*
===================
PM_AirMove

===================
*/
void Pmove::PM_AirMove( void )
{
	Vector		wishvel;
	float		   fmove;
	float       smove;
	Vector		wishdir;
	float		   wishspeed;
	float		   scale;
	usercmd_t	cmd;

	PM_GetMove( &fmove, &smove );

	pm.ps->pm_time = 0;

	cmd = pm.cmd;
	scale = PM_CmdScale( &cmd );

	wishvel[ 0 ] = pml.flat_forward[ 0 ] * fmove - pml.flat_left[ 0 ] * smove;
	wishvel[ 1 ] = pml.flat_forward[ 1 ] * fmove - pml.flat_left[ 1 ] * smove;
	wishvel[ 2 ] = 0;

	VecCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	// not on ground, so little effect on velocity
	PM_Accelerate( wishdir, wishspeed, pm_airaccelerate );

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if( pml.groundPlane )
	{
		PM_ClipVelocity( pm.ps->velocity, pml.groundTrace.plane.normal, pm.ps->velocity, OVERCLIP );
	}

	PM_StepSlideMove( true );

	PM_CheckTerminalVelocity();
}

static Vector min4x4 = { -8, 0, 0 };
static Vector max4x4 = { 4, 4, 8 };
static Vector base_rightfoot_pos = { -5.25301f, -3.10885f, 0 };
static Vector base_leftfoot_pos = { -0.123711f, 10.4893f, 0 };

bool Pmove::PM_FeetOnGround(const Vector& pos)
{
	Vector start;
	Vector end;
	trace_t trace;

	VecCopy(pos, start);
	VecCopy(pos, end);
	// Some fixes not present in Quake3/OpenMOHAA
	end[2] -= 16.1f;

	pm.traceInterface->trace( &trace, start, min4x4, max4x4, end, pm.ps->clientNum, pm.tracemask, true, false );

	return trace.fraction != 1.0f;
}

bool Pmove::PM_FindBestFallPos(const Vector& pos, Vector& bestdir)
{ 
	trace_t trace;
	Vector ang;
	Vector dir;
	Vector start;
	Vector end;
	Vector move;
	int i;
	bool set;
	float radius;

	VectorClear( bestdir );

	set = false;

	radius = pm.maxs[ 0 ] - pm.mins[ 0 ] + 1.0f;

	VecCopy( pos, start );
	start[ 2 ] -= 16.1f;

	VecSet( ang, 0, pm.ps->viewangles[ 1 ], 0 );
	for( i = 0; i < 16; i++, ang[ 1 ] += 22.5f )
	{
		AngleVectorsLeft( ang, dir, NULL, NULL );
		VectorMA( pos, radius, dir, move );

		pm.traceInterface->trace( &trace, pos, pm.mins, pm.maxs, move, pm.ps->clientNum, pm.tracemask, true, false );

		VecCopy( trace.endpos, end );
		end[ 2 ] = start[ 2 ];

		pm.traceInterface->trace( &trace, trace.endpos, pm.mins, pm.maxs, end, pm.ps->clientNum, pm.tracemask, true, false );
		if( trace.fraction == 1.0f )
		{
			VecCopy( trace.endpos, end );
			pm.traceInterface->trace( &trace, end, pm.mins, pm.maxs, start, pm.ps->clientNum, pm.tracemask, true, false );

			if( trace.fraction < 1.0f )
			{
				VecAdd( bestdir, trace.plane.normal, bestdir );
				set = true;
			}
		}
	}

	if( !set || !VectorNormalize( bestdir ) )
	{
		return false;
	}

	return true;
}

void Pmove::PM_CheckFeet(const Vector& vWishdir)
{
	Vector		temp;
	trace_t		trace;

	if( pm.stepped )
	{
		pm.ps->feetfalling = 0;
		return;
	}

	if( !pm.ps->walking )
	{
		return;
	}

	VectorMA( pm.ps->origin, 0.2f, pm.ps->velocity, temp );
	temp[ 2 ] = pm.ps->origin[ 2 ] + 2;
	if( PM_FeetOnGround( pm.ps->origin ) || PM_FeetOnGround( temp ) )
	{
		pm.ps->feetfalling = 0;
		return;
	}

	if( pm.ps->feetfalling > 0 )
	{
		pm.ps->feetfalling--;
	}

	if( !pm.ps->feetfalling )
	{
		if( !PM_FindBestFallPos( pm.ps->origin, pm.ps->falldir ) ) {
			return;
		}

		pm.ps->feetfalling = 5;
	}

	VectorMA( pm.ps->origin, 15.0f * pml.frametime, pm.ps->falldir, temp );

	pm.traceInterface->trace( &trace, pm.ps->origin, pm.mins, pm.maxs, temp, pm.ps->clientNum, pm.tracemask, true, false );
	if( trace.fraction == 0 )
	{
		pm.ps->feetfalling = 0;
		return;
	}

	if( ( vWishdir[ 0 ] == 0.0f && vWishdir[ 1 ] == 0.0f ) ||
		DotProduct( vWishdir, pm.ps->falldir ) > 0.0f )
	{
		pm.ps->walking = false;
		VecCopy( trace.endpos, pm.ps->origin );
	}
}

/*
===================
PM_WalkMove

===================
*/
void Pmove::PM_WalkMove( void ) {
	int			i;
	Vector		wishvel;
	float		fmove, smove;
	Vector		wishdir;
	float		wishspeed;
	float		scale;
	usercmd_t	cmd;
	float		accelerate;

	PM_Friction();

	PM_GetMove( &fmove, &smove );

	cmd = pm.cmd;
	scale = PM_CmdScale( &cmd );

	if( ( pm.cmd.buttons.flags & BUTTON_RUN ) && fmove && !smove )
	{
		pm.ps->pm_time += pml.msec;
	}
	else
	{
		pm.ps->pm_time = 0;
	}

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity( pml.flat_forward, pml.groundTrace.plane.normal, pml.flat_forward, OVERCLIP );
	PM_ClipVelocity( pml.flat_left, pml.groundTrace.plane.normal, pml.flat_left, OVERCLIP );
	//
	VectorNormalize( pml.flat_forward );
	VectorNormalize( pml.flat_left );

	for( i = 0; i < 3; i++ )
	{
		wishvel[ i ] = pml.flat_forward[ i ] * fmove - pml.flat_left[ i ] * smove;
	}

	VecCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	// clamp the speed lower if wading or walking on the bottom
	if( pm.waterlevel )
	{
		float	waterScale;

		if( pm.waterlevel == 1.0f )
		{
			waterScale = 0.80f;
		}
		else
		{
			waterScale = 0.5f;
		}

		if( wishspeed > pm.ps->speed * waterScale ) {
			wishspeed = pm.ps->speed * waterScale;
		}
	}

	if( pml.groundTrace.surfaceFlags & SURF_SLICK ) {
		accelerate = pm_airaccelerate;
	} else {
		accelerate = pm_accelerate;
	}

	PM_Accelerate( wishdir, wishspeed, accelerate );

	if( pml.groundTrace.surfaceFlags & SURF_SLICK ) {
		pm.ps->velocity[ 2 ] -= pm.ps->gravity * pml.frametime;
	}

	// slide along the ground plane
	PM_ClipVelocity( pm.ps->velocity, pml.groundTrace.plane.normal,
		pm.ps->velocity, OVERCLIP );

	// don't do anything if standing still
	if( pm.ps->velocity[ 0 ] || pm.ps->velocity[ 1 ] )
	{
		PM_StepSlideMove( true );
	}

	PM_CheckFeet( wishdir );
}

/*
==============
PM_DeadMove
==============
*/
void Pmove::PM_DeadMove( void ) {
	float	forward;

	if ( !pml.walking ) {
		return;
	}

	// extra friction

	forward = VectorLength (pm.ps->velocity);
	forward -= 20;
	if ( forward <= 0 ) {
		VectorClear (pm.ps->velocity);
	} else {
		VectorNormalize (pm.ps->velocity);
		VectorScale (pm.ps->velocity, forward, pm.ps->velocity);
	}
}


/*
===============
PM_NoclipMove
===============
*/
void Pmove::PM_NoclipMove()
{
	float	   speed;
	float    drop;
	float    friction;
	float    control;
	float    newspeed;
	int		i;
	Vector	wishvel;
	float		fmove;
	float    smove;
	Vector	wishdir;
	float		wishspeed;
	float		scale;

	pm.ps->viewheight = DEFAULT_VIEWHEIGHT;
	pm.ps->groundEntityNum = ENTITYNUM_NONE;

	// friction

	speed = VectorLength( pm.ps->velocity );
	if( speed < 1 )
	{
		VecCopy( vec3_origin, pm.ps->velocity );
	}
	else
	{
		drop = 0;

		// extra friction
		friction = pm_friction * 1.5f;

		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control * friction * pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if( newspeed < 0 )
		{
			newspeed = 0;
		}
		newspeed /= speed;

		VectorScale( pm.ps->velocity, newspeed, pm.ps->velocity );
	}

	// accelerate
	// allow the player to move twice as fast in noclip
	scale = PM_CmdScale( &pm.cmd ) * 2;

	PM_GetMove( &fmove, &smove );

	pm.ps->pm_time = 0;

	for( i = 0; i < 3; i++ )
	{
		wishvel[ i ] = pml.flat_forward[ i ] * fmove - pml.flat_left[ i ] * smove;
	}

	wishvel[ 2 ] += pm.cmd.upmove;

	VecCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm_accelerate );

	// move
	VectorMA( pm.ps->origin, pml.frametime, pm.ps->velocity, pm.ps->origin );
}

//============================================================================

/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/
void Pmove::PM_CrashLand( void )
{
	float delta;
	float	dist;
	float	vel;
	float	acc;
	float	t;
	float	a, b, c, den;

	// calculate the exact velocity on landing
	dist = pm.ps->origin[ 2 ] - pml.previous_origin[ 2 ];
	vel = pml.previous_velocity[ 2 ];
	acc = (float)-pm.ps->gravity;

	a = acc / 2;
	b = vel;
	c = -dist;

	den = b * b - 4 * a * c;
	if( den < 0 )
	{
		return;
	}

	//t = ( -b - sqrtf( den ) ) / ( 2 * a );
	t = sqrtf( den ) + vel;

	//delta = vel + t * acc;
	delta = vel - t;
	delta = delta * delta * 0.0001f;

	// reduce falling damage if there is standing water
	if( pm.waterlevel == 2 )
	{
		delta *= 0.25f;
	}

	if( pm.waterlevel == 1 )
	{
		delta *= 0.5f;
	}

	if( delta < 1 )
	{
		return;
	}

	// SURF_NODAMAGE is used for bounce pads where you don't ever
	// want to take damage or play a crunch sound
	if( !( pml.groundTrace.surfaceFlags & SURF_NODAMAGE ) )
	{
		if( delta > 100 )
		{
			pm.pmoveEvent = EV_FALL_FATAL;
		}
		else if( delta > 80 )
		{
			pm.pmoveEvent = EV_FALL_FAR;
		}
		else if( delta > 40 )
		{
			pm.pmoveEvent = EV_FALL_MEDIUM;
		}
		else if( delta > 20 )
		{
			pm.pmoveEvent = EV_FALL_SHORT;
		}
	}
}

/*
=============
PM_CheckStuck
=============
*/
/*
void PM_CheckStuck(void) {
	trace_t trace;

	pm.traceInterface->trace (&trace, pm.ps->origin, pm.mins, pm.maxs, pm.ps->origin, pm.ps->clientNum, pm.tracemask);
	if (trace.allsolid) {
		//int shit = true;
	}
}
*/

/*
=============
PM_CorrectAllSolid
=============
*/
// Some code has not been fixed in OpenMOHAA
int Pmove::PM_CorrectAllSolid()
{
	int			i, j, k;
	Vector		point, point2;
	trace_t		trace, trace2;

	if ( pm.debugLevel ) {
		//Com_Printf("%i:allsolid\n", c_pmove);
	}

	// jitter around
	for (i = -1; i <= 1; i++) {
		for (j = -1; j <= 1; j++) {
			for (k = -1; k <= 1; k++) {
				VecCopy(pm.ps->origin, point);
				point[0] += (float) i;
				point[1] += (float) j;
				point[2] += (float) k;
				pm.traceInterface->trace( &trace, point, pm.mins, pm.maxs, point, pm.ps->clientNum, pm.tracemask, true, false );
				if ( !trace.allsolid && !trace.startsolid ) {
					pm.ps->origin[0] = point[0];
					pm.ps->origin[1] = point[1];
					pm.ps->origin[2] = point[2];
					point2 = point;
					point2[2] -= 0.25f;

					pm.traceInterface->trace( &trace2, point, pm.mins, pm.maxs, point2, pm.ps->clientNum, pm.tracemask, true, false );
					pml.groundTrace = trace2;
					pm.ps->groundTrace = trace2;
					return true;
				}
			}
		}
	}

	//pm.ps->groundEntityNum = ENTITYNUM_NONE;
	//pml.groundPlane = false;
	//pml.walking = false;

	return false;
}

/*
=============
PM_GroundTrace
=============
*/
void Pmove::PM_GroundTrace( void ) {
	Vector		point;
	trace_t		trace;

	point[ 0 ] = pm.ps->origin[ 0 ];
	point[ 1 ] = pm.ps->origin[ 1 ];
	point[ 2 ] = pm.ps->origin[ 2 ] - 0.25f;

	pm.traceInterface->trace( &trace, pm.ps->origin, pm.mins, pm.maxs, point, pm.ps->clientNum, pm.tracemask, true, false );

	pml.groundTrace = trace;
	pm.ps->groundTrace = trace;

	// do something corrective if the trace starts in a solid...
	if ( trace.allsolid || trace.startsolid )
	{
		if( !PM_CorrectAllSolid() ) {
			trace.fraction = 1.0f;
		}
	}

	// if the trace didn't hit anything, we are in free fall
	if ( trace.fraction == 1.0 )
	{
		pm.ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = false;
		pml.walking = false;

		pm.ps->walking = pml.walking;
		pm.ps->groundPlane = pml.groundPlane;
		return;
	}

	// check if getting thrown off the ground
	if( pm.ps->velocity[ 2 ] > 0.0f && DotProduct( pm.ps->velocity, trace.plane.normal ) > 150.0f )
	{
		if ( pm.debugLevel ) {
			//Com_Printf("%i:kickoff\n", c_pmove);
		}

		pm.ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = false;
		pml.walking = false;

		pm.ps->walking = pml.walking;
		pm.ps->groundPlane = pml.groundPlane;
		return;
	}

	// slopes that are too steep will not be considered onground
	if( trace.plane.normal[ 2 ] < MIN_WALK_NORMAL )
	{
		Vector oldvel;
		float d;

		if ( pm.debugLevel ) {
			//Com_Printf("%i:steep\n", c_pmove);
		}

		VecCopy( pm.ps->velocity, oldvel );
		VecSet( pm.ps->velocity, 0, 0, -1.0f / pml.frametime );
		PM_SlideMove( false );

		d = VectorLength( pm.ps->velocity );
		VecCopy( oldvel, pm.ps->velocity );

		if( d > ( 0.1f / pml.frametime ) )
		{
			pm.ps->groundEntityNum = ENTITYNUM_NONE;
			pml.groundPlane = true;
			pml.walking = false;

			pm.ps->walking = pml.walking;
			pm.ps->groundPlane = pml.groundPlane;
			return;
		}
	}

	pml.groundPlane = true;
	pml.walking = true;

	if ( pm.ps->groundEntityNum == ENTITYNUM_NONE )
	{
		// just hit the ground
		if ( pm.debugLevel ) {
			//Com_Printf( "%i:Land\n", c_pmove );
		}

		PM_CrashLand();
	}

	pm.ps->groundEntityNum = trace.entityNum;

	PM_AddTouchEnt( trace.entityNum );

	pm.ps->walking = pml.walking;
	pm.ps->groundPlane = pml.groundPlane;
}


/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
void Pmove::PM_SetWaterLevel( void ) {
	Vector		point;
	int			cont;
	int			sample1;
	int			sample2;

	//
	// get waterlevel, accounting for ducking
	//
	pm.waterlevel = 0;
	pm.watertype = 0;

	point[0] = pm.ps->origin[0];
	point[1] = pm.ps->origin[1];
	point[2] = pm.ps->origin[2] + MINS_Z + 1;
	cont = pm.traceInterface->pointContents( point, pm.ps->clientNum );

	if ( cont & MASK_WATER ) {
		sample2 = pm.ps->viewheight - MINS_Z;
		sample1 = sample2 / 2;

		pm.watertype = cont;
		pm.waterlevel = 1;
		point[2] = pm.ps->origin[2] + MINS_Z + sample1;
		cont = pm.traceInterface->pointContents (point, pm.ps->clientNum );
		if ( cont & MASK_WATER ) {
			pm.waterlevel = 2;
			point[2] = pm.ps->origin[2] + MINS_Z + sample2;
			cont = pm.traceInterface->pointContents (point, pm.ps->clientNum );
			if ( cont & MASK_WATER ){
				pm.waterlevel = 3;
			}
		}
	}

}

/*
==============
PM_CheckDuck

Sets mins, maxs, and pm.ps->viewheight
==============
*/
void Pmove::PM_CheckDuck( void )
{
	pm.mins[ 0 ] = -15.0f;
	pm.mins[ 1 ] = -15.0f;

	pm.maxs[ 0 ] = 15.0f;
	pm.maxs[ 1 ] = 15.0f;

	pm.mins[ 2 ] = MINS_Z;

	if( pm.ps->pm_type == pmType_e::Dead )
	{
		pm.maxs[ 2 ] = DEAD_MINS_Z;
		pm.ps->viewheight = CROUCH_VIEWHEIGHT;
		return;
	}

	if( ( pm.ps->pm_flags & ( PMF_DUCKED | PMF_VIEW_PRONE ) ) == ( PMF_DUCKED | PMF_VIEW_PRONE ) )
	{
		pm.maxs[ 2 ] = 54.0f;
	}
	else if( pm.ps->pm_flags & PMF_DUCKED )
	{
		pm.maxs[ 2 ] = 60.0f;
		pm.ps->viewheight = CROUCH_VIEWHEIGHT;
	}
	else if( pm.ps->pm_flags & PMF_VIEW_PRONE )
	{
		pm.maxs[ 2 ] = 20.0f;
		pm.ps->viewheight = PRONE_VIEWHEIGHT;
	}
	else if( pm.ps->pm_flags & PMF_VIEW_DUCK_RUN )
	{
		pm.maxs[ 2 ] = 94.0f;
		pm.mins[ 2 ] = 54.0f;
		pm.ps->viewheight = DEFAULT_VIEWHEIGHT;
	}
	else if( pm.ps->pm_flags & PMF_VIEW_JUMP_START )
	{
		pm.maxs[ 2 ] = 94.0f;
		pm.ps->viewheight = JUMP_START_VIEWHEIGHT;
	}
	else
	{
		pm.maxs[ 2 ] = 94.0f;
		pm.ps->viewheight = DEFAULT_VIEWHEIGHT;
	}
}



//===================================================================


/*
===============
PM_Footsteps
===============
*/
void Pmove::PM_Footsteps( void ) {
	float		bobmove;
	int			old;
	bool	footstep;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	pm.xyspeed = sqrtf( pm.ps->velocity[0] * pm.ps->velocity[0]
		+  pm.ps->velocity[1] * pm.ps->velocity[1] );

	if ( pm.ps->groundEntityNum == ENTITYNUM_NONE ) {

//		if ( pm.ps->powerups[PW_INVULNERABILITY] ) {
//			PM_ContinueLegsAnim( LEGS_IDLECR );
//		}
		// airborne leaves position in cycle intact, but doesn't advance
		if ( pm.waterlevel > 1 ) {
//			PM_ContinueLegsAnim( LEGS_SWIM );
		}
		return;
	}

	// if not trying to move
	if ( !pm.cmd.forwardmove && !pm.cmd.rightmove ) {
		if (  pm.xyspeed < 5 ) {
			pm.ps->bobCycle = 0;	// start at beginning of cycle again
			if ( pm.ps->pm_flags & PMF_DUCKED ) {
//				PM_ContinueLegsAnim( LEGS_IDLECR );
			} else {
//				PM_ContinueLegsAnim( LEGS_IDLE );
			}
		}
		return;
	}


	footstep = false;


	if( !( pm.cmd.buttons.flags & BUTTON_RUN ) )
	{
		bobmove = 0.4f;	// faster speeds bob faster

		footstep = true;
	} else {
		bobmove = 0.3f;	// walking bobs slow
	}

	// check for footstep / splash sounds
	old = pm.ps->bobCycle;
	pm.ps->bobCycle = (int)( old + bobmove * pml.msec ) & 255;

	// if we just crossed a cycle boundary, play an apropriate footstep event
	if ( ( ( old + 64 ) ^ ( pm.ps->bobCycle + 64 ) ) & 128 ) {
		if ( pm.waterlevel == 0 ) {
			// on ground will only play sounds if running
			if ( footstep && !pm.noFootsteps ) {
//				PM_AddEvent( PM_FootstepForSurface() );
			}
		} else if ( pm.waterlevel == 1 ) {
			// splashing
//			PM_AddEvent( EV_FOOTSPLASH );
		} else if ( pm.waterlevel == 2 ) {
			// wading / swimming at surface
//			PM_AddEvent( EV_SWIM );
		} else if ( pm.waterlevel == 3 ) {
			// no sound when completely underwater

		}
	}
}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
void Pmove::PM_WaterEvents( void )
{
	// FIXME?
	//
	// if just entered a water volume, play a sound
	//
	if( !pml.previous_waterlevel && pm.waterlevel )
	{
		pm.pmoveEvent = EV_WATER_TOUCH;
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if( pml.previous_waterlevel && !pm.waterlevel )
	{
		pm.pmoveEvent = EV_WATER_LEAVE;
	}

	//
	// check for head just going under water
	//
	if( ( pml.previous_waterlevel != 3 ) && ( pm.waterlevel == 3 ) )
	{
		pm.pmoveEvent = EV_WATER_UNDER;
	}

	//
	// check for head just coming out of water
	//
	if( ( pml.previous_waterlevel == 3 ) && ( pm.waterlevel != 3 ) )
	{
		pm.pmoveEvent = EV_WATER_CLEAR;
	}
}

/*
================
PM_DropTimers
================
*/
void Pmove::PM_DropTimers( void )
{
	pm.ps->pm_flags &= ~PMF_RESPAWNED;
}

/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated isntead of a full move
================
*/
void Pmove::PM_UpdateViewAngles(playerState_t* ps, const usercmd_t* cmd)
{
	if (ps->pm_flags & PMF_FROZEN)
	{
		// no view changes at all
		return;
	}

	if (ps->stats[(size_t)playerstat_e::STAT_HEALTH] <= 0)
	{
		// no view changes at all
		return;
	}

	// circularly clamp the angles with deltas
	for (uint8_t i = 0; i < 3; i++)
	{
		short temp = cmd->angles[i] + ps->delta_angles[i];
		if (i == PITCH)
		{
			// don't let the player look up or down more than 90 degrees
			if (temp > 16000)
			{
				ps->delta_angles[i] = 16000 - cmd->angles[i];
				temp = 16000;
			}
			else if (temp < -16000)
			{
				ps->delta_angles[i] = -16000 - cmd->angles[i];
				temp = -16000;
			}
		}

		ps->viewangles[i] = ShortToAngle(temp);
	}
}


/*
================
PmoveSingle

================
*/

void Pmove::moveSingle()
{
	Vector tempVec;
	bool walking;

	// this counter lets us debug movement problems with a journal
	// by setting a conditional breakpoint fot the previous frame
	c_pmove++;

	// clear results
	pm.numtouch = 0;
	pm.watertype = 0;
	pm.waterlevel = 0;

	if( pm.ps->stats[(size_t)playerstat_e::STAT_HEALTH] <= 0 ) {
		pm.tracemask &= ~( CONTENTS_BODY | CONTENTS_NOBOTCLIP );	// corpses can fly through bodies
	}

	if( pm.ps->pm_type == pmType_e::ClimbWall )
	{
		pm.ps->fLeanAngle = 0.0f;
		pm.cmd.buttons.flags &= ~( BUTTON_LEANLEFT | BUTTON_LEANRIGHT );
	}

	// clear all pmove local vars
	memset( &pml, 0, sizeof( pml ) );

	// determine the time
	pml.msec = pm.cmd.serverTime - pm.ps->commandTime;
	if ( pml.msec < 1 ) {
		pml.msec = 1;
	} else if ( pml.msec > 200 ) {
		pml.msec = 200;
	}

	pm.ps->commandTime = pm.cmd.serverTime;

	// save old org in case we get stuck
	VecCopy( pm.ps->origin, pml.previous_origin );

	// save old velocity for crashlanding
	VecCopy( pm.ps->velocity, pml.previous_velocity );

	pml.frametime = float(pml.msec * 0.001);

	if ((pm.cmd.buttons.flags & (BUTTON_LEANLEFT | BUTTON_LEANRIGHT) &&
		(pm.cmd.buttons.flags & (BUTTON_LEANLEFT | BUTTON_LEANRIGHT)) != (BUTTON_LEANLEFT | BUTTON_LEANRIGHT))
		&& canLean(pm.cmd))
	{
		if( pm.cmd.buttons.flags & BUTTON_LEANLEFT )
		{
			if( pm.ps->fLeanAngle <= -40.0f )
			{
				pm.ps->fLeanAngle = -40.0f;
			}
			else
			{
				float fAngle = pml.frametime * ( -40.0f - pm.ps->fLeanAngle );
				float fLeanAngle = pml.frametime * -4.0f;

				if( fAngle * 10.0f <= fLeanAngle ) {
					fLeanAngle = fAngle * 10.0f;
				}

				pm.ps->fLeanAngle += fLeanAngle;
			}
		}
		else
		{
			if( pm.ps->fLeanAngle >= 40.0f )
			{
				pm.ps->fLeanAngle = 40.0f;
			}
			else
			{
				float fAngle = 40.0f - pm.ps->fLeanAngle;
				float fLeanAngle = pml.frametime * 4.0f;
				float fMult = pml.frametime * fAngle;

				if( fLeanAngle <= fMult * 10.0f )
				{
					fLeanAngle = fMult * 10.0f;
				}
				else
				{
					fLeanAngle = fMult;
				}

				pm.ps->fLeanAngle += fLeanAngle;
			}
		}
	}
	else if( pm.ps->fLeanAngle )
	{
		float fAngle = pm.ps->fLeanAngle * pml.frametime * 15.0f;

		if( pm.ps->fLeanAngle <= 0.0f )
		{
			float fLeanAngle = -4.0f * pml.frametime;

			if( fAngle <= fLeanAngle )
			{
				fLeanAngle = fAngle;
			}

			pm.ps->fLeanAngle -= fLeanAngle;
		}
		else
		{
			float fLeanAngle = pml.frametime * 4.0f;

			if( fLeanAngle <= fAngle ) {
				fLeanAngle = fAngle;
			}

			pm.ps->fLeanAngle -= fLeanAngle;
		}
	}

	if (shouldClearLean()) {
		pm.ps->fLeanAngle = 0.f;
	}

	// update the viewangles
	PM_UpdateViewAngles( pm.ps, &pm.cmd );

	AngleVectorsLeft( pm.ps->viewangles, pml.forward, pml.left, pml.up );
	VectorClear( tempVec );
	tempVec[ YAW ] = pm.ps->viewangles[ YAW ];
	AngleVectorsLeft( tempVec, pml.flat_forward, pml.flat_left, pml.flat_up );

	if (pm.ps->pm_type >= pmType_e::Dead)
	{
		pm.cmd.forwardmove = 0;
		pm.cmd.rightmove = 0;
		pm.cmd.upmove = 0;
		pm.ps->fLeanAngle = 0.0f;
	}

	if (pm.ps->pm_type == pmType_e::Noclip)
	{
		PM_NoclipMove();
		PM_DropTimers();
		return;
	}

	if ((pm.ps->pm_flags & PMF_NO_MOVE) || (pm.ps->pm_flags & PMF_FROZEN))
	{
		PM_CheckDuck();
		return;
	}

	// set watertype, and waterlevel
	PM_SetWaterLevel();
	pml.previous_waterlevel = pm.waterlevel;

	// set mins, maxs, and viewheight
	PM_CheckDuck();

	// set groundentity
	PM_GroundTrace();

	if ( pm.ps->pm_type == pmType_e::Dead ) {
		PM_DeadMove();
	}

	PM_DropTimers();

	if ( pml.walking ) {
		// walking on ground
		PM_WalkMove();
	} else {
		// airborne
		PM_AirMove();
	}

	walking = pml.walking;

	// set groundentity, watertype, and waterlevel
	PM_GroundTrace();
	PM_SetWaterLevel();

	// don't fall down stairs or do really short falls
	if( !pml.walking && ( walking || ( ( pml.previous_velocity[ 2 ] >= 0 ) && ( pm.ps->velocity[ 2 ] <= 0 ) ) ) )
	{
		Vector   point;
		trace_t  trace;

		point[ 0 ] = pm.ps->origin[ 0 ];
		point[ 1 ] = pm.ps->origin[ 1 ];
		point[ 2 ] = pm.ps->origin[ 2 ] - STEPSIZE;

		pm.traceInterface->trace( &trace, pm.ps->origin, pm.mins, pm.maxs, point, pm.ps->clientNum, pm.tracemask, true, false );
		if( ( trace.fraction < 1.0f ) && ( !trace.allsolid ) )
		{
			VecCopy( trace.endpos, pm.ps->origin );

			// allow client to smooth out the step
			pm.stepped = true;

			// requantify the player's position
			PM_GroundTrace();
			PM_SetWaterLevel();
		}
	}

	// entering / leaving water splashes
	PM_WaterEvents();
}

void Pmove::move_GroundTrace()
{
	memset( &pml, 0, sizeof( pml ) );
	pml.msec = 1;
	pml.frametime = 0.001f;
	PM_CheckDuck();
	PM_GroundTrace();
}

/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove::move()
{
	unsigned int finalTime = pm.cmd.serverTime;

	if (finalTime < pm.ps->commandTime)
	{
		// should not happen
		return;
	}

	if (finalTime > pm.ps->commandTime + 1000) {
		pm.ps->commandTime = finalTime - 1000;
	}

	// chop the move up if it is too long, to prevent framerate
	// dependent behavior
	while (pm.ps->commandTime != finalTime) {
		int		msec;

		msec = finalTime - pm.ps->commandTime;

		if (pm.pmove_fixed)
		{
			if (msec > pm.pmove_msec) {
				msec = pm.pmove_msec;
			}
		}
		else if (msec > 66) {
			msec = 66;
		}

		pm.cmd.serverTime = pm.ps->commandTime + msec;
		moveSingle();
	}
}
void Pmove::moveAdjustViewAngleSettings_OnLadder(Vector& vViewAngles, Vector& vAngles, playerState_t *pPlayerState, entityState_t *pEntState)
{
	float fDelta;
	float deltayaw;
	float yawAngle;
	float temp;

	vAngles[ 0 ] = 0.0f;
	vAngles[ 2 ] = 0.0f;

	if( vViewAngles[ 0 ] > 73.0f ) {
		vViewAngles[ 0 ] = 73.0f;
	}

	deltayaw = AngleSubtract( vViewAngles[ 1 ], vAngles[ 1 ] );

	yawAngle = 70.0f;
	if( deltayaw <= 70.0f )
	{
		yawAngle = deltayaw;
		if( deltayaw < -70.0f )
		{
			yawAngle = -70.0f;
		}
	}

	vViewAngles[ 1 ] = vAngles[ 1 ] + yawAngle;

	fDelta = sqrtf( yawAngle * yawAngle + vViewAngles[ 0 ] * vViewAngles[ 0 ] );

	if( vViewAngles[ 0 ] <= 0.0f ) {
		temp = 80.0f;
	} else {
		temp = 73.0f;
	}

	if( fDelta > temp )
	{
		float deltalimit = temp * 1.0f / fDelta;
		vViewAngles[ 0 ] *= deltalimit;
		vViewAngles[ 1 ] = yawAngle * deltalimit + vAngles[ 1 ];
	}
}

void Pmove::moveAdjustAngleSettings(Vector& vViewAngles, Vector& vAngles, playerState_t *pPlayerState, entityState_t *pEntState)
{
	Vector temp, temp2;
	Vector armsAngles, torsoAngles, headAngles;
	float fTmp;

	if( pPlayerState->pm_type == pmType_e::ClimbWall )
	{
		moveAdjustViewAngleSettings_OnLadder( vViewAngles, vAngles, pPlayerState, pEntState );
		VecSet( pEntState->bone_angles[ TORSO_TAG ], 0, 0, 0 );
		VecSet( pEntState->bone_angles[ ARMS_TAG ], 0, 0, 0 );
		VecSet( pEntState->bone_angles[ PELVIS_TAG ], 0, 0, 0 );
		QuatSet( pEntState->bone_quat[ TORSO_TAG ], 0, 0, 0, 1 );
		QuatSet( pEntState->bone_quat[ ARMS_TAG ], 0, 0, 0, 1 );
		QuatSet( pEntState->bone_quat[ PELVIS_TAG ], 0, 0, 0, 1 );

		AnglesSubtract( vViewAngles, vAngles, headAngles );
		VectorScale( headAngles, 0.5f, pEntState->bone_angles[ HEAD_TAG ] );

		EulerToQuat( headAngles, pEntState->bone_quat[ HEAD_TAG ] );
		return;
	}

	if( pPlayerState->pm_type != pmType_e::Dead )
	{
		fTmp = AngleMod( vViewAngles[ 1 ] );
		VecSet( vAngles, 0, fTmp, 0 );

		if( !( pPlayerState->pm_flags & PMF_VIEW_PRONE ) || ( pPlayerState->pm_flags & PMF_DUCKED ) )
		{
			fTmp = AngleMod( vViewAngles[ 0 ] );

			VecSet( temp, fTmp, 0, pPlayerState->fLeanAngle * 0.60f );
			VecSet( temp2, fTmp, 0, pPlayerState->fLeanAngle );

			if( fTmp > 180.0f ) {
				temp2[ 0 ] = fTmp - 360.0f;
			}

			temp2[ 0 ] = 0.90f * temp2[ 0 ] * 0.70f;

			AnglesSubtract( temp, temp2, headAngles );
			VecCopy( headAngles, pEntState->bone_angles[ HEAD_TAG ] );
			EulerToQuat( pEntState->bone_angles[ HEAD_TAG ], pEntState->bone_quat[ HEAD_TAG ] );

			if( temp2[ 0 ] <= 0.0f )
			{
				fTmp = -0.1f;
			}
			else{
				fTmp = 0.3f;
			}

			VecSet( temp, fTmp * temp2[ 0 ], 0, pPlayerState->fLeanAngle * 0.8f );
			VecCopy( temp, pEntState->bone_angles[ PELVIS_TAG ] );
			EulerToQuat( pEntState->bone_angles[ PELVIS_TAG ], pEntState->bone_quat[ PELVIS_TAG ] );

			float fDelta = ( 1.0f - fTmp ) * temp2[ 0 ];

			if( vViewAngles[ 0 ] <= 0.0f )
			{
				VecSet( torsoAngles, fDelta * 0.60f, 0, pPlayerState->fLeanAngle * 0.2f * -0.1f );
				VecSet( armsAngles, fDelta * 0.40f, 0, pPlayerState->fLeanAngle * 0.2f * 1.1f );
			}
			else
			{
				VecSet( torsoAngles, fDelta * 0.70f, 0, pPlayerState->fLeanAngle * 0.2f * -0.1f );
				VecSet( armsAngles, fDelta * 0.30f, 0, pPlayerState->fLeanAngle * 0.2f * 1.1f );
			}

			VecCopy( torsoAngles, pEntState->bone_angles[ TORSO_TAG ] );
			EulerToQuat( pEntState->bone_angles[ TORSO_TAG ], pEntState->bone_quat[ TORSO_TAG ] );

			VecCopy( armsAngles, pEntState->bone_angles[ ARMS_TAG ] );
			EulerToQuat( pEntState->bone_angles[ ARMS_TAG ], pEntState->bone_quat[ ARMS_TAG ] );
			return;
		}
	}

	// set the default angles
	VecSet( pEntState->bone_angles[ HEAD_TAG ], 0, 0, 0 );
	VecSet( pEntState->bone_angles[ TORSO_TAG ], 0, 0, 0 );
	VecSet( pEntState->bone_angles[ ARMS_TAG ], 0, 0, 0 );
	VecSet( pEntState->bone_angles[ PELVIS_TAG ], 0, 0, 0 );
	QuatSet( pEntState->bone_quat[ HEAD_TAG ], 0, 0, 0, 1 );
	QuatSet( pEntState->bone_quat[ TORSO_TAG ], 0, 0, 0, 1 );
	QuatSet( pEntState->bone_quat[ ARMS_TAG ], 0, 0, 0, 1 );
	QuatSet( pEntState->bone_quat[ PELVIS_TAG ], 0, 0, 0, 1 );
}

void Pmove::moveAdjustAngleSettings_Client(Vector& vViewAngles, Vector& vAngles, playerState_t *pPlayerState, entityState_t *pEntState)
{
	//Vector torsoAngles, headAngles;

	// called by cgame
	// FIXME
}

bool Pmove::canLean(const usercmd_t& cmd)
{
	// a new setting since SH that allow leaning while moving
	return (!cmd.forwardmove || canLeanWhileMoving)
		&& (!cmd.rightmove || canLeanWhileMoving)
		&& (!cmd.upmove || canLeanWhileMoving);
}

bool Pmove::shouldClearLean()
{
	return clearLeanOnExit && (pm.ps->getPlayerMoveFlags() & PMF_LEVELEXIT) != 0;
}
