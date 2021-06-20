#pragma once

#include "SharedPtr.h"
#include "Misc/MSG/Stream.h"
#include "RemoteIdentifier.h"
#include "Communicator.h"

#include <queue>
#include <cstdint>
#include <chrono>

namespace MOHPC
{
	class IMessageStream;
	class MessageDispatcher;
	class IRemoteIdentifier;

	class InputRequest
	{
	public:
		InputRequest(IMessageStream& stream, const IRemoteIdentifierPtr& identifier);

	public:
		IMessageStream& stream;
		const IRemoteIdentifierPtr& identifier;
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
		virtual SharedPtr<IRequestBase> process(InputRequest& data) = 0;

		/** Should return true if the reply is the correct one. */
		virtual bool isThisRequest(InputRequest& data) const { return true; }

		/** Delay added to the base timeout time. */
		virtual uint64_t timeOutDelay() { return 0; }

		/** Delay the time at which to start the request. */
		virtual uint64_t deferredTime() { return 0; }

		/** Override the timeout time. */
		virtual uint64_t overrideTimeoutTime(bool& overriden) { return 0; }

		/** Called when the timeout time has reached, giving a chance the request to retry, skip to another one or abort. */
		virtual SharedPtr<IRequestBase> timedOut() { return nullptr; };
	};
	using IRequestPtr = SharedPtr<IRequestBase>;

	/**
	 * Handle incoming requests from a specific source.
	 */
	class IncomingMessageHandler
	{
	private:
		template<typename TimeType>
		using time_point = std::chrono::time_point<TimeType>;
		using steady_clock = std::chrono::steady_clock;

	public:
		/**
		 * Construct the handler.
		 *
		 * @param dispatcher The message dispatcher.
		 * @param communicator Communicator.
		 * @param remoteId The remote identifier.
		 */
		MOHPC_UTILITY_EXPORTS IncomingMessageHandler(MessageDispatcher* dispatcher, const ICommunicatorPtr& communicator, const IRemoteIdentifierPtr& remoteId);
		MOHPC_UTILITY_EXPORTS ~IncomingMessageHandler();

		/**
		 * Send a request or enqueue it if it can't be sent right now.
		 *
		 * @param newRequest The request to send.
		 * @param param The way the data will be sent.
		 * @param timeout Timeout in ms.
		 */
		MOHPC_UTILITY_EXPORTS void sendRequest(IRequestPtr&& newRequest,  uint64_t timeout = 0);

		/** Return true if there is a request and the handler is expecting a response. */
		MOHPC_UTILITY_EXPORTS bool isRequesting() const;

		/**
		 * Handle requests.
		 *
		 * @param param The way data will be sent and received.
		 */
		MOHPC_UTILITY_EXPORTS void handleIncoming(IMessageStream& stream, const IRemoteIdentifierPtr& incomingId);

		/** Return the time at which the request should start. */
		MOHPC_UTILITY_EXPORTS time_point<steady_clock> getDeferredTime() const;

		/** Return the remote identifier that identities the request. */
		MOHPC_UTILITY_EXPORTS const IRemoteIdentifierPtr& getRemoteId() const;

		/** Return the communicator used by the request. */
		MOHPC_UTILITY_EXPORTS const ICommunicatorPtr& getComm() const;

	private:
		bool processDeferred();
		void handleNewRequest(IRequestPtr&& newRequest, bool shouldResend);
		void startRequest(const IRequestPtr& newRequest, uint64_t timeout);
		void setDeferredStart(const IRequestPtr& newRequest);
		void setRequestTimeout(const IRequestPtr& newRequest, uint64_t timeout);
		void sendData(const IRequestPtr& newRequest);
		void dequeRequest();
		const IRequestPtr& currentRequest() const;
		bool processTimeOut();
		bool expect(InputRequest& input);

	private:
		friend MessageDispatcher;

		MessageDispatcher* dispatcher;
		ICommunicatorPtr comm;
		IncomingMessageHandler* next;
		IncomingMessageHandler* prev;

		IRemoteIdentifierPtr remoteId;

	private:
		struct PendingRequest
		{
			IRequestPtr request;
			uint64_t timeout;

			PendingRequest(IRequestPtr&& inRequest, uint64_t timeout);
		};

	private:
		IRequestPtr request;
		time_point<steady_clock> startTime;
		time_point<steady_clock> timeoutTime;
		time_point<steady_clock> deferTime;
		std::queue<PendingRequest> pendingRequests;
	};
	}

//#include "RequestHandler_imp.h"
