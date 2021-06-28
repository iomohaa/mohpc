#pragma once

#include "NetGlobal.h"
#include "../Common/str.h"

namespace MOHPC
{
namespace Network
{
	class MOHPC_NET_EXPORTS NetworkException
	{
	public:
		virtual ~NetworkException() = default;
		virtual str what() const { return str(); };
	};
}
}
