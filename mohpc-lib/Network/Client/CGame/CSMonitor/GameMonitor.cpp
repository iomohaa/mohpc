#include <MOHPC/Network/Client/CGame/CSMonitor/GameMonitor.h>
#include <MOHPC/Network/Types/NetTime.h>
#include <MOHPC/Utility/Info.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

template<>
std::map<csNum_t, ConfigstringMonitorTemplate<GameMonitor>::Callback> ConfigstringMonitorTemplate<GameMonitor>::callbackMap
{
	{ CS::SERVERINFO,		&GameMonitor::parseServerInfo },
	{ CS::MESSAGE,			&GameMonitor::setMessage },
	{ CS::SAVENAME,			&GameMonitor::setSaveName },
	{ CS::MOTD,				&GameMonitor::setMotd },
	{ CS::WARMUP,			&GameMonitor::setWarmup },
	{ CS::MUSIC,			&GameMonitor::setMusic },
	{ CS::GAME_VERSION,		&GameMonitor::setGameVersion },
	{ CS::LEVEL_START_TIME,	&GameMonitor::setLevelStartTime },
	{ CS::MATCHEND,			&GameMonitor::setMatchEnd },
	{ CS::SOUNDTRACK,		&GameMonitor::setSoundtrack }
};

MOHPC_OBJECT_DEFINITION(GameMonitor);

GameMonitor::GameMonitor(const SnapshotProcessorPtr& snapshotProcessor, const cgsInfoPtr& cgsPtr)
	: ConfigstringMonitorTemplate(snapshotProcessor)
	, cgs(cgsPtr)
{
}

GameMonitor::~GameMonitor()
{
}

GameMonitor::HandlerList& GameMonitor::getHandlerList()
{
	return handlerList;
}

void GameMonitor::fillObjectives(const ReadOnlyInfo& info, str* objectives, const char* varName, size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		objectives[i] = info.ValueForKey((varName + std::to_string(i)).c_str());
	}
}

void GameMonitor::parseServerInfo(const char* cs)
{
	ReadOnlyInfo info(cs);

	// Parse match settings
	cgs->gameType = gameType_e(info.IntValueForKey("g_gametype"));
	cgs->dmFlags = info.IntValueForKey("dmflags");
	cgs->teamFlags = info.IntValueForKey("teamflags");
	cgs->fragLimit = info.IntValueForKey("fraglimit");
	cgs->timeLimit = info.IntValueForKey("timelimit");
	cgs->maxClients = info.IntValueForKey("sv_maxclients");
	cgs->allowVote = info.BoolValueForKey("g_allowVote");
	cgs->mapChecksum = info.IntValueForKey("sv_mapChecksum");
	// Parse map
	size_t mapLen;
	const char* mapName = info.ValueForKey("mapname", mapLen);
	if (*mapName)
	{
		const char* lastMapChar = strHelpers::findcharn(mapName, '$', mapLen);
		if (lastMapChar)
		{
			// don't put anything from the dollar
			cgs->mapName = str(mapName, 0, lastMapChar - mapName);
		}
		else {
			cgs->mapName = str(mapName, mapLen);
		}

		cgs->mapFilename = "maps/" + cgs->mapName + ".bsp";
	}

	// Parse scoreboard info
	fillObjectives(info, cgs->alliedText, "g_obj_alliedtext", NUM_TEAM_OBJECTIVES);
	fillObjectives(info, cgs->axisText, "g_obj_axistext", NUM_TEAM_OBJECTIVES);
	cgs->scoreboardPic = info.ValueForKey("g_scoreboardpic");
	cgs->scoreboardPicOver = info.ValueForKey("g_scoreboardpicover");
}

void GameMonitor::setWarmup(const char* cs)
{
	using namespace ticks;
	cgs->matchStartTime = tickTime_t(milliseconds(atoll(cs)));
}

void GameMonitor::setLevelStartTime(const char* cs)
{
	using namespace ticks;
	cgs->levelStartTime = tickTime_t(milliseconds(atoll(cs)));
}

void GameMonitor::setMatchEnd(const char* cs)
{
	using namespace ticks;
	cgs->matchEndTme = tickTime_t(milliseconds(atoll(cs)));
}

void GameMonitor::setSoundtrack(const char* cs)
{
	cgs->soundTrack = cs;
	getHandlerList().soundTrackHandler.broadcast(cs);
}

void GameMonitor::setGameVersion(const char* cs)
{
	cgs->gameVersion = cs;
}

void GameMonitor::setMessage(const char* cs)
{
	cgs->message = cs;
}

void GameMonitor::setSaveName(const char* cs)
{
	cgs->saveName = cs;
}

void GameMonitor::setMotd(const char* cs)
{
	cgs->motd = cs;
}

void GameMonitor::setMusic(const char* cs)
{
	cgs->music = cs;
	getHandlerList().musicHandler.broadcast(cs);
}
