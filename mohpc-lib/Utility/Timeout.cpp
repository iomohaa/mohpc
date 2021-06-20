#include <MOHPC/Utility/Timeout.h>

using namespace MOHPC;
using namespace std::chrono;

TimeoutTimer::TimeoutTimer()
{

}

TimeoutTimer::TimeoutTimer(std::chrono::milliseconds timeoutTimeValue)
	: timeoutTime(timeoutTimeValue)
{
}

void TimeoutTimer::setTimeoutTime(milliseconds timeoutTimeValue)
{
	timeoutTime = timeoutTimeValue;
}

std::chrono::milliseconds TimeoutTimer::getTimeoutTime() const
{
	return timeoutTime;
}

void TimeoutTimer::update()
{
	const steady_clock::time_point clockTime = steady_clock::now();
	updateTime = clockTime;
}

bool TimeoutTimer::hasTimedOut() const
{
	// Check if there is a timeout time first
	if (timeoutTime > milliseconds::zero())
	{
		const steady_clock::time_point clockTime = steady_clock::now();
		const steady_clock::time_point nextTimeoutTime = updateTime + timeoutTime;
		// Check if the clock has reached the timeout point
		if (clockTime >= nextTimeoutTime)
		{
			// it has timed out
			return true;
		}
	}

	return false;
}
