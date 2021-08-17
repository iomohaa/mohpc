#pragma once

#include "../NetGlobal.h"
#include "../NetObject.h"
#include "GameState.h"
#include "UserInfo.h"

namespace MOHPC
{
namespace Network
{
	/**
	 * Keeps track of client name change from remote server and synchronize it with the user info.
	 */
	class ClientReflector
	{
	public:
		ClientReflector(const ServerGameStatePtr& svGameStatePtr, const UserInfoPtr& userInfoPtr);

	private:
		void configStringModified(csNum_t csNum, const char* configString);
		void localClientChanged(const char* data);

	private:
		ServerGameStatePtr svGameState;
		UserInfoPtr userInfo;
	};
}
}