#include <MOHPC/Network/Client/CGame/CSMonitor/ObjectiveMonitor.h>
#include <MOHPC/Utility/Info.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(ObjectiveMonitor);

ObjectiveMonitor::ObjectiveMonitor(const SnapshotProcessorPtr& snapshotProcessor)
	: ConfigstringMonitor(snapshotProcessor)
{
}

ObjectiveMonitor::~ObjectiveMonitor()
{
}

ObjectiveMonitor::HandlerList& ObjectiveMonitor::getHandlerList()
{
	return handlerList;
}

bool ObjectiveMonitor::configStringModified(csNum_t num, const char* cs)
{
	if (num >= CS::OBJECTIVES && num < CS::OBJECTIVES + CS::MAX_OBJECTIVES)
	{
		objectiveChange(cs, num - CS::OBJECTIVES);

		return true;
	}
	else if (num == CS::CURRENT_OBJECTIVE)
	{
		currentObjectiveChange(cs);

		return true;
	}

	return false;
}

void ObjectiveMonitor::objectiveChange(const char* cs, uint32_t objNum)
{
	ReadOnlyInfo info(cs);

	objective_t objective;
	objective.flags = info.IntValueForKey("flags");
	// Get objective text
	objective.text = info.ValueForKey("text");

	// Get objective location
	size_t strLen;
	const char* locStr = info.ValueForKey("loc", strLen);
	sscanf(locStr, "%f %f %f", &objective.location[0], &objective.location[1], &objective.location[2]);

	getHandlerList().objectiveChangedHandler.broadcast(objective, objNum);
}

void ObjectiveMonitor::currentObjectiveChange(const char* cs)
{
	const uint32_t index = atoi(cs);

	getHandlerList().objectiveSetHandler.broadcast(index);
}

