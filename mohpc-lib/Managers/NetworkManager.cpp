#include <MOHPC/Managers/NetworkManager.h>
#include <vector>
#include <functional>

using namespace MOHPC;

CLASS_DEFINITION(NetworkManager);

static Container<ITickableNetwork*> tickables;
uint64_t lastTickTime = 0;

static uintptr_t getCurrentTime()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
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

MOHPC::ITickableNetwork::ITickableNetwork()
{
	tickables.AddObject(this);
}

MOHPC::ITickableNetwork::~ITickableNetwork()
{
	tickables.RemoveObject(this);
}
