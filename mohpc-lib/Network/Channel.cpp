#include <MOHPC/Network/Channel.h>
#include <MOHPC/Network/InfoTypes.h>
#include <MOHPC/Network/Socket.h>
#include <MOHPC/Network/UDPMessageDispatcher.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/MSG/Serializable.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>
#include <MOHPC/Utility/Misc/MSG/Codec.h>

using namespace MOHPC;
using namespace Network;

INetchan::INetchan(const ICommunicatorPtr& inSocket)
	: socket(inSocket)
{
}

MOHPC_OBJECT_DEFINITION(Netchan);

Netchan::Netchan(const ICommunicatorPtr& existingSocket, uint16_t inQport)
	: INetchan(existingSocket)
	, qport(inQport)
	, incomingSequence(0)
	, outgoingSequence(1)
	, dropped(false)
	, fragmentSequence(0)
{
}

Netchan::~Netchan()
{
}

ICommunicatorPtr Network::INetchan::getSocket() const
{
	return socket;
}

ICommunicator* Network::INetchan::getRawSocket() const
{
	return socket.get();
}

uint16_t Network::INetchan::getOutgoingSequence() const
{
	return 0;
}

bool Network::Netchan::receive(IRemoteIdentifierPtr& from, IMessageStream& stream, uint32_t& outSeqNum)
{
	uint8_t data[MAX_UDP_DATA_SIZE];
	size_t len = getSocket()->receive(from, data, sizeof(data));
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

	if (fragmented)
	{
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
			// received sequence number must match with the current fragment sequence
			// otherwise it's a new unrelated packet
			fragmentSequence = sequenceNum;
			clearFragment();
		}

		const size_t currentFragmentLength = fragmentStream.GetLength();

		if (fragmentStart != currentFragmentLength)
		{
			// this means that a packet was missed (lost, or wrong order)
			// the start fragment must match with the current length
			return false;
		}

		if (fragmentLength < 0 || msgRead.GetPosition() + fragmentLength > stream.GetLength()) {
			throw BadFragmentLengthException(fragmentLength);
		}

		stream.Seek(msgRead.GetPosition());
		//stream.Read(fragmentBuffer + this->fragmentLength, fragmentLength);
		fragmentStream.Seek(fragmentLength);
		fragmentStream.Seek(currentFragmentLength);
		streamHelpers::Copy<FRAGMENT_SIZE>(fragmentStream, stream, fragmentLength + stream.GetPosition());

		if (fragmentLength == FRAGMENT_SIZE) {
			return false;
		}

		// seek to preallocate
		stream.Seek(fragmentStream.GetLength());
		// reset position to create a message
		stream.Seek(0);

		// write all necessary stuff
		writePacketServerHeader(stream, sequenceNum);

		// save the initial position
		const size_t savedPos = stream.GetPosition();
		// set the fragment to the beginning and copy it to the message stream
		fragmentStream.Seek(0);
		streamHelpers::Copy<FRAGMENT_SIZE>(stream, fragmentStream, fragmentStream.GetLength());
		// return to the beginning of the message
		stream.Seek(savedPos);
		// now get rid of the fragment buffer
		clearFragment();
	}
	else {
		stream.Seek(msgRead.GetPosition());
	}

	if (sequenceNum != -1) incomingSequence = sequenceNum;

	return true;
}

bool Network::Netchan::transmit(const IRemoteIdentifier& to, IMessageStream& stream)
{
	const size_t bufLen = stream.GetLength();
	if (bufLen >= FRAGMENT_SIZE)
	{
		fragment_t unsentFragmentStart = 0;
		fragmentLen_t unsentLength = (fragmentLen_t)bufLen;
		bool unsentFragments = true;

		do
		{
			// send each fragment
			transmitNextFragment(to, stream, unsentFragmentStart, unsentLength, unsentFragments);
		} while(unsentFragments);

		return true;
	}
	else
	{
		// unfragmented packet
		DynamicDataMessageStream newStream;
		newStream.reserve(bufLen + sizeof(uint32_t) + sizeof(uint16_t));

		// write the header
		writePacketHeader(newStream);

		// the header length must be 6
		const size_t headerLen = newStream.GetLength();
		assert(headerLen == 6);

		stream.Seek(0);
		// seek to the end of the packet to set the correct length
		newStream.Seek(bufLen, IMessageStream::SeekPos::Current);
		newStream.Seek(headerLen, IMessageStream::SeekPos::Begin);

		// read the data and save it
		streamHelpers::Copy<MAX_PACKETLEN>(newStream, stream, bufLen);

		// send the packet now
		getSocket()->send(to, newStream.getStorage(), newStream.GetLength());

		++outgoingSequence;
	}

	return true;
}

void Netchan::transmitNextFragment(const IRemoteIdentifier& to, IMessageStream& stream, fragment_t& unsentFragmentStart, fragmentLen_t& unsentLength, bool& unsentFragments)
{
	// a fragment contains:
	// - qport [2 bytes]
	// - start offset of fragment [4 bytes]
	// - length of fragment [2 bytes]
	// - data up to FRAGMENT_SIZE

	DynamicDataMessageStream outputPacket;

	fragmentLen_t fragmentLength = FRAGMENT_SIZE;
	if (unsentFragmentStart + fragmentLength > unsentLength) {
		fragmentLength = (fragmentLen_t)(unsentLength - unsentFragmentStart);
	}

	outputPacket.reserve(sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint16_t) + fragmentLength);

	// now write the packet header with the fragment start & length
	writePacketFragment(outputPacket, unsentFragmentStart, unsentLength);

	const size_t offset = outputPacket.GetPosition();
	// seek to the end to preallocate
	outputPacket.Seek(fragmentLength, IMessageStream::SeekPos::Current);
	// return back to post-header
	outputPacket.Seek(offset, IMessageStream::SeekPos::Begin);

	// now copy the fragment to the output packet
	streamHelpers::Copy<FRAGMENT_SIZE>(outputPacket, stream, fragmentLength);

	const uint8_t* sendBuf = outputPacket.getStorage();
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
		// for the receiver receiving seq 5, the previous seq was 3, as a consequence, 1 loss
		//
		// because of this, the next packet will be dropped
		outgoingSequence++;
		unsentFragments = false;
	}
}

void Netchan::writePacketServerHeader(IMessageStream& stream, uint32_t sequenceNum)
{
	// create a new message and write the sequence number
	MSG msg(stream, msgMode_e::Writing);
	msg.SetCodec(MessageCodecs::OOB);
	msg.WriteUInteger(sequenceNum);
	msg.Flush();
}

void Netchan::clearFragment()
{
	// free memory if it's above the max message size
	fragmentStream.clear(fragmentStream.GetLength() >= MAX_MSGLEN);
}

void MOHPC::Network::Netchan::writePacketHeader(IMessageStream& stream, bool fragmented)
{
	MSG chanMsg(stream, msgMode_e::Writing);
	chanMsg.SetCodec(MessageCodecs::OOB);

	writePacketHeader(chanMsg, fragmented);
}

void MOHPC::Network::Netchan::writePacketFragment(IMessageStream& stream, fragment_t unsentFragmentStart, fragmentLen_t fragmentLength)
{
	MSG msg(stream, msgMode_e::Writing);
	msg.SetCodec(MessageCodecs::OOB);

	writePacketHeader(msg, true);

	// write the start offset of the fragment
	msg.WriteUInteger(unsentFragmentStart);
	msg.WriteShort(fragmentLength);
}

void MOHPC::Network::Netchan::writePacketHeader(MSG& msg, bool fragmented)
{
	// write the sequence num and the fragment bit
	if(!fragmented) {
		msg.WriteUInteger(outgoingSequence);
	} else {
		msg.WriteUInteger(outgoingSequence |FRAGMENT_BIT);
	}

	// write the client port
	msg.WriteShort(qport);
}

uint16_t Network::Netchan::getOutgoingSequence() const
{
	return outgoingSequence;
}

MOHPC_OBJECT_DEFINITION(ConnectionlessChan);

Network::ConnectionlessChan::ConnectionlessChan()
	: INetchan(makeShared<UDPCommunicator>())
{

}

Network::ConnectionlessChan::ConnectionlessChan(const ICommunicatorPtr& existingSocket)
	: INetchan(existingSocket)
{}

bool Network::ConnectionlessChan::receive(IRemoteIdentifierPtr& from, IMessageStream& stream, uint32_t& sequenceNum)
{
	MSG msg(stream, msgMode_e::Reading);

	// Set in OOB mode to read one single byte each call
	msg.SetCodec(MessageCodecs::OOB);

	sequenceNum = msg.ReadInteger();

	// Should be -1 for connectionless packets
	//assert(sequenceNum == -1);

	const netsrc_e dirByte = (netsrc_e)msg.ReadByte();

	if (dirByte != netsrc_e::Client) {
		return false;
	}

	// Seek after header
	stream.Seek(5);
	return true;
}

bool Network::ConnectionlessChan::transmit(const IRemoteIdentifier& to, IMessageStream& stream)
{
	uint8_t msgBuf[MAX_UDP_DATA_SIZE];

	const size_t len = stream.GetLength();
	stream.Seek(0);
	stream.Read(msgBuf, len);

	getSocket()->send(to, msgBuf, len);
	return true;
}
