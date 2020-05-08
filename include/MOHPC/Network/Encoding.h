#pragma once

#include "../Global.h"
#include "Types.h"
#include <stdint.h>
#include "../Utilities/SharedPtr.h"

namespace MOHPC
{
	class IMessageStream;

	namespace Network
	{
		class MOHPC_EXPORTS IEncoding
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

		private:
			uint32_t challenge;
			const char** reliableCommands;
			const char** serverCommands;

		public:
			Encoding(uint32_t challenge, const char** reliableCommands, const char** serverCommands);

			virtual void encode(IMessageStream& in, IMessageStream& out) override;
			virtual void decode(IMessageStream& in, IMessageStream& out) override;

		private:
			uint32_t XORKeyIndex(size_t i, size_t& index, const uint8_t* string);
			void XORValues(uint32_t key, const uint8_t* string, size_t len, IMessageStream& stream);
			void XORValues(uint32_t key, const uint8_t* string, size_t len, IMessageStream& in, IMessageStream& out);
		};
	}
}
