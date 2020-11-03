#include <MOHPC/Network/Client/ClientGame.h>
#include <MOHPC/Network/Client/CGModule.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Network/Channel.h>
#include <MOHPC/Network/SerializableTypes.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Log.h>
#include <typeinfo>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cstring>

using namespace MOHPC;
using namespace MOHPC::Network;

static constexpr size_t MAX_MSGLEN = 49152;
static constexpr size_t MINIMUM_RECEIVE_BUFFER_SIZE = 1024;
static constexpr char* MOHPC_LOG_NAMESPACE = "network_cgame";

constexpr uint8_t charByteMapping[256] =
{
	254, 120, 89, 13, 27, 73, 103, 78, 74, 102, 21, 117, 76, 86, 238, 96, 88, 62, 59, 60,
	40, 84, 52, 119, 251, 51, 75, 121, 192, 85, 44, 54, 114, 87, 25, 53, 35, 224, 67, 31,
	82, 41, 45, 99, 233, 112, 255, 11, 46, 115, 8, 32, 19, 100, 110, 95, 116, 48, 58, 107,
	70, 91, 104, 81, 118, 109, 36, 24, 17, 39, 43, 65, 49, 83, 56, 57, 33, 64, 80, 28,
	184, 160, 18, 105, 42, 20, 194, 38, 29, 26, 61, 50, 9, 90, 37, 128, 79, 2, 108, 34,
	4, 0, 47, 12, 101, 10, 92, 15, 5, 7, 22, 55, 23, 14, 3, 1, 66, 16, 63, 30,
	6, 97, 111, 248, 72, 197, 191, 122, 176, 245, 250, 68, 195, 77, 232, 106, 228, 93, 240, 98,
	208, 69, 164, 144, 186, 222, 94, 246, 148, 170, 244, 190, 205, 234, 252, 202, 230, 239, 174, 225,
	226, 209, 236, 216, 237, 151, 149, 231, 129, 188, 200, 172, 204, 154, 168, 71, 133, 217, 196, 223,
	134, 253, 173, 177, 219, 235, 214, 182, 132, 227, 183, 175, 137, 152, 158, 221, 243, 150, 210, 136,
	167, 211, 179, 193, 218, 124, 140, 178, 213, 249, 185, 113, 127, 220, 180, 145, 138, 198, 123, 162,
	189, 203, 166, 126, 159, 156, 212, 207, 146, 181, 247, 139, 142, 169, 242, 241, 171, 187, 153, 135,
	201, 155, 161, 125, 163, 130, 229, 206, 165, 157, 141, 147, 143, 199, 215, 131
};

namespace MOHPC
{
	class byteCharMapping_c
	{
	private:
		char mapping[256];

	public:
		constexpr byteCharMapping_c()
			: mapping{ 0 }
		{
			for (uint16_t i = 0; i < 256; ++i)
			{
				for (uint16_t j = 0; j < 256; ++j)
				{
					if (charByteMapping[j] == i)
					{
						mapping[i] = (char)j;
						break;
					}
				}
			}
		}

		constexpr operator const char* () const { return mapping; }
	};
	byteCharMapping_c byteCharMapping;
}

/*
const char byteCharMapping[256] =
{
	101, 115, 97, 114, 100, 108, 120, 109, 50, 92, 105, 47, 103, 3, 113, 107, 117, 68, 82, 52,
	85, 10, 110, 112, 67, 34, 89, 4, 79, 88, 119, 39, 51, 76, 99, 36, 66, 94, 87, 69,
	20, 41, 84, 70, 30, 42, 48, 102, 57, 72, 91, 25, 22, 35, 31, 111, 74, 75, 58, 18,
	19, 90, 17, 118, 77, 71, 116, 38, 131, 141, 60, 175, 124, 5, 8, 26, 12, 133, 7, 96,
	78, 63, 40, 73, 21, 29, 13, 33, 16, 2, 93, 61, 106, 137, 146, 55, 15, 121, 139, 43,
	53, 104, 9, 6, 62, 83, 135, 59, 98, 65, 54, 122, 45, 211, 32, 49, 56, 11, 64, 23,
	1, 27, 127, 218, 205, 243, 223, 212, 95, 168, 245, 255, 188, 176, 180, 239, 199, 192, 216, 231,
	206, 250, 232, 252, 143, 215, 228, 251, 148, 166, 197, 165, 193, 238, 173, 241, 225, 249, 194, 224,
	81, 242, 219, 244, 142, 248, 222, 200, 174, 233, 149, 236, 171, 182, 158, 191, 128, 183, 207, 202,
	214, 229, 187, 190, 80, 210, 144, 237, 169, 220, 151, 126, 28, 203, 86, 132, 178, 125, 217, 253,
	170, 240, 155, 221, 172, 152, 247, 227, 140, 161, 198, 201, 226, 208, 186, 254, 163, 177, 204, 184,
	213, 195, 145, 179, 37, 159, 160, 189, 136, 246, 156, 167, 134, 44, 153, 185, 162, 164, 14, 157,
	138, 235, 234, 196, 150, 129, 147, 230, 123, 209, 130, 24, 154, 181, 0, 46
};
*/

outPacket_t::outPacket_t()
	: p_cmdNumber(0)
	, p_serverTime(0)
	, p_currentTime(0)
{

}

gameState_t::gameState_t()
	: stringOffsets{ 0 }
	, stringData{ 0 }
	, dataCount(1)
{}

const char* gameState_t::getConfigString(csNum_t num) const
{
	if (num > MAX_CONFIGSTRINGS) {
		return nullptr;
	}

	return &stringData[stringOffsets[num]];
}

const char* gameState_t::getConfigStringChecked(csNum_t num) const
{
	return &stringData[stringOffsets[num]];
}

void gameState_t::setConfigString(csNum_t num, const char* configString, size_t len)
{
	const size_t newSz = dataCount + len;
	if (num > MAX_CONFIGSTRINGS) {
		throw ClientError::MaxConfigStringException("gameState_t::setConfigString", num);
	}

	// going to  
	// backup the game state
	gameState_t oldGs = *this;

	// leave the first 0 for empty/uninitialized strings
	dataCount = 1;

	// shrink the game state
	for (size_t i = 0; i < MAX_CONFIGSTRINGS; i++)
	{
		const char* dup;

		if (i == num) {
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

		len = strlen(dup);

		if (len + dataCount >= MAX_GAMESTATE_CHARS) {
			throw ClientError::MaxGameStateCharsException(newSz);
		}

		// the string is correct, append it to the gameState string buffer
		stringOffsets[i] = dataCount;
		std::memcpy(stringData + dataCount, dup, len + 1);
		dataCount += len + 1;
	}
}

ClientSnapshot::ClientSnapshot()
	: valid(false)
	, snapFlags(0)
	, serverTime(0)
	, serverTimeResidual(0)
	, messageNum(0)
	, deltaNum(0)
	, areamask{ 0 }
	, cmdNum(0)
	, numEntities(0)
	, parseEntitiesNum(0)
	, serverCommandNum(0)
	, numSounds(0)
{
}

MOHPC_OBJECT_DEFINITION(ClientGameConnection);

ClientGameConnection::ClientGameConnection(const NetworkManagerPtr& inNetworkManager, const INetchanPtr& inNetchan, const NetAddrPtr& inAdr, uint32_t challengeResponse, const protocolType_c& protoType, const ClientInfoPtr& cInfo)
	: ITickableNetwork(inNetworkManager)
	, netchan(inNetchan)	
	, adr(inAdr)
	, realTimeStart(0)
	, serverStartTime(0)
	, serverTime(0)
	, oldServerTime(0)
	, oldFrameServerTime(0)
	, lastPacketSendTime(0)
	, timeoutTime(60000)
	, parseEntitiesNum(0)
	, serverCommandSequence(0)
	, cmdNumber(0)
	, clientNum(0)
	, checksumFeed(0)
	, serverId(0)
	, reliableSequence(0)
	, reliableAcknowledge(0)
	, lastSnapFlags(0)
	, newSnapshots(false)
	, extrapolatedSnapshot(false)
	, isActive(false)
	, isReady(false)
	, reliableCommands{ nullptr }
	, serverCommands{ nullptr }
	, reliableCmdStrings{ 0 }
	, serverCmdStrings{ 0 }
	, userInfo(cInfo)
{
	ClientImports imports;
	fillClientImports(imports);

	downloadState.setImports(imports);

	switch (protoType.getProtocolVersion())
	{
	case protocolVersion_e::ver005:
	case protocolVersion_e::ver100:
	case protocolVersion_e::ver111:
		readStringMessage_pf = &ClientGameConnection::readStringMessage_normal;
		writeStringMessage_pf = &ClientGameConnection::writeStringMessage_normal;
		parseGameState_pf = &ClientGameConnection::parseGameState_ver6;
		hashKey_pf = &ClientGameConnection::hashKey_ver6;
		readEntityNum_pf = &ClientGameConnection::readEntityNum_ver6;
		readDeltaPlayerstate_pf = &ClientGameConnection::readDeltaPlayerstate_ver6;
		readDeltaEntity_pf = &ClientGameConnection::readDeltaEntity_ver6;
		getNormalizedConfigstring_pf = &ClientGameConnection::getNormalizedConfigstring_ver6;
		getMaxCommandSize_pf = &ClientGameConnection::getMaxCommandSize_ver6;
		readNonPVSClient_pf = &ClientGameConnection::readNonPVSClient_ver6;

		cgameModule = new CGameModule6(imports);
		break;
	case protocolVersion_e::ver200:
	case protocolVersion_e::ver211_demo:
	case protocolVersion_e::ver211:
		readStringMessage_pf = &ClientGameConnection::readStringMessage_scrambled;
		writeStringMessage_pf = &ClientGameConnection::writeStringMessage_scrambled;
		parseGameState_pf = &ClientGameConnection::parseGameState_ver15;
		hashKey_pf = &ClientGameConnection::hashKey_ver15;
		readEntityNum_pf = &ClientGameConnection::readEntityNum_ver15;
		readDeltaPlayerstate_pf = &ClientGameConnection::readDeltaPlayerstate_ver15;
		readDeltaEntity_pf = &ClientGameConnection::readDeltaEntity_ver15;
		getNormalizedConfigstring_pf = &ClientGameConnection::getNormalizedConfigstring_ver15;
		getMaxCommandSize_pf = &ClientGameConnection::getMaxCommandSize_ver15;
		readNonPVSClient_pf = &ClientGameConnection::readNonPVSClient_ver15;

		cgameModule = new CGameModule15(imports);
		break;
	default:
		throw ClientError::BadProtocolVersionException((uint8_t)protoType.getProtocolVersion());
		break;
	}

	for (size_t i = 0; i < MAX_RELIABLE_COMMANDS; ++i)
	{
		reliableCommands[i] = &reliableCmdStrings[i * MAX_STRING_CHARS];
		serverCommands[i] = &serverCmdStrings[i * MAX_STRING_CHARS];
	}

	encoder = std::make_shared<Encoding>(challengeResponse, (const char**)reliableCommands, (const char**)serverCommands);
}

ClientGameConnection::~ClientGameConnection()
{
	disconnect();
}

ClientGameConnection::HandlerListClient& ClientGameConnection::getHandlerList()
{
	return handlerList;
}

void ClientGameConnection::tick(uint64_t deltaTime, uint64_t currentTime)
{
	if (!isChannelValid()) {
		return;
	}

	using namespace std::chrono;
	// Check if there is a timeout time
	if (timeoutTime > milliseconds::zero())
	{
		steady_clock::time_point clockTime = steady_clock::now();
		steady_clock::time_point nextTimeoutTime = lastTimeoutTime + timeoutTime;
		// Check if the clock has reached the timeout point
		if (clockTime >= nextTimeoutTime)
		{
			// The server or the client has timed out
			handlerList.timeoutHandler.broadcast();

			// Disconnect from server
			serverDisconnected(nullptr);
			return;
		}
	}

	size_t count = 0;

	IUdpSocket* socket = getNetchan()->getRawSocket();
	// Loop until there is no valid data
	// or the max number of processed packets has reached the limit
	while(isChannelValid() && socket->dataAvailable() && count++ < settings.getMaxTickPackets())
	{
		DynamicDataMessageStream stream;
		// reserve a minimum amount to avoid reallocating each time
		stream.reserve(MINIMUM_RECEIVE_BUFFER_SIZE);

		uint32_t sequenceNum;
		NetAddrPtr from;

		if(getNetchan()->receive(from, stream, sequenceNum))
		{
			// Prepare for reading
			MSG msg(stream, msgMode_e::Reading);
			if(sequenceNum != -1)
			{
				// received connection packet
				receive(from, msg, currentTime, sequenceNum);
			}
			else
			{
				// can only happen when disconnected
				receiveConnectionLess(from, msg);
			}
		}
	}

	// Build and send client commands
	if(sendCmd(currentTime))
	{
		// Send packets if there are new commands
		writePacket(currentTime);
	}

	setCGameTime(currentTime);

	try
	{
		if (isActive && cgameModule) {
			cgameModule->tick(deltaTime, currentTime, serverTime);
		}
	}
	catch (NetworkException& e)
	{
		// call the handler
		handlerList.errorHandler.broadcast(e);

		MOHPC_LOG(Error, "ClientGameModule exception: %s", e.what().c_str());
	}
}

void ClientGameConnection::setTimeout(size_t inTimeoutTime)
{
	using namespace std::chrono;
	timeoutTime = milliseconds(inTimeoutTime);
}

const INetchanPtr& ClientGameConnection::getNetchan() const
{
	return netchan;
}

void ClientGameConnection::receive(const NetAddrPtr& from, MSG& msg, uint64_t currentTime, uint32_t sequenceNum)
{
	serverMessageSequence = (uint32_t)sequenceNum;

	msg.SetCodec(MessageCodecs::Bit);

	// Read the ack
	reliableAcknowledge = msg.ReadUInteger();
	if (reliableAcknowledge + MAX_RELIABLE_COMMANDS < reliableSequence) {
		reliableAcknowledge = reliableSequence;
	}

	static constexpr size_t decodeStart = sizeof(uint32_t) + sizeof(uint32_t);

	// decode the stream itself
	IMessageStream& stream = msg.stream();
	stream.Seek(decodeStart, IMessageStream::SeekPos::Begin);
	encoder->setReliableAcknowledge(reliableAcknowledge);
	encoder->setSecretKey(serverMessageSequence);
	encoder->decode(stream, stream);
	// seek after sequence number
	stream.Seek(sizeof(uint32_t));

	// Needs to be reset as the stream has been decoded
	msg.Reset();
	// Serialize again to read the proper number of bits
	msg.ReadInteger();

	using namespace std::chrono;
	lastTimeoutTime = steady_clock::now();

	try
	{
		// All messages will be parsed from there
		parseServerMessage(msg, currentTime);
	}
	catch (NetworkException& e)
	{
		// call the handler
		handlerList.errorHandler.broadcast(e);

		MOHPC_LOG(Error, "Network error: %s", e.what().c_str());
	}
	catch (StreamMessageException&)
	{
		MOHPC_LOG(Error, "Tried to read past end of server message (length %d)", stream.GetLength());
	}
}

void ClientGameConnection::receiveConnectionLess(const NetAddrPtr& from, MSG& msg)
{
	msg.SetCodec(MessageCodecs::OOB);

	// a message without sequence number indicates a connectionless packet
	// the only possible connectionless packet should be the disconnect command
	// it is received when the client gets kicked out during map loading

	// read the direction
	uint8_t direction = msg.ReadByte();

	// read the command
	StringMessage data = msg.ReadString();

	// parse command arguments
	TokenParser parser;
	parser.Parse(data, strlen(data));

	// get the connectionless command
	const char* cmd = parser.GetToken(true);

	if (!str::icmp(cmd, "disconnect"))
	{
		// disconnect message, may happen during map loading
		serverDisconnected(nullptr);
	}
	else if (!str::icmp(cmd, "droperror"))
	{
		// server is kicking the client due to an error

		// trim leading spaces/crlf
		parser.SkipWhiteSpace(true);
		// get the reason of the kick
		const char* reason = parser.GetCurrentScript();

		// Supply the reason when disconnecting
		serverDisconnected(reason);
	}
}

void ClientGameConnection::addReliableCommand(const char* cmd)
{
	if (reliableAcknowledge + MAX_RELIABLE_COMMANDS < reliableSequence)
	{
		// FIXME: throw?
		return;
	}

	++reliableSequence;
	const size_t index = reliableSequence & (MAX_RELIABLE_COMMANDS - 1);
	const size_t maxSize = getMaxCommandSize();

	//reliableCommands[index] = &reliableCmdStrings[MAX_STRING_CHARS * index];
	strncpy(reliableCommands[index], cmd, sizeof(reliableCmdStrings[maxSize * index]) * maxSize);
}

void ClientGameConnection::parseServerMessage(MSG& msg, uint64_t currentTime)
{
	while(cgameModule)
	{
		const svc_ops_e cmd = msg.ReadByteEnum<svc_ops_e>();
		if (cmd == svc_ops_e::Eof) {
			break;
		}

		switch (cmd)
		{
		case svc_ops_e::Nop:
			break;
		case svc_ops_e::ServerCommand:
			parseCommandString(msg);
			break;
		case svc_ops_e::Gamestate:
			parseGameState(msg);
			break;
		case svc_ops_e::Snapshot:
			parseSnapshot(msg, currentTime);
			break;
		case svc_ops_e::Download:
			parseDownload(msg);
			break;
		case svc_ops_e::Centerprint:
			parseCenterprint(msg);
			break;
		case svc_ops_e::Locprint:
			parseLocprint(msg);
			break;
		case svc_ops_e::CGameMessage:
			parseCGMessage(msg);
			break;
		default:
			throw ClientError::IllegibleServerMessageException((uint8_t)cmd);
		}
	}
}

void GetNullEntityState(entityState_t* nullState) {

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

void ClientGameConnection::parseGameState(MSG& msg)
{
	MOHPC_LOG(Verbose, "Received gamestate");

	MsgTypesHelper msgHelper(msg);

	serverCommandSequence = msg.ReadInteger();

	gameState.dataCount = 1;
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

			if (stringNum > MAX_CONFIGSTRINGS) {
				throw ClientError::MaxConfigStringException("gameStateParsing", stringNum);
			}

			const StringMessage stringValue = readStringMessage(msg);

			// don't notify yet
			configStringModified(getNormalizedConfigstring(stringNum), stringValue, false);
		}
		break;
		case svc_ops_e::Baseline:
		{
			const entityNum_t newNum = readEntityNum(msgHelper);
			if (newNum >= MAX_GENTITIES) {
				throw ClientError::BaselineOutOfRangeException(newNum);
			}

			entityState_t nullState;
			GetNullEntityState(&nullState);

			entityState_t& es = entityBaselines[newNum];
			readDeltaEntity(msg, &nullState, &es, newNum);
		}
		break;
		default:
			throw ClientError::BadCommandByteException((uint8_t)cmd);
		}
	}

	clientNum = msg.ReadUInteger();
	checksumFeed = msg.ReadUInteger();

	(this->*parseGameState_pf)(msg);
	
	// save the server id for later
	// it may be changed when parsing the system info
	const uint32_t oldServerId = serverId;

	// now notify about all received config strings
	notifyAllConfigStringChanges();

	const bool isDiff = isDifferentServer(oldServerId);
	if (isDiff)
	{
		// new map/server, reset important data
		clearState();
	} else {
		MOHPC_LOG(Warning, "Server has resent gamestate while in-game");
	}

	// notify about the new game state
	getHandlerList().gameStateParsedHandler.broadcast(getGameState(), isDiff);
}

void ClientGameConnection::parseSnapshot(MSG& msg, uint64_t currentTime)
{
	ClientSnapshot newSnap;
	newSnap.serverCommandNum = serverCommandSequence;

	newSnap.serverTime = msg.ReadUInteger();
	newSnap.serverTimeResidual = msg.ReadByte();

	// Insert the sequence num
	newSnap.messageNum = serverMessageSequence;

	// get the old frame of the snap
	const ClientSnapshot* old = readOldSnapshot(msg, newSnap);

	newSnap.snapFlags = msg.ReadByte();

	readAreaMask(msg, newSnap);

	// Read player state
	const playerState_t* oldps = old ? &old->ps : nullptr;
	readDeltaPlayerstate(msg, oldps, &newSnap.ps);
	handlerList.playerStateReadHandler.broadcast(const_cast<const playerState_t*>(oldps), const_cast<const playerState_t*>(&newSnap.ps));

	// Read all entities in this snap
	parsePacketEntities(msg, old, &newSnap);

	// Parse sounds
	parseSounds(msg, &newSnap);

	if (!newSnap.valid) {
		return;
	}

	if (currentSnap.valid && ((currentSnap.snapFlags ^ newSnap.snapFlags) & SNAPFLAG_SERVERCOUNT))
	{
		serverStartTime = newSnap.serverTime;
		realTimeStart = currentTime;
	}

	// set the new snap and calculate the ping
	setNewSnap(newSnap);
	calculatePing(currentTime);

	// read and unpack radar info on SH/BT
	readNonPVSClient(currentSnap.ps.getRadarInfo());

	getHandlerList().snapshotReceivedHandler.broadcast(currentSnap);
}

void ClientGameConnection::parsePacketEntities(MSG& msg, const ClientSnapshot* oldFrame, ClientSnapshot* newFrame)
{
	newFrame->parseEntitiesNum = parseEntitiesNum;
	newFrame->numEntities = 0;

	// delta from the entities present in oldframe
	entityState_t* oldState = NULL;
	uint32_t oldIndex = 0;
	uint32_t oldNum = 0;
	if (!oldFrame) {
		oldNum = 99999;
	}
	else
	{
		oldState = &parseEntities[oldFrame->parseEntitiesNum & (MAX_PARSE_ENTITIES - 1)];
		oldNum = oldState->number;
	}

	MsgTypesHelper msgHelper(msg);
	for (;;)
	{
		const uint16_t newNum = readEntityNum(msgHelper);

		if (newNum == ENTITYNUM_NONE) {
			break;
		}

		while (oldNum < newNum)
		{
			parseDeltaEntity(msg, newFrame, oldNum, oldState, true);

			++oldIndex;

			if (oldIndex >= oldFrame->numEntities) {
				oldNum = 99999;
			}
			else {
				oldState = &parseEntities[(oldFrame->parseEntitiesNum + oldIndex) & (MAX_PARSE_ENTITIES - 1)];
				oldNum = oldState->number;
			}
		}

		if (oldNum == newNum)
		{
			// delta from previous state
			parseDeltaEntity(msg, newFrame, newNum, oldState, false);

			++oldIndex;

			if (oldIndex >= oldFrame->numEntities) {
				oldNum = 99999;
			}
			else {
				oldState = &parseEntities[(oldFrame->parseEntitiesNum + oldIndex) & (MAX_PARSE_ENTITIES - 1)];
				oldNum = oldState->number;
			}
			continue;
		}

		if (oldNum > newNum)
		{
			// delta from baseline
			parseDeltaEntity(msg, newFrame, newNum, &entityBaselines[newNum], false);
			continue;
		}
	}

	// any remaining entities in the old frame are copied over
	while (oldNum != 99999)
	{
		// one or more entities from the old packet are unchanged
		parseDeltaEntity(msg, newFrame, oldNum, oldState, true);

		++oldIndex;

		if (oldIndex >= oldFrame->numEntities) {
			oldNum = 99999;
		}
		else
		{
			oldState = &parseEntities[(oldFrame->parseEntitiesNum + oldIndex) & (MAX_PARSE_ENTITIES - 1)];
			oldNum = oldState->number;
		}
	}
}

void ClientGameConnection::parseDeltaEntity(MSG& msg, ClientSnapshot* frame, uint32_t newNum, entityState_t* old, bool unchanged)
{
	entityState_t* state = &parseEntities[parseEntitiesNum & (MAX_PARSE_ENTITIES - 1)];

	if (unchanged) {
		*state = *old;
	}
	else {
		readDeltaEntity(msg, old, state, newNum);
	}

	if (state->number == ENTITYNUM_NONE)
	{
		// entity was delta removed
		return;
	}

	++parseEntitiesNum;
	frame->numEntities++;
}

void ClientGameConnection::parseSounds(MSG& msg, ClientSnapshot* newFrame)
{
	const bool hasSounds = msg.ReadBool();
	if (!hasSounds) {
		return;
	}

	const uint8_t numSounds = msg.ReadNumber<uint8_t>(7);
	if (numSounds > MAX_SERVER_SOUNDS) {
		return;
	}

	newFrame->numSounds = numSounds;

	MsgTypesHelper msgHelper(msg);

	for (size_t i = 0; i < numSounds; ++i)
	{
		sound_t& sound = newFrame->sounds[i];
		sound.hasStopped = msg.ReadBool();

		if (sound.hasStopped)
		{
			const uint16_t entityNum = readEntityNum(msgHelper);
			sound.entity = &entityBaselines[entityNum];

			const uint8_t channel = msg.ReadNumber<uint8_t>(7);
			sound.channel = channel;
		}
		else
		{
			sound.isStreamed = msg.ReadBool();
			sound.isSpatialized = msg.ReadBool();

			if (sound.isSpatialized) {
				sound.origin = msgHelper.ReadVectorFloat();
			}

			const uint16_t entityNum = msg.ReadNumber<uint16_t>(11);
			sound.entity = &entityBaselines[entityNum];

			if (entityNum >= MAX_GENTITIES) {
				throw BadEntityNumberException("sound", entityNum);
			}

			const uint8_t channel = msg.ReadNumber<uint8_t>(7);
			sound.channel = channel;

			const uint16_t soundIndex = msg.ReadNumber<uint16_t>(9);

			if (soundIndex < MAX_SOUNDS)
			{
				// Get the sound name from configstrings
				sound.soundName = gameState.getConfigString(soundIndex);
			}
			else {
				// FIXME: Throw?
			}

			sound.hasVolume = msg.ReadBool();
			if (sound.hasVolume) {
				sound.volume = msg.ReadFloat();
			}
			else {
				sound.volume = -1.f;
			}

			sound.hasDist = msg.ReadBool();
			if (sound.hasDist) {
				sound.minDist = msg.ReadFloat();
			}
			else {
				sound.minDist = -1.f;
			}

			sound.hasPitch = msg.ReadBool();
			if (sound.hasPitch)
			{
				sound.pitch = msg.ReadFloat();
			}
			else {
				sound.pitch = -1.f;
			}

			sound.maxDist = msg.ReadFloat();
		}

		handlerList.soundHandler.broadcast(sound);
	}
}

DownloadManager::DownloadManager()
	: downloadSize(0)
	, downloadBlock(0)
	, downloadRequested(false)
{}

void DownloadManager::setImports(ClientImports& inImports) noexcept
{
	imports = inImports;
}

void DownloadManager::processDownload(MSG& msg)
{
	if (!downloadRequested)
	{
		// not requested
		imports.addReliableCommand("stopdl");
		throw UnexpectedDownloadException();
	}

	uint8_t data[MAX_MSGLEN];

	const uint16_t block = msg.ReadUShort();
	if (!block)
	{
		// block zero = file size
		uint32_t fileSize = msg.ReadInteger();

		if (!fileSize || fileSize == -1)
		{
			// not a valid file size
			throw ClientError::DownloadException(imports.readStringMessage(msg));
		}

		downloadBlock = 0;

		MOHPC_LOG(Verbose, "downloading file of size %d", fileSize);
	}

	const uint16_t size = msg.ReadUShort();

	if (size > sizeof(data))
	{
		// invalid size
		throw ClientError::DownloadSizeException(size);
	}

	if (downloadBlock != block)
	{
		// unexpected block
		throw ClientError::BadDownloadBlockException(block, downloadBlock);
	}

	if (size > 0)
	{
		msg.ReadData(data, size);

		if (!receive(data, size))
		{
			clearDownload();
			imports.addReliableCommand("stopdl");
			return;
		}

		//std::ofstream strm("dwnl.tmp", std::ofstream::binary | std::ofstream::app);
		// append data
		//strm.write((const char*)data, size);

		MOHPC_LOG(Verbose, "downloaded block %d size %d", block, size);

		// tell the server to continue downloading
		imports.addReliableCommand(str::printf("nextdl %d", downloadBlock));

		downloadSize += size;
	}
	else
	{
		// a size of 0 means EOF
		clearDownload();

		downloadsComplete();
	}

	downloadBlock++;
}

bool DownloadManager::receive(const uint8_t* data, const size_t size)
{
	if (receiveCallback)
	{
		// let the callee do whatever with the data and possibly cancel
		return receiveCallback(data, size);
	}

	return true;
}

void DownloadManager::startDownload(const char* inDownloadName)
{
	if (startCallback)
	{
		// notify about the new download
		if (!startCallback(downloadName.c_str()))
		{
			// don't start the download if the callee didn't want it
			return;
		}
	}

	downloadName = inDownloadName;
	downloadRequested = true;
}

void DownloadManager::cancelDownload()
{
	clearDownload();
}


void DownloadManager::clearDownload()
{
	downloadName.clear();
	downloadRequested = false;
}

void DownloadManager::downloadsComplete()
{
	// inform that download has stopped
	imports.addReliableCommand("donedl");
}

void DownloadManager::nextDownload()
{
	// FIXME: list of files to download
}

void DownloadManager::setDownloadStartedCallback(startCallback_f&& callback)
{
	startCallback = std::move(callback);
}

void DownloadManager::setReceiveCallback(receiveCallback_f&& callback)
{
	receiveCallback = std::move(callback);
}

void ClientGameConnection::parseDownload(MSG& msg)
{
	downloadState.processDownload(msg);
}

void ClientGameConnection::parseCommandString(MSG& msg)
{
	const uint32_t seq = msg.ReadUInteger();

	const StringMessage s = readStringMessage(msg);

	// check if it is already stored
	if (serverCommandSequence >= seq) {
		return;
	}

	serverCommandSequence = seq;

	const uint32_t index = seq & (MAX_RELIABLE_COMMANDS - 1);

	//serverCommands[index] = &serverCmdStrings[MAX_STRING_CHARS * index];
	strncpy(serverCommands[index], s, sizeof(serverCmdStrings[index * MAX_STRING_CHARS]) * MAX_STRING_CHARS);

	Event ev;
	TokenParser parser;
	parser.Parse(s, strlen(s));

	const str commandName = parser.GetToken(false);

	while (parser.TokenAvailable(true)) {
		ev.AddString(parser.GetToken(true));
	}

#if _DEBUG
	if (!str::icmp(commandName, "stufftext"))
	{
		// Warn about stufftext
		MOHPC_LOG(Warning, "Stufftext command detected. Handle it with high precautions. Arguments : %s", s.getData() + 10);
	}
#endif

	// notify about the new command
	handlerList.serverCommandHandler.broadcast(commandName.c_str(), ev);

	if (!str::icmp(commandName, "disconnect"))
	{
		// Server kicking out the client

		// Wipe the channel as it has been closed on server already
		wipeChannel();
		// Disconnect the client
		disconnect();
	}
}

void ClientGameConnection::parseCenterprint(MSG& msg)
{
	const StringMessage s = readStringMessage(msg);
	handlerList.centerPrintHandler.broadcast(const_cast<const char*>(s.getData()));
}

void ClientGameConnection::parseLocprint(MSG& msg)
{
	const uint16_t x = msg.ReadUShort();
	const uint16_t y = msg.ReadUShort();

	const StringMessage string = readStringMessage(msg);
	handlerList.locationPrintHandler.broadcast(x, y, const_cast<const char*>(string.getData()));
}

void ClientGameConnection::parseCGMessage(MSG& msg)
{
	// Let the module handle messages
	cgameModule->parseCGMessage(msg);
}

void ClientGameConnection::notifyAllConfigStringChanges()
{
	systemInfoChanged();

	// notify about configstrings that are not empty
	for (csNum_t i = 0; i < MAX_CONFIGSTRINGS; ++i)
	{
		const char* cs = gameState.getConfigStringChecked(i);
		if (*cs) {
			notifyConfigStringChange(i, cs);
		}
	}
}

void ClientGameConnection::systemInfoChanged()
{
	ReadOnlyInfo serverSystemInfo = getServerSystemInfo();
	ReadOnlyInfo serverGameInfo = getServerGameInfo();

	serverId = serverSystemInfo.IntValueForKey("sv_serverid");
	const uint32_t sv_fps = serverGameInfo.IntValueForKey("sv_fps");
	serverDeltaFrequency = (uint64_t)(1.f / (float)sv_fps * 1000.f) * 2;
}

void ClientGameConnection::createNewCommands()
{
	++cmdNumber;
	const uint32_t cmdNum = cmdNumber & CMD_MASK;

	usercmd_t& cmd = cmds[cmdNum];
	createCmd(cmd);

	userEyes = usereyes_t();

	// all movement will happen through the event notification
	// the callee is responsible for making user input
	handlerList.userInputHandler.broadcast(cmd, userEyes);
}

void ClientGameConnection::createCmd(usercmd_t& outcmd)
{
	outcmd = usercmd_t((uint32_t)serverTime);
}

bool ClientGameConnection::sendCmd(uint64_t currentTime)
{
	if (canCreateCommand())
	{
		// only create commands if the client is completely ready
		createNewCommands();
	}

	if (!readyToSendPacket(currentTime)) {
		return false;
	}

	return true;
}

clc_ops_e ClientGameConnection::getClientOperation() const
{
	if (currentSnap.valid && serverMessageSequence == currentSnap.messageNum) {
		return clc_ops_e::Move;
	}
	else {
		return clc_ops_e::MoveNoDelta;
	}
}

uint32_t ClientGameConnection::getCommandHashKey() const
{
	uint32_t key = checksumFeed;
	// also use the message acknowledge
	key ^= serverMessageSequence;
	// also use the last acknowledged server command in the key
	key ^= hashKey(serverCommands[serverCommandSequence & (MAX_RELIABLE_COMMANDS - 1)], 32);

	return key;
}

uint8_t ClientGameConnection::getNumCommandsToWrite(uint32_t oldPacketNum) const
{
	if (cmdNumber > outPackets[oldPacketNum].p_cmdNumber + MAX_PACKET_USERCMDS)
	{
		return MAX_PACKET_USERCMDS;
		MOHPC_LOG(Warning, "MAX_PACKET_USERCMDS");
	}

	return cmdNumber - outPackets[oldPacketNum].p_cmdNumber;
}

void ClientGameConnection::writePacket(uint64_t currentTime)
{
	handlerList.preWritePacketHandler.broadcast();

	DynamicDataMessageStream stream;
	MSG msg(stream, msgMode_e::Writing);

	// a packet is around 10 and 30 bytes
	stream.reserve(32);

	writePacketHeader(msg);
	// write commands the server didn't acknowledge
	writeReliableCommands(msg);
	// write user commands
	writeUserInput(msg, currentTime);

	// end of packet
	msg.WriteByte(clc_ops_e::eof);

	// flush out pending data
	msg.Flush();

	static constexpr size_t encodeStart = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);

	stream.Seek(encodeStart, IMessageStream::SeekPos::Begin);
	encoder->setSecretKey(serverId);
	encoder->setMessageAcknowledge(serverMessageSequence);
	encoder->setReliableAcknowledge(serverCommandSequence);
	encoder->encode(stream, stream);
	stream.Seek(0, IMessageStream::SeekPos::Begin);
	
	// transmit the encoded message
	getNetchan()->transmit(*adr, stream);

	lastPacketSendTime = currentTime;
}

void ClientGameConnection::writePacketHeader(MSG& msg)
{
	// write the server id (given with the gameState)
	msg.WriteUInteger(serverId);
	// write the server sequence number (packet number)
	msg.WriteUInteger(serverMessageSequence);
	// write the command sequence acknowledge
	msg.WriteUInteger(serverCommandSequence);
}

void ClientGameConnection::writeReliableCommands(MSG& msg)
{
	for (uint32_t i = reliableAcknowledge + 1; i <= reliableSequence; ++i)
	{
		msg.WriteByte(clc_ops_e::ClientCommand);
		msg.WriteInteger(i);
		writeStringMessage(msg, reliableCommands[i & (MAX_RELIABLE_COMMANDS - 1)]);
	}
}

void ClientGameConnection::writeUserInput(MSG& msg, uint64_t currentTime)
{
	const uint32_t oldPacketNum = (getNetchan()->getOutgoingSequence() - 1) & PACKET_MASK;
	const uint8_t count = getNumCommandsToWrite(oldPacketNum);

	static const usercmd_t nullcmd;
	const usercmd_t* oldcmd = &nullcmd;

	if (count >= 1)
	{
		const clc_ops_e cmdOp = getClientOperation();

		// write the operation type
		msg.WriteByteEnum<clc_ops_e>(cmdOp);
		// write the number of commands
		msg.WriteByte(count);
		// write delta eyes
		SerializableUserEyes oldEyeInfo(outPackets[oldPacketNum].p_eyeinfo);
		SerializableUserEyes userEyesWrite(userEyes);
		msg.WriteDeltaClass(&oldEyeInfo, &userEyesWrite);

		// get the key used to xor user command data
		const uint32_t key = getCommandHashKey();

		// now write all commands
		writeAllCommands(msg, oldcmd, count, key);
	}

	// save values for later
	storeOutputPacket(currentTime, oldcmd->serverTime);

	// Write end of user commands
	msg.WriteByte(clc_ops_e::eof);
}

void ClientGameConnection::writeAllCommands(MSG& msg, const usercmd_t*& oldcmd, size_t count, uint32_t key)
{
	// write all the commands, including the predicted command
	for (size_t i = 0; i < count; i++)
	{
		const size_t j = (cmdNumber - count + i + 1) & CMD_MASK;
		const usercmd_t* cmd = &cmds[j];
		// write a delta of the command by using the old
		SerializableUsercmd oldCmdRead(*const_cast<usercmd_t*>(oldcmd));
		SerializableUsercmd inputCmd(*const_cast<usercmd_t*>(cmd));
		msg.WriteDeltaClass(&oldCmdRead, &inputCmd, key);

		oldcmd = cmd;
	}
}

void ClientGameConnection::storeOutputPacket(uint64_t currentTime, uint32_t serverTime)
{
	const uint32_t packetNum = getNetchan()->getOutgoingSequence() & PACKET_MASK;
	outPackets[packetNum].p_currentTime = currentTime;
	outPackets[packetNum].p_serverTime = serverTime;
	outPackets[packetNum].p_cmdNumber = cmdNumber;
	outPackets[packetNum].p_eyeinfo = userEyes;
}

StringMessage ClientGameConnection::readStringMessage(MSG& msg)
{
	return (*readStringMessage_pf)(msg);
}

void ClientGameConnection::writeStringMessage(MSG& msg, const char* s)
{
	return (*writeStringMessage_pf)(msg, s);
}

uint32_t ClientGameConnection::hashKey(const char* string, size_t maxlen) const
{
	return (this->*hashKey_pf)(string, maxlen);
}

entityNum_t ClientGameConnection::readEntityNum(MsgTypesHelper& msgHelper)
{
	return (this->*readEntityNum_pf)(msgHelper);
}

void ClientGameConnection::readDeltaPlayerstate(MSG& msg, const playerState_t* from, playerState_t* to)
{
	(this->*readDeltaPlayerstate_pf)(msg, from, to);
}

void ClientGameConnection::readDeltaEntity(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum)
{
	(this->*readDeltaEntity_pf)(msg, from, to, newNum);
}

StringMessage ClientGameConnection::readStringMessage_normal(MSG& msg)
{
	return msg.ReadString();
}

void ClientGameConnection::writeStringMessage_normal(MSG& msg, const char* s)
{
	msg.WriteString(s);
}

StringMessage ClientGameConnection::readStringMessage_scrambled(MSG& msg)
{
	return msg.ReadScrambledString(byteCharMapping);
}

void ClientGameConnection::writeStringMessage_scrambled(MSG& msg, const char* s)
{
	msg.WriteScrambledString(s, charByteMapping);
}

csNum_t ClientGameConnection::getNormalizedConfigstring(csNum_t num)
{
	return (this->*getNormalizedConfigstring_pf)(num);
}

size_t ClientGameConnection::getMaxCommandSize() const
{
	return (this->*getMaxCommandSize_pf)();
}

void ClientGameConnection::readNonPVSClient(radarInfo_t radarInfo)
{
	return (this->*readNonPVSClient_pf)(radarInfo);
}

const ClientSnapshot* ClientGameConnection::readOldSnapshot(MSG& msg, ClientSnapshot& snap) const
{
	const uint8_t deltaNum = msg.ReadByte();
	if (!deltaNum) {
		snap.deltaNum = -1;
	}
	else {
		snap.deltaNum = snap.messageNum - deltaNum;
	}

	if (snap.deltaNum <= 0)
	{
		// uncompressed frame
		snap.valid = true;
		return nullptr;
	}

	const ClientSnapshot* old = &snapshots[snap.deltaNum & PACKET_MASK];
	if (!old->valid) {
		// should never happen
		// FIXME: throw?
		MOHPC_LOG(Warning, "Delta from invalid frame (not supposed to happen!).");
	}
	else if (old->messageNum != snap.deltaNum) {
		// The frame that the server did the delta from
		// is too old, so we can't reconstruct it properly.
		// FIXME: throw?
		MOHPC_LOG(Warning, "Delta frame too old.");
	}
	else if (parseEntitiesNum > old->parseEntitiesNum + (MAX_PARSE_ENTITIES - 128)) {
		// FIXME: throw?
		MOHPC_LOG(Warning, "Delta parseEntitiesNum too old.");
	}
	else {
		snap.valid = true;	// valid delta parse
	}

	return old;
}

void ClientGameConnection::readAreaMask(MSG& msg, ClientSnapshot& snap)
{
	const uint8_t areaLen = msg.ReadByte();

	if (areaLen > sizeof(snap.areamask)) {
		throw ClientError::AreaMaskBadSize(areaLen);
	}

	// Read the area mask
	msg.ReadData(snap.areamask, areaLen);
}

void ClientGameConnection::setNewSnap(ClientSnapshot& newSnap)
{
	uint32_t oldMessageNum = currentSnap.messageNum + 1;

	if (newSnap.messageNum >= PACKET_BACKUP + oldMessageNum) {
		oldMessageNum = newSnap.messageNum - (PACKET_BACKUP - 1);
	}

	for (; oldMessageNum < newSnap.messageNum; oldMessageNum++) {
		snapshots[oldMessageNum & PACKET_MASK].valid = false;
	}

	currentSnap = newSnap;

	snapshots[currentSnap.messageNum & PACKET_MASK] = currentSnap;
	newSnapshots = true;
}

uint32_t ClientGameConnection::hashKey_ver6(const char* string, size_t maxlen) const
{
	uint32_t hash = 0;

	for (size_t i = 0; i < maxlen && string[i]; i++) {
		hash += string[i] * (119 + (uint32_t)i);
	}

	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	return hash;
}

uint32_t ClientGameConnection::hashKey_ver15(const char* string, size_t maxlen) const
{
	uint32_t hash = 0;

	for (size_t i = 0; i < maxlen && string[i]; i++) {
		hash += string[i] * (119 + (uint32_t)i);
	}

	hash = (hash ^ ((hash ^ (hash >> 10)) >> 10));
	return hash;
}

entityNum_t ClientGameConnection::readEntityNum_ver6(MsgTypesHelper& msgHelper)
{
	return msgHelper.ReadEntityNum();
}

entityNum_t ClientGameConnection::readEntityNum_ver15(MsgTypesHelper& msgHelper)
{
	return msgHelper.ReadEntityNum2();
}

void ClientGameConnection::parseGameState_ver6(MSG& msg)
{
}

void ClientGameConnection::parseGameState_ver15(MSG& msg)
{
	// this is the frameTime of the server (sv_fps)
	// it is practically useless because the delta frequency is already calculated earlier
	const float serverFrameTime = msg.ReadFloat();
}

void ClientGameConnection::readDeltaPlayerstate_ver6(MSG& msg, const playerState_t* from, playerState_t* to)
{
	SerializablePlayerState toSerialize(*to);
	if(from)
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

void ClientGameConnection::readDeltaPlayerstate_ver15(MSG& msg, const playerState_t* from, playerState_t* to)
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

void ClientGameConnection::readDeltaEntity_ver6(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum)
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

void ClientGameConnection::readDeltaEntity_ver15(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum)
{
	SerializableEntityState_ver15 toSerialize(*to, newNum);
	if (from)
	{
		SerializableEntityState_ver15 fromSerialize(*const_cast<entityState_t*>(from), newNum);
		msg.ReadDeltaClass(&fromSerialize, &toSerialize);
	}
	else
	{
		// no delta
		msg.ReadDeltaClass(nullptr, &toSerialize);
	}
}

csNum_t ClientGameConnection::getNormalizedConfigstring_ver6(csNum_t num)
{
	if (num <= CS_WARMUP || num >= 26) {
		return num;
	}

	return num - 2;
}

csNum_t ClientGameConnection::getNormalizedConfigstring_ver15(csNum_t num)
{
	return num;
}

size_t ClientGameConnection::getMaxCommandSize_ver6() const
{
	return 1024;
}

size_t ClientGameConnection::getMaxCommandSize_ver15() const
{
	return MAX_STRING_CHARS;
}

void ClientGameConnection::readNonPVSClient_ver6(radarInfo_t radarInfo)
{
	// unsupported in AA
}

void ClientGameConnection::readNonPVSClient_ver15(radarInfo_t radarInfo)
{
	radarUnpacked_t unpacked;

	if (unpackNonPVSClient(radarInfo, unpacked))
	{
		const Vector& origin = currentSnap.ps.getOrigin();
		unpacked.x += origin[0];
		unpacked.y += origin[1];

		handlerList.readNonPVSClientHandler.broadcast(unpacked);
	}
}

CGameModuleBase* ClientGameConnection::getCGModule()
{
	return cgameModule;
}

const NetAddr& ClientGameConnection::getRemoteAddress() const
{
	return *adr;
}

const gameState_t& ClientGameConnection::getGameState() const
{
	return gameState;
}

ReadOnlyInfo ClientGameConnection::getServerSystemInfo() const
{
	return gameState.getConfigString(CS_SYSTEMINFO);
}

ReadOnlyInfo ClientGameConnection::getServerGameInfo() const
{
	return gameState.getConfigString(CS_SERVERINFO);
}

ConstClientInfoPtr ClientGameConnection::getUserInfo() const
{
	return userInfo;
}

const ClientInfoPtr& ClientGameConnection::getUserInfo()
{
	return userInfo;
}

void ClientGameConnection::updateUserInfo()
{
	Info info;
	ClientInfoHelper::fillInfoString(*userInfo, info);
	// send the new user info to the server
	addReliableCommand(str::printf("userinfo \"%s\"", info.GetString()));
}

clientGameSettings_t& ClientGameConnection::getSettings()
{
	return settings;
}

const clientGameSettings_t& ClientGameConnection::getSettings() const
{
	return settings;
}

void ClientGameConnection::initTime(uint64_t currentTime)
{
	realTimeStart = currentTime;

	using namespace std::chrono;
	lastTimeoutTime = steady_clock::time_point(milliseconds(currentTime));
}

uintptr_t ClientGameConnection::getCurrentSnapshotNumber() const
{
	return currentSnap.messageNum;
}

bool ClientGameConnection::getSnapshot(uintptr_t snapshotNum, SnapshotInfo& outSnapshot) const
{
	// if the frame has fallen out of the circular buffer, we can't return it
	if (currentSnap.messageNum >= PACKET_BACKUP + snapshotNum) {
		return false;
	}

	// if the frame is not valid, we can't return it
	const ClientSnapshot* foundSnap = &snapshots[snapshotNum & PACKET_MASK];
	if (!foundSnap->valid) {
		return false;
	}

	// if the entities in the frame have fallen out of their
	// circular buffer, we can't return it
	if (parseEntitiesNum >= MAX_PARSE_ENTITIES + foundSnap->parseEntitiesNum) {
		return false;
	}

	// Copy snapshot data
	outSnapshot.snapFlags = foundSnap->snapFlags;
	outSnapshot.serverCommandSequence = foundSnap->serverCommandNum;
	outSnapshot.ping = foundSnap->ping;
	outSnapshot.serverTime = foundSnap->serverTime;
	memcpy(outSnapshot.areamask, foundSnap->areamask, sizeof(outSnapshot.areamask));
	outSnapshot.ps = foundSnap->ps;

	size_t count = foundSnap->numEntities;
	if (count > MAX_ENTITIES_IN_SNAPSHOT) {
		count = MAX_ENTITIES_IN_SNAPSHOT;
	}

	outSnapshot.numEntities = count;

	// Copy entities to snapshot
	for (uintptr_t i = 0; i < count; i++) {
		outSnapshot.entities[i] = parseEntities[(foundSnap->parseEntitiesNum + i) & (MAX_PARSE_ENTITIES - 1)];
	}

	outSnapshot.numSounds = foundSnap->numSounds;
	for (uintptr_t i = 0; i < outSnapshot.numSounds; i++) {
		outSnapshot.sounds[i] = foundSnap->sounds[i];
	}

	return true;
}

uint64_t ClientGameConnection::getServerStartTime() const
{
	return serverStartTime;
}

uint64_t ClientGameConnection::getServerTime() const
{
	return serverTime;
}

uint64_t ClientGameConnection::getServerFrameFrequency() const
{
	return serverDeltaFrequency;
}

uintptr_t ClientGameConnection::getCurrentCmdNumber() const
{
	return cmdNumber;
}

uint32_t ClientGameConnection::getClientNum() const
{
	return clientNum;
}

bool ClientGameConnection::getUserCmd(uintptr_t cmdNum, usercmd_t& outCmd) const
{
	// the usercmd has been overwritten in the wrapping
	// buffer because it is too far out of date
	if (cmdNum + CMD_BACKUP < this->cmdNumber) {
		return false;
	}

	outCmd = cmds[cmdNum & CMD_MASK];
	return true;
}

bool ClientGameConnection::getServerCommand(uintptr_t serverCommandNumber, TokenParser& tokenized)
{
	const char* cmdString = serverCommands[serverCommandNumber % MAX_RELIABLE_COMMANDS];
	if (!cmdString) {
		return false;
	}

	tokenized.Parse(cmdString, strlen(cmdString));

	if (!str::icmpn(cmdString, "cs ", 3))
	{
		// Skip the "cs" token
		tokenized.GetToken(true);

		// Retrieve the configstring number
		const uint32_t num = tokenized.GetInteger(true);
		// Get the content
		const char* csString = tokenized.GetString(true, false);

		// Notify about modification
		configStringModified(getNormalizedConfigstring(num), csString);

		if (num == CS_SYSTEMINFO || num == CS_SERVERINFO)
		{
			// Notify when system info has changed
			systemInfoChanged();
		}

		return false;
	}

	return true;
}

uint32_t ClientGameConnection::getCurrentServerMessageSequence() const
{
	return serverMessageSequence;
}

uint32_t ClientGameConnection::getCurrentServerCommandSequence() const
{
	return serverCommandSequence;
}

void ClientGameConnection::disconnect()
{
	if(getNetchan())
	{
		addReliableCommand("disconnect");

		for (size_t i = 0; i < 3; ++i) {
			writePacket(0);
		}

		// Network channel is not needed anymore
		wipeChannel();
	}

	// Terminate the connection
	terminateConnection(nullptr);
}

void ClientGameConnection::sendCommand(const char* command)
{
	addReliableCommand(command);
	parseClientCommand(command);
}

void ClientGameConnection::markReady()
{
	isReady = true;
}

void ClientGameConnection::unmarkReady()
{
	isReady = false;
}

bool ClientGameConnection::canCreateCommand() const
{
	return isReady;
}

void ClientGameConnection::serverDisconnected(const char* reason)
{
	wipeChannel();

	// Terminate after wiping channel
	terminateConnection(reason);
}

void ClientGameConnection::terminateConnection(const char* reason)
{
	// Clear out the server id
	serverId = 0;

	if (cgameModule)
	{
		handlerList.disconnectHandler.broadcast(reason);

		// Delete the CG module
		delete cgameModule;
		cgameModule = nullptr;
	}
}

void ClientGameConnection::configStringModified(csNum_t num, const char* newString, bool notify)
{
	// set the config string in gamestate
	gameState.setConfigString(num, newString, strlen(newString));

	if(notify)
	{
		// propagate the change
		notifyConfigStringChange(num, newString);
	}
}

void ClientGameConnection::notifyConfigStringChange(csNum_t num, const char* newString)
{
	// notify the client module about cs modification
	if (cgameModule) cgameModule->configStringModified(num, newString);

	// notify about the change
	handlerList.configStringHandler.broadcast(num, newString);
}

void ClientGameConnection::wipeChannel()
{
	netchan = nullptr;
}

bool ClientGameConnection::isChannelValid() const
{
	return netchan != nullptr;
}

void ClientGameConnection::clearState()
{
	parseEntitiesNum = 0;
	cmdNumber = 0;
	isActive = false;
	isReady = false;
}

bool ClientGameConnection::isDifferentServer(uint32_t id)
{
	return id != serverId;
}

void ClientGameConnection::parseClientCommand(const char* arguments)
{
	TokenParser parser;
	parser.Parse(arguments, strlen(arguments) + 1);

	const char* cmd = parser.GetToken(false);
	if(!str::icmp(cmd, "download"))
	{
		// a download command was issued so start it
		const char* downloadName = parser.GetToken(false);
		downloadState.startDownload(downloadName);
	}
	else if (!str::icmp(cmd, "stopdl"))
	{
		// cancel downloading
		downloadState.cancelDownload();
	}
}

void ClientGameConnection::setCGameTime(uint64_t currentTime)
{
	if(!isActive)
	{
		if (newSnapshots)
		{
			newSnapshots = false;
			firstSnapshot(currentTime);
		}
	}

	// if we have gotten to this point, cl.snap is guaranteed to be valid
	if (!currentSnap.valid)
	{
		realTimeStart = currentTime;
		return;
	}

	// Check when server has restarted
	if ((currentSnap.snapFlags ^  lastSnapFlags) & SNAPFLAG_SERVERCOUNT) {
		serverRestarted();
	}

	// FIXME: throw if snap server time went backward

	oldFrameServerTime = currentSnap.serverTime;

	int32_t tn = settings.getTimeNudge();
	if (tn < -30) {
		tn = -30;
	}
	else if (tn > 30) {
		tn = 30;
	}

	const uint64_t realTime = currentTime;
	const uint64_t realServerTime = realTime - realTimeStart + oldServerTime;
	serverTime = realServerTime - tn;
	oldRealTime = currentTime;

	// guarantee that time will never flow backwards, even if
	// serverTimeDelta made an adjustment or cl_timeNudge was changed
	if (serverTime < oldServerTime) {
		serverTime = oldServerTime;
	}

	if (realServerTime + 5 >= currentSnap.serverTime) {
		extrapolatedSnapshot = true;
	}

	if(newSnapshots) {
		adjustTimeDelta(realTime);
	}
}

void ClientGameConnection::adjustTimeDelta(uint64_t realTime)
{
	newSnapshots = false;

	constexpr size_t BASE_RESET_TIME = 400;

	const uint64_t resetTime = BASE_RESET_TIME + serverDeltaFrequency;
	const uint64_t deltaDelta = getTimeDelta(serverTime);

	if (deltaDelta > resetTime)
	{
		realTimeStart = realTime;
		oldServerTime = currentSnap.serverTime;
		serverTime = currentSnap.serverTime;
	}
	else if (deltaDelta > serverDeltaFrequency)
	{
		// fast adjust
		realTimeStart += serverDeltaFrequency - deltaDelta;
	}
	else
	{
		if (extrapolatedSnapshot)
		{
			extrapolatedSnapshot = false;
			realTimeStart += 2;
		}
		else {
			realTimeStart -= 1;
		}
	}
}

uint64_t ClientGameConnection::getTimeDelta(uint64_t time) const
{
	if (currentSnap.serverTime < time) {
		return time - currentSnap.serverTime;
	}
	else {
		return currentSnap.serverTime - time;
	}
}

void ClientGameConnection::firstSnapshot(uint64_t currentTime)
{
	if (currentSnap.snapFlags & SNAPFLAG_NOT_ACTIVE) {
		return;
	}

	updateSnapFlags();

	serverStartTime = currentSnap.serverTime;
	//serverTime = (uint32_t)(currentSnap.serverTime - currentTime);
	serverTime = serverStartTime + (currentTime - realTimeStart);
	isActive = true;

	cgameModule->init(serverMessageSequence, serverCommandSequence);

	// Notify about the snapshot
	getHandlerList().firstSnapshotHandler.broadcast(currentSnap);

	// update the userinfo to not let the server hold garbage fields like the challenge
	updateUserInfo();
}

void ClientGameConnection::serverRestarted()
{
	updateSnapFlags();

	handlerList.serverRestartedHandler.broadcast();
}

void ClientGameConnection::updateSnapFlags()
{
	lastSnapFlags = currentSnap.snapFlags;
}

void ClientGameConnection::calculatePing(uint64_t currentTime)
{
	currentSnap.ping = 999;

	uint16_t ping = 0;
	for (size_t i = 0; i < PACKET_BACKUP; ++i)
	{
		const uintptr_t packetNum = (getNetchan()->getOutgoingSequence() - 1 - i) & PACKET_MASK;
		if (currentSnap.ps.commandTime >= outPackets[packetNum].p_serverTime)
		{
			currentSnap.ping = (uint32_t)(currentTime - outPackets[packetNum].p_currentTime);
			break;
		}
	}
}

bool ClientGameConnection::readyToSendPacket(uint64_t currentTime) const
{
	if (!cgameModule)
	{
		// Disconnected from server
		return false;
	}

	if (!serverId || !canCreateCommand())
	{
		// allow one packet per second when not entered
		return currentTime >= lastPacketSendTime + 1000;
	}

	const size_t oldPacketNum = (getNetchan()->getOutgoingSequence() - 1) & PACKET_MASK;
	const uint64_t delta = currentTime - outPackets[oldPacketNum].p_currentTime;
	if (delta < 1000 / settings.getMaxPackets()) {
		return false;
	}

	return true;
}

bool ClientGameConnection::unpackNonPVSClient(radarInfo_t radarInfo, radarUnpacked_t& unpacked)
{
	unpacked.clientNum = radarInfo.clientNum();

	if (unpacked.clientNum == currentSnap.ps.getClientNum()) {
		return false;
	}

	const float radarRange = settings.getRadarRange();
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


void ClientGameConnection::fillClientImports(ClientImports& imports)
{
	using namespace std::placeholders;
	imports.getClientNum				= std::bind(&ClientGameConnection::getClientNum, this);
	imports.getCurrentSnapshotNumber	= std::bind(&ClientGameConnection::getCurrentSnapshotNumber, this);
	imports.getSnapshot					= std::bind(&ClientGameConnection::getSnapshot, this, _1, _2);
	imports.getServerStartTime			= std::bind(&ClientGameConnection::getServerStartTime, this);
	imports.getServerTime				= std::bind(&ClientGameConnection::getServerTime, this);
	imports.getServerFrameFrequency		= std::bind(&ClientGameConnection::getServerFrameFrequency, this);
	imports.getUserCmd					= std::bind(&ClientGameConnection::getUserCmd, this, _1, _2);
	imports.getCurrentCmdNumber			= std::bind(&ClientGameConnection::getCurrentCmdNumber, this);
	imports.getServerCommand			= std::bind(&ClientGameConnection::getServerCommand, this, _1, _2);
	imports.getGameState				= std::bind(&ClientGameConnection::getGameState, this);
	imports.readStringMessage			= std::bind(&ClientGameConnection::readStringMessage, this, _1);
	imports.addReliableCommand			= std::bind(&ClientGameConnection::addReliableCommand, this, _1);
	imports.getUserInfo					= std::bind(static_cast<const ClientInfoPtr&(ClientGameConnection::*)()>(&ClientGameConnection::getUserInfo), this);
}

MOHPC_OBJECT_DEFINITION(ClientInfo);

ClientInfo::ClientInfo()
	: snaps(20)
	, rate(5000)
{
}

void ClientInfo::setRate(uint32_t inRate)
{
	rate = inRate;
}

uint32_t ClientInfo::getRate() const
{
	return rate;
}

void ClientInfo::setSnaps(uint32_t inSnaps)
{
	snaps = inSnaps;
}

uint32_t ClientInfo::getSnaps() const
{
	return snaps;
}

void ClientInfo::setName(const char* newName)
{
	name = newName;
}

const char* ClientInfo::getName() const
{
	return name.c_str();
}

void ClientInfo::setPlayerAlliedModel(const char* newModel)
{
	properties.SetPropertyValue("dm_playermodel", newModel);
}

const char* ClientInfo::getPlayerAlliedModel() const
{
	return properties.GetPropertyRawValue("dm_playermodel");
}

void ClientInfo::setPlayerGermanModel(const char* newModel)
{
	properties.SetPropertyValue("dm_playergermanmodel", newModel);
}

const char* ClientInfo::getPlayerGermanModel() const
{
	return properties.GetPropertyRawValue("dm_playergermanmodel");
}

void ClientInfo::setUserKeyValue(const char* key, const char* value)
{
	properties.SetPropertyValue(key, value);
}

const char* ClientInfo::getUserKeyValue(const char* key) const
{
	return properties.GetPropertyRawValue(key);
}

const PropertyObject& ClientInfo::getPropertyObject() const
{
	return properties;
}

void ClientInfoHelper::fillInfoString(const ClientInfo& clientInfo, Info& info)
{
	// Build mandatory variables
	info.SetValueForKey("rate", str::printf("%i", clientInfo.getRate()));
	info.SetValueForKey("snaps", str::printf("%i", clientInfo.getSnaps()));
	info.SetValueForKey("name", clientInfo.getName());

	// Build miscellaneous values
	for (PropertyMapIterator it = clientInfo.getPropertyObject().GetIterator(); it; ++it)
	{
		info.SetValueForKey(
			it.key().GetFullPropertyName(),
			it.value()
		);
	}
}


clientGameSettings_t::clientGameSettings_t()
	: maxPackets(30)
	, maxTickPackets(60)
	, radarRange(1024.f)
	, timeNudge(0)
{

}

void clientGameSettings_t::setRadarRange(float value)
{
	radarRange = value;
}

float clientGameSettings_t::getRadarRange() const
{
	return radarRange;
}

uint32_t clientGameSettings_t::getMaxPackets() const
{
	return maxPackets;
}

void clientGameSettings_t::setMaxPackets(uint32_t inMaxPackets)
{
	maxPackets = inMaxPackets;
	if (maxPackets < 1) {
		maxPackets = 1;
	}
	else if (maxPackets > 125) {
		maxPackets = 125;
	}
}

uint32_t clientGameSettings_t::getMaxTickPackets() const
{
	return maxTickPackets;
}

void clientGameSettings_t::setMaxTickPackets(uint32_t inMaxPackets)
{
	maxTickPackets = inMaxPackets;
	if (maxTickPackets < 1) {
		maxTickPackets = 1;
	}
	else if (maxTickPackets > 1000) {
		maxTickPackets = 1000;
	}
}

void clientGameSettings_t::setTimeNudge(uint32_t value)
{
	timeNudge = value;
}

uint32_t clientGameSettings_t::getTimeNudge() const
{
	return timeNudge;
}

ClientError::BadCommandByteException::BadCommandByteException(uint8_t inCmdNum)
	: cmdNum(inCmdNum)
{}

uint8_t ClientError::BadCommandByteException::getLength() const
{
	return cmdNum;
}

str ClientError::BadCommandByteException::what() const
{
	return str((int)getLength());
}

ClientError::BadProtocolVersionException::BadProtocolVersionException(uint8_t inProtocolVersion)
	: protocolVersion(inProtocolVersion)
{}

uint32_t ClientError::BadProtocolVersionException::getProtocolVersion() const
{
	return protocolVersion;
}

str ClientError::BadProtocolVersionException::what() const
{
	return str((int)getProtocolVersion());
}

ClientError::IllegibleServerMessageException::IllegibleServerMessageException(uint8_t inCmdNum)
	: cmdNum(inCmdNum)
{}

uint8_t ClientError::IllegibleServerMessageException::getLength() const
{
	return cmdNum;
}

str ClientError::IllegibleServerMessageException::what() const
{
	return str((int)getLength());
}

ClientError::BaselineOutOfRangeException::BaselineOutOfRangeException(uint16_t inBaselineNum)
	: baselineNum(inBaselineNum)
{}

uint16_t ClientError::BaselineOutOfRangeException::getBaselineNum() const
{
	return baselineNum;
}

str ClientError::BaselineOutOfRangeException::what() const
{
	return str((int)getBaselineNum());
}

ClientError::MaxConfigStringException::MaxConfigStringException(const char* inName, csNum_t inConfigStringNum)
	: name(inName)
	, configStringNum(inConfigStringNum)
{}

const char* ClientError::MaxConfigStringException::getName() const
{
	return name;
}

csNum_t ClientError::MaxConfigStringException::getConfigstringNum() const
{
	return configStringNum;
}

str ClientError::MaxConfigStringException::what() const
{
	return str((int)getConfigstringNum());
}

ClientError::MaxGameStateCharsException::MaxGameStateCharsException(size_t inStringLen)
	: stringLen(inStringLen)
{}

size_t ClientError::MaxGameStateCharsException::GetStringLength() const
{
	return stringLen;
}

str ClientError::MaxGameStateCharsException::what() const
{
	return str((int)GetStringLength());
}

ClientError::AreaMaskBadSize::AreaMaskBadSize(uint8_t inSize)
	: size(inSize)
{}

uint8_t ClientError::AreaMaskBadSize::getSize() const
{
	return size;
}

str ClientError::AreaMaskBadSize::what() const
{
	return str((int)getSize());
}

ClientError::DownloadException::DownloadException(StringMessage&& inError)
	: error(std::move(inError))
{}

ClientError::DownloadSizeException::DownloadSizeException(uint16_t inSize)
	: size(inSize)
{}

const char* ClientError::DownloadException::getError() const
{
	return error;
}
str ClientError::DownloadException::what() const
{
	return str(getError());
}

uint16_t ClientError::DownloadSizeException::getSize() const
{
	return size;
}

ClientError::BadDownloadBlockException::BadDownloadBlockException(uint16_t inBlock, uint16_t inExpectedBlock)
	: block(inBlock)
	, expectedBlock(inExpectedBlock)
{
}

uint16_t ClientError::BadDownloadBlockException::getBlock() const noexcept
{
	return block;
}

uint16_t ClientError::BadDownloadBlockException::getExpectedBlock() const noexcept
{
	return expectedBlock;
}
