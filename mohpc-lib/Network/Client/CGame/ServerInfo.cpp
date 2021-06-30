#include <MOHPC/Network/Client/CGame/ServerInfo.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

cgsInfo::cgsInfo()
	: matchStartTime(0)
	, matchEndTme(0)
	, levelStartTime(0)
	, serverLagTime(0)
	, dmFlags(0)
	, teamFlags(0)
	, maxClients(0)
	, mapChecksum(0)
	, fragLimit(0)
	, timeLimit(0)
	, serverType(serverType_e::normal)
	, gameType(gameType_e::FreeForAll)
	, allowVote(false)
{
}

uint64_t cgsInfo::getMatchStartTime() const
{
	return matchStartTime;
}

uint64_t cgsInfo::getMatchEndTime() const
{
	return matchEndTme;
}

uint64_t cgsInfo::getLevelStartTime() const
{
	return levelStartTime;
}

uint64_t cgsInfo::getServerLagTime() const
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

serverType_e cgsInfo::getServerType() const
{
	return serverType;
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

