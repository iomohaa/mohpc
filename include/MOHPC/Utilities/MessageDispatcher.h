#pragma once

#include "../Global.h"
#include "SharedPtr.h"
#include "../Common/str.h"
#include "../Common/Container.h"
#include "../Object.h"
#include "RemoteIdentifier.h"
#include "Communicator.h"

namespace MOHPC
{
	class IncomingMessageHandler;

	/**
	 * Dispatch received messages through the correct handler.
	 * Messages are dispatched using a remote identifier like an IP address.
	 */
	class MessageDispatcher
	{
		MOHPC_OBJECT_DECLARATION(MessageDispatcher);

	public:
		MOHPC_EXPORTS MessageDispatcher();
		virtual ~MessageDispatcher();

		/**
		 * Add a communicator to be used by message dispatcher.
		 *
		 * @param comm Pointer to a communicator interface.
		 */
		MOHPC_EXPORTS void addComm(const ICommunicatorPtr& comm);

		/**
		 * Batch process all incoming messages.
		 * Will return once there are no more requests to process.
		 *
		 * @param maxProcessTime The maximum time, in milliseconds before it should stop waiting.
		 * @return Whether or not it processed any messages.
		 */
		MOHPC_EXPORTS bool processIncomingMessages(uint64_t maxProcessTime = -1);

		/** Remove all communications from this dispatcher. */
		MOHPC_EXPORTS void clearComms();

	private:
		friend class IncomingMessageHandler;

		bool needsProcessing() const;
		bool processComm(ICommunicator& comm, uint64_t maxWaitTime);
		void addHandler(IncomingMessageHandler* handler);
		void removeHandler(IncomingMessageHandler* handler);
		uint64_t getWaitTime() const;

	private:
		Container<ICommunicatorPtr> commList;
		IncomingMessageHandler* root;
		IncomingMessageHandler* last;
	};
	using MessageDispatcherPtr = SharedPtr<MessageDispatcher>;
}
