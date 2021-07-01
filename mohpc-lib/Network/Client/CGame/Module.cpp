#include <MOHPC/Network/Client/CGame/Module.h>
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

#define MOHPC_LOG_NAMESPACE "cgame" 

const char* effectsModel[] =
{
"models/fx/barrel_oil_leak_big.tik",
"models/fx/barrel_oil_leak_medium.tik",
"models/fx/barrel_oil_leak_small.tik",
"models/fx/barrel_oil_leak_splat.tik",
"models/fx/barrel_water_leak_big.tik",
"models/fx/barrel_water_leak_medium.tik",
"models/fx/barrel_water_leak_small.tik",
"models/fx/barrel_water_leak_splat.tik",
"models/fx/bazookaexp_base.tik",
"models/fx/bazookaexp_dirt.tik",
"models/fx/bazookaexp_snow.tik",
"models/fx/bazookaexp_stone.tik",
"models/fx/bh_carpet_hard.tik",
"models/fx/bh_carpet_lite.tik",
"models/fx/bh_dirt_hard.tik",
"models/fx/bh_dirt_lite.tik",
"models/fx/bh_foliage_hard.tik",
"models/fx/bh_foliage_lite.tik",
"models/fx/bh_glass_hard.tik",
"models/fx/bh_glass_lite.tik",
"models/fx/bh_grass_hard.tik",
"models/fx/bh_grass_lite.tik",
"models/fx/bh_human_uniform_hard.tik",
"models/fx/bh_human_uniform_lite.tik",
"models/fx/bh_metal_hard.tik",
"models/fx/bh_metal_lite.tik",
"models/fx/bh_mud_hard.tik",
"models/fx/bh_mud_lite.tik",
"models/fx/bh_paper_hard.tik",
"models/fx/bh_paper_lite.tik",
"models/fx/bh_sand_hard.tik",
"models/fx/bh_sand_lite.tik",
"models/fx/bh_snow_hard.tik",
"models/fx/bh_snow_lite.tik",
"models/fx/bh_stone_hard.tik"
"models/fx/bh_stone_lite.tik",
"models/fx/bh_water_hard.tik",
"models/fx/bh_water_lite.tik",
"models/fx/bh_wood_hard.tik",
"models/fx/bh_wood_lite.tik",
"models/fx/fs_dirt.tik",
"models/fx/fs_grass.tik",
"models/fx/fs_heavy_dust.tik",
"models/fx/fs_light_dust.tik",
"models/fx/fs_mud.tik",
"models/fx/fs_puddle.tik",
"models/fx/fs_sand.tik",
"models/fx/fs_snow.tik",
"models/fx/fx_fence_wood.tik",
"models/fx/grenexp_base.tik",
"models/fx/grenexp_carpet.tik",
"models/fx/grenexp_dirt.tik",
"models/fx/grenexp_foliage.tik",
"models/fx/grenexp_grass.tik",
"models/fx/grenexp_gravel.tik",
"models/fx/grenexp_metal.tik",
"models/fx/grenexp_mud.tik",
"models/fx/grenexp_paper.tik",
"models/fx/grenexp_sand.tik",
"models/fx/grenexp_snow.tik",
"models/fx/grenexp_stone.tik",
"models/fx/grenexp_water.tik",
"models/fx/grenexp_wood.tik",
"models/fx/heavyshellexp_base.tik",
"models/fx/heavyshellexp_dirt.tik",
"models/fx/heavyshellexp_snow.tik",
"models/fx/heavyshellexp_stone.tik",
"models/fx/tankexp_base.tik",
"models/fx/tankexp_dirt.tik",
"models/fx/tankexp_snow.tik",
"models/fx/tankexp_stone.tik",
"models/fx/water_ripple_moving.tik",
"models/fx/water_ripple_still.tik",
"models/fx/water_trail_bubble.tik"
};

const char* MOHPC::Network::CGame::getEffectName(effects_e effect)
{
	return effectsModel[(size_t)effect];
}

ModuleBase::ModuleBase()
	: processedSnapshots(serverCommandManager)
{
	gameState = nullptr;

	size_t numCommands = 8;
	numCommands += voteManager.getNumCommandsToRegister();
	serverCommandManager.reserveCommands(numCommands);

	using namespace std::placeholders;
	serverCommandManager.addCommand(Command("print", std::bind(&ModuleBase::SCmd_HudPrint, this, _1)));
	serverCommandManager.addCommand(Command("hudprint", std::bind(&ModuleBase::SCmd_Scores, this, _1)));
	serverCommandManager.addCommand(Command("scores", std::bind(&ModuleBase::SCmd_Stats, this, _1)));
	serverCommandManager.addCommand(Command("stats", std::bind(&ModuleBase::SCmd_Stopwatch, this, _1)));
	serverCommandManager.addCommand(Command("stopwatch", std::bind(&ModuleBase::SCmd_ServerLag, this, _1)));
	serverCommandManager.addCommand(Command("svlag", std::bind(&ModuleBase::SCmd_Stufftext, this, _1)));
	serverCommandManager.addCommand(Command("stufftext", std::bind(&ModuleBase::SCmd_Print, this, _1)));
	serverCommandManager.addCommand(Command("printdeathmsg", std::bind(&ModuleBase::SCmd_PrintDeathMsg, this, _1)));

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

	prediction.setProtocol(protocol);
}

void ModuleBase::setImports(const Imports& imports)
{
	prediction.setUserInputPtr(&imports.userInput);

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
	MsgTypesHelper msgHelper(msg);

	// loop until there is no message
	bool hasMessage;
	do
	{
		uint8_t msgType = msg.ReadNumber<uint8_t>(6);

		handleCGMessage(msg, msgType);
		hasMessage = msg.ReadBool();
	} while (hasMessage);
}

void ModuleBase::tick(uint64_t deltaTime, uint64_t currentTime, uint64_t serverTime)
{
	svTime = serverTime;

	// set snapshots transition
	// and process commands
	processedSnapshots.processSnapshots(serverTime);
	traceManager.buildSolidList(processedSnapshots);
	// process prediction stuff
	PredictionParm predictionParm{
		processedSnapshots,
		cgs
	};

	prediction.process(serverTime, predictionParm);

	if (voteManager.isModified())
	{
		// notify about the vote
		voteManager.notifyDirty();
	}
}

uint64_t ModuleBase::getTime() const
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
				rain.shader[i] = str::printf("%s%i", cs, i);
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
				rain.shader[i] = str::printf("%s%i", tmp.c_str(), i);
			}
		}
		break;
	case CS_WARMUP:
		cgs.matchStartTime = atoll(cs);
		break;
	case CS_FOGINFO:
		// Fog differs between games
		environmentParse->parseEnvironment(cs, environment);
		break;
	case CS_SKYINFO:
		environmentParse->parseSky(cs, environment);
		break;
	case CS_LEVEL_START_TIME:
		cgs.levelStartTime = atoll(cs);
		break;
	case CS_MATCHEND:
		cgs.matchEndTme = atoll(cs);
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
		const char* lastMapChar = str::findcharn(mapName, '$', mapLen);
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
	if (*killType == tolower(*killType))
	{
		// enemy kill
		hudMessage = hudMessage_e::ChatRed;
	}
	else
	{
		// allied kill
		hudMessage = hudMessage_e::ChatGreen;
	}

	switch (tolower(*killType))
	{
	// suicide
	case 's':
	case 'w':
		handlers().printHandler.broadcast(
			hudMessage,
			str::printf("%s %s", attackerName.c_str(), deathMessage1.c_str()).c_str()
			);
		break;
	// killed by a player
	case 'p':
		if (*deathMessage2 != 'x')
		{
			handlers().printHandler.broadcast(
				hudMessage,
				str::printf("%s %s %s %s", attackerName.c_str(), deathMessage1.c_str(), victimName.c_str(), deathMessage2.c_str()).c_str()
			);
		}
		else
		{
			handlers().printHandler.broadcast(
				hudMessage,
				str::printf("%s %s %s", attackerName.c_str(), deathMessage1.c_str(), victimName.c_str()).c_str()
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

const TraceManager& ModuleBase::getTraceManager() const
{
	return traceManager;
}

Prediction& ModuleBase::getPrediction()
{
	return prediction;
}

const Prediction& ModuleBase::getPrediction() const
{
	return prediction;
}

CGameModule6::CGameModule6()
{
}

void CGameModule6::handleCGMessage(MSG& msg, uint8_t msgId)
{
	// Version 6
	enum class cgmessage_e
	{
		bullet1 = 1,
		bullet2,
		bullet3,
		bullet4,
		bullet5,
		impact1,
		impact2,
		impact3,
		impact4,
		impact5,
		impact_melee,
		explo1,
		explo2,
		unk1,
		effect1,
		effect2,
		effect3,
		effect4,
		effect5,
		effect6,
		effect7,
		effect8,
		debris_crate,
		debris_window,
		tracer_visible,
		tracer_hidden,
		huddraw_shader,
		huddraw_align,
		huddraw_rect,
		huddraw_virtualscreen,
		huddraw_color,
		huddraw_alpha,
		huddraw_string,
		huddraw_font,
		notify_hit,
		notify_kill,
		playsound_entity,
	};

	uint32_t temp;
	uint32_t count;
	uint32_t large = false;
	StringMessage strVal;
	Vector vecTmp;
	Vector vecStart, vecEnd;
	Vector vecArray[64];
	MsgTypesHelper msgHelper(msg);
	CommonMessageHandler commonMessage(msg, handlers());

	const cgmessage_e msgType((cgmessage_e)msgId);
	switch (msgType)
	{
	case cgmessage_e::bullet1:
		vecTmp = msgHelper.ReadVectorCoord();
	case cgmessage_e::bullet2:
	case cgmessage_e::bullet5:
	{
		vecStart = msgHelper.ReadVectorCoord();
		vecTmp = vecStart;
		vecArray[0] = msgHelper.ReadVectorCoord();
		large = msg.ReadBool();

		if (msgType == cgmessage_e::bullet1 || msgType == cgmessage_e::bullet2)
		{
			handlers().makeBulletTracerHandler.broadcast(
				vecStart,
				vecTmp,
				vecArray[0],
				1u,
				large,
				msgType == cgmessage_e::bullet1 ? 1u : 0u,
				1.f
			);
		}
		else {
			handlers().makeBubbleTrailHandler.broadcast(vecStart, vecEnd, large, 1.f);
		}
		break;
	}
	case cgmessage_e::bullet3:
	{
		vecTmp = msgHelper.ReadVectorCoord();
		temp = msg.ReadNumber<uint32_t>(6);
	}
	case cgmessage_e::bullet4:
	{
		vecStart = msgHelper.ReadVectorCoord();
		large = msg.ReadBool();
		count = msg.ReadNumber<uint32_t>(6);

		for (size_t i = 0; i < count; ++i) {
			vecArray[i] = msgHelper.ReadVectorCoord();
		}

		handlers().makeBulletTracerHandler.broadcast(
			vecTmp,
			vecTmp,
			vecArray[0],
			count,
			large,
			temp,
			1.f
			);
		break;
	}
	case cgmessage_e::impact1:
	case cgmessage_e::impact2:
	case cgmessage_e::impact3:
	case cgmessage_e::impact4:
	case cgmessage_e::impact5:
		vecStart = msgHelper.ReadVectorCoord();
		vecEnd = msgHelper.ReadDir();
		large = msg.ReadBool();

		handlers().impactHandler.broadcast(
			vecStart,
			vecEnd,
			large
		);
		break;
	case cgmessage_e::impact_melee:
		commonMessage.impactMelee();
		break;
	case cgmessage_e::explo1:
	case cgmessage_e::explo2:
	{
		uint32_t effectId = msgId == 12 || msgId != 13 ? 63 : 64;
		vecStart = msgHelper.ReadVectorCoord();

		handlers().makeExplosionEffectHandler.broadcast(
			vecStart,
			getEffectName(getEffectId(effectId))
			);
		break;
	}
	case cgmessage_e::effect1:
	case cgmessage_e::effect2:
	case cgmessage_e::effect3:
	case cgmessage_e::effect4:
	case cgmessage_e::effect5:
	case cgmessage_e::effect6:
	case cgmessage_e::effect7:
	case cgmessage_e::effect8:
		vecStart = msgHelper.ReadVectorCoord();
		vecEnd = msgHelper.ReadDir();

		handlers().makeEffectHandler.broadcast(
			vecStart,
			vecEnd,
			getEffectName(getEffectId(msgId + 67))
		);
		break;
	case cgmessage_e::debris_crate:
		commonMessage.debrisCrate();
		break;
	case cgmessage_e::debris_window:
		commonMessage.debrisWindow();
		break;
	case cgmessage_e::tracer_visible:
		vecTmp = msgHelper.ReadVectorCoord();
		vecStart = msgHelper.ReadVectorCoord();
		vecArray[0] = msgHelper.ReadVectorCoord();
		large = msg.ReadBool();

		handlers().makeBulletTracerHandler.broadcast(
			vecTmp,
			vecStart,
			vecArray[0],
			1u,
			large,
			1u,
			1.f
		);
		break;
	case cgmessage_e::tracer_hidden:
		vecTmp = vec_zero;
		vecStart = msgHelper.ReadVectorCoord();
		vecArray[0] = msgHelper.ReadVectorCoord();
		large = msg.ReadBool();
		
		handlers().makeBulletTracerHandler.broadcast(
			vecTmp,
			vecStart,
			vecArray[0],
			1u,
			large,
			0u,
			1.f
		);
		break;
	case cgmessage_e::huddraw_shader:
		commonMessage.huddrawShader();
		break;
	case cgmessage_e::huddraw_align:
		commonMessage.huddrawAlign();
		break;
	case cgmessage_e::huddraw_rect:
		commonMessage.huddrawRect();
		break;
	case cgmessage_e::huddraw_virtualscreen:
		commonMessage.huddrawVirtualScreen();
		break;
	case cgmessage_e::huddraw_color:
		commonMessage.huddrawColor();
		break;
	case cgmessage_e::huddraw_alpha:
		commonMessage.huddrawAlpha();
		break;
	case cgmessage_e::huddraw_string:
		commonMessage.huddrawString();
		break;
	case cgmessage_e::huddraw_font:
		commonMessage.huddrawFont();
		break;
	case cgmessage_e::notify_hit:
		commonMessage.notifyHit();
		break;
	case cgmessage_e::notify_kill:
		commonMessage.notifyKill();
		break;
	case cgmessage_e::playsound_entity:
		commonMessage.playSoundEntity();
		break;
	default:
		break;
	}
}

effects_e CGameModule6::getEffectId(uint32_t effectId)
{
	static effects_e effectList[] =
	{
	effects_e::bh_paper_lite,
	effects_e::bh_paper_hard,
	effects_e::bh_wood_lite,
	effects_e::bh_wood_hard,
	effects_e::bh_metal_lite,
	effects_e::bh_metal_hard,
	effects_e::bh_stone_lite,
	effects_e::bh_stone_hard,
	effects_e::bh_dirt_lite,
	effects_e::bh_dirt_hard,
	// 10
	effects_e::bh_metal_lite,
	effects_e::bh_metal_hard,
	effects_e::bh_grass_lite,
	effects_e::bh_grass_hard,
	effects_e::bh_mud_lite,
	// 15
	effects_e::bh_mud_hard,
	effects_e::bh_water_lite,
	effects_e::bh_water_hard,
	effects_e::bh_glass_lite,
	effects_e::bh_glass_hard,
	// 20
	effects_e::bh_stone_lite,
	effects_e::bh_stone_hard,
	effects_e::bh_sand_lite,
	effects_e::bh_sand_hard,
	effects_e::bh_foliage_lite,
	// 25
	effects_e::bh_foliage_hard,
	effects_e::bh_snow_lite,
	effects_e::bh_snow_hard,
	effects_e::bh_carpet_lite,
	effects_e::bh_carpet_hard,
	// 30
	effects_e::bh_human_uniform_lite,
	effects_e::bh_human_uniform_hard,
	effects_e::water_trail_bubble,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 35
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 40
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 45
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 50
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 55
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 60
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::grenexp_base,
	effects_e::bazookaexp_base,
	// 65
	effects_e::grenexp_paper,
	effects_e::grenexp_wood,
	effects_e::grenexp_metal,
	effects_e::grenexp_stone,
	effects_e::grenexp_dirt,
	// 70
	effects_e::grenexp_metal,
	effects_e::grenexp_grass,
	effects_e::grenexp_mud,
	effects_e::grenexp_water,
	effects_e::bh_stone_hard,
	// 75
	effects_e::grenexp_gravel,
	effects_e::grenexp_sand,
	effects_e::grenexp_foliage,
	effects_e::grenexp_snow,
	effects_e::grenexp_carpet,
	// 80
	effects_e::water_ripple_still,
	effects_e::water_ripple_moving,
	effects_e::barrel_oil_leak_big,
	effects_e::barrel_oil_leak_medium,
	effects_e::barrel_oil_leak_small,
	// 85
	effects_e::barrel_oil_leak_splat,
	effects_e::barrel_water_leak_big,
	effects_e::barrel_water_leak_medium,
	effects_e::barrel_water_leak_small,
	effects_e::barrel_water_leak_splat,
	// 90
	effects_e::fs_light_dust,
	effects_e::fs_heavy_dust,
	effects_e::fs_dirt,
	effects_e::fs_grass,
	effects_e::fs_mud,
	// 95
	effects_e::fs_puddle,
	effects_e::fs_sand,
	effects_e::fs_snow,
	effects_e::bh_stone_hard
	};

	static constexpr size_t numEffects = sizeof(effectList) / sizeof(effectList[0]);
	static_assert(numEffects == 99);

	if (effectId < numEffects) {
		return effectList[effectId];
	}

	return effects_e::bh_stone_hard;
}

CGameModule15::CGameModule15()
{
}

void CGameModule15::handleCGMessage(MSG& msg, uint8_t msgId)
{
	// Version 15
	enum class cgmessage_e
	{
		bullet1 = 1,
		bullet2,
		bullet3,
		bullet4,
		bullet5,
		impact1,
		impact2,
		impact3,
		impact4,
		impact5,
		impact6,
		impact_melee,
		explo1,
		explo2,
		explo3,
		explo4,
		unk1,
		effect1,
		effect2,
		effect3,
		effect4,
		effect5,
		effect6,
		effect7,
		effect8,
		debris_crate,
		debris_window,
		tracer_visible,
		tracer_hidden,
		huddraw_shader,
		huddraw_align,
		huddraw_rect,
		huddraw_virtualscreen,
		huddraw_color,
		huddraw_alpha,
		huddraw_string,
		huddraw_font,
		notify_hit,
		notify_kill,
		playsound_entity,
		// FIXME: must analyze precisely what this one does
		effect9
	};

	uint32_t temp;
	uint32_t count;
	uint32_t effectId;
	uint32_t large = 0;
	StringMessage strVal;
	Vector vecTmp;
	Vector vecStart, vecEnd;
	Vector vecArray[64];
	MsgTypesHelper msgHelper(msg);
	CommonMessageHandler commonMessage(msg, handlers());

	struct
	{
		float readBulletSize(MSG& msg)
		{
			static constexpr float MIN_BULLET_SIZE = 1.f / 512.f;

			const bool hasSize = msg.ReadBool();
			if (hasSize)
			{
				const uint16_t intSize = msg.ReadNumber<uint16_t>(10);

				float bulletSize = (float)intSize / 512.f;
				if (bulletSize < MIN_BULLET_SIZE) bulletSize = MIN_BULLET_SIZE;

				return bulletSize;
			}
			else {
				return 1.f;
			}
		}
	} utils;

	cgmessage_e msgType((cgmessage_e)msgId);
	switch (msgType)
	{
	case cgmessage_e::bullet1:
		vecTmp = msgHelper.ReadVectorCoord();
	case cgmessage_e::bullet2:
	case cgmessage_e::bullet5:
	{
		vecStart = msgHelper.ReadVectorCoord();
		vecTmp = vecStart;
		vecArray[0] = msgHelper.ReadVectorCoord();

		const uint8_t iLarge = msg.ReadNumber<uint8_t>(2);

		const float bulletSize = utils.readBulletSize(msg);

		if (msgType == cgmessage_e::bullet1 || msgType == cgmessage_e::bullet2)
		{
			handlers().makeBulletTracerHandler.broadcast(
				vecStart,
				vecTmp,
				vecArray[0],
				1u,
				large,
				msgType == cgmessage_e::bullet1 ? 1u : 0u,
				1.f
			);
		}
		else
		{
			handlers().makeBubbleTrailHandler.broadcast(
				vecStart,
				vecEnd,
				large,
				1.f
			);
		}
		break;
	}
	case cgmessage_e::bullet3:
	case cgmessage_e::bullet4:
	{
		if (msgType == cgmessage_e::bullet3)
		{
			vecTmp = msgHelper.ReadVectorCoord();
			temp = msg.ReadNumber<uint32_t>(6);
		}
		else {
			temp = 0;
		}

		vecStart = msgHelper.ReadVectorCoord();

		large = msg.ReadNumber<uint32_t>(2);

		const float bulletSize = utils.readBulletSize(msg);

		count = msg.ReadNumber<uint32_t>(6);

		for (size_t i = 0; i < count; ++i) {
			vecArray[i] = msgHelper.ReadVectorCoord();
		}

		if (count)
		{
			handlers().makeBulletTracerHandler.broadcast(
				vecStart,
				vecTmp,
				vecArray[0],
				count,
				large,
				temp,
				1.f
				);
		}
		break;
	}
	case cgmessage_e::impact1:
	case cgmessage_e::impact2:
	case cgmessage_e::impact3:
	case cgmessage_e::impact4:
	case cgmessage_e::impact5:
	{
		vecStart = msgHelper.ReadVectorCoord();
		vecEnd = msgHelper.ReadDir();

		const uint8_t iLarge = msg.ReadNumber<uint8_t>(2);

		handlers().impactHandler.broadcast(
			vecStart,
			vecEnd,
			large
		);
		break;
	}
	case cgmessage_e::impact_melee:
		commonMessage.impactMelee();
		break;
	case cgmessage_e::explo1:
	case cgmessage_e::explo2:
	case cgmessage_e::explo3:
	case cgmessage_e::explo4:
		vecStart = msgHelper.ReadVectorCoord();

		switch (msgType)
		{
		case cgmessage_e::explo1:
			effectId = 63;
			break;
		case cgmessage_e::explo2:
			effectId = 64;
			break;
		case cgmessage_e::explo3:
			effectId = 65;
			break;
		case cgmessage_e::explo4:
			effectId = 66;
			break;
		default:
			effectId = 63;
		}

		handlers().makeExplosionEffectHandler.broadcast(
			vecStart,
			getEffectName(getEffectId(effectId))
		);
		break;
	case cgmessage_e::effect1:
	case cgmessage_e::effect2:
	case cgmessage_e::effect3:
	case cgmessage_e::effect4:
	case cgmessage_e::effect5:
	case cgmessage_e::effect6:
	case cgmessage_e::effect7:
	case cgmessage_e::effect8:
		vecStart = msgHelper.ReadVectorCoord();
		vecEnd = msgHelper.ReadDir();

		handlers().makeEffectHandler.broadcast(
			vecStart,
			vecEnd,
			getEffectName(getEffectId(msgId + 75))
		);
		break;
	case cgmessage_e::debris_crate:
		commonMessage.debrisCrate();
		break;
	case cgmessage_e::debris_window:
		commonMessage.debrisWindow();
		break;
	case cgmessage_e::tracer_visible:
	{
		vecTmp = msgHelper.ReadVectorCoord();
		vecStart = msgHelper.ReadVectorCoord();
		vecArray[0] = msgHelper.ReadVectorCoord();

		const uint8_t iLarge = msg.ReadNumber<uint8_t>(2);
		const float bulletSize = utils.readBulletSize(msg);

		handlers().makeBulletTracerHandler.broadcast(
			vecTmp,
			vecStart,
			vecArray[0],
			1u,
			large,
			1u,
			1.f
		);
		break;
	}
	case cgmessage_e::tracer_hidden:
	{
		vecTmp = vec_zero;
		vecStart = msgHelper.ReadVectorCoord();
		vecArray[0] = msgHelper.ReadVectorCoord();

		const uint8_t iLarge = msg.ReadNumber<uint8_t>(2);
		const float bulletSize = utils.readBulletSize(msg);

		handlers().makeBulletTracerHandler.broadcast(
			vecTmp,
			vecStart,
			vecArray[0],
			1u,
			large,
			0u,
			1.f
		);
		break;
	}
	case cgmessage_e::huddraw_shader:
		commonMessage.huddrawShader();
		break;
	case cgmessage_e::huddraw_align:
		commonMessage.huddrawAlign();
		break;
	case cgmessage_e::huddraw_rect:
		commonMessage.huddrawRect();
		break;
	case cgmessage_e::huddraw_virtualscreen:
		commonMessage.huddrawVirtualScreen();
		break;
	case cgmessage_e::huddraw_color:
		commonMessage.huddrawColor();
		break;
	case cgmessage_e::huddraw_alpha:
		commonMessage.huddrawAlpha();
		break;
	case cgmessage_e::huddraw_string:
		commonMessage.huddrawString();
		break;
	case cgmessage_e::huddraw_font:
		commonMessage.huddrawFont();
		break;
	case cgmessage_e::notify_hit:
		commonMessage.notifyHit();
		break;
	case cgmessage_e::notify_kill:
		commonMessage.notifyKill();
		break;
	case cgmessage_e::playsound_entity:
		commonMessage.playSoundEntity();
		break;
	case cgmessage_e::effect9:
	{
		vecStart = msgHelper.ReadVectorCoord();
		vecEnd = msgHelper.ReadVectorCoord();

		const uint8_t val1 = msg.ReadByte();
		const uint8_t val2 = msg.ReadByte();
		// FIXME: not sure what it does
	}
	default:
		break;

	}
}

effects_e CGameModule15::getEffectId(uint32_t effectId)
{
	static effects_e effectList[] =
	{
	effects_e::bh_paper_lite,
	effects_e::bh_paper_hard,
	effects_e::bh_wood_lite,
	effects_e::bh_wood_hard,
	effects_e::bh_metal_lite,
	effects_e::bh_metal_hard,
	effects_e::bh_stone_lite,
	effects_e::bh_stone_hard,
	effects_e::bh_dirt_lite,
	effects_e::bh_dirt_hard,
	// 10
	effects_e::bh_metal_lite,
	effects_e::bh_metal_hard,
	effects_e::bh_grass_lite,
	effects_e::bh_grass_hard,
	effects_e::bh_mud_lite,
	// 15
	effects_e::bh_mud_hard,
	effects_e::bh_water_lite,
	effects_e::bh_water_hard,
	effects_e::bh_glass_lite,
	effects_e::bh_glass_hard,
	// 20
	effects_e::bh_stone_lite,
	effects_e::bh_stone_hard,
	effects_e::bh_sand_lite,
	effects_e::bh_sand_hard,
	effects_e::bh_foliage_lite,
	// 25
	effects_e::bh_foliage_hard,
	effects_e::bh_snow_lite,
	effects_e::bh_snow_hard,
	effects_e::bh_carpet_lite,
	effects_e::bh_carpet_hard,
	// 30
	effects_e::bh_human_uniform_lite,
	effects_e::bh_human_uniform_hard,
	effects_e::water_trail_bubble,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 35
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 40
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 45
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 50
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 55
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 60
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::grenexp_base,
	effects_e::bazookaexp_base,
	// 65
	effects_e::heavyshellexp_base,
	effects_e::tankexp_base,
	effects_e::grenexp_paper,
	effects_e::grenexp_wood,
	effects_e::grenexp_metal,
	// 70
	effects_e::grenexp_stone,
	effects_e::grenexp_dirt,
	effects_e::grenexp_metal,
	effects_e::grenexp_grass,
	effects_e::grenexp_mud,
	// 75
	effects_e::grenexp_water,
	effects_e::bh_stone_hard,
	effects_e::grenexp_gravel,
	effects_e::grenexp_sand,
	effects_e::grenexp_foliage,
	// 80
	effects_e::grenexp_snow,
	effects_e::grenexp_carpet,
	effects_e::heavyshellexp_dirt,
	effects_e::heavyshellexp_stone,
	effects_e::heavyshellexp_snow,
	// 85
	effects_e::tankexp_dirt,
	effects_e::tankexp_stone,
	effects_e::tankexp_snow,
	effects_e::bazookaexp_dirt,
	effects_e::bazookaexp_stone,
	// 90
	effects_e::bazookaexp_snow,
	effects_e::water_ripple_still,
	effects_e::water_ripple_moving,
	effects_e::barrel_oil_leak_big,
	effects_e::barrel_oil_leak_medium,
	// 95
	effects_e::barrel_oil_leak_small,
	effects_e::barrel_oil_leak_splat,
	effects_e::barrel_water_leak_big,
	effects_e::barrel_water_leak_medium,
	effects_e::barrel_water_leak_small,
	// 100
	effects_e::barrel_water_leak_splat,
	effects_e::fs_light_dust,
	effects_e::fs_heavy_dust,
	effects_e::fs_dirt,
	effects_e::fs_grass,
	// 105
	effects_e::fs_mud,
	effects_e::fs_puddle,
	effects_e::fs_sand,
	effects_e::fs_snow,
	effects_e::fx_fence_wood,
	// 110
	effects_e::bh_stone_hard,
	};

	static constexpr size_t numEffects = sizeof(effectList) / sizeof(effectList[0]);
	static_assert(numEffects == 111);

	if (effectId < numEffects) {
		return effectList[effectId];
	}

	return effects_e::bh_stone_hard;
}

CommonMessageHandler::CommonMessageHandler(MSG& inMsg, const ModuleBase::HandlerList& inHandlerList)
	: msg(inMsg)
	, handlerList(inHandlerList)
{}

void CommonMessageHandler::impactMelee()
{
	MsgTypesHelper msgHelper(msg);

	const Vector vecStart = msgHelper.ReadVectorCoord();
	const Vector vecEnd = msgHelper.ReadVectorCoord();

	handlerList.meleeImpactHandler.broadcast(vecStart, vecEnd);
}

void CommonMessageHandler::debrisCrate()
{
	MsgTypesHelper msgHelper(msg);

	const Vector vecStart = msgHelper.ReadVectorCoord();
	const uint8_t numDebris = msg.ReadByte();

	handlerList.spawnDebrisHandler.broadcast(Handlers::debrisType_e::crate, vecStart, numDebris);
}

void CommonMessageHandler::debrisWindow()
{
	MsgTypesHelper msgHelper(msg);

	const Vector vecStart = msgHelper.ReadVectorCoord();
	const uint8_t numDebris = msg.ReadByte();

	handlerList.spawnDebrisHandler.broadcast(Handlers::debrisType_e::window, vecStart, numDebris);
}

void CommonMessageHandler::huddrawShader()
{
	const uint8_t index = msg.ReadByte();
	const StringMessage strVal = msg.ReadString();

	handlerList.huddrawShaderHandler.broadcast(index, (const char*)strVal);
}

void CommonMessageHandler::huddrawAlign()
{
	const uint8_t index = msg.ReadByte();
	const uint8_t hAlign = msg.ReadNumber<uint8_t>(2);
	const uint8_t vAlign = msg.ReadNumber<uint8_t>(2);

	using namespace Handlers;
	handlerList.huddrawAlignHandler.broadcast(index, horizontalAlign_e(hAlign), verticalAlign_e(vAlign));
}

void CommonMessageHandler::huddrawRect()
{
	const uint8_t index = msg.ReadByte();
	const uint16_t x = msg.ReadUShort();
	const uint16_t y = msg.ReadUShort();
	const uint16_t width = msg.ReadUShort();
	const uint16_t height = msg.ReadUShort();

	handlerList.huddrawRectHandler.broadcast(index, x, y, width, height);
}

void CommonMessageHandler::huddrawVirtualScreen()
{
	const uint8_t index = msg.ReadByte();
	const bool virtualScreen = msg.ReadBool();

	handlerList.huddrawVSHandler.broadcast(index, virtualScreen);
}

void CommonMessageHandler::huddrawColor()
{
	const uint8_t index = msg.ReadByte();
	const Vector col =
	{
		(float)msg.ReadByte() / 255.f,
		(float)msg.ReadByte() / 255.f,
		(float)msg.ReadByte() / 255.f
	};

	// Divide by 255 to get float color
	handlerList.huddrawColorHandler.broadcast(index, col);
}

void CommonMessageHandler::huddrawAlpha()
{
	const uint8_t index = msg.ReadByte();
	const float alpha = (float)msg.ReadByte() / 255.f;

	handlerList.huddrawAlphaHandler.broadcast(index, alpha);
}

void CommonMessageHandler::huddrawString()
{
	const uint8_t index = msg.ReadByte();
	const StringMessage strVal = msg.ReadString();

	handlerList.huddrawStringHandler.broadcast(index, strVal.c_str());
}

void CommonMessageHandler::huddrawFont()
{
	const uint8_t index = msg.ReadByte();
	const StringMessage strVal = msg.ReadString();

	handlerList.huddrawFontHandler.broadcast(index, strVal.c_str());
}

void CommonMessageHandler::notifyHit()
{
	handlerList.hitNotifyHandler.broadcast();
}

void CommonMessageHandler::notifyKill()
{
	handlerList.killNotifyHandler.broadcast();
}

void CommonMessageHandler::playSoundEntity()
{
	MsgTypesHelper msgHelper(msg);

	const Vector vecStart = msgHelper.ReadVectorCoord();

	const bool temp = msg.ReadBool();
	const uint8_t index = msg.ReadNumber<uint8_t>(6);
	const StringMessage strVal = msg.ReadString();

	handlerList.voiceMessageHandler.broadcast(vecStart, temp, index, strVal);
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

ProtocolClassInstancier_Template<CGameModule6, ModuleBase, 5, 8> cgameVersion8;
ProtocolClassInstancier_Template<CGameModule15, ModuleBase, 15, 17> cgameVersion17;
