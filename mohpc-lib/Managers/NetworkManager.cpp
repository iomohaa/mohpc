#include <MOHPC/Managers/NetworkManager.h>
#include <vector>
#include <functional>

using namespace MOHPC;
using namespace Network;

CLASS_DEFINITION(NetworkManager);

uint64_t lastTickTime = 0;

uint64_t Network::getCurrentTime()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

NetworkManager::NetworkManager()
{

}

NetworkManager::~NetworkManager()
{
}

bool NetworkManager::hasAnyTicks() const
{
	return tickables.NumObjects() > 0;
}

void MOHPC::NetworkManager::processTicks()
{
	const uint64_t currentTime = getCurrentTime();
	const uint64_t deltaTime = currentTime - lastTickTime;
	lastTickTime = currentTime;

	for (size_t i = 0; i < tickables.NumObjects(); ++i)
	{
		// Tick every tickables objects
		ITickableNetwork* tickable = tickables[i];
		tickable->tick(deltaTime, currentTime);
	}
}

void MOHPC::NetworkManager::addTickable(ITickableNetwork* tickable)
{
	tickables.AddObject(tickable);
}

void MOHPC::NetworkManager::removeTickable(ITickableNetwork* tickable)
{
	tickables.RemoveObject(tickable);
}

MOHPC::ITickableNetwork::ITickableNetwork(NetworkManager* manager)
	: owner(manager)
{
	owner->addTickable(this);
}

MOHPC::NetworkManager* MOHPC::ITickableNetwork::getManager() const
{
	return owner;
}

MOHPC::ITickableNetwork::~ITickableNetwork()
{
	owner->removeTickable(this);
}
