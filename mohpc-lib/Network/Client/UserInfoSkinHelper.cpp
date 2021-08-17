#include <MOHPC/Network/Client/UserInfoSkinHelper.h>
#include <MOHPC/Network/Client/UserInfo.h>

using namespace MOHPC;
using namespace MOHPC::Network;

void UserInfoHelpers::setPlayerAlliedModel(UserInfo& info, const char* newModel)
{
	info.setUserKeyValue("dm_playermodel", newModel);
}

const char* UserInfoHelpers::getPlayerAlliedModel(const UserInfo& info)
{
	return info.getUserKeyValue("dm_playermodel");
}

void UserInfoHelpers::setPlayerGermanModel(UserInfo& info, const char* newModel)
{
	info.setUserKeyValue("dm_playergermanmodel", newModel);
}

const char* UserInfoHelpers::getPlayerGermanModel(const UserInfo& info)
{
	return info.getUserKeyValue("dm_playergermanmodel");
}
