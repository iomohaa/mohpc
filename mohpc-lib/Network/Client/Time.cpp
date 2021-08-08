#include <MOHPC/Network/Client/Time.h>

using namespace MOHPC;
using namespace MOHPC::Network;

ClientTime::ClientTime()
	: timeNudge(0)
	, serverDeltaTime(0)
	, serverDeltaTimeSeconds(0.f)
	, extrapolation(extrapolation_e::None)
{
}

ClientTime::~ClientTime()
{

}

void ClientTime::setTimeNudge(int32_t timeNudgeValue)
{
	timeNudge = timeNudgeValue;
}

void ClientTime::initRemoteTime(tickTime_t currentTime, netTime_t remoteTimeValue)
{
	using namespace std::chrono;

	serverStartTime = remoteTimeValue;
	simulatedServerTime = tickTime_t(currentTime - realTimeStart) + milliseconds(serverStartTime.time_since_epoch());
}

void ClientTime::setStartTime(tickTime_t currentTime)
{
	realTimeStart = currentTime;
}

void ClientTime::setRemoteStartTime(netTime_t remoteTimeValue)
{
	serverStartTime = remoteTimeValue;
}

tickTime_t ClientTime::getStartTime() const
{
	return realTimeStart;
}

netTime_t ClientTime::getRemoteStartTime() const
{
	return serverStartTime;
}

netTime_t ClientTime::getRemoteTime() const
{
	return oldFrameServerTime;
}

tickTime_t ClientTime::getSimulatedRemoteTime() const
{
	return simulatedServerTime;
}

deltaTime_t ClientTime::getDeltaTime() const
{
	return serverDeltaTime;
}

deltaTimeFloat_t ClientTime::getDeltaTimeSeconds() const
{
	return serverDeltaTimeSeconds;
}

void ClientTime::setDeltaTime(deltaTime_t remoteDeltaTime)
{
	using namespace std::chrono;

	serverDeltaTime = remoteDeltaTime;
	serverDeltaTimeSeconds = duration_cast<deltaTimeFloat_t>(serverDeltaTime);
}

void ClientTime::setTime(tickTime_t newTime, netTime_t remoteTime, bool adjust)
{
	using namespace ticks;

	// NOTE: don't think it's an issue
	if (remoteTime < oldFrameServerTime)
	{
		// received remote time < previous remote time
		throw ClientTimeErrors::ServerTimeWentBackward(oldFrameServerTime, remoteTime);
	}

	oldFrameServerTime = remoteTime;

	const tickTime_t simulated = time_cast<tickTime_t>(serverStartTime) + (newTime - realTimeStart);
	simulatedServerTime = simulated - milliseconds(timeNudge);

	// guarantee that time will never flow backwards, even if
	// serverTimeDelta made an adjustment or cl_timeNudge was changed
	if (simulatedServerTime < time_cast<tickTime_t>(oldSimulated)) {
		simulatedServerTime = time_cast<tickTime_t>(oldSimulated);
	}
	oldSimulated = simulatedServerTime;

	if (simulated + milliseconds(5) >= time_cast<tickTime_t>(remoteTime))
	{
		// slow down a little bit
		extrapolation = extrapolation_e::Extrapolate;
	}
	else if (simulated < time_cast<tickTime_t>(remoteTime) + milliseconds(5))
	{
		extrapolation = extrapolation_e::Catchup;
	}

	if (adjust) {
		adjustTimeDelta(newTime, remoteTime);
	}
}

void ClientTime::adjustTimeDelta(tickTime_t realTime, netTime_t remoteTime)
{
	using namespace std::chrono;
	constexpr deltaTime_t BASE_RESET_TIME(milliseconds(400));

	//const deltaTime_t maxDeltaTime = serverDeltaTime * 2;
	//const deltaTime_t resetTime = BASE_RESET_TIME + maxDeltaTime;
	constexpr deltaTime_t resetTime = milliseconds(500);
	constexpr deltaTime_t maxDeltaTime = milliseconds(100);
	const deltaTime_t deltaDelta = getTimeDelta(simulatedServerTime, remoteTime);

	if (deltaDelta > resetTime)
	{
		realTimeStart = realTime;
		serverStartTime = remoteTime;
		simulatedServerTime = time_cast<tickTime_t>(remoteTime);
		oldSimulated = simulatedServerTime;
	}
	else if (deltaDelta > maxDeltaTime)
	{
		// fast adjust
		serverStartTime -= duration_cast<netclock_t::duration>((deltaDelta - maxDeltaTime) / 2);
	}
	else
	{
		if (extrapolation == extrapolation_e::Extrapolate)
		{
			extrapolation = extrapolation_e::None;
			serverStartTime -= milliseconds(2);
		}
		else if (extrapolation == extrapolation_e::Catchup)
		{
			serverStartTime += milliseconds(1);
		}
	}
}

deltaTime_t ClientTime::getTimeDelta(tickTime_t time, netTime_t remoteTime) const
{
	if (time_cast<tickTime_t>(remoteTime) < time) {
		return time - time_cast<tickTime_t>(remoteTime);
	}
	else {
		return time_cast<tickTime_t>(remoteTime) - time;
	}
}

ClientTimeErrors::ServerTimeWentBackward::ServerTimeWentBackward(netTime_t oldTimeVal, netTime_t newTimeVal)
	: oldTime(oldTimeVal)
	, newTime(newTimeVal)
{
}

netTime_t ClientTimeErrors::ServerTimeWentBackward::getOldTime() const
{
	return oldTime;
}

netTime_t ClientTimeErrors::ServerTimeWentBackward::getNewTime() const
{
	return newTime;
}
