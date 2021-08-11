#include <MOHPC/Network/Client/CGame/CSMonitor/GameMonitor.h>
#include <MOHPC/Network/Types/NetTime.h>
#include <MOHPC/Utility/Info.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

std::map<csNum_t, ConfigstringMonitorTemplate<GameMonitor>::Callback> ConfigstringMonitorTemplate<GameMonitor>::callbackMap
{
	{ CS::SERVERINFO, &GameMonitor::parseServerInfo },
	{ CS::WARMUP, &GameMonitor::setWarmup },
	{ CS::LEVEL_START_TIME, &GameMonitor::setLevelStartTime },
	{ CS::MATCHEND, &GameMonitor::setMatchEnd }
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
	cgs->alliedText[0] = info.ValueForKey("g_obj_alliedtext1");
	cgs->alliedText[1] = info.ValueForKey("g_obj_alliedtext2");
	cgs->alliedText[2] = info.ValueForKey("g_obj_alliedtext3");
	cgs->alliedText[3] = info.ValueForKey("g_obj_alliedtext4");
	cgs->alliedText[4] = info.ValueForKey("g_obj_alliedtext5");
	cgs->axisText[0] = info.ValueForKey("g_obj_axistext1");
	cgs->axisText[1] = info.ValueForKey("g_obj_axistext2");
	cgs->axisText[2] = info.ValueForKey("g_obj_axistext3");
	cgs->axisText[3] = info.ValueForKey("g_obj_axistext4");
	cgs->axisText[4] = info.ValueForKey("g_obj_axistext5");
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
