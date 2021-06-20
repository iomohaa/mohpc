#include <MOHPC/Utility/MessageQueue.h>
#include <utility>

using namespace MOHPC;

IMessage::IMessage()
{
}

IMessage::~IMessage()
{
}

void IMessage::setDoneHandler(MessageDoneHandler&& doneFuncValue)
{
	doneFunc = std::move(doneFuncValue);
}

void IMessage::done()
{
	if (doneFunc) doneFunc();
}

MOHPC_OBJECT_DEFINITION(MessageQueue);

MessageQueue::MessageQueue(size_t limit)
	: numMaxMessages(limit)
	, numPendingResponses(0)
{
}

size_t MessageQueue::getNumPendingResponses() const
{
	return numPendingResponses;
}

void MessageQueue::transmit(const IMessagePtr& message)
{
	if (numPendingResponses < numMaxMessages)
	{
		// the message can be transmitted immediately
		transmitImmediately(message);
	}
	else
	{
		// enqueue the message
		messageQueue.push(message);
	}
}

void MessageQueue::transmitImmediately(const IMessagePtr& message)
{
	const size_t oldNum = numPendingResponses++;

	using namespace std::placeholders;
	message->setDoneHandler(std::bind(&MessageQueue::doneWith, this, message));
	message->transmit();
}

void MessageQueue::doneWith(const IMessagePtr& message)
{
	numPendingResponses--;
	// add reference to avoid crashes
	doneList.insert(message);
	message->setDoneHandler(nullptr);
}

void MessageQueue::processPendingMessages()
{
	if (numPendingResponses >= numMaxMessages)
	{
		// still waiting for other messages to finish
		return;
	}

	doneList.clear();

	const size_t numToTransmit = numMaxMessages - numPendingResponses;
	// transmit messages in the queue until the limit is met or until the queue is empty
	for(size_t i = 0; !messageQueue.empty() && i < numToTransmit; ++i)
	{
		const IMessagePtr& message = messageQueue.front();
		// now transmit the queued message
		transmitImmediately(message);
		messageQueue.pop();
	}
}
