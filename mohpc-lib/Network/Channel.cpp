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

bool Network::Netchan::receive(IMessageStream& stream)
{
	stream.Seek(0);

	MSG msgRead(stream, msgMode_e::Reading);

	msgRead.SetCodec(MessageCodecs::OOB);

	// Read the sequence num
	uint32_t sequenceNum;
	msgRead.SerializeUInteger(sequenceNum);

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

	uint32_t fragmentStart = 0;
	uint16_t fragmentLength = 0;

	if (fragmented) {
		msgRead.SerializeUInteger(fragmentStart);
		msgRead.SerializeUShort(fragmentLength);
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

		if (this->fragmentLength >= stream.GetLength()) {
			return false;
		}

		// Copy the full message
		stream.Seek(0);

		// Write the sequence num
		MSG msgW(stream, msgMode_e::Writing);
		msgW.SetCodec(MessageCodecs::OOB);
		msgW.SerializeUInteger(sequenceNum);
		msgW.Flush();

		stream.Write(fragmentBuffer, this->fragmentLength);
	}

	incomingSequence = sequenceNum;

	return true;
}

bool Network::Netchan::transmit(const netadr_t& to, IMessageStream& stream)
{
	unsentFragmentStart = 0;

	size_t bufLen = stream.GetPosition();
	if (bufLen > FRAGMENT_SIZE)
	{
		unsentFragments = true;
		unsentLength = bufLen;
		unsentBuffer = new uint8_t[MAX_MSGLEN];

		stream.Seek(0);
		stream.Read(unsentBuffer, bufLen);

		while (unsentFragments) {
			transmitNextFragment(to);
		}

		return true;
	}

	uint8_t* newBuf = new uint8_t[bufLen + 6];
	FixedDataMessageStream newStream(newBuf, bufLen + 6);

	{
		MSG chanMsg(newStream, msgMode_e::Writing);

		chanMsg.SetCodec(MessageCodecs::OOB);
		chanMsg.SerializeUInteger(outgoingSequence);
		++outgoingSequence;

		chanMsg.SerializeUShort(qport);
	}

	stream.Seek(0);

	for (size_t i = 6; i < bufLen + 6; ++i)
	{
		uint8_t byteValue;
		stream.Read(&byteValue, sizeof(byteValue));
		newStream.Write(&byteValue, sizeof(byteValue));
	}

	getSocket()->send(to, newBuf, bufLen + 6);
	delete[] newBuf;

	return true;
}

uint16_t Network::Netchan::getOutgoingSequence() const
{
	return outgoingSequence;
}

void Netchan::transmitNextFragment(const netadr_t& to)
{
	uint8_t sendBuf[MAX_PACKETLEN];
	FixedDataMessageStream stream(sendBuf, MAX_PACKETLEN);

	MSG msg(stream, msgMode_e::Writing);
	msg.SetCodec(MessageCodecs::OOB);

	msg.WriteShort(qport);

	size_t fragmentLength = FRAGMENT_SIZE;
	if (this->unsentFragmentStart + fragmentLength > this->unsentLength) {
		fragmentLength = this->unsentLength - this->unsentFragmentStart;
	}

	msg.WriteUInteger((uint32_t)unsentFragmentStart);
	msg.WriteShort((uint16_t)fragmentLength);
	msg.WriteData(unsentBuffer + unsentFragmentStart, fragmentLength);

	getSocket()->send(to, sendBuf, msg.GetPosition());

	unsentFragmentStart += fragmentLength;

	// this exit condition is a little tricky, because a packet
	// that is exactly the fragment length still needs to send
	// a second packet of zero length so that the other side
	// can tell there aren't more to follow
	if (this->unsentFragmentStart == this->unsentLength && fragmentLength != FRAGMENT_SIZE) {
		this->outgoingSequence++;
		this->unsentFragments = false;
	}
}

Network::ConnectionlessChan::ConnectionlessChan()
	: INetchan(ISocketFactory::get()->createUdp(addressType_e::IPv4))
{

}

bool Network::ConnectionlessChan::receive(IMessageStream& stream)
{
	MSG msg(stream, msgMode_e::Reading);

	// Set in OOB mode to read one single byte each call
	msg.SetCodec(MessageCodecs::OOB);

	int32_t marker = msg.ReadInteger();

	// Should be -1 for connectionless packets
	assert(marker == -1);

	uint8_t dirByte = msg.ReadByte();
	return true;
}

bool Network::ConnectionlessChan::transmit(const netadr_t& to, IMessageStream& stream)
{
	uint8_t msgBuf[32768];

	const size_t len = stream.GetPosition();
	stream.Seek(0);
	stream.Read(msgBuf, len);

	getSocket()->send(to, msgBuf, len - 1);
	return true;
}
