#include <MOHPC/Network/Client/GameState.h>
#include <MOHPC/Network/Client/Time.h>
#include <MOHPC/Network/Types/Reliable.h>

#include <MOHPC/Utility/CommandManager.h>
#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;
using namespace Network;

ServerGameState::ServerGameState()
	: csHandler(*this)
	, gameStateParser(nullptr)
	, clientTime(nullptr)
	, serverId(0)
	, clientNum(0)
	, checksumFeed(0)
{
}

ServerGameState::ServerGameState(protocolType_c protocol, ClientTime* clientTimePtr)
	: ServerGameState()
{
	gameStateParser = Parsing::IGameState::get(protocol.getProtocolVersionNumber());
	clientTime = clientTimePtr;
}

void ServerGameState::reset()
{
	get().reset();

	serverId = 0;
	clientNum = 0;
	checksumFeed = 0;
}

void ServerGameState::registerCommands(CommandManager& commandManager)
{
	commandManager.add("cs", &csHandler);
}

ServerGameState::HandlerList& ServerGameState::handlers()
{
	return handlerList;
}

const ServerGameState::HandlerList& ServerGameState::handlers() const
{
	return handlerList;
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
			handlers().configStringHandler.broadcast(i, cs);
		}
	}
}

bool ServerGameState::parseGameState(MSG& msg, ICommandSequence* serverCommands)
{
	if (!gameStateParser)
	{
		// null parser?
		return false;
	}

	// create a new gameState with correct version
	gameState = gameStateParser->create();

	Parsing::gameStateClient_t clientData;
	gameStateParser->parseGameState(msg, gameState, clientData);

	// update the command sequence
	serverCommands->setSequence(clientData.commandSequence);

	clientNum = clientData.clientNum;
	// seems to be always zero, is it really useful?
	checksumFeed = clientData.checksumFeed;

	const bool isDiff = reloadGameState();

	// take care of each config-string
	// the client must be fully aware of the current state
	notifyAllConfigStringChanges();

	// gameState has been parsed, notify
	handlers().gameStateParsedHandler.broadcast(*this, isDiff);

	return isDiff;
}

bool ServerGameState::reloadGameState()
{
	if (!gameStateParser)
	{
		// don't reload the game state if no parser exists
		return false;
	}

	Parsing::gameStateResults_t results;
	// Parse the new config
	gameStateParser->parseConfig(gameState, results);

	if (clientTime)
	{
		// set the new delta time from the game state
		clientTime->setDeltaTime(results.serverDeltaTime);
	}

	const bool isDiff = serverId != results.serverId;
	serverId = results.serverId;

	return isDiff;
}

void ServerGameState::modifyConfigString(csNum_t num, const char* string)
{
	get().getConfigstringManager().setConfigString(num, string);

	// Notify about modification
	handlers().configStringHandler.broadcast(num, string);
}

void ServerGameState::ConfigstringCommand(TokenParser& tokenized)
{
	// Retrieve the configstring number
	const uint32_t num = tokenized.GetInteger(true);
	// Get the content
	const char* csString = tokenized.GetString(true, false);

	// can set the config-string right now
	modifyConfigString(num, csString);

	if (num == CS_SYSTEMINFO || num == CS_SERVERINFO)
	{
		// reload gameState settings including the sv_fps value
		reloadGameState();
	}
}

uint32_t ServerGameState::getServerId() const
{
	return serverId;
}

uint32_t ServerGameState::getClientNum() const
{
	return clientNum;
}

uint32_t ServerGameState::getChecksumFeed() const
{
	return checksumFeed;
}
