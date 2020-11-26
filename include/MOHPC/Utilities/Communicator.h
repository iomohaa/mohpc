#pragma once

#include "SharedPtr.h"
#include "RemoteIdentifier.h"

#include <cstddef>
#include <cstdint>

namespace MOHPC
{
	/**
	 * Abstract class for communicating with any destination.
	 */
	class ICommunicator
	{
	public:
		virtual ~ICommunicator() = default;

		/**
		 * Send a message to the remote target.
		 *
		 * @param identifier The destination to send to.
		 * @param data Pointer to a buffer of data.
		 * @param size Size of the buffer.
		 * @return The number of bytes that was sent.
		 */
		virtual size_t send(const IRemoteIdentifier& identifier, const uint8_t* data, size_t size) = 0;

		/**
		 * (Blocking) receive a message from any source.
		 *
		 * @param remoteAddress The incoming remote address.
		 * @param data Pointer to a buffer that will receive the incoming data.
		 * @param size The maximum size of the buffer used to store incoming data.
		 */
		virtual size_t receive(IRemoteIdentifierPtr& remoteAddress, uint8_t* data, size_t size) = 0;

		/** Return the incoming message size. */
		virtual size_t getIncomingSize() = 0;

		/**
		 * Wait for an incoming message.
		 *
		 * @param timeout Wait timeout.
		 */
		virtual bool waitIncoming(uint64_t timeout) = 0;
	};
	using ICommunicatorPtr = SharedPtr<ICommunicator>;
}
