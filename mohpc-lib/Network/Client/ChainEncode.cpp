#include <MOHPC/Network/Client/ChainEncode.h>
#include <MOHPC/Network/Remote/Encoding.h>
#include <MOHPC/Network/Types/GameState.h>
#include <MOHPC/Network/Types/Reliable.h>
#include <MOHPC/Utility/Misc/MSG/Codec.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>

using namespace MOHPC;
using namespace MOHPC::Network;

MOHPC_OBJECT_DEFINITION(ClientEncoding);

ClientEncoding::ClientEncoding(const gameState_t& gameStateRef, const IReliableSequence* reliableSequencePtr, const ICommandSequence* commandSequencePtr, uint32_t challengeValue)
	: gameState(gameStateRef)
	, reliableSequence(reliableSequencePtr)
	, commandSequence(commandSequencePtr)
	, challenge(challengeValue)
{
}

void ClientEncoding::handleTransmit(IMessageStream& stream)
{
	if (getNext())
	{
		getNext()->handleTransmit(stream);
	}

	static constexpr size_t encodeStart = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);

	if (stream.GetLength() <= encodeStart)
	{
		// too small
		return;
	}

	stream.Seek(0, IMessageStream::SeekPos::Begin);
	MSG msg(stream, msgMode_e::Reading);

	// server id
	msg.ReadInteger();
	const uint32_t sequenceNum = msg.ReadInteger();

	stream.Seek(encodeStart, IMessageStream::SeekPos::Begin);

	XOREncoding encoder(challenge, *commandSequence);
	encoder.setSecretKey(gameState.getMapInfo().getServerId());
	encoder.setMessageAcknowledge(sequenceNum);
	encoder.setReliableAcknowledge(commandSequence->getCommandSequence());
	encoder.convert(stream, stream);

	stream.Seek(0, IMessageStream::SeekPos::Begin);
}

void ClientEncoding::handleReceive(IMessageStream& stream)
{
	if (getNext())
	{
		getNext()->handleReceive(stream);
	}

	static constexpr size_t decodeStart = sizeof(uint32_t) + sizeof(uint32_t);

	stream.Seek(0, IMessageStream::SeekPos::Begin);
	MSG msg(stream, msgMode_e::Reading);
	msg.SetCodec(MessageCodecs::OOB);

	const uint32_t sequenceNum = msg.ReadInteger();

	// decode the stream itself
	stream.Seek(decodeStart, IMessageStream::SeekPos::Begin);

	XOREncoding decoder(challenge, *reliableSequence);
	decoder.setMessageAcknowledge(sequenceNum);
	decoder.setReliableAcknowledge(reliableSequence->getReliableAcknowledge());
	decoder.convert(stream, stream);
	// seek after sequence number
	stream.Seek(sizeof(uint32_t));
}
