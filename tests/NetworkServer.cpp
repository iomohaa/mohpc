#include <MOHPC/Network/Server/ServerHost.h>
#include "Common/Common.h"
#include "Common/platform.h"

#define MOHPC_LOG_NAMESPACE "testsrv"

using namespace MOHPC;
using namespace MOHPC::Network;

int main(int argc, const char* argv[])
{
	const MOHPC::AssetManagerPtr AM = AssetLoad();

	srand((unsigned int)time(NULL));

	const TickableObjectsPtr tickableObjects = TickableObjects::create();
	ServerHostPtr host = ServerHost::create();
	tickableObjects->addTickable(host.get());

	for (;;)
	{
		tickableObjects->processTicks();
		sleepTime(50);
	}
	
	return 0;
}
