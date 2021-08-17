#pragma once

#include "../NetGlobal.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	using userButton_t = uint32_t;

	namespace UserButtons
	{
		MOHPC_NET_EXPORTS extern const userButton_t AttackPrimary;
		MOHPC_NET_EXPORTS extern const userButton_t AttackSecondary;
		MOHPC_NET_EXPORTS extern const userButton_t Run;
		MOHPC_NET_EXPORTS extern const userButton_t Use;
		MOHPC_NET_EXPORTS extern const userButton_t LeanLeft;
		MOHPC_NET_EXPORTS extern const userButton_t LeanRight;
	}
}
}