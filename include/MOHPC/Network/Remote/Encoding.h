#pragma once

#include "../NetGlobal.h"
#include "../NetObject.h"
#include "../../Utility/SharedPtr.h"

#include <cstdint>

namespace MOHPC
{
	class IMessageStream;

	namespace Network
	{
		class IAbstractSequence;
		class IReliableSequence;
		class ICommandSequence;

		/**
		 * Encoder/Decoder class.
		 */
		class MOHPC_NET_EXPORTS IEndec
		{
		public:
			virtual ~IEndec() = default;

			/** Process the data inside the stream. */
			virtual void convert(IMessageStream& in, IMessageStream& out) = 0;
		};

		class XOREncoding : public IEndec
		{
			MOHPC_NET_OBJECT_DECLARATION(XOREncoding);

		public:
			MOHPC_NET_EXPORTS XOREncoding(uint32_t challengeValue, const IAbstractSequence& remoteCommandsValue);

			virtual void convert(IMessageStream& in, IMessageStream& out) override;

			MOHPC_NET_EXPORTS void setMessageAcknowledge(uint32_t num);
			MOHPC_NET_EXPORTS uint32_t getMessageAcknowledge() const;
			MOHPC_NET_EXPORTS void setReliableAcknowledge(uint32_t num);
			MOHPC_NET_EXPORTS uint32_t getReliableAcknowledge() const;
			MOHPC_NET_EXPORTS void setSecretKey(uint32_t num);
			MOHPC_NET_EXPORTS uint32_t getSecretKey() const;

		private:
			uint32_t challenge;
			uint32_t secretKey;
			uint32_t messageAcknowledge;
			uint32_t reliableAcknowledge;
			const IAbstractSequence& remoteCommands;
		};
	}
}
