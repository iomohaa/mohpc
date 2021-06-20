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
// bg_local.h -- local definitions for the bg (both games) files

#include <MOHPC/Common/Math.h>

namespace MOHPC
{
	static constexpr unsigned int JUMP_VELOCITY		= 270;

	static constexpr unsigned int TIMER_LAND		= 130;
	static constexpr unsigned int TIMER_GESTURE		= (34 * 66 + 50);

	static constexpr unsigned int HEAD_TAG		= 0;
	static constexpr unsigned int TORSO_TAG		= 1;
	static constexpr unsigned int ARMS_TAG		= 2;
	static constexpr unsigned int PELVIS_TAG	= 3;
	static constexpr unsigned int MOUTH_TAG		= 4;

	// movement parameters
	extern	float	pm_stopspeed;
	extern	float	pm_duckScale;
	extern	float	pm_swimScale;
	extern	float	pm_wadeScale;

	extern	float	pm_accelerate;
	extern	float	pm_airaccelerate;
	extern	float	pm_wateraccelerate;
	extern	float	pm_flyaccelerate;

	extern	float	pm_friction;
	extern	float	pm_waterfriction;
	extern	float	pm_flightfriction;
}


