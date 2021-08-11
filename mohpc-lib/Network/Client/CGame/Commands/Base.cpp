#include <MOHPC/Network/Client/CGame/Commands/Base.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

ServerCommandHandlerBase::ServerCommandHandlerBase(const SnapshotProcessorPtr& snapshotProcessorPtr)
	: snapshotProcessor(snapshotProcessorPtr)
{
}

ServerCommandHandlerBase::~ServerCommandHandlerBase()
{
}

SnapshotProcessor& ServerCommandHandlerBase::getSnapshotProcessor()
{
	return *snapshotProcessor;
}

const SnapshotProcessor& ServerCommandHandlerBase::getSnapshotProcessor() const
{
	return *snapshotProcessor;
}
