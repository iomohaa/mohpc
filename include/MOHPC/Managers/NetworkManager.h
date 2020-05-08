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

	public:
		MOHPC_EXPORTS void processTicks();
	};

	class MOHPC_EXPORTS ITickableNetwork
	{
	public:
		ITickableNetwork();
		virtual ~ITickableNetwork();

		ITickableNetwork(const ITickableNetwork&) = delete;
		ITickableNetwork(ITickableNetwork&&) = delete;
		ITickableNetwork& operator=(const ITickableNetwork&) = delete;
		ITickableNetwork& operator=(ITickableNetwork&&) = delete;

		/** Tick function. */
		virtual void tick(uint64_t deltaTime, uint64_t currentTime) = 0;
	};
}
