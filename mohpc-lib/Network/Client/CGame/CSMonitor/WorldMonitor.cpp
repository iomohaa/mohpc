#include <MOHPC/Network/Client/CGame/CSMonitor/WorldMonitor.h>
#include <MOHPC/Network/Types/Protocol.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

template<>
std::map<csNum_t, ConfigstringMonitorTemplate<WorldMonitor>::Callback> ConfigstringMonitorTemplate<WorldMonitor>::callbackMap
{
	{ CS::FOGINFO, &WorldMonitor::setFogInfo},
	{ CS::SKYINFO, &WorldMonitor::setSkyInfo}
};

MOHPC_OBJECT_DEFINITION(WorldMonitor);

WorldMonitor::WorldMonitor(const SnapshotProcessorPtr& snapshotProcessor, const EnvironmentPtr& envPtr, const protocolType_c& protocolType)
	: ConfigstringMonitorTemplate(snapshotProcessor)
	, env(envPtr)
{
	environmentParse = Parsing::IEnvironment::get(protocolType.getProtocolVersionNumber());
}

WorldMonitor::~WorldMonitor()
{
}

void WorldMonitor::setFogInfo(const char* cs)
{
	environmentParse->parseEnvironment(cs, *env);
}

void WorldMonitor::setSkyInfo(const char* cs)
{
	environmentParse->parseSky(cs, *env);
}
