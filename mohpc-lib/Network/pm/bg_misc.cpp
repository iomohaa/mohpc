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
// bg_misc.c -- both games misc functions, all completely stateless

#include <MOHPC/Network/pm/bg_public.h>
#include <MOHPC/Network/InfoTypes.h>

using namespace MOHPC;
using namespace Network;

/*
========================
BG_PlayerStateToEntityState

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, bool snap )
{
	if ( ps->pm_type == pmType_e::PM_NOCLIP ) {
		s->eType = (entityType_e)0;//ET_INVISIBLE;
	//} else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH ) {
	//	s->eType = 0;//ET_INVISIBLE;
	} else {
		s->eType = entityType_e::player;
	}

	s->number = ps->clientNum;

	VecCopy( ps->origin, s->origin );
	if ( snap ) {
		SnapVector( s->origin );
	}
	// set the trDelta for flag direction
	VecCopy( ps->velocity, s->pos.trDelta );

	VecCopy( ps->viewangles, s->angles );
	if ( snap ) {
		SnapVector( s->angles );
	}

	//s->angles2[YAW] = ps->movementDir;
	s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config
	//s->eFlags = ps->eFlags;
	//if ( ps->stats[STAT_HEALTH] <= 0 ) {
	//	s->eFlags |= EF_DEAD;
	//} else {
	//	s->eFlags &= ~EF_DEAD;
	//}

	
	s->groundEntityNum = ps->groundEntityNum;

}

/*
========================
BG_PlayerStateToEntityStateExtraPolate

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, bool snap )
{
	if ( ps->pm_type == pmType_e::PM_NOCLIP ) {
		s->eType = (entityType_e)0;//ET_INVISIBLE;
	//} else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH ) {
	//	s->eType = 0;//ET_INVISIBLE;
	} else {
		s->eType = entityType_e::player;
	}

	s->number = ps->clientNum;

	VecCopy( ps->origin, s->origin );
	if ( snap ) {
		SnapVector( s->origin );
	}
	// set the trDelta for flag direction and linear prediction
	VecCopy( ps->velocity, s->pos.trDelta );
	// set the time for linear prediction
	s->pos.trTime = time;

	VecCopy( ps->viewangles, s->angles );
	if ( snap ) {
		SnapVector( s->angles );
	}
	s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config


	s->groundEntityNum = ps->groundEntityNum;

}
