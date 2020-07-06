#include <MOHPC/Network/ClientGame.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Network/Channel.h>
#include <MOHPC/Network/SerializableTypes.h>
#include <MOHPC/Network/CGModule.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Log.h>
#include <typeinfo>
#include <filesystem>
#include <fstream>
#include <chrono>

using namespace MOHPC;
using namespace Network;

static constexpr size_t MAX_MSGLEN = 49152;
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

Network::DownloadException::DownloadException(StringMessage&& inError)
	: error(std::move(inError))
{}

Network::gameState_t::gameState_t()
	: stringOffsets{ 0 }
	, stringData{ 0 }
	, dataCount(0)
{}

const char* Network::gameState_t::getConfigString(size_t num) const
{
	if (num > MAX_CONFIGSTRINGS) {
		return nullptr;
	}

	return &stringData[stringOffsets[num]];
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

ClientGameConnection::ClientGameConnection(NetworkManager* inNetworkManager, const INetchanPtr& inNetchan, const netadr_t& inAdr, uint32_t challengeResponse, const protocolType_c& protoType, const ClientInfoPtr& cInfo)
	: ITickableNetwork(inNetworkManager)
	, netchan(inNetchan)	
	, adr(inAdr)
	, realTimeStart(0)
	, serverStartTime(0)
	, serverTime(0)
	, oldServerTime(0)
	, oldFrameServerTime(0)
	, lastPacketSendTime(0)
	, timeoutTime(30000)
	, maxPackets(30)
	, maxTickPackets(60)
	, parseEntitiesNum(0)
	, serverCommandSequence(0)
	, cmdNumber(0)
	, clientNum(0)
	, checksumFeed(0)
	, serverId(0)
	, downloadedBlock(0)
	, reliableSequence(0)
	, reliableAcknowledge(0)
	, newSnapshots(false)
	, extrapolatedSnapshot(false)
	, isActive(false)
	, reliableCommands{ nullptr }
	, serverCommands{ nullptr }
	, reliableCmdStrings{ 0 }
	, serverCmdStrings{ 0 }
	, userInfo(cInfo)
{
	CGameImports imports;

	using namespace std::placeholders;
	imports.getCurrentSnapshotNumber	= std::bind(&ClientGameConnection::getCurrentSnapshotNumber, this);
	imports.getSnapshot					= std::bind(&ClientGameConnection::getSnapshot, this, _1, _2);
	imports.getServerStartTime			= std::bind(&ClientGameConnection::getServerStartTime, this);
	imports.getServerTime				= std::bind(&ClientGameConnection::getServerTime, this);
	imports.getServerFrameFrequency		= std::bind(&ClientGameConnection::getServerFrameFrequency, this);
	imports.getUserCmd					= std::bind(&ClientGameConnection::getUserCmd, this, _1, _2);
	imports.getCurrentCmdNumber			= std::bind(&ClientGameConnection::getCurrentCmdNumber, this);
	imports.getServerCommand			= std::bind(&ClientGameConnection::getServerCommand, this, _1, _2);
	imports.getGameState				= std::bind(&ClientGameConnection::getGameState, this);

	switch (protoType.getProtocolVersion())
	{
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

		cgameModule = new CGameModule6(imports);
		break;
	case protocolVersion_e::ver200:
	case protocolVersion_e::ver211:
		readStringMessage_pf = &ClientGameConnection::readStringMessage_scrambled;
		writeStringMessage_pf = &ClientGameConnection::writeStringMessage_scrambled;
		parseGameState_pf = &ClientGameConnection::parseGameState_ver15;
		hashKey_pf = &ClientGameConnection::hashKey_ver15;
		readEntityNum_pf = &ClientGameConnection::readEntityNum_ver15;
		readDeltaPlayerstate_pf = &ClientGameConnection::readDeltaPlayerstate_ver15;
		readDeltaEntity_pf = &ClientGameConnection::readDeltaEntity_ver15;
		getNormalizedConfigstring_pf = &ClientGameConnection::getNormalizedConfigstring_ver15;

		cgameModule = new CGameModule15(imports);
		break;
	default:
		throw BadProtocolVersionException((uint8_t)protoType.getProtocolVersion());
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

MOHPC::Network::ClientGameConnection::HandlerListClient& Network::ClientGameConnection::getHandlerList()
{
	return handlerList;
}

void ClientGameConnection::tick(uint64_t deltaTime, uint64_t currentTime)
{
	if (!isChannelValid()) {
		return;
	}

	using namespace std::chrono;
	if (timeoutTime > milliseconds::zero())
	{
		steady_clock::time_point clockTime = steady_clock::now();
		steady_clock::time_point nextTimeoutTime = lastTimeoutTime + timeoutTime;
		if (clockTime >= nextTimeoutTime)
		{
			// The server or the client timed out
			handlerList.notify<ClientHandlers::Timeout>();

			// Disconnect from server
			serverDisconnected(nullptr);
			return;
		}
	}

	size_t count = 0;

	IUdpSocket* socket = getNetchan()->getRawSocket();
	while(isChannelValid() && socket->dataAvailable() && count++ < maxTickPackets)
	{
		std::vector<uint8_t> data(65536);
		FixedDataMessageStream stream(data.data(), data.size());

		netadr_t from;
		socket->receive(data.data(), data.size(), from);

		// Prepare for reading
		MSG msg(stream, msgMode_e::Reading);
		receive(from, msg, currentTime);
	}

	// Build and send client commands
	if(sendCmd(currentTime))
	{
		// Send packets if there are new commands
		writePacket(serverMessageSequence, currentTime);
	}

	setCGameTime(currentTime);

	if (serverId) {
		getCGModule().tick(deltaTime, currentTime, serverTime);
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

void Network::ClientGameConnection::receive(const netadr_t& from, MSG& msg, uint64_t currentTime)
{
	IMessageStream& stream = msg.stream();

	stream.Seek(0);
	msg.SetCodec(MessageCodecs::OOB);
	serverMessageSequence = msg.ReadUInteger();

	if (getNetchan()->receive(stream))
	{
		encoder->decode(stream, stream);

		msg.Reset();

		try
		{
			parseServerMessage(msg, serverMessageSequence, currentTime);
		}
		catch (NetworkException& e)
		{
			MOHPC_LOG(Error, "got exception of type %s: \"%s\"", typeid(e).name(), e.what().c_str());

			// call the handler
			handlerList.notify<ClientHandlers::Error>(e);
		}

		using namespace std::chrono;
		lastTimeoutTime = steady_clock::now();
	}
	else if (serverMessageSequence == -1)
	{
		// a message without sequence number indicates a connectionless packet
		// the only possible connectionless packet should be the disconnect command
		// it may trigger if the client gets kicke during map loading

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
}

void Network::ClientGameConnection::addReliableCommand(const char* cmd)
{
	if (reliableSequence - reliableAcknowledge > MAX_RELIABLE_COMMANDS) {
		// FIXME: throw
		return;
	}

	++reliableSequence;
	const size_t index = reliableSequence & (MAX_RELIABLE_COMMANDS - 1);

	reliableCommands[index] = &reliableCmdStrings[MAX_STRING_CHARS * index];
	strncpy(reliableCommands[index], cmd, sizeof(reliableCmdStrings[0]) * MAX_STRING_CHARS);
}

void Network::ClientGameConnection::parseServerMessage(MSG& msg, uint32_t serverMessageSequence, uint64_t currentTime)
{
	msg.SetCodec(MessageCodecs::Bit);

	// Read the ack
	reliableAcknowledge = msg.ReadInteger();
	if (reliableAcknowledge < reliableSequence - (int32_t)MAX_RELIABLE_COMMANDS) {
		reliableAcknowledge = reliableSequence;
	}

	while(cgameModule)
	{
		uint8_t cmdNum = msg.ReadByte();

		svc_ops_e cmd = (svc_ops_e)cmdNum;
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
			parseSnapshot(msg, serverMessageSequence, currentTime);
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
			throw IllegibleServerMessageException(cmdNum);
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

void Network::ClientGameConnection::parseGameState(MSG& msg)
{
	MOHPC_LOG(Verbose, "Received gamestate");

	if (serverId) {
		MOHPC_LOG(Warning, "Server has resent gamestate while in-game");
	}

	MsgTypesHelper msgHelper(msg);

	serverCommandSequence = msg.ReadInteger();

	gameState.dataCount = 1;
	for (;;)
	{
		const svc_ops_e cmd = (svc_ops_e)msg.ReadByte();
		if (cmd == svc_ops_e::Eof) {
			break;
		}

		switch (cmd)
		{
		case svc_ops_e::Configstring:
		{
			const uint16_t stringNum = msg.ReadUShort();

			if (stringNum > MAX_CONFIGSTRINGS) {
				throw MaxConfigStringException(stringNum);
			}

			const StringMessage stringValue = readStringMessage(msg);

			configStringModified(getNormalizedConfigstring(stringNum), stringValue);
		}
		break;
		case svc_ops_e::Baseline:
		{
			const entityNum_t newNum = readEntityNum(msgHelper);
			if (newNum >= MAX_GENTITIES) {
				throw BaselineOutOfRangeException(newNum);
			}

			entityState_t nullState;
			GetNullEntityState(&nullState);

			entityState_t& es = entityBaselines[newNum];
			readDeltaEntity(msg, &nullState, &es, newNum);

			// Call handler
			handlerList.notify<ClientHandlers::EntityRead>((const entityState_t*)nullptr, const_cast<const entityState_t*>(&es));
		}
		break;
		default:
			throw BadCommandByteException((uint8_t)cmd);
		}
	}

	clientNum = msg.ReadUInteger();
	checksumFeed = msg.ReadUInteger();

	(this->*parseGameState_pf)(msg);
	systemInfoChanged();

	// Notify about the new game state
	getHandlerList().notify<ClientHandlers::GameStateParsed, const gameState_t&>(getGameState());
}

void Network::ClientGameConnection::parseSnapshot(MSG& msg, uint32_t serverMessageSequence, uint64_t currentTime)
{
	ClientSnapshot newSnap;
	ClientSnapshot* old;

	newSnap.serverCommandNum = serverCommandSequence;

	newSnap.serverTime = msg.ReadUInteger();
	newSnap.serverTimeResidual = msg.ReadByte();

	// Insert the sequence num
	newSnap.messageNum = serverMessageSequence;

	const uint8_t deltaNum = msg.ReadByte();
	if (!deltaNum) {
		newSnap.deltaNum = -1;
	}
	else {
		newSnap.deltaNum = newSnap.messageNum - deltaNum;
	}

	if (newSnap.deltaNum <= 0)
	{
		// uncompressed frame
		newSnap.valid = true;
		old = NULL;
	}
	else
	{
		old = &snapshots[newSnap.deltaNum & PACKET_MASK];
		if (!old->valid) {
			// should never happen
			// FIXME: throw?
		}

		if (old->messageNum != newSnap.deltaNum) {
			// The frame that the server did the delta from
			// is too old, so we can't reconstruct it properly.
			// FIXME: throw?
		}
		else if (parseEntitiesNum - old->parseEntitiesNum > MAX_PARSE_ENTITIES - 128) {
			// FIXME: throw?
		}
		else {
			newSnap.valid = true;	// valid delta parse
		}
	}

	newSnap.snapFlags = msg.ReadByte();

	uint8_t len = msg.ReadByte();

	if (len > sizeof(newSnap.areamask)) {
		throw AreaMaskBadSize(len);
	}

	// Read the area mask
	msg.ReadData(newSnap.areamask, len);

	// Read player state
	playerState_t* oldps = old ? &old->ps : nullptr;
	readDeltaPlayerstate(msg, oldps, &newSnap.ps);
	handlerList.notify<ClientHandlers::PlayerstateRead>(const_cast<const playerState_t*>(oldps), const_cast<const playerState_t*>(&newSnap.ps));

	// Read all entities in this snap
	parsePacketEntities(msg, old, &newSnap);

	// Parse sounds
	parseSounds(msg, &newSnap);

	if (!newSnap.valid) {
		return;
	}

	uint32_t oldMessageNum = currentSnap.messageNum + 1;

	if (newSnap.messageNum - oldMessageNum >= PACKET_BACKUP) {
		oldMessageNum = newSnap.messageNum - (PACKET_BACKUP - 1);
	}

	for (; oldMessageNum < newSnap.messageNum; oldMessageNum++) {
		snapshots[oldMessageNum & PACKET_MASK].valid = false;
	}

	if (currentSnap.valid && (newSnap.snapFlags ^ newSnap.snapFlags) & SNAPFLAG_SERVERCOUNT)
	{
		serverStartTime = newSnap.serverTime;
		realTimeStart = currentTime;
	}

	currentSnap = newSnap;
	currentSnap.ping = 999;

	// FIXME: calculate ping time
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

	snapshots[currentSnap.messageNum & PACKET_MASK] = currentSnap;
	newSnapshots = true;

	getHandlerList().notify<ClientHandlers::SnapReceived>(currentSnap);
}

void Network::ClientGameConnection::parsePacketEntities(MSG& msg, ClientSnapshot* oldFrame, ClientSnapshot* newFrame)
{
	MsgTypesHelper msgHelper(msg);

	newFrame->parseEntitiesNum = parseEntitiesNum;
	newFrame->numEntities = 0;

	// delta from the entities present in oldframe
	uint32_t oldIndex = 0;
	uint32_t oldNum = 0;
	entityState_t* oldState = NULL;
	if (!oldFrame) {
		oldNum = 99999;
	}
	else
	{
		oldState = &parseEntities[oldFrame->parseEntitiesNum & (MAX_PARSE_ENTITIES - 1)];
		oldNum = oldState->number;
	}

	std::bitset<MAX_GENTITIES> currentValidEntities;

	for (;;)
	{
		const uint16_t newNum = readEntityNum(msgHelper);

		if (newNum == ENTITYNUM_NONE) {
			break;
		}

		currentValidEntities.set(newNum, true);

		// FIXME: throw if end of message

		while (oldNum < newNum)
		{
			currentValidEntities.set(oldNum, true);
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
			currentValidEntities.set(oldNum, true);
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
		currentValidEntities.set(oldNum, true);
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

	// Find entities that were not sent and mark them as deleted
	for (size_t i = 0; i < MAX_GENTITIES; ++i)
	{
		if (!currentValidEntities.test(i) && validEntities.test(i))
		{
			// Find parse entity with the same number
			const size_t start = newFrame->parseEntitiesNum & (MAX_PARSE_ENTITIES - 1);
			for (size_t j = start ; j < MAX_PARSE_ENTITIES; ++j)
			{
				const entityState_t* parsed = &parseEntities[j];
				if (parsed->number == i)
				{
					// Notify about deletion
					handlerList.notify<ClientHandlers::EntityRead>(
						const_cast<const entityState_t*>(parsed),
						const_cast<const entityState_t*>(nullptr)
					);
					validEntities.set(i, false);
					break;
				}
			}
		}
	}
}

void Network::ClientGameConnection::parseDeltaEntity(MSG& msg, ClientSnapshot* frame, uint32_t newNum, entityState_t* old, bool unchanged)
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
		validEntities.set(newNum, false);
		handlerList.notify<ClientHandlers::EntityRead>(const_cast<const entityState_t*>(old), const_cast<const entityState_t*>(nullptr));
		return;
	}

	if (!validEntities.test(newNum))
	{
		// Notify for a new entity
		handlerList.notify<ClientHandlers::EntityRead>(const_cast<const entityState_t*>(nullptr), const_cast<const entityState_t*>(state));
	}
	else
	{
		// Notify only if it has changed
		handlerList.notify<ClientHandlers::EntityRead>(const_cast<const entityState_t*>(old), const_cast<const entityState_t*>(state));
	}

	++parseEntitiesNum;
	frame->numEntities++;
	validEntities.set(newNum, true);
}

void Network::ClientGameConnection::parseSounds(MSG& msg, ClientSnapshot* newFrame)
{
	const bool hasSounds = msg.ReadBool();
	if (!hasSounds) {
		return;
	}

	uint8_t numSounds = 0;
	msg.ReadBits(&numSounds, 7);
	if (numSounds > MAX_SERVER_SOUNDS) {
		return;
	}

	newFrame->numSounds = numSounds;

	MsgTypesHelper msgHelper(msg);

	// FIXME: set number of sounds and assign them
	for (size_t i = 0; i < numSounds; ++i)
	{
		sound_t& sound = newFrame->sounds[i];
		sound.hasStopped = msg.ReadBool();

		if (sound.hasStopped)
		{
			const uint16_t entityNum = readEntityNum(msgHelper);
			sound.entity = &entityBaselines[entityNum];

			uint8_t channel;
			msg.ReadBits(&channel, 7);
			sound.channel = channel;
		}
		else
		{
			sound.isStreamed = msg.ReadBool();
			sound.isSpatialized = msg.ReadBool();

			if (sound.isSpatialized) {
				sound.origin = msgHelper.ReadVectorFloat();
			}

			uint16_t entityNum = 0;
			msg.ReadBits(&entityNum, 11);
			sound.entity = &entityBaselines[entityNum];

			if (entityNum >= MAX_GENTITIES) {
				throw BadEntityNumberException(entityNum);
			}

			uint8_t channel = 0;
			msg.ReadBits(&channel, 7);
			sound.channel = channel;

			uint16_t soundIndex = 0;
			msg.ReadBits(&soundIndex, 9);

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

		handlerList.notify<ClientHandlers::Sound>(sound);
	}
}

void Network::ClientGameConnection::parseDownload(MSG& msg)
{
	uint8_t data[MAX_MSGLEN];

	uint16_t block = msg.ReadUShort();
	if (!block)
	{
		// block zero = file size
		int32_t fileSize = msg.ReadInteger();

		if (fileSize < 0)
		{
			StringMessage errorString = readStringMessage_scrambled(msg);
			throw DownloadException(std::move(errorString));
		}

		downloadedBlock = 0;

		std::filesystem::remove("dwnl.tmp");

		MOHPC_LOG(Verbose, "downloading file of size %d", fileSize);
	}

	uint16_t size = msg.ReadUShort();

	if (size < 0 || size > sizeof(data))
	{
		// FIXME: throw exception
		return;
	}

	if (size > 0) {
		msg.ReadData(data, size);

		std::ofstream strm("dwnl.tmp", std::ofstream::binary | std::ofstream::app);
		// append data
		strm.write((const char*)data, size);

		MOHPC_LOG(Verbose, "downloaded block %d size %d", block, size);
	}

	addReliableCommand(str::printf("nextdl %d", downloadedBlock++));

	// FIXME: actually download the file
}

void Network::ClientGameConnection::parseCommandString(MSG& msg)
{
	const uint32_t seq = msg.ReadUInteger();

	const StringMessage s = readStringMessage(msg);

	// check if it is already stored
	if (serverCommandSequence >= seq) {
		return;
	}

	serverCommandSequence = seq;

	const uint32_t index = seq & (MAX_RELIABLE_COMMANDS - 1);

	serverCommands[index] = &serverCmdStrings[MAX_STRING_CHARS * index];
	strncpy(serverCommands[index], s, sizeof(serverCmdStrings[0]) * MAX_STRING_CHARS);

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
	handlerList.notify<ClientHandlers::ServerCommand>(commandName.c_str(), ev);

	if (!str::icmp(commandName, "disconnect"))
	{
		// Server kicking out the client

		// Wipe the channel as it has been closed on server already
		wipeChannel();
		// Disconnect the client
		disconnect();
	}
}

void Network::ClientGameConnection::parseCenterprint(MSG& msg)
{
	const StringMessage s = readStringMessage(msg);
	handlerList.notify<ClientHandlers::CenterPrint>(const_cast<const char*>(s.getData()));
}

void Network::ClientGameConnection::parseLocprint(MSG& msg)
{
	const uint16_t x = msg.ReadUShort();
	const uint16_t y = msg.ReadUShort();

	const StringMessage string = readStringMessage(msg);
	handlerList.notify<ClientHandlers::LocationPrint>(x, y, const_cast<const char*>(string.getData()));
}

void Network::ClientGameConnection::parseCGMessage(MSG& msg)
{
	// Let the module handle messages
	cgameModule->parseCGMessage(msg);
}

void Network::ClientGameConnection::systemInfoChanged()
{
	const char* systemInfoStr = gameState.stringData + gameState.stringOffsets[CS_SYSTEMINFO];
	const char* serverInfoStr = gameState.stringData + gameState.stringOffsets[CS_SERVERINFO];

	serverSystemInfo = systemInfoStr;
	serverGameInfo = serverInfoStr;

	serverId = atoi(serverSystemInfo.ValueForKey("sv_serverid"));
	const uint32_t sv_fps = atoi(serverGameInfo.ValueForKey("sv_fps"));
	serverDeltaFrequency = (uint64_t)(1.f / (float)sv_fps * (1000.f * 2.f));
}

void Network::ClientGameConnection::createNewCommands()
{
	++cmdNumber;
	const uint32_t cmdNum = cmdNumber & CMD_MASK;

	usercmd_t& cmd = cmds[cmdNum];
	createCmd(cmd);

	userEyes = usereyes_t();

	// Let the app set user input
	handlerList.notify<ClientHandlers::UserInput>(cmd, userEyes);
}

void Network::ClientGameConnection::createCmd(usercmd_t& outcmd)
{
	outcmd = usercmd_t((uint32_t)serverTime);
}

bool Network::ClientGameConnection::sendCmd(uint64_t currentTime)
{
	if (!readyToSendPacket(currentTime)) {
		return false;
	}

	createNewCommands();
	return true;
}

void Network::ClientGameConnection::writePacket(uint32_t serverMessageSequence, uint64_t currentTime)
{
	std::vector<uint8_t> data(MAX_MSGLEN);
	FixedDataMessageStream stream(data.data(), data.size());
	MSG msg(stream, msgMode_e::Writing);

	usercmd_t nullcmd;
	usercmd_t* oldcmd = &nullcmd;

	// Write the server id
	msg.WriteUInteger(serverId);

	// Write the server sequence
	msg.WriteUInteger(serverMessageSequence);
	msg.WriteUInteger(serverCommandSequence);

	// FIXME: write any unacknowledged clientCommands
	for (int32_t i = reliableAcknowledge + 1; i <= reliableSequence; ++i)
	{
		msg.WriteByte(clc_ops_e::ClientCommand);
		msg.WriteInteger(i);
		writeStringMessage(msg, reliableCommands[i & (MAX_RELIABLE_COMMANDS - 1)]);
	}

	const uint32_t oldPacketNum = (getNetchan()->getOutgoingSequence() - 1) & PACKET_MASK;
	uint32_t count = cmdNumber - outPackets[oldPacketNum].p_cmdNumber;

	if (count > MAX_PACKET_USERCMDS)
	{
		count = MAX_PACKET_USERCMDS;
		// FIXME: message?
	}

	if (count >= 1)
	{
		uint8_t cmdOp;
		//if (currentSnap.valid && serverMessageSequence == currentSnap.messageNum) {
		//	cmdNum = clc_ops_e::Move;
		//}
		//else {
		cmdOp = clc_ops_e::MoveNoDelta;
		//}

		// Write the command number
		msg.WriteByte(cmdOp);

		// Write the number of commands
		msg.WriteByte(count);

		// Write delta eyes
		msg.WriteDeltaClass(&SerializableUserEyes(outPackets[oldPacketNum].p_eyeinfo), &SerializableUserEyes(userEyes));

		uint32_t key = checksumFeed;
		// also use the message acknowledge
		key ^= serverMessageSequence;
		// also use the last acknowledged server command in the key
		key ^= hashKey(serverCommands[serverCommandSequence & (MAX_RELIABLE_COMMANDS - 1)], 32);

		// write all the commands, including the predicted command
		for (size_t i = 0; i < count; i++)
		{
			const size_t j = (cmdNumber - count + i + 1) & CMD_MASK;
			usercmd_t* cmd = &cmds[j];
			// Write delta cmd
			msg.WriteDeltaClass(&SerializableUsercmd(*oldcmd), &SerializableUsercmd(*cmd), key);
			oldcmd = cmd;
		}
	}

	// Retrieve the time
	const uint32_t packetNum = getNetchan()->getOutgoingSequence() & PACKET_MASK;
	outPackets[packetNum].p_currentTime = currentTime;
	outPackets[packetNum].p_serverTime = oldcmd->serverTime;
	outPackets[packetNum].p_cmdNumber = cmdNumber;
	outPackets[packetNum].p_eyeinfo = userEyes;

	// Write end of command
	msg.WriteByte(clc_ops_e::eof);

	// End of transmission
	msg.WriteByte(clc_ops_e::eof);

	// Flush out pending data
	msg.Flush();

	encoder->encode(msg.stream(), msg.stream());
	
	// Transmit the encoded message
	getNetchan()->transmit(adr, msg.stream());

	lastPacketSendTime = currentTime;
}

StringMessage ClientGameConnection::readStringMessage(MSG& msg)
{
	return (*readStringMessage_pf)(msg);
}

void ClientGameConnection::writeStringMessage(MSG& msg, const char* s)
{
	return (*writeStringMessage_pf)(msg, s);
}

uint32_t ClientGameConnection::hashKey(const char* string, size_t maxlen)
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

MOHPC::StringMessage ClientGameConnection::readStringMessage_normal(MSG& msg)
{
	return msg.ReadString();
}

void ClientGameConnection::writeStringMessage_normal(MSG& msg, const char* s)
{
	msg.WriteString(s);
}

MOHPC::StringMessage ClientGameConnection::readStringMessage_scrambled(MSG& msg)
{
	return msg.ReadScrambledString(byteCharMapping);
}

void ClientGameConnection::writeStringMessage_scrambled(MSG& msg, const char* s)
{
	msg.WriteScrambledString(s, charByteMapping);
}

uint32_t ClientGameConnection::hashKey_ver6(const char* string, size_t maxlen)
{
	uint32_t hash = 0;

	for (size_t i = 0; i < maxlen && string[i]; i++) {
		hash += string[i] * (119 + (uint32_t)i);
	}

	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	return hash;
}

uint32_t ClientGameConnection::hashKey_ver15(const char* string, size_t maxlen)
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
	// FIXME: Should it be stored somewhere?
	const float deltaTime = msg.ReadFloat();
}

void ClientGameConnection::readDeltaPlayerstate_ver6(MSG& msg, const playerState_t* from, playerState_t* to)
{
	msg.ReadDeltaClass(from ? &SerializablePlayerState(*const_cast<playerState_t*>(from)) : nullptr, &SerializablePlayerState(*to));
}

void ClientGameConnection::readDeltaPlayerstate_ver15(MSG& msg, const playerState_t* from, playerState_t* to)
{
	msg.ReadDeltaClass(from ? &SerializablePlayerState_ver17(*const_cast<playerState_t*>(from)) : nullptr, &SerializablePlayerState_ver17(*to));
}

void ClientGameConnection::readDeltaEntity_ver6(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum)
{
	msg.ReadDeltaClass(from ? &SerializableEntityState(*const_cast<entityState_t*>(from), newNum) : nullptr, &SerializableEntityState(*to, newNum));
}

void ClientGameConnection::readDeltaEntity_ver15(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum)
{
	msg.ReadDeltaClass(from ? &SerializableEntityState_ver15(*const_cast<entityState_t*>(from), newNum) : nullptr, &SerializableEntityState_ver15(*to, newNum));
}

const gameState_t& MOHPC::Network::ClientGameConnection::getGameState() const
{
	return gameState;
}

ConstClientInfoPtr MOHPC::Network::ClientGameConnection::getUserInfo() const
{
	return userInfo;
}

const ClientInfoPtr& MOHPC::Network::ClientGameConnection::getUserInfo()
{
	return userInfo;
}

void MOHPC::Network::ClientGameConnection::updateUserInfo()
{
	Info info;
	userInfo->fillInfoString(info);
	// Send the new user info.
	addReliableCommand(str::printf("userinfo \"%s\"", info.GetString()));
}

uint32_t MOHPC::Network::ClientGameConnection::getMaxPackets() const
{
	return maxPackets;
}

void MOHPC::Network::ClientGameConnection::setMaxPackets(uint32_t inMaxPackets)
{
	maxPackets = inMaxPackets;
	if (maxPackets < 1) {
		maxPackets = 1;
	}
	else if (maxPackets > 125) {
		maxPackets = 125;
	}
}

uint32_t MOHPC::Network::ClientGameConnection::getMaxTickPackets() const
{
	return maxTickPackets;
}

void MOHPC::Network::ClientGameConnection::setMaxTickPackets(uint32_t inMaxPackets)
{
	maxTickPackets = inMaxPackets;
	if (maxTickPackets < 1) {
		maxTickPackets = 1;
	}
	else if (maxTickPackets > 1000) {
		maxTickPackets = 1000;
	}
}

void MOHPC::Network::ClientGameConnection::initTime(uint64_t currentTime)
{
	realTimeStart = currentTime;

	using namespace std::chrono;
	lastTimeoutTime = steady_clock::time_point(milliseconds(currentTime));
}

uintptr_t MOHPC::Network::ClientGameConnection::getCurrentSnapshotNumber() const
{
	return currentSnap.messageNum;
}

bool MOHPC::Network::ClientGameConnection::getSnapshot(uintptr_t snapshotNum, SnapshotInfo& outSnapshot) const
{
	// if the frame has fallen out of the circular buffer, we can't return it
	if (currentSnap.messageNum - snapshotNum >= PACKET_BACKUP) {
		return false;
	}

	// if the frame is not valid, we can't return it
	const ClientSnapshot* foundSnap = &snapshots[snapshotNum & PACKET_MASK];
	if (!foundSnap->valid) {
		return false;
	}

	// if the entities in the frame have fallen out of their
	// circular buffer, we can't return it
	if (parseEntitiesNum - foundSnap->parseEntitiesNum >= MAX_PARSE_ENTITIES) {
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

uint64_t MOHPC::Network::ClientGameConnection::getServerStartTime() const
{
	return serverStartTime;
}

uint64_t MOHPC::Network::ClientGameConnection::getServerTime() const
{
	return serverTime;
}

uint64_t MOHPC::Network::ClientGameConnection::getServerFrameFrequency() const
{
	return serverDeltaFrequency;
}

uintptr_t MOHPC::Network::ClientGameConnection::getCurrentCmdNumber()
{
	return cmdNumber;
}

bool MOHPC::Network::ClientGameConnection::getUserCmd(uintptr_t cmdNum, usercmd_t& outCmd)
{
	// the usercmd has been overwritten in the wrapping
	// buffer because it is too far out of date
	if (cmdNum < this->cmdNumber - CMD_BACKUP) {
		return false;
	}

	outCmd = cmds[cmdNum & CMD_MASK];
	return true;
}

bool MOHPC::Network::ClientGameConnection::getServerCommand(uintptr_t serverCommandNumber, TokenParser& tokenized)
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

void MOHPC::Network::ClientGameConnection::disconnect()
{
	if(getNetchan())
	{
		addReliableCommand("disconnect");

		for (size_t i = 0; i < 3; ++i) {
			writePacket(serverMessageSequence, 0);
		}

		// Network channel is not needed anymore
		wipeChannel();
	}

	// Terminate the connection
	terminateConnection(nullptr);
}

void MOHPC::Network::ClientGameConnection::serverDisconnected(const char* reason)
{
	wipeChannel();

	// Terminate after wiping channel
	terminateConnection(reason);
}

void MOHPC::Network::ClientGameConnection::terminateConnection(const char* reason)
{
	// Clear out the server id
	serverId = 0;

	if (cgameModule)
	{
		handlerList.notify<ClientHandlers::Disconnect>(reason);

		// Delete the CG module
		delete cgameModule;
		cgameModule = nullptr;
	}
}

void MOHPC::Network::ClientGameConnection::configStringModified(uint16_t num, const char* newString)
{
	// Set the config string in gamestate
	gameState.setConfigString(num, newString, strlen(newString));

	// Notify the client module about cs modification
	if(cgameModule) cgameModule->configStringModified(num);

	// Notify about the change
	handlerList.notify<ClientHandlers::Configstring>(num, newString);
}

void MOHPC::Network::ClientGameConnection::wipeChannel()
{
	netchan = nullptr;
}

bool MOHPC::Network::ClientGameConnection::isChannelValid() const
{
	return netchan != nullptr;
}

void MOHPC::Network::ClientGameConnection::setCGameTime(uint64_t currentTime)
{
	if(!serverId)
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

	oldFrameServerTime = currentSnap.serverTime;

	int32_t tn = 10;
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

	if (realServerTime >= currentSnap.serverTime - 5) {
		extrapolatedSnapshot = true;
	}

	if(newSnapshots) {
		adjustTimeDelta(realTime);
	}
}

void MOHPC::Network::ClientGameConnection::adjustTimeDelta(uint64_t realTime)
{
	newSnapshots = false;

	constexpr size_t BASE_RESET_TIME = 400;

	const uint64_t resetTime = BASE_RESET_TIME + serverDeltaFrequency;
	uint64_t deltaDelta = abs((int64_t)(currentSnap.serverTime - serverTime));

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

void MOHPC::Network::ClientGameConnection::firstSnapshot(uint64_t currentTime)
{
	if (currentSnap.snapFlags & SNAPFLAG_NOT_ACTIVE) {
		return;
	}

	serverStartTime = currentSnap.serverTime;
	serverTime = (uint32_t)(currentSnap.serverTime - currentTime);
	isActive = true;

	// Notify about the snapshot
	getHandlerList().notify<ClientHandlers::FirstSnapshot>(currentSnap);
}

bool MOHPC::Network::ClientGameConnection::readyToSendPacket(uint64_t currentTime) const
{
	if (!cgameModule)
	{
		// Disconnected from server
		return false;
	}

	if (!serverId)
	{
		// Send one packet per second when loading
		return currentTime >= lastPacketSendTime + 1000;
	}

	const size_t oldPacketNum = (getNetchan()->getOutgoingSequence() - 1) & PACKET_MASK;
	const uint64_t delta = currentTime - outPackets[oldPacketNum].p_currentTime;
	if (delta < 1000 / maxPackets) {
		return false;
	}

	return true;
}

cs_t MOHPC::Network::ClientGameConnection::getNormalizedConfigstring(cs_t num)
{
	return (this->*getNormalizedConfigstring_pf)(num);
}

MOHPC::Network::CGameModuleBase& Network::ClientGameConnection::getCGModule()
{
	return *cgameModule;
}

cs_t ClientGameConnection::getNormalizedConfigstring_ver6(cs_t num)
{
	if (num <= CS_WARMUP || num >= 26) {
		return num;
	}

	return num - 2;
}

cs_t ClientGameConnection::getNormalizedConfigstring_ver15(cs_t num)
{
	return num;
}

void MOHPC::Network::gameState_t::setConfigString(size_t num, const char* configString, size_t len)
{
	const size_t newSz = dataCount + len;
	if (newSz > MAX_GAMESTATE_CHARS) {
		throw MaxGameStateCharsException(newSz);
	}

	stringOffsets[num] = dataCount;
	memcpy(stringData + dataCount, configString, len + 1);
	dataCount += len + 1;
}

MOHPC_OBJECT_DEFINITION(ClientInfo);

ClientInfo::ClientInfo()
	: snaps(20)
	, rate(5000)
{
}

void MOHPC::Network::ClientInfo::setRate(uint32_t inRate)
{
	rate = inRate;
}

uint32_t MOHPC::Network::ClientInfo::getRate() const
{
	return rate;
}

void MOHPC::Network::ClientInfo::setSnaps(uint32_t inSnaps)
{
	snaps = inSnaps;
}

uint32_t MOHPC::Network::ClientInfo::getSnaps() const
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

void MOHPC::Network::ClientInfo::setUserKeyValue(const char* key, const char* value)
{
	properties.SetPropertyValue(key, value);
}

const char* MOHPC::Network::ClientInfo::getUserKeyValue(const char* key) const
{
	return properties.GetPropertyRawValue(key);
}

void ClientInfo::fillInfoString(Info& info) const
{
	// Build mandatory variables
	info.SetValueForKey("rate", str::printf("%i", rate));
	info.SetValueForKey("snaps", str::printf("%i", snaps));
	info.SetValueForKey("name", name.c_str());

	// Build miscellaneous values
	for (PropertyMapIterator it = properties.GetIterator(); it; ++it)
	{
		info.SetValueForKey(
			it.key().GetFullPropertyName(),
			it.value()
		);
	}
}

