#pragma once

#include "../NetGlobal.h"

namespace MOHPC
{
namespace Network
{
	class UserInfo;

	namespace UserInfoHelpers
	{
		MOHPC_NET_EXPORTS void setPlayerAlliedModel(UserInfo& info, const char* newModel);
		/** Get the client deathmatch allied model. */
		MOHPC_NET_EXPORTS const char* getPlayerAlliedModel(const UserInfo& info);

		/** Set the client deathmatch german model. */
		MOHPC_NET_EXPORTS void setPlayerGermanModel(UserInfo& info, const char* newModel);
		/** Get the client deathmatch german model. */
		MOHPC_NET_EXPORTS const char* getPlayerGermanModel(const UserInfo& info);
	}
}
}