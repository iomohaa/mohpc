#include <MOHPC/Network/Client/Time.h>

using namespace MOHPC;
using namespace MOHPC::Network;

ClientTime::ClientTime()
	: realTimeStart(0)
	, serverStartTime(0)
	, serverTime(0)
	, oldServerTime(0)
	, oldFrameServerTime(0)
	, oldRealTime(0)
	, timeNudge(0)
{
}

ClientTime::~ClientTime()
{

}

void ClientTime::setTimeNudge(int32_t timeNudgeValue)
{
	timeNudge = timeNudgeValue;
}

void ClientTime::initRemoteTime(uint64_t currentTime, uint64_t remoteTimeValue)
{
	serverStartTime = remoteTimeValue;
	//serverTime = (uint32_t)(currentSnap.serverTime - currentTime);
	serverTime = serverStartTime + (currentTime - realTimeStart);
}

void ClientTime::setStartTime(uint64_t currentTime)
{
	realTimeStart = currentTime;
}

void ClientTime::setRemoteStartTime(uint64_t remoteTimeValue)
{
	serverStartTime = remoteTimeValue;
}

uint64_t ClientTime::getStartTime() const
{
	return realTimeStart;
}

uint64_t ClientTime::getRemoteStartTime() const
{
	return serverStartTime;
}

uint64_t ClientTime::getRemoteTime() const
{
	return serverTime;
}

uint64_t ClientTime::getDeltaTime() const
{
	return serverDeltaTime;
}

float ClientTime::getDeltaTimeSeconds() const
{
	return serverDeltaTimeSeconds;
}

void ClientTime::setDeltaTime(uint64_t remoteDeltaTime)
{
	serverDeltaTime = remoteDeltaTime;
	serverDeltaTimeSeconds = serverDeltaTime / 1000.f;
}

void ClientTime::setTime(uint64_t newTime, uint64_t remoteTime, bool adjust)
{
	// FIXME: throw if snap remote time went backward

	oldFrameServerTime = remoteTime;

	const uint64_t realTime = newTime;
	const uint64_t realServerTime = realTime - realTimeStart + oldServerTime;
	serverTime = realServerTime - timeNudge;
	oldRealTime = newTime;

	// guarantee that time will never flow backwards, even if
	// serverTimeDelta made an adjustment or cl_timeNudge was changed
	if (serverTime < oldServerTime) {
		serverTime = oldServerTime;
	}

	if (realServerTime + 5 >= remoteTime) {
		extrapolatedSnapshot = true;
	}

	if (adjust) {
		adjustTimeDelta(realTime, remoteTime);
	}
}

void ClientTime::adjustTimeDelta(uint64_t realTime, uint64_t remoteTime)
{
	constexpr size_t BASE_RESET_TIME = 400;

	const uint64_t maxDeltaTime = serverDeltaTime * 2;
	const uint64_t resetTime = BASE_RESET_TIME + maxDeltaTime;
	const uint64_t deltaDelta = getTimeDelta(serverTime, remoteTime);

	if (deltaDelta > resetTime)
	{
		realTimeStart = realTime;
		oldServerTime = remoteTime;
		serverTime = remoteTime;
	}
	else if (deltaDelta > maxDeltaTime)
	{
		// fast adjust
		realTimeStart += maxDeltaTime - deltaDelta;
	}
	else
	{
		if (extrapolatedSnapshot)
		{
			extrapolatedSnapshot = false;
			realTimeStart += 2;
		}
		else {
			realTimeStart -= 1;
		}
	}
}

uint64_t ClientTime::getTimeDelta(uint64_t time, uint64_t remoteTime) const
{
	if (remoteTime < time) {
		return time - remoteTime;
	}
	else {
		return remoteTime - time;
	}
}
