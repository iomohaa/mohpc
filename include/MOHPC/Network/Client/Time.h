#pragma once

#include "../NetGlobal.h"
#include "../../Utility/TickTypes.h"
#include "../Types/NetTime.h"

#include <cstdint>
#include <cstddef>
#include <exception>

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
		MOHPC_NET_EXPORTS void initRemoteTime(tickTime_t currentTime, netTime_t remoteTimeValue);

		/** Set the clock time starting point for the client. */
		MOHPC_NET_EXPORTS void setStartTime(tickTime_t currentTime);

		/** Set the server time starting point for the server. */
		MOHPC_NET_EXPORTS void setRemoteStartTime(netTime_t remoteTimeValue);

		/** Return the clock time at which the client entered the game. */
		MOHPC_NET_EXPORTS tickTime_t getStartTime() const;

		/** Return the server time at which the client entered the game. */
		MOHPC_NET_EXPORTS netTime_t getRemoteStartTime() const;

		/** Return the current remote time (non-simulated). */
		MOHPC_NET_EXPORTS netTime_t getRemoteTime() const;

		/** Return the previous remote time (non-simulated). */
		MOHPC_NET_EXPORTS netTime_t getOldRemoteTime() const;

		/** Return the current simulated server time. */
		MOHPC_NET_EXPORTS tickTime_t getSimulatedRemoteTime() const;

		/** Return the previous simulated server time. */
		MOHPC_NET_EXPORTS tickTime_t getOldSimulatedRemoteTime() const;

		/** Return the frequency (milliseconds) at which the game server is running (1 / sv_fps). */
		MOHPC_NET_EXPORTS deltaTime_t getDeltaTime() const;

		/** Return the server delta time, in seconds. */
		MOHPC_NET_EXPORTS deltaTimeFloat_t getDeltaTimeSeconds() const;

		/** Set the new delta time. */
		MOHPC_NET_EXPORTS void setDeltaTime(deltaTime_t remoteDeltaTime);

		/** Set the new time to use as a starting point for the client and the server. */
		MOHPC_NET_EXPORTS void setTime(tickTime_t newTime, netTime_t remoteTime, bool adjust);

		/** Return the extrapolation time. */
		MOHPC_NET_EXPORTS void setTimeNudge(int32_t timeNudgeValue);

	private:
		void adjustTimeDelta(tickTime_t realTime, netTime_t remoteTime);
		deltaTime_t getTimeDelta(tickTime_t time, netTime_t remoteTime) const;

	private:
		enum class extrapolation_e
		{
			None,
			Extrapolate,
			Catchup
		};
	private:
		tickTime_t realTimeStart;
		tickTime_t simulatedServerTime;
		tickTime_t oldSimulated;
		deltaTime_t serverDeltaTime;
		deltaTimeFloat_t serverDeltaTimeSeconds;
		netTime_t serverStartTime;
		netTime_t frameServerTime;
		netTime_t oldFrameServerTime;
		int32_t timeNudge;
		extrapolation_e extrapolation;
	};

	namespace ClientTimeErrors
	{
		class Base : public std::exception {};

		/**
		 * Exception when the server time went backward.
		 */
		class ServerTimeWentBackward
		{
		public:
			ServerTimeWentBackward(netTime_t oldTimeVal, netTime_t newTimeVal);

			netTime_t getOldTime() const;
			netTime_t getNewTime() const;

		private:
			netTime_t oldTime;
			netTime_t newTime;
		};
	}
}
}
