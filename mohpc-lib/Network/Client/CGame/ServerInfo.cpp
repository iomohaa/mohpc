#include <MOHPC/Network/Client/CGame/ServerInfo.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

unsigned int DF::NO_HEALTH = (1 << 0);
unsigned int DF::NO_POWERUPS = (1 << 1);
unsigned int DF::WEAPONS_STAY = (1 << 2);
unsigned int DF::NO_FALLING = (1 << 3);
unsigned int DF::INSTANT_ITEMS = (1 << 4);
unsigned int DF::SAME_LEVEL = (1 << 5);
unsigned int DF::NO_ARMOR = (1 << 11);
unsigned int DF::INFINITE_AMMO = (1 << 14);
unsigned int DF::NO_FOOTSTEPS = (1 << 17);

unsigned int DF::ALLOW_LEAN = (1 << 18);
unsigned int DF::OLD_SNIPERRIFLE = (1 << 19);

unsigned int DF::GERMAN_SHOTGUN = (1 << 20);
unsigned int DF::ALLOW_OLDMAP_MINES = (1 << 21);

unsigned int DF::BAN_WEAP_RIFLE = (1 << 22);
unsigned int DF::BAN_WEAP_SNIPER = (1 << 23);
unsigned int DF::BAN_WEAP_SMG = (1 << 24);
unsigned int DF::BAN_WEAP_MG = (1 << 25);
unsigned int DF::BAN_WEAP_HEAVY = (1 << 26);
unsigned int DF::BAN_WEAP_SHOTGUN = (1 << 27);
unsigned int DF::BAN_WEAP_LANDMINE = (1 << 28);

cgsInfo::cgsInfo()
	: matchStartTime(ticks::milliseconds())
	, matchEndTme(ticks::milliseconds())
	, levelStartTime(ticks::milliseconds())
	, serverLagTime(ticks::milliseconds())
	, dmFlags(0)
	, teamFlags(0)
	, maxClients(0)
	, mapChecksum(0)
	, fragLimit(0)
	, timeLimit(0)
	, gameType(gameType_e::FreeForAll)
	, allowVote(false)
{
}

cgsInfo::~cgsInfo()
{
}

tickTime_t cgsInfo::getMatchStartTime() const
{
	return matchStartTime;
}

tickTime_t cgsInfo::getMatchEndTime() const
{
	return matchEndTme;
}

tickTime_t cgsInfo::getLevelStartTime() const
{
	return levelStartTime;
}

tickTime_t cgsInfo::getServerLagTime() const
{
	return serverLagTime;
}

gameType_e cgsInfo::getGameType() const
{
	return gameType;
}

uint32_t cgsInfo::getDeathmatchFlags() const
{
	return dmFlags;
}

uint32_t cgsInfo::getTeamFlags() const
{
	return teamFlags;
}

uint32_t cgsInfo::getMaxClients() const
{
	return maxClients;
}

int32_t cgsInfo::getFragLimit() const
{
	return fragLimit;
}

int32_t cgsInfo::getTimeLimit() const
{
	return timeLimit;
}

bool cgsInfo::isVotingAllowed() const
{
	return allowVote;
}

uint32_t cgsInfo::getMapChecksum() const
{
	return mapChecksum;
}

const char* cgsInfo::getMapName() const
{
	return mapName.c_str();
}

const char* cgsInfo::getMapFilename() const
{
	return mapFilename.c_str();
}

const char* cgsInfo::getAlliedText(size_t index) const
{
	return alliedText[index].c_str();
}

const char* cgsInfo::getAxisText(size_t index)
{
	return axisText[index].c_str();
}

const char* cgsInfo::getScoreboardPic() const
{
	return scoreboardPic.c_str();
}

const char* cgsInfo::getScoreboardPicOver() const
{
	return scoreboardPicOver.c_str();
}

const str& cgsInfo::getMapNameStr() const
{
	return mapName;
}

const str& cgsInfo::getMapFilenameStr() const
{
	return mapFilename;
}

bool cgsInfo::hasAnyDMFlags(uint32_t flags) const
{
	return (dmFlags & flags) != 0;
}

bool cgsInfo::hasAllDMFlags(uint32_t flags) const
{
	return (dmFlags & flags) == flags;
}

