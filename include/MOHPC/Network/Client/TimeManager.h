#pragma once

#include "../NetGlobal.h"
#include "../../Utility/TickTypes.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
namespace Network
{
	class ClientTime;
	class ServerSnapshotManager;

	class ClientTimeManager
	{
	public:
		ClientTimeManager(ClientTime& clientTimeRef, ServerSnapshotManager& snapshotManagerRef);

		void setTime(tickTime_t currentTime);
		bool hasEntered() const;

	private:
		ClientTime& clientTime;
		ServerSnapshotManager& snapshotManager;
		bool entered;
	};
}
}