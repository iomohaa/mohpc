#pragma once

#include "../../NetGlobal.h"
#include "../../NetObject.h"
#include "../../Types/Protocol.h"
#include "../../../Common/str.h"
#include "../../../Utility/TickTypes.h"
#include "../../../Utility/SharedPtr.h"

#include "GameType.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		using dmFlag_t = uint32_t;

		/**
		 * DM Flags.
		 */
		namespace DF
		{
			/**
			 * Values for dmflags
			 */
			 /** Players don't drop health on death. */
			MOHPC_NET_EXPORTS extern dmFlag_t NO_HEALTH;
			/** Players don't drop powerups on death. */
			MOHPC_NET_EXPORTS extern dmFlag_t NO_POWERUPS;
			/** Whether or not weapons in the level stays available on player pick up. */
			MOHPC_NET_EXPORTS extern dmFlag_t WEAPONS_STAY;
			/** Prevent falling damage. */
			MOHPC_NET_EXPORTS extern dmFlag_t NO_FALLING;
			/** This flag doesn't seem to be used at all. */
			MOHPC_NET_EXPORTS extern dmFlag_t INSTANT_ITEMS;
			/** TriggerChangeLevel won't switch level. */
			MOHPC_NET_EXPORTS extern dmFlag_t SAME_LEVEL;
			/** Prevent players from having an armor. */
			MOHPC_NET_EXPORTS extern dmFlag_t NO_ARMOR;
			/** MOH:AA: Infinite clip ammo. MOH:SH/MOH:BT: Infinite magazines. */
			MOHPC_NET_EXPORTS extern dmFlag_t INFINITE_AMMO;
			/** This should prevent footstep sounds to play. */
			MOHPC_NET_EXPORTS extern dmFlag_t NO_FOOTSTEPS;

			/**
			 * protocol version >= 15
			 * (SH)
			 */

			 /** Allow leaning while in movement. */
			MOHPC_NET_EXPORTS extern dmFlag_t ALLOW_LEAN;
			/** Specify that G43 is replaced with Kar98. */
			MOHPC_NET_EXPORTS extern dmFlag_t OLD_SNIPERRIFLE;

			/**
			 * protocol version >= 17
			 * (BT)
			 */

			 /** Axis use a shotgun rather than kar98 mortar. */
			MOHPC_NET_EXPORTS extern dmFlag_t GERMAN_SHOTGUN;
			/** Allow landmine to be used on AA maps. */
			MOHPC_NET_EXPORTS extern dmFlag_t ALLOW_OLDMAP_MINES;

			/**
			 * [BT]
			 * Weapon type filtering
			 */
			 /** Disallow the usage of rifles. */
			MOHPC_NET_EXPORTS extern dmFlag_t BAN_WEAP_RIFLE;
			/** Disallow the usage of rifles. */
			MOHPC_NET_EXPORTS extern dmFlag_t BAN_WEAP_SNIPER;
			/** Disallow the usage of snipers. */
			MOHPC_NET_EXPORTS extern dmFlag_t BAN_WEAP_SMG;
			/** Disallow the usage of sub-machine guns. */
			MOHPC_NET_EXPORTS extern dmFlag_t BAN_WEAP_MG;
			/** Disallow the usage of machine guns. */
			MOHPC_NET_EXPORTS extern dmFlag_t BAN_WEAP_HEAVY;
			/** Disallow the usage of shotgun. */
			MOHPC_NET_EXPORTS extern dmFlag_t BAN_WEAP_SHOTGUN;
			/** Disallow the usage of landmine. */
			MOHPC_NET_EXPORTS extern dmFlag_t BAN_WEAP_LANDMINE;
		}

		static constexpr size_t NUM_TEAM_OBJECTIVES = 5;

		/**
		 * Parsed server info data.
		 */
		class cgsInfo
		{
			MOHPC_NET_OBJECT_DECLARATION(cgsInfo);

		public:
			MOHPC_NET_EXPORTS cgsInfo();
			MOHPC_NET_EXPORTS ~cgsInfo();

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
			MOHPC_NET_EXPORTS dmFlag_t getDeathmatchFlags() const;

			/**
			 * Return true if dmflags contain one or more of the specified flags.
			 *
			 * @param	flags	Flags to look for.
			 * @return	true	if one of the following flags are valid.
			 */
			MOHPC_NET_EXPORTS bool hasAnyDMFlags(dmFlag_t flags) const;

			/**
			 * Return true if dmflags contain the specified flags.
			 *
			 * @param	flags	Flags to look for.
			 * @return	true	if all of the flags are valid.
			 */
			MOHPC_NET_EXPORTS bool hasAllDMFlags(dmFlag_t flags) const;

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

			/** Return the game version that the server is running. */
			MOHPC_NET_EXPORTS const char* getGameVersion() const;

			/** Return the world message for the server. */
			MOHPC_NET_EXPORTS const char* getMessage() const;

			/** Return the file name of the game save. */
			MOHPC_NET_EXPORTS const char* getSaveName() const;

			/** Return the server's message of the day. */
			MOHPC_NET_EXPORTS const char* getMotd() const;

			/** Return the currently playing music. */
			MOHPC_NET_EXPORTS const char* getMusic() const;

			/** Return the current server soundtrack. */
			MOHPC_NET_EXPORTS const char* getSoundTrack() const;

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
			str gameVersion;
			str soundTrack;
			str message;
			str motd;
			str music;
			str saveName;
			uint32_t dmFlags;
			uint32_t teamFlags;
			uint32_t maxClients;
			uint32_t mapChecksum;
			int32_t fragLimit;
			int32_t timeLimit;
			gameType_e gameType;
			bool allowVote;
		};
		using cgsInfoPtr = SharedPtr<cgsInfo>;
	}
}
}