#pragma once

#include "Manager.h"
#include <exception>
#include "../Script/Container.h"
#include "../Script/str.h"
#include "../Network/Types.h"

namespace MOHPC
{
	/**
	 * Class for handling MOHPC networking
	 */
	class NetworkManager : public Manager
	{
		CLASS_BODY(NetworkManager);

	private:
		Container<class ITickableNetwork*> tickables;

	public:
		NetworkManager();
		~NetworkManager();

		NetworkManager(const NetworkManager&) = delete;
		NetworkManager& operator=(const NetworkManager&) = delete;
		NetworkManager(NetworkManager&&) = delete;
		NetworkManager& operator=(NetworkManager&&) = delete;

		MOHPC_EXPORTS bool hasAnyTicks() const;
		MOHPC_EXPORTS void processTicks();

		MOHPC_EXPORTS void addTickable(ITickableNetwork* tickable);
		MOHPC_EXPORTS void removeTickable(ITickableNetwork* tickable);
	};

	class MOHPC_EXPORTS ITickableNetwork
	{
	private:
		NetworkManager* owner;

	public:
		ITickableNetwork(NetworkManager* networkManager);
		virtual ~ITickableNetwork();

		ITickableNetwork(const ITickableNetwork&) = delete;
		ITickableNetwork(ITickableNetwork&&) = delete;
		ITickableNetwork& operator=(const ITickableNetwork&) = delete;
		ITickableNetwork& operator=(ITickableNetwork&&) = delete;

		/** Tick function. */
		virtual void tick(uint64_t deltaTime, uint64_t currentTime) = 0;

		NetworkManager* getManager() const;
	};

	namespace Network
	{
		uint64_t getCurrentTime();
	}
}
