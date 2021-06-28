#pragma once

#include "../NetGlobal.h"
#include "../NetObject.h"
#include "../../Utility/SharedPtr.h"
#include "../../Utility/Communicator.h"
#include "../../Utility/Misc/MSG/Stream.h"
#include "../Exception.h"
#include "../Remote/Socket.h"

#include <cstdint>

namespace MOHPC
{
	class IMessageStream;
	class MSG;

	namespace Network
	{
		class IUdpSocket;

		// an offset can be higher than a short so use a big number
		using fragment_t = uint32_t;
		// a fragment is no more than FRAGMENT_SIZE
		using fragmentLen_t = uint16_t;

		class BadFragmentLengthException : public NetworkException
		{
		private:
			size_t length;

		public:
			BadFragmentLengthException(size_t inLength)
				: length(inLength)
			{}

			size_t getLength() const { return length; }
		};

		class INetchan
		{
		private:
			ICommunicatorPtr socket;

		public:
			INetchan(const ICommunicatorPtr& inSocket);
			virtual ~INetchan() = default;

			/** Read data from the socket to the stream. */
			virtual bool receive(IRemoteIdentifierPtr& from, IMessageStream& stream, uint32_t& sequenceNum) = 0;

			/** Transmit data from stream to the socket. */
			virtual bool transmit(const IRemoteIdentifier& to, IMessageStream& stream) = 0;

			/** Return the socket of this channel. */
			ICommunicatorPtr getSocket() const;

			/** Return the socket of this channel. */
			ICommunicator* getRawSocket() const;

			/** Return the outgoing sequence number */
			virtual uint16_t getOutgoingSequence() const;
		};

		using INetchanPtr = SharedPtr<INetchan>;

		class Netchan : public INetchan
		{
			MOHPC_NET_OBJECT_DECLARATION(Netchan);

		protected:
			uint16_t qport;
			uint16_t incomingSequence;
			uint32_t outgoingSequence;
			uint16_t dropped;
			uint16_t fragmentSequence;
			DynamicDataMessageStream fragmentStream;

		public:
			MOHPC_NET_EXPORTS Netchan(const ICommunicatorPtr& existingSocket, uint16_t inQport);
			~Netchan();

			virtual bool receive(IRemoteIdentifierPtr& from, IMessageStream& stream, uint32_t& sequenceNum) override;
			virtual bool transmit(const IRemoteIdentifier& to, IMessageStream& stream) override;
			virtual uint16_t getOutgoingSequence() const override;

		private:
			void transmitNextFragment(const IRemoteIdentifier& to, IMessageStream& stream, fragment_t& unsentFragmentStart, fragmentLen_t& unsentLength, bool& unsentFragments);
			void clearFragment();
			void writePacketServerHeader(IMessageStream& stream, uint32_t sequenceNum);
			void writePacketHeader(IMessageStream& stream, bool fragmented = false);
			void writePacketHeader(MSG& msg, bool fragmented = false);
			void writePacketFragment(IMessageStream& stream, fragment_t unsentFragmentStart, fragmentLen_t fragmentLength);
		};

		class ConnectionlessChan : public INetchan
		{
			MOHPC_NET_OBJECT_DECLARATION(ConnectionlessChan);

		public:
			MOHPC_NET_EXPORTS ConnectionlessChan();
			MOHPC_NET_EXPORTS ConnectionlessChan(const ICommunicatorPtr& existingSocket);

			virtual bool receive(IRemoteIdentifierPtr& from, IMessageStream& stream, uint32_t& sequenceNum) override;
			virtual bool transmit(const IRemoteIdentifier& to, IMessageStream& stream) override;
		};
	}
}
