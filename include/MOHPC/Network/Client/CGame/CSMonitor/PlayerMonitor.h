#pragma once

#include "ConfigStringMonitor.h"
#include "../ClientInfo.h"
#include "../../../../Utility/SharedPtr.h"

namespace MOHPC
{
namespace Network
{
namespace CGame
{
	class PlayerMonitor : public ConfigstringMonitor
	{
		MOHPC_NET_OBJECT_DECLARATION(PlayerMonitor);

	public:
		MOHPC_NET_EXPORTS PlayerMonitor(const SnapshotProcessorPtr& snapshotProcessor, const ClientInfoListPtr& clientListPtr);
		MOHPC_NET_EXPORTS ~PlayerMonitor();

		void configStringModified(csNum_t num, const char* cs) override;

	private:
		ClientInfoListPtr clientList;
	};
	using PlayerMonitorPtr = SharedPtr<PlayerMonitor>;
}
}
}
