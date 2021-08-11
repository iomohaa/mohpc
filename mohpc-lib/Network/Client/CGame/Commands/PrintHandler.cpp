#include <MOHPC/Network/Client/CGame/Commands/PrintHandler.h>
#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(PrintCommandHandler);

PrintCommandHandler::PrintCommandHandler(const SnapshotProcessorPtr& snapshotProcessorPtr)
	: ServerCommandHandlerBase(snapshotProcessorPtr)
	, printHandler(*this)
	, hudPrintHandler(*this)
{
	CommandManager& m = getSnapshotProcessor().getCommandManager();
	m.add("print", &printHandler);
	m.add("hudprint", &hudPrintHandler);
}

PrintCommandHandler::~PrintCommandHandler()
{
	CommandManager& m = getSnapshotProcessor().getCommandManager();
	m.remove(&printHandler);
	m.remove(&hudPrintHandler);
}

PrintCommandHandler::HandlerList& PrintCommandHandler::getHandlerList()
{
	return handlerList;
}

void PrintCommandHandler::handlePrint(TokenParser& args)
{
	const char* text = args.GetString(true, false);
	if (*text < (uint8_t)hudMessage_e::Max)
	{
		const hudMessage_e type = (hudMessage_e) * (text++);
		getHandlerList().printHandler.broadcast(type, text);
	}
	else
	{
		// should print in console if unspecified
		getHandlerList().printHandler.broadcast(hudMessage_e::Console, text);
	}
}

void PrintCommandHandler::handleHudPrint(TokenParser& args)
{
	const char* text = args.GetString(true, false);
	getHandlerList().hudPrintHandler.broadcast(text);
}
