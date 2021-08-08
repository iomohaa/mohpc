#pragma once

#include "UtilityGlobal.h"
#include "UtilityObject.h"
#include "TickTypes.h"

#include <vector>
#include <chrono>

namespace MOHPC
{
	class TickableObjects;

	/**
	 * Object inheriting from this interface can be ticked
	 * with the help of a tickable object.
	 */
	class MOHPC_UTILITY_EXPORTS ITickable
	{
	public:
		ITickable();
		virtual ~ITickable();

		ITickable(const ITickable&) = delete;
		ITickable(ITickable&&) = delete;
		ITickable& operator=(const ITickable&) = delete;
		ITickable& operator=(ITickable&&) = delete;

	public:
		/** Tick function. */
		virtual void tick(deltaTime_t deltaTime, tickTime_t currentTime) = 0;

	private:
		TickableObjects* owner;

		friend class TickableObjects;
	};

	/**
	 * Contains all objects that can be ticked in a loop.
	 */
	class TickableObjects
	{
	public:
		MOHPC_UTILITY_OBJECT_DECLARATION(TickableObjects);

	public:
		MOHPC_UTILITY_EXPORTS TickableObjects();
		~TickableObjects();
		MOHPC_UTILITY_EXPORTS bool hasAnyTicks() const;
		MOHPC_UTILITY_EXPORTS void processTicks();

		MOHPC_UTILITY_EXPORTS void addTickable(ITickable* tickable);
		MOHPC_UTILITY_EXPORTS void removeTickable(ITickable* tickable);

	private:
		std::vector<ITickable*> tickables;
		tickTime_t lastTickTime;
	};
	using TickableObjectsPtr = SharedPtr<TickableObjects>;
}