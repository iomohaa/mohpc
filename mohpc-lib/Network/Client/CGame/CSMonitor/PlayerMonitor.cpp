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

void PlayerMonitor::configStringModified(csNum_t num, const char* cs)
{
	if (num >= CS::PLAYERS && num < CS::PLAYERS + CS::MAX_CLIENTS)
	{
		ReadOnlyInfo info(cs);

		const uint32_t clientNum = num - CS::PLAYERS;
		clientList->set(info, clientNum);
	}
}
