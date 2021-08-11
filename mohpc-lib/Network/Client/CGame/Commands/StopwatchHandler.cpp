#include <MOHPC/Network/Client/CGame/Commands/StopwatchHandler.h>
#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(StopwatchCommandHandler);

StopwatchCommandHandler::StopwatchCommandHandler(const SnapshotProcessorPtr& snapshotProcessorPtr)
	: SingleCommandHandlerBase(snapshotProcessorPtr, "stopwatch")
{
}

StopwatchCommandHandler::~StopwatchCommandHandler()
{
}

void StopwatchCommandHandler::handle(TokenParser& args)
{
	using namespace std::chrono;

	const netTime_t startTime = netTime_t(milliseconds(args.GetInteger64(false)));
	const netTime_t endTime = netTime_t(milliseconds(args.GetInteger64(false)));
	getHandler().broadcast(startTime, endTime);
}
