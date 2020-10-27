#include "UnitTest.h"

#include <MOHPC/Managers/NetworkManager.h>
#include <MOHPC/Network/Server/ServerHost.h>
#include "platform.h"

#define MOHPC_LOG_NAMESPACE "testsrv"

using namespace MOHPC;
using namespace MOHPC::Network;

class CNetworkServerUnitTest : public IUnitTest, public TAutoInst<CNetworkServerUnitTest>
{
private:
	virtual unsigned int priority()
	{
		return 0;
	}

	virtual const char* name() override
	{
		return "Networking (Server)";
	}

	virtual void run(const MOHPC::AssetManagerPtr& AM) override
	{
		NetworkManagerPtr netMan = AM->GetManager<NetworkManager>();

		srand((unsigned int)time(NULL));

		ServerHostPtr host = ServerHost::create(netMan);
		
		for(;;)
		{
			netMan->processTicks();
			sleepTime(50);
		}
	}
};
static CNetworkServerUnitTest unitTest;
