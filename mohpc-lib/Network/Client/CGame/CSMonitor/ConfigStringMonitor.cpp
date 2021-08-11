#include <MOHPC/Network/Client/CGame/CSMonitor/ConfigStringMonitor.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

ConfigstringMonitor::ConfigstringMonitor(const SnapshotProcessorPtr& snapshotProcessorPtr)
	: snapshotProcessor(snapshotProcessorPtr)
{
	using namespace std::placeholders;
	csHandle = snapshotProcessor->handlers().configstringModifiedHandler.add(std::bind(&ConfigstringMonitor::configStringModified, this, _1, _2));
}

ConfigstringMonitor::~ConfigstringMonitor()
{
}
