#pragma once

#include "../NetGlobal.h"
#include "../../Utility/TickTypes.h"

#include <cstdint>
#include <chrono>

namespace MOHPC
{
namespace Network
{
	struct netclock_t {
		using rep = uint32_t;
		using period = std::milli;
		using duration = std::chrono::duration<uint32_t, period>;
		using time_point = std::chrono::time_point<netclock_t>;
		static constexpr bool is_steady = true;
	};

	using netTime_t = std::chrono::time_point<netclock_t>;

	MOHPC_NET_EXPORTS netTime_t netTimeFromTick(tickTime_t time);
	MOHPC_NET_EXPORTS tickTime_t tickFromNetTime(netTime_t time);
}
}