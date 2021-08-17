#include <MOHPC/Network/Client/ClientReflector.h>
#include <MOHPC/Utility/Info.h>

#include <MOHPC/Common/Log.h>

using namespace MOHPC;
using namespace MOHPC::Network;

static constexpr char MOHPC_LOG_NAMESPACE[] = "net_client_reflector";

ClientReflector::ClientReflector(const ServerGameStatePtr& svGameStatePtr, const UserInfoPtr& userInfoPtr)
	: svGameState(svGameStatePtr)
	, userInfo(userInfoPtr)
{
	using namespace std::placeholders;
	svGameState->handlers().configStringHandler.add(std::bind(&ClientReflector::configStringModified, this, _1, _2));
}

void ClientReflector::configStringModified(csNum_t csNum, const char* configString)
{
	if (csNum >= CS::PLAYERS && csNum < CS::PLAYERS + CS::MAX_CLIENTS)
	{
		const uint32_t clientNum = csNum - CS::PLAYERS;
		if (clientNum == svGameState->getClientNum())
		{
			// this is the local client
			localClientChanged(configString);
		}
	}
}

void ClientReflector::localClientChanged(const char* data)
{
	ReadOnlyInfo info(data);

	size_t nameLength;
	const char* name = info.ValueForKey("name", nameLength);
	const char* currentName = userInfo->getName();
	// check if the name is different
	if (strHelpers::cmpn(name, currentName, nameLength))
	{
		MOHPC_LOG(Info, "Name changed from \"%s\" to \"%.*s\"", currentName, nameLength, name);
		// the name has changed (can be because it was sanitized)
		// as a consequence, the change must be reflected on the client
		userInfo->setName(name);

		// don't resend user info, it would be useless
	}
}

