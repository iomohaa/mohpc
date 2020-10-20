#include <MOHPC/Network/Encoding.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Misc/MSG/Stream.h>

using namespace MOHPC;
using namespace Network;

MOHPC_OBJECT_DEFINITION(Encoding);

Encoding::Encoding(uint32_t inChallenge, const char** inReliableCommands, const char** inServerCommands)
	: challenge(inChallenge)
	, reliableCommands(inReliableCommands)
	, serverCommands(inServerCommands)
{

}

void Encoding::encode(IMessageStream& in, IMessageStream& out)
{
	uint32_t sequenceNum = 0;

	const size_t savedPos = in.GetPosition();
	const size_t len = in.GetLength();

	// Ack index
	const uint8_t* string = (const uint8_t*)serverCommands[reliableAcknowledge & (MAX_RELIABLE_COMMANDS - 1)];
	// xor the client challenge with the netchan sequence number
	uint8_t key = challenge ^ secretKey ^ messageAcknowledge;
	// encode the message
	if (&in != &out)
	{
		XORValues(key, string, len, in, out);
		// Seek to the start of the message
		out.Seek(0);
	}
	else
	{
		XORValues(key, string, len, out);
		out.Seek(savedPos);
	}
}

void Encoding::decode(IMessageStream& in, IMessageStream& out)
{
	const size_t savedPos = in.GetPosition();
	const size_t len = in.GetLength();

	// Ack index
	const uint8_t* string = (const uint8_t*)reliableCommands[reliableAcknowledge & (MAX_RELIABLE_COMMANDS - 1)];
	// xor the client challenge with the netchan sequence number
	uint8_t key = challenge ^ secretKey;
	// decode the message
	if (&in != &out)
	{
		XORValues(key, string, len, in, out);
		// Seek to the start of the message
		out.Seek(0);
	}
	else
	{
		XORValues(key, string, len, out);
		out.Seek(savedPos);
	}
}

uint32_t Encoding::XORKeyIndex(size_t i, size_t& index, const uint8_t* string)
{
	if (!string[index]) {
		index = 0;
	}

	if (string[index] > 127 || string[index] == '%') {
		return ('.' << (i & 1));
	}
	else {
		return (string[index] << (i & 1));
	}
}

void Encoding::XORValues(uint32_t key, const uint8_t* string, size_t len, IMessageStream& in, IMessageStream& out)
{
	size_t index = 0;

	for (size_t i = in.GetPosition(); i < len; ++i, ++index)
	{
		key ^= XORKeyIndex(i, index, string);

		uint8_t byteValue;
		in.Read(&byteValue, sizeof(byteValue));
		// XOR the value with the key
		byteValue ^= key;
		out.Write(&byteValue, sizeof(byteValue));
	}
}

void Encoding::XORValues(uint32_t key, const uint8_t* string, size_t len, IMessageStream& stream)
{
	size_t index = 0;

	for (size_t i = stream.GetPosition(); i < len; ++i, ++index)
	{
		key ^= XORKeyIndex(i, index, string);

		uint8_t byteValue;
		stream.Read(&byteValue, sizeof(byteValue));
		// XOR the value with the key
		byteValue ^= key;
		stream.Seek(i);
		stream.Write(&byteValue, sizeof(byteValue));
	}
}

void Encoding::setMessageAcknowledge(uint32_t num)
{
	messageAcknowledge = num;
}

uint32_t Encoding::getMessageAcknowledge() const
{
	return messageAcknowledge;
}

void Encoding::setReliableAcknowledge(uint32_t num)
{
	reliableAcknowledge = num;
}

uint32_t Encoding::getReliableAcknowledge() const
{
	return reliableAcknowledge;
}

void Encoding::setSecretKey(uint32_t num)
{
	secretKey = num;
}

uint32_t Encoding::getSecretKey() const
{
	return secretKey;
}
