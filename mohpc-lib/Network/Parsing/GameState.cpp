#include <MOHPC/Network/Parsing/GameState.h>
#include <MOHPC/Network/Types/GameState.h>
#include <MOHPC/Network/Parsing/String.h>
#include <MOHPC/Network/Parsing/Entity.h>
#include <MOHPC/Network/SerializableTypes.h>
#include <MOHPC/Common/Log.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Info.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::Parsing;

static constexpr char MOHPC_LOG_NAMESPACE[] = "parsing_gamestate";

gameStateClient_t::gameStateClient_t()
	: commandSequence(0)
	, clientNum(0)
	, checksumFeed(0)
{
}

gameStateResults_t::gameStateResults_t()
	: serverDeltaTime(0)
	, serverDeltaTimeSeconds(0.f)
{
}

class GameState_ver8 : public Parsing::IGameState
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 5;
		maxRange = 8;
	}

	gameState_t create() const
	{
		return gameState_t(MAX_CONFIGSTRINGS, 40000, 1024);
	}

	float getFrameTime(const ReadOnlyInfo& serverInfo) const
	{
		const uint32_t sv_fps = serverInfo.IntValueForKey("sv_fps");
		// calculate the server frame time
		return 1.f / sv_fps;
	}

	void parseGameState(MSG& msg, gameState_t& gameState, gameStateResults_t& results) const override
	{
		uint32_t minProto = 0, maxProto = 0;
		getProtocol(minProto, maxProto);

		const Parsing::IString* stringParser = Parsing::IString::get(maxProto);
		const Parsing::IEntity* entityParser = Parsing::IEntity::get(maxProto);
		const IConfigStringTranslator* translator = IConfigStringTranslator::get(maxProto);

		ConfigStringManager& csMan = gameState.getConfigstringManager();
		const size_t maxConfigstrings = csMan.getMaxConfigStrings();

		MOHPC_LOG(Debug, "Parsing gamestate");

		results.client.commandSequence = msg.ReadInteger();

		// reset the game state
		gameState.reset();
		for (;;)
		{
			const svc_ops_e cmd = msg.ReadByteEnum<svc_ops_e>();
			if (cmd == svc_ops_e::Eof) {
				break;
			}

			switch (cmd)
			{
			case svc_ops_e::Configstring:
			{
				const csNum_t stringNum = msg.ReadUShort();

				if (stringNum > maxConfigstrings) {
					throw ConfigstringErrors::MaxConfigStringException("gameStateParsing", stringNum);
				}

				const StringMessage stringValue = stringParser->readString(msg);

				// any version must pass the same CS number
				csMan.setConfigString(translator->translateFromNet(stringNum), stringValue.c_str());
			}
			break;
			case svc_ops_e::Baseline:
			{
				const entityNum_t newNum = entityParser->readEntityNum(msg);

				entityState_t& es = gameState.getEntityBaselines().getBaseline(newNum);

				entityParser->readDeltaEntity(
					msg,
					nullptr,
					&es,
					newNum,
					// no frametime yet
					0.f
				);
			}
			break;
			default:
				throw SerializableErrors::BadCommandByteException((uint8_t)cmd);
			}
		}

		results.client.clientNum = msg.ReadInteger();
		results.client.checksumFeed = msg.ReadInteger();

		ReadOnlyInfo systemInfo(csMan.getConfigString(CS_SYSTEMINFO));
		ReadOnlyInfo serverInfo(csMan.getConfigString(CS_SERVERINFO));

		// the server ID is inside system-info values
		results.serverId = systemInfo.IntValueForKey("sv_serverid");

		// calculate the server frame time
		results.serverDeltaTimeSeconds = getFrameTime(serverInfo);
		results.serverDeltaTime = (uint64_t)floorf(results.serverDeltaTimeSeconds * 1000.f);
	}

	void saveGameState(MSG& msg, gameState_t& gameState, const gameStateClient_t& client) const
	{
		uint32_t minProto = 0, maxProto = 0;
		getProtocol(minProto, maxProto);

		const Parsing::IString* stringParser = Parsing::IString::get(maxProto);
		const Parsing::IEntity* entityParser = Parsing::IEntity::get(maxProto);
		const IConfigStringTranslator* translator = IConfigStringTranslator::get(maxProto);

		ConfigStringManager& csMan = gameState.getConfigstringManager();
		BaseLines& baselines = gameState.getEntityBaselines();

		msg.WriteInteger(client.commandSequence);

		for (ConfigStringManager::iterator it(csMan); it; ++it)
		{
			// write the command number
			msg.WriteByteEnum<svc_ops_e>(svc_ops_e::Configstring);

			// write the cs number and its associated string
			msg.WriteUShort(translator->translateToNet(it.getNumber()));
			stringParser->writeString(msg, it.getConfigString());
		}

		for (baselineNum_t i = 0; i < baselines.getMaxBaselines(); ++i)
		{
			msg.WriteByteEnum<svc_ops_e>(svc_ops_e::Baseline);

			const entityState_t& ent = baselines.getBaseline(i);

			entityParser->writeEntityNum(msg, ent.number);

			entityParser->writeDeltaEntity(
				msg,
				nullptr,
				&ent,
				ent.number,
				// the client won't have the frametime
				// until all commands are processed
				0.f
			);
		}

		// tell the client that command parsing is finished
		msg.WriteByteEnum<svc_ops_e>(svc_ops_e::Eof);

		msg.WriteInteger(client.clientNum);
		msg.WriteInteger(client.checksumFeed);
	}
};

class GameState_ver17 : public GameState_ver8
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 15;
		maxRange = 17;
	}

	void parseGameState(MSG& msg, gameState_t& gameState, gameStateResults_t& results) const override
	{
		GameState_ver8::parseGameState(msg, gameState, results);

		// this is the frame time of the server set by **sv_fps**
		// it is practically useless because the delta frequency can be calculated with 1 / sv_fps
		const float receivedFrameTimeSec = msg.ReadFloat();
		if (receivedFrameTimeSec != results.serverDeltaTimeSeconds)
		{
			// the server frame time is 1 / sv_fps, it shouldn't be any other value
			MOHPC_LOG(Warn, "Server has sent a frame time that doesn't match sv_fps");
		}
	}

	void saveGameState(MSG& msg, gameState_t& gameState, const gameStateClient_t& client) const
	{
		GameState_ver8::saveGameState(msg, gameState, client);

		ReadOnlyInfo serverInfo(gameState.getConfigstringManager().getConfigString(CS_SERVERINFO));

		const float serverDeltaTimeSeconds = getFrameTime(serverInfo);

		msg.WriteFloat(serverDeltaTimeSeconds);
	}
};

static GameState_ver8 gameStateVersion8;
static GameState_ver17 gameStateVersion17;
