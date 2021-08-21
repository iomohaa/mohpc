#include <MOHPC/Utility/Tick.h>
#include <MOHPC/Utility/TickTypes.h>

#include <algorithm>

using namespace MOHPC;

ITickable::ITickable()
{

}

ITickable::~ITickable()
{
	if(owner) {
		owner->removeTickable(this);
	}
}

MOHPC_OBJECT_DEFINITION(TickableObjects);

TickableObjects::TickableObjects()
{
	lastTickTime = tickClock_t::now();
}

TickableObjects::~TickableObjects()
{
	const size_t numTickables = tickables.size();
	for (size_t i = 0; i < numTickables; ++i)
	{
		tickables[i]->owner = nullptr;
	}
}

bool TickableObjects::hasAnyTicks() const
{
	return tickables.size() > 0;
}

void TickableObjects::processTicks()
{
	const tickTime_t currentTime = tickClock_t::now();
	const tickClock_t::duration deltaTime = currentTime - lastTickTime;
	lastTickTime = currentTime;

	const size_t numTickables = tickables.size();
	for (size_t i = 0; i < numTickables; ++i)
	{
		// Tick every tickables objects
		ITickable* tickable = tickables[i];
		tickable->tick(deltaTime, currentTime);
	}
}

void TickableObjects::addTickable(ITickable* tickable)
{
	tickable->owner = this;
	tickables.push_back(tickable);
}

void TickableObjects::removeTickable(ITickable* tickable)
{
	auto it = std::find(tickables.begin(), tickables.end(), tickable);
	if (it != tickables.end()) tickables.erase(it);
	tickable->owner = nullptr;
}
