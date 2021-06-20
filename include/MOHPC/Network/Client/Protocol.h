#pragma once

#include "../../Utility/ClassList.h"
#include "../NetGlobal.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	class MOHPC_NET_EXPORTS IClientProtocol : public ClassList<IClientProtocol>
	{
	public:
		/** Return the server protocol version. */
		virtual uint32_t getServerProtocol() const = 0;

		/** Return the protocol version that the client should use. */
		virtual uint32_t getBestCompatibleProtocol() const = 0;

		/** Return the client type to use for the protocol (can be null). */
		virtual const char* getClientType() const = 0;

		/** Return the client version of the protocol. */
		virtual const char* getVersion() const = 0;
	};
}
}
