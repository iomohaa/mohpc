#pragma once

#include "ConfigStringMonitor.h"
#include "../Objective.h"
#include "../../../../Utility/SharedPtr.h"
#include "../../../../Utility/HandlerList.h"

namespace MOHPC
{
namespace Network
{
namespace CGame
{
	namespace Handlers
	{
		/**
		 * An objective was created, updated or removed.
		 *
		 * @param obj The objective being updated.
		 * @param objNum The objective index.
		 */
		class ObjectiveChanged : public HandlerNotifyBase<void(const objective_t& obj, uint32_t objNum)> {};

		/**
		 * The current objective was set.
		 *
		 * @param objNum The index of the objective that was set to current.
		 */
		class ObjectiveSet : public HandlerNotifyBase<void(uint32_t objNum)> {};
	}

	class ObjectiveMonitor : public ConfigstringMonitor
	{
		MOHPC_NET_OBJECT_DECLARATION(ObjectiveMonitor);

		struct HandlerList
		{
			FunctionList<Handlers::ObjectiveChanged> objectiveChangedHandler;
			FunctionList<Handlers::ObjectiveSet> objectiveSetHandler;
		};

	public:
		MOHPC_NET_EXPORTS ObjectiveMonitor(const SnapshotProcessorPtr& snapshotProcessor);
		MOHPC_NET_EXPORTS ~ObjectiveMonitor();

		HandlerList& getHandlerList();
		
		bool configStringModified(csNum_t num, const char* cs) override;

	private:
		void objectiveChange(const char* cs, uint32_t objNum);
		void currentObjectiveChange(const char* cs);

	private:
		HandlerList handlerList;
	};
	using ObjectiveMonitorPtr = SharedPtr<ObjectiveMonitor>;
}
}
}
