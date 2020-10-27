#pragma once

#include "SharedPtr.h"
#include "../Misc/MSG/Stream.h"
#include <queue>
#include <stdint.h>
#include <chrono>

namespace MOHPC
{
	class str;
	class IMessageStream;

	class RequestData
	{
	public:
		IMessageStream& stream;
		void** param;

	public:
		RequestData(IMessageStream& inStream, void** inParam);

		template<typename Param>
		const Param& getParam() const
		{
			return reinterpret_cast<Param&>(*param);
		}
	};

	class IRequestBase
	{
	public:
		virtual ~IRequestBase() = default;

		/** Return a supplied info request string. */
		virtual void generateOutput(IMessageStream& output) = 0;

		/** Return true if the request has done processing. */
		virtual bool mustProcess() const { return true; };

		/** Return another request to execute, or finish it by returning NULL. */
		virtual SharedPtr<IRequestBase> process(RequestData& data) = 0;

		/** Delay added to the base timeout time. */
		virtual size_t timeOutDelay() { return 0; }

		/** Delay the time at which to start the request. */
		virtual size_t deferredTime() { return 0; }

		/** Override the timeout time. */
		virtual size_t overrideTimeoutTime(bool& overriden) { return 0; }

		/** Called when the timeout time has reached, giving a chance the request to retry, skip to another one or abort. */
		virtual SharedPtr<IRequestBase> timedOut() { return nullptr; };
	};
	using IRequestPtr = SharedPtr<IRequestBase>;

	/** Template for request parameters. */
	class RequestParam
	{
	public:
		void send(const uint8_t* buf, size_t size) {};
		size_t receive(uint8_t* buf, size_t size) { return 0; };
		bool hasData() { return false; };
		size_t receiveSize() { return 0; }
	};

	template<class T = IRequestBase, class Param = RequestParam>
	class RequestHandler
	{
	private:
		using IRequestPtr = SharedPtr<T>;

		struct PendingRequest
		{
			IRequestPtr request;
			Param param;
			size_t timeout;

			PendingRequest(IRequestPtr&& inRequest, Param&& inParam, size_t timeout);
		};

		template<typename TimeType>
		using time_point = std::chrono::time_point<TimeType>;
		using steady_clock = std::chrono::steady_clock;

	private:
		IRequestPtr request;
		Param param;
		time_point<steady_clock> startTime;
		time_point<steady_clock> timeoutTime;
		time_point<steady_clock> deferTime;
		std::queue<PendingRequest> pendingRequests;

	public:
		RequestHandler();

		/**
		 * Handle requests
		 *
		 * @param	param	The way data will be sent and received.
		 */
		void handle();

		/**
		 * Send a request or enqueue it if it can't be sent right now
		 *
		 * @param	newRequest	request to send.
		 * @param	param		The way the data will be sent.
		 * @param	timeout		Timeout in ms.
		 */
		void sendRequest(IRequestPtr&& newRequest, Param&& param, size_t timeout = 0);

		/** Return true if a request is waiting a response. */
		bool isRequesting() const;

	private:
		bool processDeferred();
		void handleNewRequest(IRequestPtr&& newRequest, bool shouldResend);
		void startRequest(const IRequestPtr& newRequest, size_t timeout);
		void setDeferredStart(const IRequestPtr& newRequest);
		void setRequestTimeout(const IRequestPtr& newRequest, size_t timeout);
		void sendData(const IRequestPtr& newRequest);
		void dequeRequest();
		void clearRequest();
		const IRequestPtr& currentRequest() const;
	};
}

#include "RequestHandler_imp.h"
