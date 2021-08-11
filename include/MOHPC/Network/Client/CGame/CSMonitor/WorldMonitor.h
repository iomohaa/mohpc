#pragma once

#include "ConfigStringMonitor.h"
#include "../../../Parsing/Environment.h"
#include "../../../Types/Environment.h"
#include "../../../../Utility/SharedPtr.h"

namespace MOHPC
{
namespace Network
{
class protocolType_c;

namespace CGame
{
	class WorldMonitor : public ConfigstringMonitorTemplate<WorldMonitor>
	{
		MOHPC_NET_OBJECT_DECLARATION(WorldMonitor);

		friend ConfigstringMonitorTemplate;

	public:
		MOHPC_NET_EXPORTS WorldMonitor(const SnapshotProcessorPtr& snapshotProcessor, const EnvironmentPtr& envPtr, const protocolType_c& protocolType);
		MOHPC_NET_EXPORTS ~WorldMonitor();

	private:
		void setFogInfo(const char* cs);
		void setSkyInfo(const char* cs);

	private:
		const Parsing::IEnvironment* environmentParse;
		EnvironmentPtr env;
	};
	using WorldMonitorPtr = SharedPtr<WorldMonitor>;
}
}
}
