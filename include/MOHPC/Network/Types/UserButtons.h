#pragma once

#include "../NetGlobal.h"
#include "../../Utility/DynamicEnum.h"

namespace MOHPC
{
namespace Network
{
	class MOHPC_NET_EXPORTS UserButton : public DynamicEnum<UserButton, uint8_t, 0, EnumShiftCounter<uint8_t>>
	{
	};

	namespace UserButtons
	{
		MOHPC_NET_EXPORTS extern const UserButton AttackPrimary;
		MOHPC_NET_EXPORTS extern const UserButton AttackSecondary;
		MOHPC_NET_EXPORTS extern const UserButton Run;
		MOHPC_NET_EXPORTS extern const UserButton Use;
		MOHPC_NET_EXPORTS extern const UserButton LeanLeft;
		MOHPC_NET_EXPORTS extern const UserButton LeanRight;
	}
}
}