#include <MOHPC/Network/Remote/Socket.h>
#include "../../Network/Platform/network.h"

using namespace MOHPC;
using namespace Network;

static ISocketFactoryPtr defaultSocketFactory = Platform::Network::createSockFactory();
static ISocketFactoryWeakPtr socketFactory = defaultSocketFactory;

MOHPC::Network::ISocketFactory* MOHPC::Network::ISocketFactory::get()
{
	return socketFactory.lock().get();
}

void MOHPC::Network::ISocketFactory::set(const ISocketFactoryWeakPtr& newFactory)
{
	if (!newFactory.expired()) {
		socketFactory = newFactory;
	}
	else {
		socketFactory = defaultSocketFactory;
	}
}
