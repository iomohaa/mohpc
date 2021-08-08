#include <MOHPC/Network/Client/CGame/Objective.h>
#include <MOHPC/Utility/Info.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

const objective_t& ObjectiveManager::get(uint32_t objNum) const
{
	return objectives[objNum];
}

const objective_t& ObjectiveManager::set(const ReadOnlyInfo& info, uint32_t objNum)
{
	objective_t& objective = objectives[objNum];
	objective.flags = info.IntValueForKey("flags");
	// Get objective text
	objective.text = info.ValueForKey("text");

	// Get objective location
	size_t strLen;
	const char* locStr = info.ValueForKey("loc", strLen);
	sscanf(locStr, "%f %f %f", &objective.location[0], &objective.location[1], &objective.location[2]);

	return objective;
}

objective_t::objective_t()
	: flags(0)
{
}

uint32_t objective_t::getFlags() const
{
	return flags;
}

const char* objective_t::getText() const
{
	return text.c_str();
}

const_vec3p_t objective_t::getLocation() const
{
	return location;
}
