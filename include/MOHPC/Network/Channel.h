#pragma once

#include "../Global.h"
#include "../Object.h"
#include "../Utilities/SharedPtr.h"
#include "../Misc/MSG/Stream.h"
#include "Types.h"
#include <stdint.h>

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
			IUdpSocketPtr socket;

		public:
			INetchan(const IUdpSocketPtr& inSocket);
			virtual ~INetchan() = default;

			/** Read data from the socket to the stream. */
			virtual bool receive(netadr_t& from, IMessageStream& stream, uint32_t& sequenceNum) = 0;

			/** Transmit data from stream to the socket. */
			virtual bool transmit(const netadr_t& to, IMessageStream& stream) = 0;

			/** Return the socket of this channel. */
			IUdpSocketPtr getSocket() const;

			/** Return the socket of this channel. */
			IUdpSocket* getRawSocket() const;

			/** Return the outgoing sequence number */
			virtual uint16_t getOutgoingSequence() const;
		};

		using INetchanPtr = SharedPtr<INetchan>;

		class Netchan : public INetchan
		{
			MOHPC_OBJECT_DECLARATION(Netchan);

		protected:
			uint16_t qport;
			uint16_t incomingSequence;
			uint32_t outgoingSequence;
			uint16_t dropped;
			uint16_t fragmentSequence;
			DynamicDataMessageStream fragmentStream;
			netadr_t from;

		public:
			MOHPC_EXPORTS Netchan(const IUdpSocketPtr& existingSocket, const netadr_t& from, uint16_t inQport);
			~Netchan();

			virtual bool receive(netadr_t& from, IMessageStream& stream, uint32_t& sequenceNum) override;
			virtual bool transmit(const netadr_t& to, IMessageStream& stream) override;
			virtual uint16_t getOutgoingSequence() const override;

		private:
			void transmitNextFragment(const netadr_t& to, IMessageStream& stream, fragment_t& unsentFragmentStart, fragmentLen_t& unsentLength, bool& unsentFragments);
			void clearFragment();
			void writePacketServerHeader(IMessageStream& stream, uint32_t sequenceNum);
			void writePacketHeader(IMessageStream& stream, bool fragmented = false);
			void writePacketHeader(MSG& msg, bool fragmented = false);
			void writePacketFragment(IMessageStream& stream, fragment_t unsentFragmentStart, fragmentLen_t fragmentLength);
		};

		class ConnectionlessChan : public INetchan
		{
			MOHPC_OBJECT_DECLARATION(ConnectionlessChan);

		public:
			MOHPC_EXPORTS ConnectionlessChan();
			MOHPC_EXPORTS ConnectionlessChan(const IUdpSocketPtr& existingSocket);

			virtual bool receive(netadr_t& from, IMessageStream& stream, uint32_t& sequenceNum) override;
			virtual bool transmit(const netadr_t& to, IMessageStream& stream) override;
		};
	}
}
