#pragma once

#include "../NetGlobal.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
namespace Network
{
	class ClientTime
	{
	public:
		ClientTime();
		~ClientTime();

		/** Initialize the client time the new server time. */
		MOHPC_NET_EXPORTS void initRemoteTime(uint64_t currentTime, uint64_t remoteTimeValue);

		/** Set the clock time starting point for the client. */
		MOHPC_NET_EXPORTS void setStartTime(uint64_t currentTime);

		/** Set the server time starting point for the server. */
		MOHPC_NET_EXPORTS void setRemoteStartTime(uint64_t remoteTimeValue);

		/** Return the clock time at which the client entered the game. */
		MOHPC_NET_EXPORTS uint64_t getStartTime() const;

		/** Return the server time at which the client entered the game. */
		MOHPC_NET_EXPORTS uint64_t getRemoteStartTime() const;

		/** Return the current server time. */
		MOHPC_NET_EXPORTS uint64_t getRemoteTime() const;

		/** Return the frequency (milliseconds) at which the game server is running (1 / sv_fps). */
		MOHPC_NET_EXPORTS uint64_t getDeltaTime() const;

		/** Return the server delta time, in seconds. */
		MOHPC_NET_EXPORTS float getDeltaTimeSeconds() const;

		/** Set the new delta time. */
		MOHPC_NET_EXPORTS void setDeltaTime(uint64_t remoteDeltaTime);

		/** Set the new time to use as a starting point for the client and the server. */
		MOHPC_NET_EXPORTS void setTime(uint64_t newTime, uint64_t remoteTime, bool adjust);

		/** Return the extrapolation time. */
		MOHPC_NET_EXPORTS void setTimeNudge(int32_t timeNudgeValue);

	private:
		void adjustTimeDelta(uint64_t realTime, uint64_t remoteTime);
		uint64_t getTimeDelta(uint64_t time, uint64_t remoteTime) const;

	private:
		uint64_t realTimeStart;
		uint64_t serverStartTime;
		uint64_t serverTime;
		uint64_t serverDeltaTime;
		uint64_t oldServerTime;
		uint64_t oldFrameServerTime;
		uint64_t oldRealTime;
		float serverDeltaTimeSeconds;
		int32_t timeNudge;
		bool extrapolatedSnapshot;
	};
}
}