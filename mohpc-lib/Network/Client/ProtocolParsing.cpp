#include <MOHPC/Network/Client/ProtocolParsing.h>
#include <MOHPC/Network/InfoTypes.h>
#include <MOHPC/Network/SerializableTypes.h>
#include <MOHPC/Network/Client/ClientGame.h>
#include <MOHPC/Network/Types/GameState.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Info.h>
#include <MOHPC/Common/Log.h>

#include <morfuse/Common/str.h>

using namespace MOHPC;
using namespace Network;

#define MOHPC_LOG_NAMESPACE "gamestate"

ReadOnlyInfo getGameStateSystemInfo(const IGameState& gameState)
{
	return gameState.getConfigString(CS_SYSTEMINFO);
}

ReadOnlyInfo getGameStateServerInfo(const IGameState& gameState)
{
	return gameState.getConfigString(CS_SERVERINFO);
}

static void GetNullEntityState(entityState_t* nullState) {

	*nullState = entityState_t();
	nullState->alpha = 1.0f;
	nullState->scale = 1.0f;
	nullState->parent = ENTITYNUM_NONE;
	nullState->tag_num = -1;
	nullState->constantLight = -1;
	nullState->renderfx = 16;
	nullState->bone_tag[4] = -1;
	nullState->bone_tag[3] = -1;
	nullState->bone_tag[2] = -1;
	nullState->bone_tag[1] = -1;
	nullState->bone_tag[0] = -1;
}

struct gameStateData_t
{
public:
	static constexpr size_t MAX_GAMESTATE_CHARS = 40000;

public:
	gameStateData_t()
		: dataCount(1)
		, stringOffsets{0}
		, stringData{0}
	{
	}

public:
	size_t dataCount;
	size_t stringOffsets[MAX_CONFIGSTRINGS];
	// could have made stringData a dynamic buffer
	char stringData[MAX_GAMESTATE_CHARS];
};

IClientGameProtocol::~IClientGameProtocol()
{

}

IGameState::IGameState()
{
}

IGameState::~IGameState()
{
}

const char* IGameState::getConfigString(csNum_t num) const
{
	if (num > getNumConfigStrings()) {
		return nullptr;
	}

	return getConfigStringChecked(num);
}

void IGameState::systemInfoChanged()
{
	const ReadOnlyInfo serverSystemInfo = getConfigString(CS_SYSTEMINFO);
	const ReadOnlyInfo serverGameInfo = getConfigString(CS_SERVERINFO);

	serverId = serverSystemInfo.IntValueForKey("sv_serverid");
	const uint32_t sv_fps = serverGameInfo.IntValueForKey("sv_fps");
	// calculate the server frame time
	serverDeltaTimeSeconds = 1.f / sv_fps;
	serverDeltaTime = (uint64_t)floorf(serverDeltaTimeSeconds * 1000.f);
}

void IGameState::configStringModified(csNum_t num, const char* newString, bool notify)
{
	// set the config string in gamestate
	setConfigString(num, newString, str::len(newString));

	if (notify)
	{
		// propagate the change
		notifyConfigStringChange(num, newString);
	}
}

void IGameState::notifyConfigStringChange(csNum_t num, const char* newString)
{
	// notify about the change
	getHandlerList().configStringHandler.broadcast(num, newString);
}

void IGameState::notifyAllConfigStringChanges()
{
	const size_t maxConfigstrings = getNumConfigStrings();

	// notify about configstrings that are not empty
	for (csNum_t i = 0; i < maxConfigstrings; ++i)
	{
		const char* cs = getConfigStringChecked(i);
		if (*cs) {
			notifyConfigStringChange(i, cs);
		}
	}
}

IGameState::HandlerList& IGameState::getHandlerList()
{
	return handlerList;
}

bool IGameState::isDifferentServer(uint32_t id) const
{
	return id != serverId;
}

void IGameState::postParse(bool differentServer)
{
	getHandlerList().gameStateParsedHandler.broadcast(*this, differentServer);
}

IGameStateParser::IGameStateParser(const IClientGameProtocol& clientProtoValue, IGameState& gameStateValue)
	: clientProto(clientProtoValue)
	, gameState(gameStateValue)
	, commandSequence(0)
{
}

size_t IGameStateParser::getCommandSequence() const
{
	return commandSequence;
}

class GameStateParser_ver6 : public IGameStateParser
{
public:
	using IGameStateParser::IGameStateParser;

public:
	void Save(MSG& msg) const override
	{
	}

	void Load(MSG& msg) override
	{
		const size_t maxConfigstrings = gameState.getNumConfigStrings();

		MOHPC_LOG(Debug, "Received gamestate");

		MsgTypesHelper msgHelper(msg);

		commandSequence = msg.ReadInteger();

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

				const StringMessage stringValue = clientProto.readString(msg);

				// don't notify yet
				gameState.configStringModified(stringNum, stringValue, false);
			}
			break;
			case svc_ops_e::Baseline:
			{
				const entityNum_t newNum = clientProto.readEntityNum(msg);
				if (newNum >= MAX_GENTITIES) {
					throw GameStateErrors::BaselineOutOfRangeException(newNum);
				}

				entityState_t nullState;
				GetNullEntityState(&nullState);

				entityState_t& es = gameState.getBaseline(newNum);
				clientProto.readDeltaEntity(msg, gameState, &nullState, &es, newNum);
			}
			break;
			default:
				throw SerializableErrors::BadCommandByteException((uint8_t)cmd);
			}
		}

		gameState.clientNum = msg.ReadUInteger();
		gameState.checksumFeed = msg.ReadUInteger();

		// save the server id for later
		// it may be changed when parsing the system info
		const uint32_t oldServerId = gameState.serverId;

		gameState.systemInfoChanged();

		FinishParsing(msg);

		// now notify about all received config strings
		gameState.notifyAllConfigStringChanges();

		const bool isDiff = gameState.isDifferentServer(oldServerId);
		if (!isDiff) {
			MOHPC_LOG(Warn, "Server has resent gamestate while in-game");
		}

		// notify about the new game state
		gameState.postParse(isDiff);
	}

	virtual void FinishParsing(MSG& msg)
	{
		
	}
};

class GameStateParser_ver15 : public GameStateParser_ver6
{
public:
	using GameStateParser_ver6::GameStateParser_ver6;

	virtual void FinishParsing(MSG& msg)
	{
		// this is the frame time of the server set by **sv_fps**
		// it is practically useless because the delta frequency is already calculated earlier
		const float receivedFrameTimeSec = msg.ReadFloat();
		if (receivedFrameTimeSec != gameState.serverDeltaTimeSeconds)
		{
			// the server frame time is 1 / sv_fps, it shouldn't be any other value
			MOHPC_LOG(Warn, "Server has sent a frame time that doesn't match sv_fps");
		}
	}
};

class GameState_ver6 : public IGameState
{
public:
	using IGameState::IGameState;

	void reset() override
	{
		data.dataCount = 1;
	}

	const char* getConfigStringChecked(csNum_t num) const override
	{
		return &data.stringData[data.stringOffsets[normalizeConfigstring(num)]];
	}

	size_t getNumConfigStrings() const override
	{
		return MAX_CONFIGSTRINGS;
	}

	void setConfigString(csNum_t num, const char* configString, size_t sz) override
	{
		csNum_t normalizedNum = normalizeConfigstring(num);
		const size_t newSz = data.dataCount + sz;
		if (normalizedNum >= getNumConfigStrings()) {
			throw ConfigstringErrors::MaxConfigStringException("gameState_t::setConfigString", num);
		}

		// going to  
		// backup the game state
		gameStateData_t oldGs = data;

		// leave the first 0 for empty/uninitialized strings
		data.dataCount = 1;

		// shrink the game state
		for (size_t i = 0; i < MAX_CONFIGSTRINGS; i++)
		{
			const char* dup;

			if (i == normalizedNum) {
				dup = configString;
			}
			else {
				dup = oldGs.stringData + oldGs.stringOffsets[i];
			}

			if (!dup[0])
			{
				// leave with the default empty string
				continue;
			}

			const size_t newLen = str::len(dup);

			if (newLen + data.dataCount >= data.MAX_GAMESTATE_CHARS) {
				throw ConfigstringErrors::MaxGameStateCharsException(newSz);
			}

			// the string is correct, append it to the gameState string buffer
			data.stringOffsets[i] = data.dataCount;
			std::memcpy(data.stringData + data.dataCount, dup, newLen + 1);
			data.dataCount += newLen + 1;
		}
	}

	entityState_t& getBaseline(size_t num) override
	{
		return entityBaselines[num];
	}

protected:
	virtual csNum_t normalizeConfigstring(csNum_t num) const
	{
		if (num <= CS_WARMUP || num >= 26) {
			return num;
		}

		return num - 2;
	}

private:
	gameStateData_t data;
	entityState_t entityBaselines[MAX_GENTITIES];
};

class GameState_ver15 : public GameState_ver6
{
public:
	using GameState_ver6::GameState_ver6;

	virtual csNum_t normalizeConfigstring(csNum_t num) const
	{
		return num;
	}
};

class ClientGameProtocol_ver5 : public IClientGameProtocol
{
public:
	uint32_t getProtocolVersion() const override
	{
		return 5;
	}

	StringMessage readString(MSG& msg) const override
	{
		return msg.ReadString();
	}

	void writeString(MSG& msg, const char* s) const override
	{
		msg.WriteString(s);
	}

	uint32_t hashKey(const char* string, size_t maxlen) const override
	{
		uint32_t hash = 0;

		for (size_t i = 0; i < maxlen && string[i]; i++) {
			hash += string[i] * (119 + (uint32_t)i);
		}

		hash = (hash ^ (hash >> 10) ^ (hash >> 20));
		return hash;
	}

	entityNum_t readEntityNum(MSG& msg) const override
	{
		MsgTypesHelper msgHelper(msg);
		return msgHelper.ReadEntityNum();
	}

	void readDeltaPlayerstate(MSG& msg, const playerState_t* from, playerState_t* to) const override
	{
		SerializablePlayerState toSerialize(*to);
		if (from)
		{
			SerializablePlayerState fromSerialize(*const_cast<playerState_t*>(from));
			msg.ReadDeltaClass(&fromSerialize, &toSerialize);
		}
		else
		{
			// no delta
			msg.ReadDeltaClass(nullptr, &toSerialize);
		}
	}

	void readDeltaEntity(MSG& msg, const IGameState& gameState, const entityState_t* from, entityState_t* to, entityNum_t newNum) const override
	{
		SerializableEntityState toSerialize(*to, newNum);
		if (from)
		{
			SerializableEntityState fromSerialize(*const_cast<entityState_t*>(from), newNum);
			msg.ReadDeltaClass(&fromSerialize, &toSerialize);
		}
		else
		{
			// no delta
			msg.ReadDeltaClass(nullptr, &toSerialize);
		}
	}

	bool readNonPVSClient(radarInfo_t radarInfo, const ClientSnapshot& currentSnap, float radarRange, radarUnpacked_t& unpacked) const override
	{
		return false;
	}

	IGameState* createGameState() const override
	{
		return new GameState_ver6();
	}

	void readGameState(MSG& msg, IGameState& gameState) const override
	{
		// read the game state and parse it
		GameStateParser_ver6 parser(*this, gameState);
		msg.ReadClass(parser);
	}
};

class ClientGameProtocol_ver6 : public ClientGameProtocol_ver5
{
public:
	uint32_t getProtocolVersion() const override
	{
		return 6;
	}
};

class ClientGameProtocol_ver8 : public ClientGameProtocol_ver6
{
public:
	uint32_t getProtocolVersion() const override
	{
		return 8;
	}
};

class ClientGameProtocol_ver15 : public ClientGameProtocol_ver8
{
public:
	StringMessage readString(MSG& msg) const override
	{
		return StringMessage(); // msg.ReadScrambledString(byteCharMapping);
	}

	void writeString(MSG& msg, const char* s) const override
	{
		//msg.WriteScrambledString(s, charByteMapping);
	}

	uint32_t hashKey(const char* string, size_t maxlen) const override
	{
		uint32_t hash = 0;

		for (size_t i = 0; i < maxlen && string[i]; i++) {
			hash += string[i] * (119 + (uint32_t)i);
		}

		hash = (hash ^ ((hash ^ (hash >> 10)) >> 10));
		return hash;
	}

	entityNum_t readEntityNum(MSG& msg) const override
	{
		MsgTypesHelper msgHelper(msg);
		return msgHelper.ReadEntityNum2();
	}

	void readDeltaPlayerstate(MSG& msg, const playerState_t* from, playerState_t* to) const override
	{
		SerializablePlayerState_ver15 toSerialize(*to);
		if (from)
		{
			SerializablePlayerState_ver15 fromSerialize(*const_cast<playerState_t*>(from));
			msg.ReadDeltaClass(&fromSerialize, &toSerialize);
		}
		else
		{
			// no delta
			msg.ReadDeltaClass(nullptr, &toSerialize);
		}
	}

	void readDeltaEntity(MSG& msg, const IGameState& gameState, const entityState_t* from, entityState_t* to, entityNum_t newNum) const override
	{
		SerializableEntityState_ver15 toSerialize(*to, newNum, gameState.serverDeltaTimeSeconds);
		if (from)
		{
			SerializableEntityState_ver15 fromSerialize(*const_cast<entityState_t*>(from), newNum, gameState.serverDeltaTimeSeconds);
			msg.ReadDeltaClass(&fromSerialize, &toSerialize);
		}
		else
		{
			// no delta
			msg.ReadDeltaClass(nullptr, &toSerialize);
		}
	}

	bool readNonPVSClient(radarInfo_t radarInfo, const ClientSnapshot& currentSnap, float radarRange, radarUnpacked_t& unpacked) const override
	{
		if (unpackNonPVSClient(radarInfo, unpacked, currentSnap, radarRange))
		{
			const Vector& origin = currentSnap.ps.getOrigin();
			unpacked.x += origin[0];
			unpacked.y += origin[1];

			return true;
		}

		return false;
	}

	bool unpackNonPVSClient(radarInfo_t radarInfo, radarUnpacked_t& unpacked, const ClientSnapshot& currentSnap, float radarRange) const
	{
		unpacked.clientNum = radarInfo.clientNum();

		if (unpacked.clientNum == currentSnap.ps.getClientNum()) {
			return false;
		}

		const float radarScaled = radarRange / 63.f;

		unpacked.x = radarInfo.x() * radarScaled;
		unpacked.y = radarInfo.y() * radarScaled;

		if (radarInfo.flags() & RADAR_PLAYER_FAR)
		{
			// when it's too far it needs to be scaled to make it look very far away
			unpacked.x = unpacked.x * 1024.f;
			unpacked.y = unpacked.y * 1024.f;
		}

		// retrieve the yaw from 5-bits value
		unpacked.yaw = radarInfo.yaw() * (360.f / 32.f);

		return true;
	}

	IGameState* createGameState() const override
	{
		return new GameState_ver15();
	}

	uint32_t getProtocolVersion() const override
	{
		return 15;
	}
};

class ClientGameProtocol_ver16 : public ClientGameProtocol_ver15
{
public:
	uint32_t getProtocolVersion() const override
	{
		return 16;
	}
};

class ClientGameProtocol_ver17 : public ClientGameProtocol_ver15
{
public:
	uint32_t getProtocolVersion() const override
	{
		return 17;
	}

	void readGameState(MSG& msg, IGameState& gameState) const
	{
		// read the game state and parse it
		GameStateParser_ver15 parser(*this, gameState);
		msg.ReadClass(parser);
	}
};

static ClientGameProtocol_ver5 protocolVersion5;
static ClientGameProtocol_ver6 protocolVersion6;
static ClientGameProtocol_ver8 protocolVersion8;
static ClientGameProtocol_ver15 protocolVersion15;
static ClientGameProtocol_ver16 protocolVersion16;
static ClientGameProtocol_ver17 protocolVersion17;

template<typename T, uint32_t minProtocolVersion, uint32_t maxProtocolVersion>
class GameStateInstancier_Template : public ProtocolClassInstancier_Template<T, IGameState, minProtocolVersion, maxProtocolVersion> {};

GameStateInstancier_Template<GameState_ver6, 5, 8> gameStateInstancier8;
GameStateInstancier_Template<GameState_ver15, 15, 17> gameStateInstancier17;
