#pragma once

#include <chrono>

namespace MOHPC
{
	namespace ticks = std::chrono;

	using deltaTime_t = std::chrono::nanoseconds;
	using deltaTimeFloat_t = std::chrono::duration<float>;
	using deltaTimeMs_t = std::chrono::milliseconds;
	using deltaTime16_t = std::chrono::duration<uint16_t, std::milli>;
	using tickClock_t = std::chrono::steady_clock;
	using tickTime_t = std::chrono::time_point<tickClock_t>;
	using timeStampMs_t = std::chrono::milliseconds;

	template<typename Target, typename Source, typename Duration = typename Target::duration>
	constexpr Target time_cast(const Source& time) noexcept
	{
		return Target(std::chrono::duration_cast<Duration>(time.time_since_epoch()));
	}
}