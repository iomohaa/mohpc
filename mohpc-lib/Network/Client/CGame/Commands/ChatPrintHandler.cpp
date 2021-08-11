#include <MOHPC/Network/Client/CGame/Commands/ChatPrintHandler.h>
#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(ChatPrintCommandHandler);

ChatPrintCommandHandler::ChatPrintCommandHandler(const SnapshotProcessorPtr& snapshotProcessorPtr)
	: SingleCommandHandlerBase(snapshotProcessorPtr, "printdeathmsg")
{
}

ChatPrintCommandHandler::~ChatPrintCommandHandler()
{
}

void ChatPrintCommandHandler::handle(TokenParser& args)
{
	const str deathMessage1 = args.GetToken(true);
	const str deathMessage2 = args.GetToken(true);
	const str victimName = args.GetToken(true);
	const str attackerName = args.GetToken(true);
	const str killType = args.GetToken(true);

	hudMessage_e hudMessage;
	if (*killType.c_str() == std::tolower(*killType.c_str()))
	{
		// enemy kill
		hudMessage = hudMessage_e::ChatRed;
	}
	else
	{
		// allied kill
		hudMessage = hudMessage_e::ChatGreen;
	}

	switch (std::tolower(*killType.c_str()))
	{
		// suicide
	case 's':
	case 'w':
		getHandler().broadcast(
			hudMessage,
			(attackerName + " " + deathMessage1).c_str()
		);
		break;
		// killed by a player
	case 'p':
		if (*deathMessage2.c_str() != 'x')
		{
			getHandler().broadcast(
				hudMessage,
				(attackerName + " " + deathMessage1 + " " + victimName + " " + deathMessage2).c_str()
			);
		}
		else
		{
			getHandler().broadcast(
				hudMessage,
				(attackerName + " " + deathMessage1 + " " + victimName).c_str()
			);
		}
		break;
		// raw message
	default:
		getHandler().broadcast(hudMessage, deathMessage1.c_str());
		break;
	}
}
