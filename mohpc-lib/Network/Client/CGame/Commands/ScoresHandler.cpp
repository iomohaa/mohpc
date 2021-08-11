#include <MOHPC/Network/Client/CGame/Commands/ScoresHandler.h>
#include <MOHPC/Network/Client/CGame/Scoreboard.h>
#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(ScoresCommandHandler);

ScoresCommandHandler::ScoresCommandHandler(const SnapshotProcessorPtr& snapshotProcessorPtr, const cgsInfoPtr& cgsPtr)
	: SingleCommandHandlerBase(snapshotProcessorPtr, "scores")
	, cgs(cgsPtr)
{
}

ScoresCommandHandler::~ScoresCommandHandler()
{
}

void ScoresCommandHandler::handle(TokenParser& args)
{
	Scoreboard scoreboard;

	ScoreboardParser scoreboardParser(scoreboard, cgs->getGameType());
	scoreboardParser.parse(args);
	// Pass the parsed scoreboard
	getHandler().broadcast(scoreboard);
}
