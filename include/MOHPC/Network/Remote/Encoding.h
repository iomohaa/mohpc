#pragma once

#include "../NetGlobal.h"
#include "../Types.h"
#include "../../Utility/SharedPtr.h"
#include "../NetObject.h"

#include <cstdint>

namespace MOHPC
{
	class IMessageStream;

	namespace Network
	{
		class IReliableSequence;
		class ICommandSequence;

		class MOHPC_NET_EXPORTS IEncoding
		{
		public:
			virtual ~IEncoding() = default;

			/** Encode the data inside the stream. */
			virtual void encode(IMessageStream& in, IMessageStream& out) = 0;

			/** Decode the data inside the stream. */
			virtual void decode(IMessageStream& in, IMessageStream& out) = 0;
		};

		using IEncodingPtr = SharedPtr<IEncoding>;

		class Encoding : public IEncoding
		{
			MOHPC_NET_OBJECT_DECLARATION(Encoding);

		private:
			uint32_t challenge;
			uint32_t secretKey;
			uint32_t messageAcknowledge;
			uint32_t reliableAcknowledge;
			const IReliableSequence& reliableCommands;
			const ICommandSequence& serverCommands;

		public:
			//MOHPC_NET_EXPORTS Encoding(uint32_t challenge, const char** reliableCommands, const char** serverCommands);
			MOHPC_NET_EXPORTS Encoding(uint32_t challenge, const IReliableSequence& reliableCommands, const ICommandSequence& serverCommands);

			virtual void encode(IMessageStream& in, IMessageStream& out) override;
			virtual void decode(IMessageStream& in, IMessageStream& out) override;

			MOHPC_NET_EXPORTS void setMessageAcknowledge(uint32_t num);
			MOHPC_NET_EXPORTS uint32_t getMessageAcknowledge() const;
			MOHPC_NET_EXPORTS void setReliableAcknowledge(uint32_t num);
			MOHPC_NET_EXPORTS uint32_t getReliableAcknowledge() const;
			MOHPC_NET_EXPORTS void setSecretKey(uint32_t num);
			MOHPC_NET_EXPORTS uint32_t getSecretKey() const;

		private:
			uint32_t XORKeyIndex(size_t i, size_t& index, const uint8_t* string);
			void XORValues(uint32_t key, const uint8_t* string, size_t len, IMessageStream& stream);
			void XORValues(uint32_t key, const uint8_t* string, size_t len, IMessageStream& in, IMessageStream& out);
		};

		using EncodingPtr = SharedPtr<Encoding>;
	}
}
