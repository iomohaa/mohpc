#pragma once

#include <chrono>

namespace MOHPC
{
	/**
	 * The message queue dispatcher allows to dispatch and queue messages.
	 */
	class MessageQueueDispatcher
	{
	public:
		MessageQueueDispatcher(size_t queueLength = 10);

		/**
		 * Process messages.
		 *
		 * @param maxDispatchTime The maximum time to allow for dispatching message, before processing new messages.
		 * @param maxProcessTime The maximum time before stopping processing.
		 * @return True if all messages were processed. False if there are remaining.
		 */
		bool process(uint64_t maxDispatchTime, uint64_t maxProcessTime = InfiniteProcessTime);

		const MessageQueuePtr& getQueue() const;
		const MessageDispatcherPtr& getDispatcher() const;

	public:
		static constexpr uint64_t InfiniteProcessTime = ~0U;

	private:
		MessageQueuePtr queue;
		MessageDispatcherPtr dispatcher;
	};

	inline MessageQueueDispatcher::MessageQueueDispatcher(size_t queueLength)
	{
		dispatcher = MessageDispatcher::create();
		queue = MessageQueue::create(queueLength);
	}

	bool MessageQueueDispatcher::process(uint64_t maxDispatchTime, uint64_t maxProcessTime)
	{
		using namespace std::chrono;

		time_point<steady_clock> currentTime = steady_clock::now();
		const time_point<steady_clock> maxTime = currentTime + milliseconds(maxProcessTime);

		for(;;)
		{
			dispatcher->processIncomingMessages(maxDispatchTime);
			queue->processPendingMessages();

			if (!dispatcher->hasMessagesToProcess() && !queue->getNumPendingResponses())
			{
				// everything was processed
				return true;
			}

			if (maxProcessTime == 0)
			{
				// no process time
				break;
			}
			else if (maxProcessTime != InfiniteProcessTime)
			{
				currentTime = steady_clock::now();
				if (currentTime >= maxTime)
				{
					// reached max process time
					break;
				}
			}
		}

		return false;
	}

	const MOHPC::MessageQueuePtr& MessageQueueDispatcher::getQueue() const
	{
		return queue;
	}

	const MOHPC::MessageDispatcherPtr& MessageQueueDispatcher::getDispatcher() const
	{
		return dispatcher;
	}
}
