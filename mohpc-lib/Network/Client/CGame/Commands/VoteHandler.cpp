#include <MOHPC/Network/Client/CGame/Commands/VoteHandler.h>
#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(VoteCommandHandler);

VoteCommandHandler::VoteCommandHandler(const SnapshotProcessorPtr& snapshotProcessorPtr)
	: ServerCommandHandlerBase(snapshotProcessorPtr)
	, startReadFromServerHandler(*this)
	, continueReadFromServerHandler(*this)
	, finishReadFromServerHandler(*this)
{
	CommandManager& m = getSnapshotProcessor().getCommandManager();
	m.add("vo0", &startReadFromServerHandler);
	m.add("vo1", &continueReadFromServerHandler);
	m.add("vo2", &finishReadFromServerHandler);
}

VoteCommandHandler::~VoteCommandHandler()
{
	CommandManager& m = getSnapshotProcessor().getCommandManager();
	m.remove(&startReadFromServerHandler);
	m.remove(&continueReadFromServerHandler);
	m.remove(&finishReadFromServerHandler);
}

FunctionList<MOHPC::Network::CGame::Handlers::VoteParsed>& VoteCommandHandler::getHandler()
{
	return voteParsedHandler;
}

void VoteCommandHandler::commandStartReadFromServer(TokenParser& args)
{
	parser.begin(args.GetString(true, false));
}

void VoteCommandHandler::commandContinueReadFromServer(TokenParser& args)
{
	parser.append(args.GetString(true, false));
}

void VoteCommandHandler::commandFinishReadFromServer(TokenParser& args)
{
	VoteOptions options;
	parser.end(args.GetString(true, false), options);

	getHandler().broadcast(options);
}
