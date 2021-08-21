#pragma once

#include "../NetGlobal.h"
#include "../ProtocolSingleton.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
namespace Network
{
	namespace Parsing
	{
		/**
		 * Abstract hash interface singleton for string hashing.
		 */
		class MOHPC_NET_EXPORTS IHash : public IProtocolSingleton<IHash>
		{
		public:
			virtual uint32_t hashKey(const char* string, size_t maxlen) const = 0;
		};
	}
}
}