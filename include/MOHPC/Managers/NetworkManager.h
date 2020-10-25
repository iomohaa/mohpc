#pragma once

#include "Manager.h"
#include <exception>
#include "../Common/Container.h"
#include "../Common/str.h"
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
		MOHPC_EXPORTS NetworkManager();
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
	using NetworkManagerPtr = SharedPtr<NetworkManager>;

	class ITickableNetwork
	{
	private:
		WeakPtr<NetworkManager> owner;

	public:
		ITickableNetwork(const NetworkManagerPtr& networkManager);
		virtual ~ITickableNetwork();

		ITickableNetwork(const ITickableNetwork&) = delete;
		ITickableNetwork(ITickableNetwork&&) = delete;
		ITickableNetwork& operator=(const ITickableNetwork&) = delete;
		ITickableNetwork& operator=(ITickableNetwork&&) = delete;

		/** Tick function. */
		virtual void tick(uint64_t deltaTime, uint64_t currentTime) = 0;

		NetworkManagerPtr getManager() const;
	};

	namespace Network
	{
		uint64_t getCurrentTime();
	}
}
