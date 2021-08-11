#include <MOHPC/Network/Client/CGame/CSMonitor/VoteMonitor.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

std::map<csNum_t, ConfigstringMonitorTemplate<VoteMonitor>::Callback> ConfigstringMonitorTemplate<VoteMonitor>::callbackMap
{
	{ CS::VOTE_TIME,		&VoteMonitor::setVoteTime },
	{ CS::VOTE_STRING,		&VoteMonitor::setVoteString },
	{ CS::VOTES_YES,		&VoteMonitor::setVotesYes },
	{ CS::VOTES_NO,			&VoteMonitor::setVotesNo },
	{ CS::VOTES_UNDECIDED,	&VoteMonitor::setVotesUndecided }
};

MOHPC_OBJECT_DEFINITION(VoteMonitor);

VoteMonitor::VoteMonitor(const SnapshotProcessorPtr& snapshotProcessor, const VoteManagerPtr& voteManPtr)
	: ConfigstringMonitorTemplate(snapshotProcessor)
	, voteMan(voteManPtr)
{
}

VoteMonitor::~VoteMonitor()
{
}

void VoteMonitor::setVoteTime(const char* cs)
{
	voteMan->setVoteTime(atoll(cs));
}

void VoteMonitor::setVoteString(const char* cs)
{
	voteMan->setVoteString(cs);
}

void VoteMonitor::setVotesYes(const char* cs)
{
	voteMan->setNumVotesYes(atoi(cs));
}

void VoteMonitor::setVotesNo(const char* cs)
{
	voteMan->setNumVotesNo(atoi(cs));
}

void VoteMonitor::setVotesUndecided(const char* cs)
{
	voteMan->setNumVotesUndecided(atoi(cs));
}
