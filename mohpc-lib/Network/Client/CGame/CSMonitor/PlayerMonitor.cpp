#include <MOHPC/Network/Client/CGame/CSMonitor/PlayerMonitor.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(PlayerMonitor);

PlayerMonitor::PlayerMonitor(const SnapshotProcessorPtr& snapshotProcessor, const ClientInfoListPtr& clientListPtr)
	: ConfigstringMonitor(snapshotProcessor)
	, clientList(clientListPtr)
{
}

PlayerMonitor::~PlayerMonitor()
{
}

bool PlayerMonitor::configStringModified(csNum_t num, const char* cs)
{
	if (num >= CS::PLAYERS && num < CS::PLAYERS + CS::MAX_CLIENTS)
	{
		ReadOnlyInfo info(cs);

		const uint32_t clientNum = num - CS::PLAYERS;
		clientList->set(info, clientNum);

		return true;
	}
	else if(num >= CS::WEAPONS && num < CS::WEAPONS + CS::MAX_WEAPONS)
	{
		weaponModified(num - CS::WEAPONS, cs);

		return true;
	}
	else
	{
		if (num == CS::TEAMS)
		{
			teamsModified(cs);

			return true;
		}
		else if (num == CS::GENERAL_STRINGS)
		{
			stringsModified(cs);

			return true;
		}
		else if (num == CS::SPECTATORS)
		{
			spectatorsModified(cs);

			return true;
		}
		else if (num == CS::ALLIES)
		{
			alliesModified(cs);

			return true;
		}
		else if (num == CS::AXIS)
		{
			axisModified(cs);

			return true;
		}
	}

	return false;
}

PlayerMonitor::HandlerList& PlayerMonitor::getHandlerList()
{
	return handlerList;
}

void PlayerMonitor::parseTeam(const char* data, uint32_t& type, uint32_t& count)
{
	sscanf(data, "%d %*s %d", &type, &count);
}

void PlayerMonitor::weaponModified(size_t index, const char* cs)
{
	getHandlerList().weaponModifiedHandler.broadcast(index, cs);
}

void PlayerMonitor::teamsModified(const char* cs)
{
	getHandlerList().teamsModifiedHandler.broadcast(atoi(cs));
}

void PlayerMonitor::stringsModified(const char* cs)
{
	getHandlerList().stringsModifiedHandler.broadcast(cs);
}

void PlayerMonitor::spectatorsModified(const char* cs)
{
	uint32_t type, count;
	parseTeam(cs, type, count);

	getHandlerList().teamCountModifiedHandler.broadcast(teamType_e::Spectator, type, count);
}

void PlayerMonitor::alliesModified(const char* cs)
{
	uint32_t type, count;
	parseTeam(cs, type, count);

	getHandlerList().teamCountModifiedHandler.broadcast(teamType_e::Allies, type, count);
}

void PlayerMonitor::axisModified(const char* cs)
{
	uint32_t type, count;
	parseTeam(cs, type, count);

	getHandlerList().teamCountModifiedHandler.broadcast(teamType_e::Axis, type, count);
}
