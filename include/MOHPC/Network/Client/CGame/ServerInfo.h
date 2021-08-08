#pragma once

#include "../../NetGlobal.h"
#include "../../Types/Protocol.h"
#include "../../../Common/str.h"
#include "../../../Utility/TickTypes.h"

#include "GameType.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		namespace DMFlags
		{
			/**
			 * Values for dmflags
			 */
			 /** Players don't drop health on death. */
			static constexpr unsigned int DF_NO_HEALTH = (1 << 0);
			/** Players don't drop powerups on death. */
			static constexpr unsigned int DF_NO_POWERUPS = (1 << 1);
			/** Whether or not weapons in the level stays available on player pick up. */
			static constexpr unsigned int DF_WEAPONS_STAY = (1 << 2);
			/** Prevent falling damage. */
			static constexpr unsigned int DF_NO_FALLING = (1 << 3);
			/** This flag doesn't seem to be used at all. */
			static constexpr unsigned int DF_INSTANT_ITEMS = (1 << 4);
			/** TriggerChangeLevel won't switch level. */
			static constexpr unsigned int DF_SAME_LEVEL = (1 << 5);
			/** Prevent players from having an armor. */
			static constexpr unsigned int DF_NO_ARMOR = (1 << 11);
			/** MOH:AA: Infinite clip ammo. MOH:SH/MOH:BT: Infinite magazines. */
			static constexpr unsigned int DF_INFINITE_AMMO = (1 << 14);
			/** This should prevent footstep sounds to play. */
			static constexpr unsigned int DF_NO_FOOTSTEPS = (1 << 17);

			/**
			 * protocol version >= 15
			 * (SH)
			 */

			 /** Allow leaning while in movement. */
			static constexpr unsigned int DF_ALLOW_LEAN = (1 << 18);
			/** Specify that G43 is replaced with Kar98. */
			static constexpr unsigned int DF_OLD_SNIPERRIFLE = (1 << 19);

			/**
			 * protocol version >= 17
			 * (BT)
			 */

			 /** Axis use a shotgun rather than kar98 mortar. */
			static constexpr unsigned int DF_GERMAN_SHOTGUN = (1 << 20);
			/** Allow landmine to be used on AA maps. */
			static constexpr unsigned int DF_ALLOW_OLDMAP_MINES = (1 << 21);

			/**
			 * [BT]
			 * Weapon type filtering
			 */
			 /** Disallow the usage of rifles. */
			static constexpr unsigned int DF_BAN_WEAP_RIFLE = (1 << 22);
			/** Disallow the usage of rifles. */
			static constexpr unsigned int DF_BAN_WEAP_SNIPER = (1 << 23);
			/** Disallow the usage of snipers. */
			static constexpr unsigned int DF_BAN_WEAP_SMG = (1 << 24);
			/** Disallow the usage of sub-machine guns. */
			static constexpr unsigned int DF_BAN_WEAP_MG = (1 << 25);
			/** Disallow the usage of machine guns. */
			static constexpr unsigned int DF_BAN_WEAP_HEAVY = (1 << 26);
			/** Disallow the usage of shotgun. */
			static constexpr unsigned int DF_BAN_WEAP_SHOTGUN = (1 << 27);
			/** Disallow the usage of landmine. */
			static constexpr unsigned int DF_BAN_WEAP_LANDMINE = (1 << 28);
		}

		static constexpr size_t NUM_TEAM_OBJECTIVES = 5;

		/**
		 * Parsed server info data.
		 */
		class cgsInfo
		{
		public:
			cgsInfo();

			/** Return the server time at which the match has started. */
			MOHPC_NET_EXPORTS tickTime_t getMatchStartTime() const;

			/** Return the server time at which the match has ended. */
			MOHPC_NET_EXPORTS tickTime_t getMatchEndTime() const;

			/** Return the server time at which the level has started. */
			MOHPC_NET_EXPORTS tickTime_t getLevelStartTime() const;

			/** Get the last lag time. */
			MOHPC_NET_EXPORTS tickTime_t getServerLagTime() const;

			/** Return the current game type. */
			MOHPC_NET_EXPORTS gameType_e getGameType() const;

			/** Return current DF_ flags. */
			MOHPC_NET_EXPORTS uint32_t getDeathmatchFlags() const;

			/**
			 * Return true if dmflags contain one or more of the specified flags.
			 *
			 * @param	flags	Flags to look for.
			 * @return	true	if one of the following flags are valid.
			 */
			MOHPC_NET_EXPORTS bool hasAnyDMFlags(uint32_t flags) const;

			/**
			 * Return true if dmflags contain the specified flags.
			 *
			 * @param	flags	Flags to look for.
			 * @return	true	if all of the flags are valid.
			 */
			MOHPC_NET_EXPORTS bool hasAllDMFlags(uint32_t flags) const;

			/**
			 * User version of hasAnyDMFlags that split each flags by arguments.
			 * Checks if any of the specified flags matches.
			 */
			template<typename...Args>
			bool hasAnyDMFlagsArgs(Args... args)
			{
				const uint32_t flags = (args | ...);
				return hasAnyDMFlags(flags);
			}

			/**
			 * User version of hasAnyDMFlags that split each flags by arguments.
			 * Checks if all of the specified flags matches.
			 */
			template<typename...Args>
			bool hasAllDMFlagsArgs(Args... args)
			{
				const uint32_t flags = (args | ...);
				return hasAllDMFlags(flags);
			}

			/** Return teamFlags (doesn't seem to be used). */
			MOHPC_NET_EXPORTS uint32_t getTeamFlags() const;

			/** Return the number of maximum clients that can be connected in the current server. */
			MOHPC_NET_EXPORTS uint32_t getMaxClients() const;

			/** Return the frag limit before the match ends. */
			MOHPC_NET_EXPORTS int32_t getFragLimit() const;

			/** Returns the time limit before the match ends. */
			MOHPC_NET_EXPORTS int32_t getTimeLimit() const;

			/** Return the server type the client is connected on. */
			MOHPC_NET_EXPORTS serverType_e getServerType() const;

			/** Return whether or not voting is allowed. */
			MOHPC_NET_EXPORTS bool isVotingAllowed() const;

			/** Return the map checksum on the server. Useful to compare against the checksum of the map on the client. */
			MOHPC_NET_EXPORTS uint32_t getMapChecksum() const;

			/** Return the current map name. */
			MOHPC_NET_EXPORTS const char* getMapName() const;
			MOHPC_NET_EXPORTS const str& getMapNameStr() const;

			/** Return the path to the map name. */
			MOHPC_NET_EXPORTS const char* getMapFilename() const;
			MOHPC_NET_EXPORTS const str& getMapFilenameStr() const;

			/** Return the allied text NUM_TEAM_OBJECTIVES is the max. */
			MOHPC_NET_EXPORTS const char* getAlliedText(size_t index) const;

			/** Return the axis text. NUM_TEAM_OBJECTIVES is the max. */
			MOHPC_NET_EXPORTS const char* getAxisText(size_t index);

			/** Return the scoreboard pic shader. */
			MOHPC_NET_EXPORTS const char* getScoreboardPic() const;

			/** Return the scoreboard pic shader when game is over. */
			MOHPC_NET_EXPORTS const char* getScoreboardPicOver() const;

		public:
			tickTime_t matchStartTime;
			tickTime_t matchEndTme;
			tickTime_t levelStartTime;
			tickTime_t serverLagTime;
			str mapName;
			str mapFilename;
			str alliedText[NUM_TEAM_OBJECTIVES];
			str axisText[NUM_TEAM_OBJECTIVES];
			str scoreboardPic;
			str scoreboardPicOver;
			uint32_t dmFlags;
			uint32_t teamFlags;
			uint32_t maxClients;
			uint32_t mapChecksum;
			int32_t fragLimit;
			int32_t timeLimit;
			serverType_e serverType;
			gameType_e gameType;
			bool allowVote;
		};
	}
}
}