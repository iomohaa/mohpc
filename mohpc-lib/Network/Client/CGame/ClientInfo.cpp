#include <MOHPC/Network/Client/CGame/ClientInfo.h>
#include <MOHPC/Network/Client/GameState.h>

#include <MOHPC/Common/Log.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

static constexpr char MOHPC_LOG_NAMESPACE[] = "cgame_clientinfo";

static constexpr char DEFAULT_CLIENT_NAME[] = "UnnamedSoldier";

MOHPC_OBJECT_DEFINITION(ClientInfoList);

ClientInfoList::ClientInfoList()
{
	clientInfo = new clientInfo_t[CS::MAX_CLIENTS];
}

ClientInfoList::~ClientInfoList()
{
	delete[] clientInfo;
}

const clientInfo_t& ClientInfoList::get(uint32_t clientNum) const
{
	return clientInfo[clientNum];
}

const clientInfo_t& ClientInfoList::set(const ReadOnlyInfo& info, uint32_t clientNum)
{
	clientInfo_t& client = clientInfo[clientNum];
	client.name = info.ValueForKey("name");
	if (!client.name.length())
	{
		// Specify a valid name if empty (the default name)
		client.name = DEFAULT_CLIENT_NAME;
	}

	// Get the current team
	client.team = teamType_e(info.IntValueForKey("team"));

	// Add other unknown properties
	for (InfoIterator it = info.createConstIterator(); it; ++it)
	{
		const char* key = it.key();
		// Don't add first properties above
		if (strHelpers::icmp(key, "name") && strHelpers::icmp(key, "team")) {
			client.properties.SetPropertyValue(key, it.value());
		}
	}

	return client;
}

clientInfo_t::clientInfo_t()
	: team(teamType_e::None)
{
}

const char* clientInfo_t::getName() const
{
	return name.c_str();
}

teamType_e clientInfo_t::getTeam() const
{
	return team;
}

const PropertyObject& clientInfo_t::getProperties() const
{
	return properties;
}
