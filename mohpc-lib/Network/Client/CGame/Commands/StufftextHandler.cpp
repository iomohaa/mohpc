#include <MOHPC/Network/Client/CGame/Commands/StufftextHandler.h>
#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(StufftextCommandHandler);

StufftextCommandHandler::StufftextCommandHandler(const SnapshotProcessorPtr& snapshotProcessorPtr)
	: SingleCommandHandlerBase(snapshotProcessorPtr, "stufftext")
{
}

StufftextCommandHandler::~StufftextCommandHandler()
{
}

void StufftextCommandHandler::handle(TokenParser& args)
{
	// just give the command string
	getHandler().broadcast(args);
}