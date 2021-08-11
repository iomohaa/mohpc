#include <MOHPC/Network/Client/CGame/CSMonitor/ObjectiveMonitor.h>
#include <MOHPC/Utility/Info.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(ObjectiveMonitor);

ObjectiveMonitor::ObjectiveMonitor(const SnapshotProcessorPtr& snapshotProcessor, const ObjectiveManagerPtr& objManPtr)
	: ConfigstringMonitor(snapshotProcessor)
	, objMan(objManPtr)
{
}

ObjectiveMonitor::~ObjectiveMonitor()
{
}

void ObjectiveMonitor::configStringModified(csNum_t num, const char* cs)
{
	if (num >= CS::OBJECTIVES && num < CS::OBJECTIVES + CS::MAX_OBJECTIVES)
	{
		ReadOnlyInfo info(cs);

		const uint32_t objNum = num - CS::OBJECTIVES;
		objMan->set(info, objNum);
	}
}

