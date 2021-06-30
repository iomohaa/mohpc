#include <MOHPC/Network/Client/GameState.h>
#include <MOHPC/Network/Client/Time.h>
#include <MOHPC/Network/Types/Reliable.h>

using namespace MOHPC;
using namespace Network;

ServerGameState::ServerGameState(protocolType_c protocol)
	: serverId(0)
	, clientNum(0)
	, checksumFeed(0)
{
	gameStateParser = Parsing::IGameState::get(protocol.getProtocolVersionNumber());
}

ServerGameState::HandlerList& ServerGameState::getHandlers()
{
	return handlers;
}

const ServerGameState::HandlerList& ServerGameState::getHandlers() const
{
	return handlers;
}

gameState_t& ServerGameState::get()
{
	return gameState;
}

const gameState_t& ServerGameState::get() const
{
	return gameState;
}

void ServerGameState::notifyAllConfigStringChanges() const
{
	const ConfigStringManager& csMan = gameState.getConfigstringManager();
	const size_t maxConfigstrings = csMan.getMaxConfigStrings();

	// notify about configstrings that are not empty
	for (csNum_t i = 0; i < maxConfigstrings; ++i)
	{
		const char* cs = csMan.getConfigStringChecked(i);
		if (*cs) {
			getHandlers().configStringHandler.broadcast(i, cs);
		}
	}
}

bool ServerGameState::parseGameState(MSG& msg, ICommandSequence* serverCommands, ClientTime& clientTime)
{
	// create a new gameState with correct version
	gameState = gameStateParser->create();

	Parsing::gameStateClient_t clientData;
	gameStateParser->parseGameState(msg, gameState, clientData);

	// update the command sequence
	serverCommands->setSequence(clientData.commandSequence);

	clientNum = clientData.clientNum;
	// seems to be always zero, is it really useful?
	checksumFeed = clientData.checksumFeed;

	const bool isDiff = reloadGameState(clientTime);

	// take care of each config-string
	// the user must be fully aware of the current state
	notifyAllConfigStringChanges();

	// gameState has been parsed, notify
	getHandlers().gameStateParsedHandler.broadcast(*this, isDiff);

	return isDiff;
}

bool ServerGameState::reloadGameState(ClientTime& clientTime)
{
	Parsing::gameStateResults_t results;
	// Parse the new config
	gameStateParser->parseConfig(gameState, results);

	clientTime.setDeltaTime(results.serverDeltaTime);

	const bool isDiff = serverId != results.serverId;
	serverId = results.serverId;

	return isDiff;
}

void ServerGameState::modifyConfigString(csNum_t num, const char* string)
{
	get().getConfigstringManager().setConfigString(num, string);

	// Notify about modification
	getHandlers().configStringHandler.broadcast(num, string);
}
