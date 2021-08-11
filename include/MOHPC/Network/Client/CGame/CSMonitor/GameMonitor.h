#pragma once

#include "ConfigStringMonitor.h"
#include "../ServerInfo.h"
#include "../../../../Utility/SharedPtr.h"

namespace MOHPC
{
namespace Network
{
namespace CGame
{
	class GameMonitor : public ConfigstringMonitorTemplate<GameMonitor>
	{
		MOHPC_NET_OBJECT_DECLARATION(GameMonitor);

		friend ConfigstringMonitorTemplate;

	public:
		MOHPC_NET_EXPORTS GameMonitor(const SnapshotProcessorPtr& snapshotProcessor, const cgsInfoPtr& cgsPtr);
		MOHPC_NET_EXPORTS ~GameMonitor();

	private:
		void parseServerInfo(const char* cs);
		void setWarmup(const char* cs);
		void setLevelStartTime(const char* cs);
		void setMatchEnd(const char* cs);

	private:
		cgsInfoPtr cgs;
	};
	using GameMonitorPtr = SharedPtr<GameMonitor>;
}
}
}
