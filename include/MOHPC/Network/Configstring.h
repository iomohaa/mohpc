#pragma once

#include "NetGlobal.h"
#include <cstdint>

namespace MOHPC
{
namespace Network
{
	using csNum_t = uint16_t;

	namespace CS
	{
		/**
		 * Configstring values here are matching for SH/BT (and normalized for AA to match SH/BT).
		 */

		MOHPC_NET_EXPORTS extern csNum_t MAX_CONFIGSTRINGS;

		/** these are sent over the net as 8 bits */
		MOHPC_NET_EXPORTS extern csNum_t MAX_MODELS;
		MOHPC_NET_EXPORTS extern csNum_t MAX_IMAGES;
		/** so they cannot be blindly increased */
		MOHPC_NET_EXPORTS extern csNum_t MAX_SOUNDS;
		MOHPC_NET_EXPORTS extern csNum_t MAX_OBJECTIVES;
		MOHPC_NET_EXPORTS extern csNum_t MAX_LIGHTSTYLES;
		MOHPC_NET_EXPORTS extern csNum_t MAX_WEAPONS;
		MOHPC_NET_EXPORTS extern csNum_t MAX_CLIENTS;

		/** an info string with all the serverinfo cvars. */
		MOHPC_NET_EXPORTS extern csNum_t SERVERINFO;
		/** an info string for server system to client system configuration (timescale, etc). */
		MOHPC_NET_EXPORTS extern csNum_t SYSTEMINFO;
		/** game can't modify below this, only the system can. */
		MOHPC_NET_EXPORTS extern csNum_t RESERVED_CONFIGSTRINGS;

		/** from the map worldspawn's message field */
		MOHPC_NET_EXPORTS extern csNum_t MESSAGE;
		/** current save. */
		MOHPC_NET_EXPORTS extern csNum_t SAVENAME;
		/** g_motd string for server message of the day. */
		MOHPC_NET_EXPORTS extern csNum_t MOTD;
		/** server time when the match will be restarted. */
		MOHPC_NET_EXPORTS extern csNum_t WARMUP;

		/** MUSIC_NewSoundtrack(cs). */
		MOHPC_NET_EXPORTS extern csNum_t MUSIC;
		/** cg.farplane_cull cg.farplane_distance cg.farplane_color[3]. */
		MOHPC_NET_EXPORTS extern csNum_t FOGINFO;
		/** cg.sky_alpha cg.sky_portal. */
		MOHPC_NET_EXPORTS extern csNum_t SKYINFO;

		MOHPC_NET_EXPORTS extern csNum_t GAME_VERSION;
		/** so the timer only shows the current level cgs.levelStartTime. */
		MOHPC_NET_EXPORTS extern csNum_t LEVEL_START_TIME;

		/** Index to the current objective. */
		MOHPC_NET_EXPORTS extern csNum_t CURRENT_OBJECTIVE;

		/**
		 * Rain values.
		 */

		MOHPC_NET_EXPORTS extern csNum_t RAIN_DENSITY;
		MOHPC_NET_EXPORTS extern csNum_t RAIN_SPEED;
		MOHPC_NET_EXPORTS extern csNum_t RAIN_SPEEDVARY;
		MOHPC_NET_EXPORTS extern csNum_t RAIN_SLANT;
		MOHPC_NET_EXPORTS extern csNum_t RAIN_LENGTH;
		MOHPC_NET_EXPORTS extern csNum_t RAIN_MINDIST;
		MOHPC_NET_EXPORTS extern csNum_t RAIN_WIDTH;
		MOHPC_NET_EXPORTS extern csNum_t RAIN_SHADER;
		MOHPC_NET_EXPORTS extern csNum_t RAIN_NUMSHADERS;

		/**
		 * Voting.
		 */

		/** Time at which the vote started. */
		MOHPC_NET_EXPORTS extern csNum_t VOTE_TIME;
		/** Vote command. */
		MOHPC_NET_EXPORTS extern csNum_t VOTE_STRING;
		/** Number of users who voted yes. */
		MOHPC_NET_EXPORTS extern csNum_t VOTES_YES;
		/** Number of users who voted no. */
		MOHPC_NET_EXPORTS extern csNum_t VOTES_NO;
		/** Number of users who didn't vote. */
		MOHPC_NET_EXPORTS extern csNum_t VOTES_UNDECIDED;

		/** Intermission time. */
		MOHPC_NET_EXPORTS extern csNum_t MATCHEND;

		/** List of loaded models. */
		MOHPC_NET_EXPORTS extern csNum_t MODELS;
		/** List of objectives. */
		MOHPC_NET_EXPORTS extern csNum_t OBJECTIVES;
		/** List of sounds to load. */
		MOHPC_NET_EXPORTS extern csNum_t SOUNDS;

		/** The list of iamges to load. */
		MOHPC_NET_EXPORTS extern csNum_t IMAGES;

		MOHPC_NET_EXPORTS extern csNum_t LIGHTSTYLES;
		/** Contains a list of players with their name. */
		MOHPC_NET_EXPORTS extern csNum_t PLAYERS;

		/**
		 * Player-related configstrings.
		 */

		/** List of weapons name. */
		MOHPC_NET_EXPORTS extern csNum_t WEAPONS;
		MOHPC_NET_EXPORTS extern csNum_t TEAMS;
		/** Miscellaneous strings (for example "NPCs: xxx"). */
		MOHPC_NET_EXPORTS extern csNum_t GENERAL_STRINGS;
		/** Number of spectators in game. */
		MOHPC_NET_EXPORTS extern csNum_t SPECTATORS;
		/** Number of allies players. */
		MOHPC_NET_EXPORTS extern csNum_t ALLIES;
		/** Number of axis players. */
		MOHPC_NET_EXPORTS extern csNum_t AXIS;

		/** The soundtrack that should be played. */
		MOHPC_NET_EXPORTS extern csNum_t SOUNDTRACK;
	}
}
}
