#pragma once

#include "UtilityGlobal.h"

#include <chrono>

namespace MOHPC
{
	class TimeoutTimer
	{
	public:
		MOHPC_UTILITY_EXPORTS TimeoutTimer();
		MOHPC_UTILITY_EXPORTS TimeoutTimer(std::chrono::milliseconds timeoutTimeValue);

		MOHPC_UTILITY_EXPORTS void setTimeoutTime(std::chrono::milliseconds timeoutTimeValue);
		MOHPC_UTILITY_EXPORTS std::chrono::milliseconds getTimeoutTime() const;

		MOHPC_UTILITY_EXPORTS void update();
		MOHPC_UTILITY_EXPORTS bool hasTimedOut() const;

	private:
		std::chrono::milliseconds timeoutTime;
		std::chrono::steady_clock::time_point updateTime;
	};
}
