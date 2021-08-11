#pragma once

#include "ConfigStringMonitor.h"
#include "../Objective.h"
#include "../../../../Utility/SharedPtr.h"

namespace MOHPC
{
namespace Network
{
namespace CGame
{
	class ObjectiveMonitor : public ConfigstringMonitor
	{
		MOHPC_NET_OBJECT_DECLARATION(ObjectiveMonitor);

	public:
		MOHPC_NET_EXPORTS ObjectiveMonitor(const SnapshotProcessorPtr& snapshotProcessor, const ObjectiveManagerPtr& objManPtr);
		MOHPC_NET_EXPORTS ~ObjectiveMonitor();

		void configStringModified(csNum_t num, const char* cs) override;

	private:
		ObjectiveManagerPtr objMan;
	};
	using ObjectiveMonitorPtr = SharedPtr<ObjectiveMonitor>;
}
}
}
