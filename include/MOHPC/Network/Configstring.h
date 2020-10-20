#pragma once

namespace MOHPC
{
	namespace Network
	{
		static constexpr unsigned int MAX_CONFIGSTRINGS = 2736;

		// these are sent over the net as 8 bits
		static constexpr unsigned int MAX_MODELS = 1024;
		// so they cannot be blindly increased
		static constexpr unsigned int MAX_SOUNDS = 512;
		static constexpr unsigned int MAX_OBJECTIVES = 20;
		static constexpr unsigned int MAX_LIGHTSTYLES = 32;
		static constexpr unsigned int MAX_WEAPONS = 48;
		static constexpr unsigned int MAX_CLIENTS = 64;

		// Configstring values here are matching for SH/BT (and normalized for AA to match SH/BT)
		// an info string with all the serverinfo cvars
		static constexpr unsigned int CS_SERVERINFO = 0;
		// an info string for server system to client system configuration (timescale, etc)
		static constexpr unsigned int CS_SYSTEMINFO = 1;
		// game can't modify below this, only the system can
		static constexpr unsigned int RESERVED_CONFIGSTRINGS = 2;

		// from the map worldspawn's message field
		static constexpr unsigned int CS_MESSAGE = 2;
		// current save
		static constexpr unsigned int CS_SAVENAME = 3;
		// g_motd string for server message of the day
		static constexpr unsigned int CS_MOTD = 4;
		// server time when the match will be restarted
		static constexpr unsigned int CS_WARMUP = 5;

		// MUSIC_NewSoundtrack(cs)
		static constexpr unsigned int CS_MUSIC = 6;
		// cg.farplane_cull cg.farplane_distance cg.farplane_color[3]
		static constexpr unsigned int CS_FOGINFO = 7;
		// cg.sky_alpha cg.sky_portal
		static constexpr unsigned int CS_SKYINFO = 8;

		static constexpr unsigned int CS_GAME_VERSION = 9;
		// so the timer only shows the current level cgs.levelStartTime
		static constexpr unsigned int CS_LEVEL_START_TIME = 10;

		static constexpr unsigned int CS_CURRENT_OBJECTIVE = 11;

		// cg.rain
		static constexpr unsigned int CS_RAIN_DENSITY = 12;
		static constexpr unsigned int CS_RAIN_SPEED = 13;
		static constexpr unsigned int CS_RAIN_SPEEDVARY = 14;
		static constexpr unsigned int CS_RAIN_SLANT = 15;
		static constexpr unsigned int CS_RAIN_LENGTH = 16;
		static constexpr unsigned int CS_RAIN_MINDIST = 17;
		static constexpr unsigned int CS_RAIN_WIDTH = 18;
		static constexpr unsigned int CS_RAIN_SHADER = 19;
		static constexpr unsigned int CS_RAIN_NUMSHADERS = 20;

		// voting (only in SH and BT)
		static constexpr unsigned int CS_VOTE_TIME = 21;
		static constexpr unsigned int CS_VOTE_STRING = 22;
		static constexpr unsigned int CS_VOTES_YES = 23;
		static constexpr unsigned int CS_VOTES_NO = 24;
		static constexpr unsigned int CS_VOTES_UNDECIDED = 25;

		// cgs.matchEndTime
		static constexpr unsigned int CS_MATCHEND = 26;

		static constexpr unsigned int CS_MODELS = 32;
		static constexpr unsigned int CS_OBJECTIVES = (CS_MODELS + MAX_MODELS);
		static constexpr unsigned int CS_SOUNDS = (CS_OBJECTIVES + MAX_OBJECTIVES);

		static constexpr unsigned int CS_IMAGES = (CS_SOUNDS + MAX_SOUNDS);
		static constexpr unsigned int MAX_IMAGES = 64;

		static constexpr unsigned int CS_LIGHTSTYLES(CS_IMAGES + MAX_IMAGES);
		static constexpr unsigned int CS_PLAYERS = (CS_LIGHTSTYLES + MAX_LIGHTSTYLES);

		static constexpr unsigned int CS_WEAPONS = (CS_PLAYERS + MAX_CLIENTS);
		static constexpr unsigned int CS_TEAMS = 1876;
		static constexpr unsigned int CS_GENERAL_STRINGS = 1877;
		static constexpr unsigned int CS_SPECTATORS = 1878;
		static constexpr unsigned int CS_ALLIES = 1879;
		static constexpr unsigned int CS_AXIS = 1880;
		static constexpr unsigned int CS_SOUNDTRACK = 1881;

		static constexpr unsigned int CS_TEAMINFO = 1;

		//static constexpr unsigned int CS_MAX = (CS_PARTICLES + MAX_LOCATIONS); }

		using csNum_t = uint16_t;
	}
}
