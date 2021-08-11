#pragma once

#include "ConfigStringMonitor.h"
#include "../Vote.h"
#include "../../../../Utility/SharedPtr.h"

namespace MOHPC
{
namespace Network
{
namespace CGame
{
	class VoteMonitor : public ConfigstringMonitorTemplate<VoteMonitor>
	{
		MOHPC_NET_OBJECT_DECLARATION(VoteMonitor);

		friend ConfigstringMonitorTemplate;

	public:
		MOHPC_NET_EXPORTS VoteMonitor(const SnapshotProcessorPtr& snapshotProcessor, const VoteManagerPtr& voteManPtr);
		MOHPC_NET_EXPORTS ~VoteMonitor();

	private:
		void setVoteTime(const char* cs);
		void setVoteString(const char* cs);
		void setVotesYes(const char* cs);
		void setVotesNo(const char* cs);
		void setVotesUndecided(const char* cs);

	private:
		VoteManagerPtr voteMan;
	};
	using VoteMonitorPtr = SharedPtr<VoteMonitor>;
}
}
}
