#pragma once

#include "../../NetGlobal.h"
#include "GameType.h"

#include <vector>
#include <cstdint>
#include <cstddef>

namespace MOHPC
{
class TokenParser;

namespace Network
{
	namespace CGame
	{
		class Scoreboard
		{
		public:
			struct teamEntry_t
			{
				uint32_t teamNum;
				uint32_t numKills;
				uint32_t numDeaths;
				uint32_t ping;

			public:
				/** Team number. See teamType_e. */
				MOHPC_NET_EXPORTS uint32_t getTeamNum() const;

				/** The number of kills (or score) the team has made. */
				MOHPC_NET_EXPORTS uint32_t getNumKills() const;

				/** The number of deaths from this team. */
				MOHPC_NET_EXPORTS uint32_t getNumDeaths() const;

				/** Team's ping (usually the average ping of players in team). */
				MOHPC_NET_EXPORTS uint32_t getPing() const;
			};

			struct player_t
			{
				uint64_t timeStamp;
				uint32_t clientNum;
				uint32_t teamNum;
				uint32_t numKills;
				uint32_t numDeaths;
				uint32_t ping;
				bool alive;

			public:
				/** The client number. */
				MOHPC_NET_EXPORTS uint32_t getClientNum() const;

				/** Team number. See teamType_e. */
				MOHPC_NET_EXPORTS uint32_t getTeamNum() const;

				/** The number of kills made by the player. */
				MOHPC_NET_EXPORTS uint32_t getNumKills() const;

				/** The number of time the player died. */
				MOHPC_NET_EXPORTS uint32_t getNumDeaths() const;

				/** Time in milliseconds since the player is in the match. */
				MOHPC_NET_EXPORTS uint64_t getTimeStamp() const;

				/** Player's ping. */
				MOHPC_NET_EXPORTS uint32_t getPing() const;

				/** Whether or not the player is currently alive. */
				MOHPC_NET_EXPORTS bool isAlive() const;
			};

		public:
			Scoreboard();

			/** Return the number of team entries. */
			MOHPC_NET_EXPORTS size_t getNumTeams() const;

			/** Return the team at the specified index. */
			MOHPC_NET_EXPORTS const teamEntry_t& getTeam(size_t index);

			/** Return the team by the specified type. */
			MOHPC_NET_EXPORTS const teamEntry_t* getTeamByType(teamType_e type);

			/** Return the number of players. */
			MOHPC_NET_EXPORTS size_t getNumPlayers() const;

			/** Return player at the specified index. */
			MOHPC_NET_EXPORTS const player_t& getPlayer(size_t index) const;

			void reserveTeamEntries(size_t num);
			void reservePlayerList(size_t num);

			teamEntry_t& createTeamEntry();
			player_t& createPlayerEntry();

			// Parsing functions
		public:
			void parse(TokenParser& tokenizer);
			void parseTeamInfo(TokenParser& tokenizer);
			void parseTeamPlayer(uint32_t clientNum, TokenParser& tokenizer);
			void parseTeamEmpty(TokenParser& tokenizer);
			void parsePlayer(uint32_t clientNum, TokenParser& tokenizer);
			void parseEmpty(TokenParser& tokenizer);
			uint64_t parseTime(const char* timeStr);

		private:
			std::vector<teamEntry_t> teamEntries;
			std::vector<player_t> playerList;
		};

		class ScoreboardParser
		{
			static constexpr size_t MAX_ENTRIES = 256;

		public:
			ScoreboardParser(Scoreboard& scoreboardRef, gameType_e gameTypeVal);

			// Parsing functions

			void parse(TokenParser& tokenizer);
			void parseTeamInfo(TokenParser& tokenizer);
			void parseTeamPlayer(uint32_t clientNum, TokenParser& tokenizer);
			void parseTeamEmpty(TokenParser& tokenizer);
			void parsePlayer(uint32_t clientNum, TokenParser& tokenizer);
			void parseEmpty(TokenParser& tokenizer);
			uint64_t parseTime(const char* timeStr);

		private:
			Scoreboard& scoreboard;
			gameType_e gameType;
		};
	}
}
}