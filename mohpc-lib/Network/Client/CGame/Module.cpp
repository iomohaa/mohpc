#include <MOHPC/Network/Client/CGame/Module.h>
#include <MOHPC/Network/Client/CGame/MessageParse.h>
#include <MOHPC/Network/Client/ServerConnection.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Common/Vector.h>
#include <MOHPC/Common/Log.h>
#include <MOHPC/Utility/TokenParser.h>

#include <bitset>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

static constexpr char MOHPC_LOG_NAMESPACE[] = "cgame";

ModuleBase::ModuleBase()
	: processedSnapshots(serverCommandManager)
	, printHandler(*this)
	, hudPrintHandler(*this)
	, scoresHandler(*this)
	, statsHandler(*this)
	, stopwatchHandler(*this)
	, serverLagHandler(*this)
	, stufftextHandler(*this)
	, printDeathMsgHandler(*this)
{
	gameState = nullptr;

	size_t numCommands = 8;
	numCommands += voteManager.getNumCommandsToRegister();
	serverCommandManager.reserve(numCommands);

	using namespace std::placeholders;
	serverCommandManager.add("print", &printHandler);
	serverCommandManager.add("hudprint", &hudPrintHandler);
	serverCommandManager.add("scores", &scoresHandler);
	serverCommandManager.add("stats", &statsHandler);
	serverCommandManager.add("stopwatch", &stopwatchHandler);
	serverCommandManager.add("svlag", &serverLagHandler);
	serverCommandManager.add("stufftext", &stufftextHandler);
	serverCommandManager.add("printdeathmsg", &printDeathMsgHandler);

	voteManager.registerCommands(serverCommandManager);
}

ModuleBase::~ModuleBase()
{
	if (gameState)
	{
		gameState->handlers().configStringHandler.remove(configStringHandler);
	}
}

void ModuleBase::init(uintptr_t serverMessageSequence, rsequence_t serverCommandSequence)
{
	processedSnapshots.init(serverMessageSequence, serverCommandSequence);
}

void ModuleBase::setProtocol(protocolType_c protocol)
{
	const uint32_t version = protocol.getProtocolVersionNumber();
	environmentParse = Parsing::IEnvironment::get(version);
	gameStateParse = Parsing::IGameState::get(version);
	messageParser = IMessageParser::get(version);
}

void ModuleBase::setImports(const Imports& imports)
{
	processedSnapshots.setPtrs(
		&imports.clientTime,
		&imports.snapshotManager,
		&imports.commandSequence
	);

	if (gameState)
	{
		// clear from previous game state
		gameState->handlers().configStringHandler.remove(configStringHandler);
	}

	gameState = &imports.gameState;

	using namespace std::placeholders;
	configStringHandler = gameState->handlers().configStringHandler.add(std::bind(&ModuleBase::configStringModified, this, _1, _2));

	clientInfoList.setPtrs(&imports.gameState, imports.userInfo);
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
	svTime = simulatedServerTime;

	// set snapshots transition
	// and process commands
	processedSnapshots.processSnapshots(simulatedServerTime);

	if (voteManager.isModified())
	{
		// notify about the vote
		voteManager.notifyDirty();
	}
}

tickTime_t ModuleBase::getTime() const
{
	return svTime;
}

const rain_t& ModuleBase::getRain() const
{
	return rain;
}

const environment_t& ModuleBase::getEnvironment() const
{
	return environment;
}

const cgsInfo& ModuleBase::getServerInfo() const
{
	return cgs;
}

void ModuleBase::configStringModified(csNum_t num, const char* cs)
{
	using namespace ticks;

	switch (num)
	{
	case CS_SERVERINFO:
		parseServerInfo(cs);
		break;
	case CS_RAIN_DENSITY:
		rain.density = (float)atof(cs);
		break;
	case CS_RAIN_SPEED:
		rain.speed = (float)atof(cs);
		break;
	case CS_RAIN_SPEEDVARY:
		rain.speedVary = atoi(cs);
		break;
	case CS_RAIN_SLANT:
		rain.slant = atoi(cs);
		break;
	case CS_RAIN_LENGTH:
		rain.length = (float)atof(cs);
		break;
	case CS_RAIN_MINDIST:
		rain.minDist = (float)atof(cs);
		break;
	case CS_RAIN_WIDTH:
		rain.width = (float)atof(cs);
		break;
	case CS_RAIN_SHADER:
		if(rain.numShaders)
		{
			for (size_t i = 0; i < rain.numShaders; ++i)
			{
				// Multiple shaders
				rain.shader[i] = cs + std::to_string(i);
			}
		}
		else
		{
			// Single shader
			rain.shader[0] = cs;
		}
		break;
	case CS_RAIN_NUMSHADERS:
		rain.numShaders = atoi(cs);
		{
			const str tmp = rain.shader[0];
			for (size_t i = 0; i < rain.numShaders; ++i)
			{
				// Append shader number to the previous shader
				rain.shader[i] = tmp + std::to_string(i);
			}
		}
		break;
	case CS_WARMUP:
		cgs.matchStartTime = tickTime_t(milliseconds(atoll(cs)));
		break;
	case CS_FOGINFO:
		// Fog differs between games
		environmentParse->parseEnvironment(cs, environment);
		break;
	case CS_SKYINFO:
		environmentParse->parseSky(cs, environment);
		break;
	case CS_LEVEL_START_TIME:
		cgs.levelStartTime = tickTime_t(milliseconds(atoll(cs)));
		break;
	case CS_MATCHEND:
		cgs.matchEndTme = tickTime_t(milliseconds(atoll(cs)));
		break;
	case CS_VOTE_TIME:
		voteManager.setVoteTime(atoll(cs));
		break;
	case CS_VOTE_STRING:
		voteManager.setVoteString(cs);
		break;
	case CS_VOTES_YES:
		voteManager.setNumVotesYes(atoi(cs));
		break;
	case CS_VOTES_NO:
		voteManager.setNumVotesNo(atoi(cs));
		break;
	case CS_VOTES_UNDECIDED:
		voteManager.setNumVotesUndecided(atoi(cs));
		break;
	}

	// Add objectives
	if (num >= CS_OBJECTIVES && num < CS_OBJECTIVES + MAX_OBJECTIVES)
	{
		ReadOnlyInfo info(cs);

		const uint32_t objNum = num - CS_OBJECTIVES;
		objectiveManager.set(info, objNum);
	}

	// Add all clients
	if (num >= CS_PLAYERS && num < CS_PLAYERS + MAX_CLIENTS)
	{
		ReadOnlyInfo info(cs);

		const uint32_t clientNum = num - CS_PLAYERS;
		const clientInfo_t& client = clientInfoList.set(info, clientNum);
	}
}

void ModuleBase::parseServerInfo(const char* cs)
{
	ReadOnlyInfo info(cs);

	// Parse match settings
	size_t versionLen;
	const char* version = info.ValueForKey("version", versionLen);
	cgs.serverType = gameStateParse->parseServerType(version, versionLen);
	cgs.gameType = gameType_e(info.IntValueForKey("g_gametype"));
	cgs.dmFlags = info.IntValueForKey("dmflags");
	cgs.teamFlags = info.IntValueForKey("teamflags");
	cgs.fragLimit = info.IntValueForKey("fraglimit");
	cgs.timeLimit = info.IntValueForKey("timelimit");
	cgs.maxClients = info.IntValueForKey("sv_maxclients");
	cgs.allowVote = info.BoolValueForKey("g_allowVote");
	cgs.mapChecksum = info.IntValueForKey("sv_mapChecksum");
	// Parse map
	size_t mapLen;
	const char* mapName = info.ValueForKey("mapname", mapLen);
	if(*mapName)
	{
		const char* lastMapChar = strHelpers::findcharn(mapName, '$', mapLen);
		if(lastMapChar)
		{
			// don't put anything from the dollar
			cgs.mapName = str(mapName, 0, lastMapChar - mapName);
		}
		else {
			cgs.mapName = str(mapName, mapLen);
		}

		cgs.mapFilename = "maps/" + cgs.mapName + ".bsp";
	}

	// Parse scoreboard info
	cgs.alliedText[0] = info.ValueForKey("g_obj_alliedtext1");
	cgs.alliedText[1] = info.ValueForKey("g_obj_alliedtext2");
	cgs.alliedText[2] = info.ValueForKey("g_obj_alliedtext3");
	cgs.alliedText[3] = info.ValueForKey("g_obj_alliedtext4");
	cgs.alliedText[4] = info.ValueForKey("g_obj_alliedtext5");
	cgs.axisText[0] = info.ValueForKey("g_obj_axistext1");
	cgs.axisText[1] = info.ValueForKey("g_obj_axistext2");
	cgs.axisText[2] = info.ValueForKey("g_obj_axistext3");
	cgs.axisText[3] = info.ValueForKey("g_obj_axistext4");
	cgs.axisText[4] = info.ValueForKey("g_obj_axistext5");
	cgs.scoreboardPic = info.ValueForKey("g_scoreboardpic");
	cgs.scoreboardPicOver = info.ValueForKey("g_scoreboardpicover");
}

void ModuleBase::SCmd_Print(TokenParser& args)
{
	const char* text = args.GetString(true, false);
	if(*text < (uint8_t)hudMessage_e::Max)
	{
		const hudMessage_e type = (hudMessage_e) * (text++);
		handlers().printHandler.broadcast(type, text);
	}
	else
	{
		// should print in console if unspecified
		handlers().printHandler.broadcast(hudMessage_e::Console, text);
	}
}

void ModuleBase::SCmd_HudPrint(TokenParser& args)
{
	const char* text = args.GetString(true, false);
	handlers().hudPrintHandler.broadcast(text);
}

void ModuleBase::SCmd_Scores(TokenParser& args)
{
	Scoreboard scoreboard;

	ScoreboardParser scoreboardParser(scoreboard, cgs.gameType);
	scoreboardParser.parse(args);
	// Pass the parsed scoreboard
	handlers().scmdScoresHandler.broadcast(scoreboard);
}

void ModuleBase::SCmd_Stats(TokenParser& args)
{
	stats_t stats;

	stats.numObjectives			= args.GetInteger(false);
	stats.numComplete			= args.GetInteger(false);
	stats.numShotsFired			= args.GetInteger(false);
	stats.numHits				= args.GetInteger(false);
	stats.accuracy				= args.GetInteger(false);
	stats.preferredWeapon		= args.GetToken(false);
	stats.numHitsTaken			= args.GetInteger(false);
	stats.numObjectsDestroyed	= args.GetInteger(false);
	stats.numEnemysKilled		= args.GetInteger(false);
	stats.headshots				= args.GetInteger(false);
	stats.torsoShots			= args.GetInteger(false);
	stats.leftLegShots			= args.GetInteger(false);
	stats.rightLegShots			= args.GetInteger(false);
	stats.groinShots			= args.GetInteger(false);
	stats.leftArmShots			= args.GetInteger(false);
	stats.rightArmShots			= args.GetInteger(false);
	stats.gunneryEvaluation		= args.GetInteger(false);
	stats.gotMedal				= args.GetInteger(false);
	stats.success				= args.GetInteger(false);
	stats.failed				= args.GetInteger(false);

	// Notify about stats
	handlers().scmdStatsHandler.broadcast(stats);
}

void ModuleBase::SCmd_Stopwatch(TokenParser& args)
{
	const uint64_t startTime = args.GetInteger64(false);
	const uint64_t endTime = args.GetInteger64(false);
	handlers().scmdStopwatchHandler.broadcast(startTime, endTime);
}

void ModuleBase::SCmd_ServerLag(TokenParser& args)
{
	cgs.serverLagTime = getTime();
	handlers().scmdServerLagHandler.broadcast();
}

void ModuleBase::SCmd_Stufftext(TokenParser& args)
{
	handlers().scmdStufftextHandler.broadcast(args);
}

ModuleBase::HandlerList& ModuleBase::handlers()
{
	return handlerList;
}

const ModuleBase::HandlerList& ModuleBase::handlers() const
{
	return handlerList;
}

void ModuleBase::SCmd_PrintDeathMsg(TokenParser& args)
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
		handlers().printHandler.broadcast(
			hudMessage,
			(attackerName + " " + deathMessage1).c_str()
			);
		break;
	// killed by a player
	case 'p':
		if (*deathMessage2.c_str() != 'x')
		{
			handlers().printHandler.broadcast(
				hudMessage,
				(attackerName + " " + deathMessage1 + " " + victimName + " " + deathMessage2).c_str()
			);
		}
		else
		{
			handlers().printHandler.broadcast(
				hudMessage,
				(attackerName + " " + deathMessage1 + " " + victimName).c_str()
			);
		}
		break;
	// raw message
	default:
		handlers().printHandler.broadcast(hudMessage, deathMessage1.c_str());
		break;
	}
}

SnapshotProcessor& ModuleBase::getSnapshotProcessor()
{
	return processedSnapshots;
}

const SnapshotProcessor& ModuleBase::getSnapshotProcessor() const
{
	return processedSnapshots;
}

VoteManager& ModuleBase::getVoteManager()
{
	return voteManager;
}

const VoteManager& ModuleBase::getVoteManager() const
{
	return voteManager;
}

const ObjectiveManager& ModuleBase::getObjectiveManager() const
{
	return objectiveManager;
}

const ClientInfoList& ModuleBase::getClientInfoList() const
{
	return clientInfoList;
}

GameplayNotify& ModuleBase::getGameplayNotify()
{
	return gameplayNotify;
}

const GameplayNotify& ModuleBase::getGameplayNotify() const
{
	return gameplayNotify;
}

rain_t::rain_t()
	: density(0.f)
	, speed(0.f)
	, speedVary(0)
	, slant(0)
	, length(0.f)
	, minDist(0.f)
	, width(0.f)
	, numShaders(0)
{
}

float rain_t::getDensity() const
{
	return density;
}

float rain_t::getSpeed() const
{
	return speed;
}

float rain_t::getLength() const
{
	return length;
}

float rain_t::getMinimumDistance() const
{
	return minDist;
}

float rain_t::getWidth() const
{
	return width;
}

uint32_t rain_t::getSpeedVariation() const
{
	return speedVary;
}

uint32_t rain_t::getSlant() const
{
	return slant;
}

uint32_t rain_t::getNumShaders() const
{
	return numShaders;
}

const char* rain_t::getShader(uint8_t index) const
{
	return shader[index].c_str();
}
