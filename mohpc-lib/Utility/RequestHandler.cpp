#include <MOHPC/Utility/RequestHandler.h>
#include <MOHPC/Utility/MessageDispatcher.h>

using namespace MOHPC;
using namespace std::chrono;

InputRequest::InputRequest(IMessageStream& inStream, const IRemoteIdentifierPtr& inIdentifier)
	: stream(inStream)
	, identifier(inIdentifier)
{}

IncomingMessageHandler::PendingRequest::PendingRequest(IRequestPtr&& inRequest, uint64_t inTimeout)
	: request(std::forward<IRequestPtr>(inRequest))
	, timeout(inTimeout)
{}

IncomingMessageHandler::IncomingMessageHandler(MessageDispatcher* inDispatcher, const ICommunicatorPtr& inCommunicator, const IRemoteIdentifierPtr& inRemoteId)
	: dispatcher(inDispatcher)
	, comm(inCommunicator)
	, next(nullptr)
	, prev(nullptr)
	, remoteId(inRemoteId)
{
	dispatcher->addHandler(this);
}

IncomingMessageHandler::~IncomingMessageHandler()
{
	if (dispatcher) dispatcher->removeHandler(this);
}

void IncomingMessageHandler::handleIncoming(IMessageStream& stream, const IRemoteIdentifierPtr& incomingId)
{
	if(isRequesting())
	{
		InputRequest input(stream, incomingId);

		const size_t savedPos = stream.GetPosition();

		if (currentRequest()->isThisRequest(input))
		{
			milliseconds timeoutDuration = duration_cast<milliseconds>(timeoutTime - startTime);
			time_point<steady_clock> currentTime = steady_clock::now();
			// set the new timeout time
			timeoutTime = currentTime + timeoutDuration;

			// restore the stream position
			stream.Seek(savedPos);

			IRequestPtr newRequest = currentRequest()->process(input);
			handleNewRequest(std::move(newRequest), false);
		}
	}
}

void IncomingMessageHandler::sendRequest(IRequestPtr&& newRequest, uint64_t timeout)
{
	if (isRequesting())
	{
		// Don't overwrite the active request
		// Queue the new request instead
		pendingRequests.emplace(std::forward<IRequestPtr>(newRequest), timeout);
		return;
	}

	// Override the timeout if necessary
	startRequest(newRequest, timeout);

	if (newRequest->mustProcess()) {
		request = newRequest;
	}
	else {
		return dequeRequest();
	}
}

bool IncomingMessageHandler::isRequesting() const
{
	return request != nullptr;
}

IncomingMessageHandler::time_point<IncomingMessageHandler::steady_clock> IncomingMessageHandler::getDeferredTime() const
{
	return deferTime;
}

const MOHPC::IRemoteIdentifierPtr& IncomingMessageHandler::getRemoteId() const
{
	return remoteId;
}

bool IncomingMessageHandler::expect(InputRequest& input)
{
	if (isRequesting() && request) {
		return request->isThisRequest(input);
	}

	return false;
}

bool MOHPC::IncomingMessageHandler::processDeferred()
{
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

	return false;
}

const ICommunicatorPtr& IncomingMessageHandler::getComm() const
{
	return comm;
}

const MOHPC::IRequestPtr& IncomingMessageHandler::currentRequest() const
{
	return request;
}

void IncomingMessageHandler::dequeRequest()
{
	if (!pendingRequests.empty())
	{
		PendingRequest& pendingReq = pendingRequests.front();
		// Dequeue the first pending request and send it
		IRequestPtr newRequest = pendingReq.request;
		if (newRequest)
		{
			const uint64_t timeout = pendingReq.timeout;
			pendingRequests.pop();
			return sendRequest(std::move(newRequest), timeout);
		}
	}
}

void IncomingMessageHandler::sendData(const IRequestPtr& newRequest)
{
	DynamicDataMessageStream output;
	newRequest->generateOutput(output);

	if (output.GetPosition())
	{
		// Send query
		comm->send(*remoteId, output.getStorage(), output.GetLength());
	}
}

void IncomingMessageHandler::setRequestTimeout(const IRequestPtr& newRequest, uint64_t timeout)
{
	bool overriden = false;
	uint64_t time = newRequest->overrideTimeoutTime(overriden);
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

void IncomingMessageHandler::setDeferredStart(const IRequestPtr& newRequest)
{
	const uint64_t time = newRequest->deferredTime();
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

void IncomingMessageHandler::startRequest(const IRequestPtr& newRequest, uint64_t timeout)
{
	startTime = steady_clock::now();

	setDeferredStart(newRequest);
	setRequestTimeout(newRequest, timeout);
}

void IncomingMessageHandler::handleNewRequest(IRequestPtr&& newRequest, bool shouldResend)
{
	if (newRequest)
	{
		// Send the new request if specified
		if (newRequest != request)
		{
			// Clear the current request to avoid queueing
			request.reset();

			milliseconds tm = duration_cast<milliseconds>(timeoutTime - startTime);

			const uint64_t timeout = tm.count();
			sendRequest(std::forward<IRequestPtr>(newRequest), timeout);
		}
		else if (shouldResend)
		{
			milliseconds tm = duration_cast<milliseconds>(timeoutTime - startTime);

			// refresh the timeout time
			const uint64_t timeout = tm.count();
			startRequest(newRequest, timeout);
		}
	}
	else
	{
		// Clear current request and dequeue
		IRequestPtr tempRequest = std::move(request);
		dequeRequest();
	}
}

bool IncomingMessageHandler::processTimeOut()
{
	if(!isRequesting() || timeoutTime == startTime) {
		return false;
	}

	time_point<steady_clock> currentTime = steady_clock::now();
	if (currentTime >= timeoutTime)
	{
		IRequestPtr newRequest = currentRequest()->timedOut();
		handleNewRequest(std::move(newRequest), true);
		return true;
	}

	return false;
}
