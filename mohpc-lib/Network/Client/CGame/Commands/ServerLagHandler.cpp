#include <MOHPC/Network/Client/CGame/Commands/ServerLagHandler.h>
#include <MOHPC/Network/Client/Time.h>
#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(ServerLagCommandHandler);

ServerLagCommandHandler::ServerLagCommandHandler(const SnapshotProcessorPtr& snapshotProcessorPtr)
	: SingleCommandHandlerBase(snapshotProcessorPtr, "svlag")
{
}

ServerLagCommandHandler::~ServerLagCommandHandler()
{
}

void ServerLagCommandHandler::handle(TokenParser& args)
{
	// use the server time rather than the client time
	const netTime_t time = getSnapshotProcessor().getClientTime().getRemoteTime();
	getHandler().broadcast(time);
}