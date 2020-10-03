#include <MOHPC/Network/Channel.h>
#include <MOHPC/Network/InfoTypes.h>
#include <MOHPC/Network/Socket.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Serializable.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Misc/MSG/Codec.h>

using namespace MOHPC;
using namespace Network;

INetchan::INetchan(const IUdpSocketPtr& inSocket)
	: socket(inSocket)
{
}

MOHPC_OBJECT_DEFINITION(Netchan);

Netchan::Netchan(const IUdpSocketPtr& existingSocket, const netadr_t& inFrom, uint16_t inQport)
	: INetchan(existingSocket)
	, qport(inQport)
	, incomingSequence(0)
	, outgoingSequence(1)
	, from(inFrom)
	, dropped(false)
	, fragmentSequence(0)
{
	fragmentBuffer = new uint8_t[MAX_MSGLEN];
}

Netchan::~Netchan()
{
	if (fragmentBuffer) {
		delete[] fragmentBuffer;
	}
}

IUdpSocketPtr Network::INetchan::getSocket() const
{
	return socket;
}

IUdpSocket* Network::INetchan::getRawSocket() const
{
	return socket.get();
}

uint16_t Network::INetchan::getOutgoingSequence() const
{
	return 0;
}

bool Network::Netchan::receive(netadr_t& from, IMessageStream& stream, size_t& outSeqNum)
{
	uint8_t data[MAX_UDP_DATA_SIZE];
	size_t len = getSocket()->receive(data, sizeof(data), from);
	if (len == -1) {
		return false;
	}

	stream.Write(data, len);

	stream.Seek(0);

	MSG msgRead(stream, msgMode_e::Reading);

	msgRead.SetCodec(MessageCodecs::OOB);

	// Read the sequence num
	uint32_t sequenceNum = msgRead.ReadUInteger();
	if (sequenceNum == -1)
	{
		outSeqNum = -1;
		stream.Seek(sizeof(sequenceNum));
		return true;
	}

	bool fragmented;
	// check for fragment information
	if (sequenceNum & FRAGMENT_BIT)
	{
		sequenceNum &= ~FRAGMENT_BIT;
		fragmented = true;
	}
	else {
		fragmented = false;
	}

	outSeqNum = sequenceNum;

	uint32_t fragmentStart = 0;
	uint16_t fragmentLength = 0;

	if (fragmented) {
		fragmentStart = msgRead.ReadUInteger();
		fragmentLength = msgRead.ReadUShort();
	}

	// discard out of order or duplicated packets
	if (sequenceNum < incomingSequence) {
		return false;
	}

	dropped = sequenceNum - (incomingSequence + 1);
	if (fragmented)
	{
		if (sequenceNum != fragmentSequence)
		{
			fragmentSequence = sequenceNum;
			this->fragmentLength = 0;
		}

		if (fragmentStart != this->fragmentLength) {
			return false;
		}

		if (fragmentLength < 0 || msgRead.GetPosition() + fragmentLength > stream.GetLength()) {
			throw BadFragmentLengthException(fragmentLength);
		}

		stream.Seek(msgRead.GetPosition());
		stream.Read(fragmentBuffer + this->fragmentLength, fragmentLength);
		this->fragmentLength += fragmentLength;

		if (fragmentLength == FRAGMENT_SIZE) {
			return false;
		}

		//if (this->fragmentLength >= stream.GetLength()) {
		//	return false;
		//}

		// Copy the full message
		stream.Seek(0);

		// Write the sequence num
		MSG msgW(stream, msgMode_e::Writing);
		msgW.SetCodec(MessageCodecs::OOB);
		msgW.SerializeUInteger(sequenceNum);
		msgW.Flush();

		const size_t savedPos = stream.GetPosition();
		stream.Write(fragmentBuffer, this->fragmentLength);
		stream.Seek(savedPos);
	}
	else {
		stream.Seek(msgRead.GetPosition());
	}

	if (sequenceNum != -1) incomingSequence = sequenceNum;

	return true;
}

bool Network::Netchan::transmit(const netadr_t& to, IMessageStream& stream)
{
	size_t bufLen = stream.GetLength();
	if (bufLen >= FRAGMENT_SIZE)
	{
		size_t unsentFragmentStart = 0;
		size_t unsentLength = bufLen;
		bool unsentFragments = true;

		while (unsentFragments)
		{
			// send fragment by fragment
			transmitNextFragment(to, stream, unsentFragmentStart, unsentLength, unsentFragments);
		}

		return true;
	}
	else
	{
		// unfragmented packet

		uint8_t newBuf[MAX_PACKETLEN];
		FixedDataMessageStream newStream(newBuf, sizeof(newBuf), 0);

		// write the header
		writePacketHeader(newStream);

		// get the length of the header (normally 6)
		const size_t headerLen = newStream.GetLength();
		assert(headerLen == 6);

		// seek to the end of the packet to set the correct length
		newStream.Seek(bufLen, IMessageStream::SeekPos::Current);
		stream.Seek(0);
		// read the data and save it
		stream.Read(newBuf + headerLen, bufLen);

		// send the packet now
		getSocket()->send(to, newBuf, newStream.GetLength());

		++outgoingSequence;
	}

	return true;
}

void Netchan::transmitNextFragment(const netadr_t& to, IMessageStream& stream, size_t& unsentFragmentStart, size_t& unsentLength, bool& unsentFragments)
{
	// a fragment contains:
	// - qport [2 bytes]
	// - start offset of fragment [4 bytes]
	// - length of fragment [2 bytes]
	// - data up to FRAGMENT_SIZE

	uint8_t sendBuf[MAX_PACKETLEN];
	FixedDataMessageStream outputPacket(sendBuf, sizeof(sendBuf), 0);

	MSG msg(outputPacket, msgMode_e::Writing);
	msg.SetCodec(MessageCodecs::OOB);

	uint16_t fragmentLength = FRAGMENT_SIZE;
	if (unsentFragmentStart + fragmentLength > unsentLength) {
		fragmentLength = (uint16_t)(unsentLength - unsentFragmentStart);
	}

	// write the sequence num and the fragment bit
	msg.WriteUInteger(outgoingSequence | FRAGMENT_BIT);

	// write the client qport
	msg.WriteShort(qport);

	// write the start offset of the fragment
	msg.WriteUInteger((uint32_t)unsentFragmentStart);
	msg.WriteShort((uint16_t)fragmentLength);
	msg.Flush();

	const size_t offset = outputPacket.GetPosition();
	outputPacket.Seek(fragmentLength, IMessageStream::SeekPos::Current);
	stream.Read(sendBuf + offset, fragmentLength);
	//msg.WriteData(unsentBuffer + unsentFragmentStart, fragmentLength);

	getSocket()->send(to, sendBuf, outputPacket.GetLength());

	unsentFragmentStart += fragmentLength;

	// this exit condition is a little tricky, because a packet
	// that is exactly the fragment length still needs to send
	// a second packet of zero length so that the other side
	// can tell there aren't more to follow
	if (unsentFragmentStart == unsentLength && fragmentLength != FRAGMENT_SIZE)
	{
		// there is a bug in the Quake 3 code where the receiver won't increment the incoming sequence
		// in source channel when receiving fragments.
		// i.e send seq=3, receiving fragment: seq=4, send seq=5
		// for the receiver the previous seq was 3, so 1 loss assumed which will be dropped
		//
		// because of this, the next packet will be dropped
		outgoingSequence++;
		unsentFragments = false;
	}
}

void MOHPC::Network::Netchan::writePacketHeader(IMessageStream& stream)
{
	MSG chanMsg(stream, msgMode_e::Writing);

	chanMsg.SetCodec(MessageCodecs::OOB);
	chanMsg.WriteUInteger(outgoingSequence);

	// write the client port
	chanMsg.WriteShort(qport);
}

uint16_t Network::Netchan::getOutgoingSequence() const
{
	return outgoingSequence;
}

MOHPC_OBJECT_DEFINITION(ConnectionlessChan);

Network::ConnectionlessChan::ConnectionlessChan()
	: INetchan(ISocketFactory::get()->createUdp(addressType_e::IPv4))
{

}

Network::ConnectionlessChan::ConnectionlessChan(const IUdpSocketPtr& existingSocket)
	: INetchan(existingSocket)
{

}

bool Network::ConnectionlessChan::receive(netadr_t& from, IMessageStream& stream, size_t& sequenceNum)
{
	MSG msg(stream, msgMode_e::Reading);

	// Set in OOB mode to read one single byte each call
	msg.SetCodec(MessageCodecs::OOB);

	const int32_t marker = msg.ReadInteger();

	// Should be -1 for connectionless packets
	//assert(marker == -1);

	sequenceNum = marker;

	const uint8_t dirByte = msg.ReadByte();

	// Seek after header
	stream.Seek(5);
	return true;
}

bool Network::ConnectionlessChan::transmit(const netadr_t& to, IMessageStream& stream)
{
	uint8_t msgBuf[MAX_UDP_DATA_SIZE];

	const size_t len = stream.GetLength();
	stream.Seek(0);
	stream.Read(msgBuf, len);

	getSocket()->send(to, msgBuf, len);
	return true;
}
