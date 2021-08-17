#pragma once

#include "../NetGlobal.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	using weaponCommand_t = uint32_t;

	namespace WeaponCommands
	{
		MOHPC_NET_EXPORTS extern const weaponCommand_t UsePistol;
		MOHPC_NET_EXPORTS extern const weaponCommand_t UseRifle;
		MOHPC_NET_EXPORTS extern const weaponCommand_t UseSmg;
		MOHPC_NET_EXPORTS extern const weaponCommand_t UseMg;
		MOHPC_NET_EXPORTS extern const weaponCommand_t UseGrenade;
		MOHPC_NET_EXPORTS extern const weaponCommand_t UseHeavy;
		MOHPC_NET_EXPORTS extern const weaponCommand_t UseItem1;
		MOHPC_NET_EXPORTS extern const weaponCommand_t UseItem2;
		MOHPC_NET_EXPORTS extern const weaponCommand_t UseItem3;
		MOHPC_NET_EXPORTS extern const weaponCommand_t UseItem4;
		MOHPC_NET_EXPORTS extern const weaponCommand_t PreviousWeapon;
		MOHPC_NET_EXPORTS extern const weaponCommand_t NextWeapon;
		MOHPC_NET_EXPORTS extern const weaponCommand_t UseLast;
		MOHPC_NET_EXPORTS extern const weaponCommand_t Holster;
		MOHPC_NET_EXPORTS extern const weaponCommand_t Drop;
		MOHPC_NET_EXPORTS extern const uint32_t MAX;
	}
}
}