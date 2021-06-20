#pragma once

#include "../NetGlobal.h"
#include "../ProtocolSingleton.h"

namespace MOHPC
{
class MSG;
class StringMessage;
class ReadOnlyInfo;

namespace Network
{
namespace Parsing
{
	/**
	 * Abstract protocol string class.
	 */
	class MOHPC_NET_EXPORTS IString : public IProtocolSingleton<IString>
	{
	public:
		virtual StringMessage readString(MSG& msg) const = 0;
		virtual void writeString(MSG& msg, const char* s) const = 0;
	};
}
}
}
