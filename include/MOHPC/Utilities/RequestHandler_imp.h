#pragma once

#include "../Network/Types.h"
#include <algorithm>

namespace MOHPC
{
	inline RequestData::RequestData(IMessageStream& inStream, void** inParam)
		: stream(inStream)
		, param(inParam)
	{}

	template<class T, class Param>
	RequestHandler<T, Param>::RequestHandler()
	{
	}

	template<class T, class Param>
	RequestHandler<T, Param>::PendingRequest::PendingRequest(IRequestPtr&& inRequest, Param&& inParam, size_t inTimeout)
		: request(std::forward<IRequestPtr>(inRequest))
		, param(std::forward<Param>(inParam))
		, timeout(inTimeout)
	{
	}

	template<class T, class Param>
	void RequestHandler<T, Param>::sendRequest(IRequestPtr&& newRequest, Param&& inParam, size_t timeout)
	{
		if (isRequesting())
		{
			// Don't overwrite the active request
			// Queue the new request instead
			pendingRequests.emplace(std::forward<IRequestPtr>(newRequest), std::forward<Param>(inParam), timeout);
			return;
		}

		param = std::move(inParam);

		// Override the timeout if necessary
		startRequest(newRequest, timeout);

		if (newRequest->mustProcess()) {
			request = newRequest;
		}
		else {
			return dequeRequest();
		}
	}

	template<class T, class Param>
	void MOHPC::RequestHandler<T, Param>::startRequest(const IRequestPtr& newRequest, size_t timeout)
	{
		startTime = std::chrono::steady_clock::now();

		setDeferredStart(newRequest);
		setRequestTimeout(newRequest, timeout);
	}

	template<class T, class Param>
	void MOHPC::RequestHandler<T, Param>::setDeferredStart(const IRequestPtr& newRequest)
	{
		using namespace std::chrono;

		const size_t time = newRequest->deferredTime();
		if (time > 0) {
			deferTime = startTime + milliseconds(time);
		}
		else
		{
			deferTime = time_point<steady_clock>(milliseconds(0));
			// send data right now
			sendData(newRequest);
		}
	}

	template<class T, class Param>
	void MOHPC::RequestHandler<T, Param>::setRequestTimeout(const IRequestPtr& newRequest, size_t timeout)
	{
		using namespace std::chrono;

		bool overriden = false;
		size_t time = newRequest->overrideTimeoutTime(overriden);
		if (!overriden) {
			time = timeout + newRequest->timeOutDelay();
		}

		if (deferTime == time_point<steady_clock>(milliseconds(0))) {
			timeoutTime = startTime + milliseconds(time);
		}
		else
		{
			// timeout after the request has started
			timeoutTime = deferTime + milliseconds(time);
		}
	}

	template<class T, class Param>
	void MOHPC::RequestHandler<T, Param>::sendData(const IRequestPtr& newRequest)
	{
		DynamicDataMessageStream output;
		newRequest->generateOutput(output);

		if (output.GetPosition())
		{
			// Send query
			param.send(output.getStorage(), output.GetPosition());
		}
	}

	template<class T, class Param>
	void RequestHandler<T, Param>::dequeRequest()
	{
		if (!pendingRequests.empty())
		{
			PendingRequest& pendingReq = pendingRequests.front();
			// Dequeue the first pending request and send it
			IRequestPtr newRequest = pendingReq.request;
			if (newRequest)
			{
				param = std::move(pendingReq.param);
				const size_t timeout = pendingReq.timeout;
				pendingRequests.pop();
				return sendRequest(std::move(newRequest), std::move(param), timeout);
			}
		}
	}

	template<class T, class Param>
	void RequestHandler<T, Param>::handle()
	{
		if (!isRequesting())
		{
			// nothing to process
			return;
		}

		if (processDeferred())
		{
			// something was delayed
			return;
		}

		const size_t maxSize = param.receiveSize();
		uint8_t* data = new uint8_t[maxSize];

		bool shouldRetry = true;
		while (shouldRetry)
		{
			const size_t len = param.receive((uint8_t*)data, maxSize);
			if (len && len != -1)
			{
				FixedDataMessageStream stream(data, len);
				RequestData data(stream, (void**)&param);

				// Process the request using the data
				IRequestPtr newRequest = staticPointerCast<T>(currentRequest()->process(data));
				handleNewRequest(std::move(newRequest), false);

				shouldRetry = currentRequest() && param.hasData();
			}
			else {
				shouldRetry = false;
			}
		}

		delete[] data;
	}

	template<class T, class Param>
	bool MOHPC::RequestHandler<T, Param>::processDeferred()
	{
		using namespace std::chrono;
		time_point<steady_clock> currentTime = steady_clock::now();

		if (deferTime != time_point<steady_clock>(milliseconds(0)))
		{
			if (currentTime < deferTime)
			{
				// start request at a later time
				return true;
			}

			// nullify time
			deferTime = time_point<steady_clock>(milliseconds(0));
			sendData(request);
		}

		if (!param.hasData())
		{
			if (timeoutTime != startTime)
			{
				if (currentTime >= timeoutTime)
				{
					IRequestPtr newRequest = staticPointerCast<T>(currentRequest()->timedOut());
					handleNewRequest(std::move(newRequest), true);
					return true;
				}
			}

			// don't get stuck in receive due to the absence of data
			return true;
		}

		return false;
	}

	template<class T, class Param>
	void MOHPC::RequestHandler<T, Param>::handleNewRequest(IRequestPtr&& newRequest, bool shouldResend)
	{
		if (newRequest)
		{
			// Send the new request if specified
			if (newRequest != request)
			{
				// Clear the current request to avoid queueing
				request.reset();

				using namespace std::chrono;
				milliseconds tm = duration_cast<milliseconds>(timeoutTime - startTime);

				const size_t timeout = tm.count();
				sendRequest(std::forward<IRequestPtr>(newRequest), std::move(param), timeout);
			}
			else if(shouldResend)
			{
				using namespace std::chrono;
				milliseconds tm = duration_cast<milliseconds>(timeoutTime - startTime);

				// refresh the timeout time
				const size_t timeout = tm.count();
				startRequest(newRequest, timeout);
			}
		}
		else
		{
			// Clear current request and dequeue
			request.reset();
			dequeRequest();
		}
	}

	template<class T, class Param>
	bool RequestHandler<T, Param>::isRequesting() const
	{
		return (bool)request;
	}

	template<class T, class Param>
	const typename RequestHandler<T, Param>::IRequestPtr& RequestHandler<T, Param>::currentRequest() const
	{
		return request;
	}
}
