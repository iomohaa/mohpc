#include <MOHPC/Network/Client/CGame/Scoreboard.h>

#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

Scoreboard::Scoreboard()
{
}

size_t Scoreboard::getNumTeams() const
{
	return teamEntries.NumObjects();
}

const Scoreboard::teamEntry_t& Scoreboard::getTeam(size_t index)
{
	return teamEntries[index];
}

const Scoreboard::teamEntry_t* Scoreboard::getTeamByType(teamType_e type)
{
	const size_t numTeams = teamEntries.NumObjects();
	for (size_t i = 0; i < numTeams; ++i)
	{
		const teamEntry_t& entry = teamEntries[i];
		if (entry.teamNum == (uint32_t)type) {
			return &entry;
		}
	}

	return nullptr;
}

size_t Scoreboard::getNumPlayers() const
{
	return playerList.NumObjects();
}

const Scoreboard::player_t& Scoreboard::getPlayer(size_t index) const
{
	return playerList[index];
}

uint32_t Scoreboard::teamEntry_t::getTeamNum() const
{
	return teamNum;
}

uint32_t Scoreboard::teamEntry_t::getNumKills() const
{
	return numKills;
}

uint32_t Scoreboard::teamEntry_t::getNumDeaths() const
{
	return numDeaths;
}

uint32_t Scoreboard::teamEntry_t::getPing() const
{
	return ping;
}

uint32_t Scoreboard::player_t::getClientNum() const
{
	return clientNum;
}

uint32_t Scoreboard::player_t::getTeamNum() const
{
	return teamNum;
}

uint32_t Scoreboard::player_t::getNumKills() const
{
	return numKills;
}

uint32_t Scoreboard::player_t::getNumDeaths() const
{
	return numDeaths;
}

uint64_t Scoreboard::player_t::getTimeStamp() const
{
	return timeStamp;
}

uint32_t Scoreboard::player_t::getPing() const
{
	return ping;
}

bool Scoreboard::player_t::isAlive() const
{
	return alive;
}

void Scoreboard::reserveTeamEntries(size_t num)
{
	teamEntries.Resize(num);
}

void Scoreboard::reservePlayerList(size_t num)
{
	playerList.Resize(num);
}

Scoreboard::teamEntry_t& Scoreboard::createTeamEntry()
{
	teamEntry_t* entry = new (teamEntries) teamEntry_t();
	return *entry;
}

Scoreboard::player_t& Scoreboard::createPlayerEntry()
{
	player_t* entry = new (playerList) player_t();
	return *entry;
}

ScoreboardParser::ScoreboardParser(Scoreboard& scoreboardRef, gameType_e gameTypeVal)
	: scoreboard(scoreboardRef)
	, gameType(gameTypeVal)
{
}

void ScoreboardParser::parse(TokenParser& tokenizer)
{
	uint32_t numEntries = tokenizer.GetInteger(false);
	if (numEntries > MAX_ENTRIES)
	{
		// Better not overflow with a bad number
		numEntries = MAX_ENTRIES;
	}

	scoreboard.reservePlayerList(numEntries);

	if (gameType > gameType_e::FreeForAll)
	{
		scoreboard.reserveTeamEntries(4);

		// TDM
		for (size_t i = 0; i < numEntries; ++i)
		{
			const int32_t id = tokenizer.GetInteger(false);
			if (id == -1) {
				parseTeamInfo(tokenizer);
			}
			else if (id == -2) {
				parseTeamEmpty(tokenizer);
			}
			else {
				parseTeamPlayer(id, tokenizer);
			}
		}
	}
	else
	{
		// Free-for-All
		for (size_t i = 0; i < numEntries; ++i)
		{
			const int32_t id = tokenizer.GetInteger(false);
			if (id == -1 || id == -2) {
				parseEmpty(tokenizer);
			}
			else {
				parsePlayer(id, tokenizer);
			}
		}
	}
}

void ScoreboardParser::parseTeamInfo(TokenParser& tokenizer)
{
	Scoreboard::teamEntry_t& entry = scoreboard.createTeamEntry();

	entry.teamNum = tokenizer.GetInteger(false);
	entry.numKills = tokenizer.GetInteger(false);
	entry.numDeaths = tokenizer.GetInteger(false);

	// Skip the team string
	tokenizer.GetToken(false);

	entry.ping = tokenizer.GetInteger(false);
}

void ScoreboardParser::parseTeamPlayer(uint32_t clientNum, TokenParser& tokenizer)
{
	Scoreboard::player_t& player = scoreboard.createPlayerEntry();

	const int32_t teamNum = tokenizer.GetInteger(false);

	player.clientNum = clientNum;
	player.teamNum = abs(teamNum);
	// A negative teamNumber means that the player is dead
	player.alive = teamNum >= 0;
	player.numKills = tokenizer.GetInteger(false);
	player.numDeaths = tokenizer.GetInteger(false);
	const str timeString = tokenizer.GetToken(false);
	player.ping = tokenizer.GetInteger(false);

	// Parse time
	player.timeStamp = parseTime(timeString.c_str());
}

void ScoreboardParser::parseTeamEmpty(TokenParser& tokenizer)
{
	for (size_t i = 0; i < 5; ++i) {
		tokenizer.GetToken(false);
	}
}

void ScoreboardParser::parsePlayer(uint32_t clientNum, TokenParser& tokenizer)
{
	Scoreboard::player_t& player = scoreboard.createPlayerEntry();

	player.clientNum = clientNum;
	player.numKills = tokenizer.GetInteger(false);
	player.numDeaths = tokenizer.GetInteger(false);
	const str timeString = tokenizer.GetToken(false);
	player.ping = tokenizer.GetInteger(false);

	// Parse time
	player.timeStamp = parseTime(timeString.c_str());

	// No team
	player.teamNum = 0;
	// Can't determine with the data if the player is dead or alive.
	player.alive = true;
}

void ScoreboardParser::parseEmpty(TokenParser& tokenizer)
{
	for (size_t i = 0; i < 4; ++i) {
		tokenizer.GetToken(false);
	}
}

uint64_t ScoreboardParser::parseTime(const char* timeStr)
{
	uint32_t hours, minutes, seconds;
	if (sscanf(timeStr, "%i:%02i:%02i", &hours, &minutes, &seconds) != 3)
	{
		hours = 0;
		sscanf(timeStr, "%i:%02i", &minutes, &seconds);
	}

	return (seconds + minutes * 60 + hours * 24 * 60) * 1000;
}
