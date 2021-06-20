#include <MOHPC/Utility/Tick.h>

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

}

TickableObjects::~TickableObjects()
{
	const size_t numTickables = tickables.NumObjects();
	for (size_t i = 0; i < numTickables; ++i)
	{
		tickables[i]->owner = nullptr;
	}
}

bool TickableObjects::hasAnyTicks() const
{
	return tickables.NumObjects() > 0;
}

void TickableObjects::processTicks()
{
	using namespace std::chrono;
	const time_point<steady_clock> currentTime = steady_clock::now();
	const nanoseconds deltaTime = currentTime - lastTickTime;
	lastTickTime = currentTime;

	const milliseconds msSinceEpoch = duration_cast<milliseconds>(currentTime.time_since_epoch());

	const size_t numTickables = tickables.NumObjects();
	for (size_t i = 0; i < numTickables; ++i)
	{
		// Tick every tickables objects
		ITickable* tickable = tickables[i];
		tickable->tick(deltaTime.count(), msSinceEpoch.count());
	}
}

void TickableObjects::addTickable(ITickable* tickable)
{
	tickable->owner = this;
	tickables.AddObject(tickable);
}

void TickableObjects::removeTickable(ITickable* tickable)
{
	tickables.RemoveObject(tickable);
	tickable->owner = nullptr;
}
