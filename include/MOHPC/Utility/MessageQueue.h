#pragma once

#include "UtilityGlobal.h"
#include "UtilityObject.h"
#include "SharedPtr.h"

#include <queue>
#include <set>
#include <functional>

namespace MOHPC
{
	using MessageDoneHandler = std::function<void()>;

	class IMessage
	{
	public:
		MOHPC_UTILITY_EXPORTS IMessage();
		MOHPC_UTILITY_EXPORTS virtual ~IMessage();

		/**
		 * Transmit the message.
		 * If necessary, the done() method can be safely called in the implementation.
		 */
		MOHPC_UTILITY_EXPORTS virtual void transmit() = 0;

		/**
		 * Set the handler to be called when done messaging.
		 *
		 * @param doneFuncValue The handler to use.
		 */
		MOHPC_UTILITY_EXPORTS void setDoneHandler(MessageDoneHandler&& doneFuncValue);

	protected:
		/** This method must be called when a response has been received. */
		MOHPC_UTILITY_EXPORTS void done();

	private:
		MessageDoneHandler doneFunc;
	};
	using IMessagePtr = SharedPtr<IMessage>;

	template<typename Func>
	class MessageFunction : public IMessage
	{
	public:
		MessageFunction(Func funcValue)
			: func(funcValue)
		{}

		~MessageFunction() = default;

		void transmit() override
		{
			using namespace std::placeholders;
			func(std::bind(&MessageFunction::done, this));
		}

	private:
		Func func;
	};

	template<typename Func>
	SharedPtr<MessageFunction<Func>> createMessageFunc(Func func)
	{
		return makeShared<MessageFunction<Func>>(func);
	}

	/**
	 * The purpose of the message queue (FIFO) is to send messages until a limit is met.
	 * When the limit is met, messages are placed in the queue and will be sent when other messages have been processed.
	 * The reason to not make an ICommunicator proxy for queue is because some code expect a reply after **send**
	 * and will timeout if no reply is received.
	 */
	class MessageQueue
	{
		MOHPC_UTILITY_OBJECT_DECLARATION(MessageQueue);

	public:
		MOHPC_UTILITY_EXPORTS MessageQueue(size_t limit);

		/** Return the number of pending responses. */
		MOHPC_UTILITY_EXPORTS size_t getNumPendingResponses() const;

		/**
		 * Transmit or enqueue the message.
		 *
		 * @param message The message to transmit.
		 */
		MOHPC_UTILITY_EXPORTS void transmit(const IMessagePtr& message);

		/** Process all pending messages. */
		MOHPC_UTILITY_EXPORTS void processPendingMessages();

	private:
		void transmitImmediately(const IMessagePtr& message);
		void doneWith(const IMessagePtr& message);

	private:
		size_t numMaxMessages;
		size_t numPendingResponses;
		std::queue<IMessagePtr> messageQueue;
		std::set<IMessagePtr> doneList;
	};
	using MessageQueuePtr = SharedPtr<MessageQueue>;
}
