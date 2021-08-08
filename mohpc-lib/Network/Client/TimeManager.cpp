#include <MOHPC/Network/Client/TimeManager.h>
#include <MOHPC/Network/Client/ServerSnapshot.h>
#include <MOHPC/Network/Client/Time.h>

using namespace MOHPC;
using namespace MOHPC::Network;

ClientTimeManager::ClientTimeManager(ClientTime& clientTimeRef, ServerSnapshotManager& snapshotManagerRef)
	: clientTime(clientTimeRef)
	, snapshotManager(snapshotManagerRef)
	, entered(false)
{
}

void ClientTimeManager::setTime(tickTime_t currentTime)
{
	ClientTimeManager timeManager(clientTime, snapshotManager);

	const bool adjustTime = snapshotManager.hasNewSnapshots();
	if (snapshotManager.checkTime(clientTime))
	{
		// at this point the client has entered the game
		clientTime.initRemoteTime(currentTime, snapshotManager.getServerTime());
		entered = true;
	}

	// if we have gotten to this point, cl.snap is guaranteed to be valid
	if (!snapshotManager.isSnapshotValid())
	{
		clientTime.setStartTime(currentTime);
		return;
	}

	clientTime.setTime(
		currentTime,
		snapshotManager.getServerTime(),
		adjustTime
	);
}

bool ClientTimeManager::hasEntered() const
{
	return entered;
}
