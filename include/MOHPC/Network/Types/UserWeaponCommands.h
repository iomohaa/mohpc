#pragma once

#include "../NetGlobal.h"
#include "../../Utility/DynamicEnum.h"

namespace MOHPC
{
namespace Network
{
	class MOHPC_NET_EXPORTS WeaponCommand : public DynamicEnum<WeaponCommand, uint8_t, 1>
	{
	};

	namespace WeaponCommands
	{
		MOHPC_NET_EXPORTS extern const WeaponCommand UsePistol;
		MOHPC_NET_EXPORTS extern const WeaponCommand UseRifle;
		MOHPC_NET_EXPORTS extern const WeaponCommand UseSmg;
		MOHPC_NET_EXPORTS extern const WeaponCommand UseMg;
		MOHPC_NET_EXPORTS extern const WeaponCommand UseGrenade;
		MOHPC_NET_EXPORTS extern const WeaponCommand UseHeavy;
		MOHPC_NET_EXPORTS extern const WeaponCommand UseItem1;
		MOHPC_NET_EXPORTS extern const WeaponCommand UseItem2;
		MOHPC_NET_EXPORTS extern const WeaponCommand UseItem3;
		MOHPC_NET_EXPORTS extern const WeaponCommand UseItem4;
		MOHPC_NET_EXPORTS extern const WeaponCommand PreviousWeapon;
		MOHPC_NET_EXPORTS extern const WeaponCommand NextWeapon;
		MOHPC_NET_EXPORTS extern const WeaponCommand UseLast;
		MOHPC_NET_EXPORTS extern const WeaponCommand Holster;
		MOHPC_NET_EXPORTS extern const WeaponCommand Drop;
	}
}
}