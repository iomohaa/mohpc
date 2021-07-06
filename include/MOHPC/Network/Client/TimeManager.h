#pragma once

#include "../NetGlobal.h"

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

		void setTime(uint64_t currentTime);
		bool hasEntered() const;

	private:
		ClientTime& clientTime;
		ServerSnapshotManager& snapshotManager;
		bool entered;
	};
}
}