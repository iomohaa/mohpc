#include <MOHPC/Utilities/MessageDispatcher.h>
#include <MOHPC/Utilities/RequestHandler.h>

using namespace MOHPC;

bool IRemoteIdentifier::operator==(const IRemoteIdentifier& other) const
{
	return isIdentifier(other);
}

bool IRemoteIdentifier::operator!=(const IRemoteIdentifier& other) const
{
	return !isIdentifier(other);
}

MOHPC_OBJECT_DEFINITION(MessageDispatcher);

MessageDispatcher::MessageDispatcher()
{
	commList.Resize(1);
	root = last = nullptr;
}

MessageDispatcher::~MessageDispatcher()
{
	for(IncomingMessageHandler* p = root; p; p = p->next) {
		p->dispatcher = nullptr;
	}
}

void MessageDispatcher::addHandler(IncomingMessageHandler* handler)
{
	if(!root) root = handler;
	if(last) last->next = handler;

	handler->prev = last;
	last = handler;
}

void MessageDispatcher::removeHandler(IncomingMessageHandler* handler)
{
	if(handler == root) root = root->next;
	if(handler == last) last = last->prev;
	if(handler->next) handler->next->prev = handler->prev;
	if(handler->prev) handler->prev->next = handler->next;
}

bool MessageDispatcher::needsProcessing() const
{
	for (const IncomingMessageHandler* p = root; p; p = p->next)
	{
		if(p->isRequesting()) {
			return true;
		}
	}

	return false;
}

uint64_t MessageDispatcher::getWaitTime() const
{
	using namespace std::chrono;

	time_point<steady_clock> minDeferredTime;
	// get the first non-zero deferred time
	for (IncomingMessageHandler* p = root; p; p = p->next)
	{
		const time_point<steady_clock> deferredTime = p->getDeferredTime();
		if(deferredTime != time_point<steady_clock>(milliseconds(0)))
		{
			minDeferredTime = deferredTime;
			break;
		}
	}

	// get the nearest time to process deferred requests
	for (IncomingMessageHandler* p = root; p; p = p->next)
	{
		const time_point<steady_clock> deferredTime = p->getDeferredTime();
		if (deferredTime != time_point<steady_clock>(milliseconds(0)) && deferredTime < minDeferredTime) {
			minDeferredTime = deferredTime;
		}
	}

	if(minDeferredTime != time_point<steady_clock>(milliseconds(0)))
	{
		time_point<steady_clock> currentTime = steady_clock::now();
		if(currentTime > minDeferredTime)
		{
			// avoid waiting and process the deferred request now
			return 0;
		}

		return duration_cast<milliseconds>(minDeferredTime - currentTime).count();
	}

	const size_t numObjects = commList.NumObjects();
	return 1000 / numObjects;
}

bool MessageDispatcher::processIncomingMessages(uint64_t maxProcessTime)
{
	using namespace std::chrono;

	time_point<steady_clock> currentTime = steady_clock::now();
	const time_point<steady_clock> maxTime = currentTime + milliseconds(maxProcessTime);

	bool processed = false;
	while(needsProcessing())
	{
		IncomingMessageHandler* next;

		const size_t numComms = commList.NumObjects();
		if(numComms)
		{
			const uint64_t waitTime = std::min(getWaitTime(), maxProcessTime);
			for (size_t i = 0; i < numComms; ++i)
			{
				// wait for a communicator to receive a message
				processed |= processComm(*commList[i], waitTime);
			}
		}

		// process awaiting requests
		for (IncomingMessageHandler* p = root; p; p = next)
		{
			next = p->next;
			p->processDeferred();
		}

		// process timed out requests
		for (IncomingMessageHandler* p = root; p; p = next)
		{
			next = p->next;
			p->processTimeOut();
		}

		if(maxProcessTime == 0)
		{
			// No wait
			break;
		}
		else if(maxProcessTime != -1)
		{
			currentTime = steady_clock::now();
			if (currentTime >= maxTime)
			{
				// Max time reached
				break;
			}
		}
	}

	return processed;
}

void MessageDispatcher::addComm(const ICommunicatorPtr& comm)
{
	commList.AddObject(comm);
}

void MessageDispatcher::clearComms()
{
	commList.FreeObjectList();
}

bool MessageDispatcher::processComm(ICommunicator& comm, uint64_t maxWaitTime)
{
	// wait for a message
	if (!comm.waitIncoming(maxWaitTime))
	{
		// not received any message
		return false;
	}

	// a message was received
	const size_t messageSize = comm.getIncomingSize();
	if (messageSize > 0)
	{
		uint8_t* buf = new uint8_t[messageSize];
		IRemoteIdentifierPtr remoteAddress;
		// receive and get remote address
		const size_t receivedSize = comm.receive(remoteAddress, buf, messageSize);
		assert(remoteAddress != nullptr);

		IncomingMessageHandler* next;
		// check for handlers that are attached to the identifier
		for (IncomingMessageHandler* p = root; p; p = next)
		{
			next = p->next;
			if (!p->isRequesting())
			{
				// skip inactive handlers
				continue;
			}

			if (p->getComm().get() == &comm && (!p->remoteId || *p->remoteId == *remoteAddress))
			{
				// handle the incoming request
				FixedDataMessageStream stream(buf, receivedSize, receivedSize);
				p->handleIncoming(stream, remoteAddress);
			}
		}
	}
	else
	{
		// empty message, ignore
		uint8_t nullBuffer = 0;
		IRemoteIdentifierPtr remoteAddress;
		comm.receive(remoteAddress, &nullBuffer, 1);
	}

	return true;
}
