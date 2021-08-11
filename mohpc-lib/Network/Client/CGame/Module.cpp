#include <MOHPC/Network/Client/CGame/Module.h>
#include <MOHPC/Network/Client/CGame/MessageParse.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Common/Log.h>

#include <bitset>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

static constexpr char MOHPC_LOG_NAMESPACE[] = "cgame";

ModuleBase::ModuleBase()
	: processedSnapshots(makeShared<SnapshotProcessor>())
	, cgs(makeShared<cgsInfo>())
	, gameMonitor(processedSnapshots, cgs)
{
}

ModuleBase::~ModuleBase()
{
}

void ModuleBase::init(uintptr_t serverMessageSequence, rsequence_t serverCommandSequence)
{
	processedSnapshots->init(serverMessageSequence, serverCommandSequence);
}

void ModuleBase::setProtocol(protocolType_c protocol)
{
	const uint32_t version = protocol.getProtocolVersionNumber();
	messageParser = IMessageParser::get(version);
}

void ModuleBase::setImports(const Imports& imports)
{
	processedSnapshots->setPtrs(
		&imports.clientTime,
		&imports.snapshotManager,
		&imports.commandSequence
	);
}

void ModuleBase::parseCGMessage(MSG& msg)
{
	MessageInterfaces interfaces;
	interfaces.bullet = &gameplayNotify.getBulletNotify();
	interfaces.effect = &gameplayNotify.getEffectNotify();
	interfaces.hud = &gameplayNotify.getHUDNotify();
	interfaces.impact = &gameplayNotify.getImpactNotify();
	interfaces.event = &gameplayNotify.getEventNotify();
	messageParser->parseGameMessage(msg, interfaces);
}

void ModuleBase::tick(deltaTime_t deltaTime, tickTime_t currentTime, tickTime_t simulatedServerTime)
{
	// set snapshots transition
	// and process commands
	processedSnapshots->processSnapshots(simulatedServerTime);
}

SnapshotProcessor& ModuleBase::getSnapshotProcessor()
{
	return *processedSnapshots;
}

const SnapshotProcessor& ModuleBase::getSnapshotProcessor() const
{
	return *processedSnapshots;
}

const SnapshotProcessorPtr& ModuleBase::getSnapshotProcessorPtr()
{
	return processedSnapshots;
}

ConstSnapshotProcessorPtr ModuleBase::getSnapshotProcessorPtr() const
{
	return processedSnapshots;
}

GameplayNotify& ModuleBase::getGameplayNotify()
{
	return gameplayNotify;
}

const GameplayNotify& ModuleBase::getGameplayNotify() const
{
	return gameplayNotify;
}

const cgsInfo& ModuleBase::getServerInfo() const
{
	return *cgs;
}
