#pragma once

#include "../NetObject.h"
#include "../Remote/Chain.h"

#include <cstdint>

namespace MOHPC
{
class IMessageStream;

namespace Network
{
	struct gameState_t;
	class IReliableSequence;
	class ICommandSequence;

	class ClientEncoding : public IChain
	{
		MOHPC_NET_OBJECT_DECLARATION(ClientEncoding);

	public:
		MOHPC_NET_EXPORTS ClientEncoding(
			const gameState_t& gameStateRef,
			const IReliableSequence* reliableSequencePtr,
			const ICommandSequence* commandSequencePtr,
			uint32_t challengeValue
		);

		void handleTransmit(IMessageStream& stream) override;
		void handleReceive(IMessageStream& stream) override;

	private:
		const gameState_t& gameState;
		const IReliableSequence* reliableSequence;
		const ICommandSequence* commandSequence;
		uint32_t challenge;
	};
}
}
