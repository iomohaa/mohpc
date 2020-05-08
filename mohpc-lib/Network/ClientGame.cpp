#include <MOHPC/Network/ClientGame.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Network/Channel.h>
#include <MOHPC/Network/SerializableTypes.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Log.h>
#include <typeinfo>
#include <filesystem>
#include <fstream>

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
	, number_of_sounds(0)
{
}

MOHPC::Network::ClientGameConnection::ClientGameConnection(const INetchanPtr& inNetchan, const netadr_t& inAdr, uint32_t challengeResponse, protocolVersion_e protocolVersion)
	: netchan(inNetchan)	
	, adr(inAdr)
	, parseEntitiesNum(0)
	, serverCommandSequence(0)
	, cmdNumber(0)
	, clientNum(0)
	, checksumFeed(0)
	, serverId(0)
	, downloadedBlock(0)
	, reliableSequence(0)
	, reliableAcknowledge(0)
	, reliableCommands{ nullptr }
	, serverCommands{ nullptr }
	, reliableCmdStrings{ 0 }
	, serverCmdStrings{ 0 }
{
	switch (protocolVersion)
	{
	case 6:
		// FIXME: Unimplemented
		break;
	case 8:
		readStringMessage_pf = &ClientGameConnection::readStringMessage_normal;
		writeStringMessage_pf = &ClientGameConnection::writeStringMessage_normal;
		parseGameState_pf = &ClientGameConnection::parseGameState_ver8;
		hashKey_pf = &ClientGameConnection::hashKey_ver8;
		readEntityNum_pf = &ClientGameConnection::readEntityNum_ver8;
		readDeltaPlayerstate_pf = &ClientGameConnection::readDeltaPlayerstate_ver8;
		readDeltaEntity_pf = &ClientGameConnection::readDeltaEntity_ver8;
		break;
	case 15:
		// FIXME: Unimplemented
		break;
	case 17:
		readStringMessage_pf = &ClientGameConnection::readStringMessage_scrambled;
		writeStringMessage_pf = &ClientGameConnection::writeStringMessage_scrambled;
		parseGameState_pf = &ClientGameConnection::parseGameState_ver17;
		hashKey_pf = &ClientGameConnection::hashKey_ver17;
		readEntityNum_pf = &ClientGameConnection::readEntityNum_ver17;
		readDeltaPlayerstate_pf = &ClientGameConnection::readDeltaPlayerstate_ver17;
		readDeltaEntity_pf = &ClientGameConnection::readDeltaEntity_ver17;
		break;
	default:
		throw BadProtocolVersionException((uint8_t)protocolVersion);
		break;
	}

	for (size_t i = 0; i < MAX_RELIABLE_COMMANDS; ++i) {
		reliableCommands[i] = &reliableCmdStrings[i * MAX_STRING_CHARS];
		serverCommands[i] = &serverCmdStrings[i * MAX_STRING_CHARS];
	}

	encoder = std::make_shared<Encoding>(challengeResponse, (const char**)reliableCommands, (const char**)serverCommands);
}

MOHPC::Network::ClientGameConnection::~ClientGameConnection()
{
	addReliableCommand("disconnect");

	for (size_t i = 0; i < 3; ++i) {
		writePacket(serverMessageSequence);
	}
}

void MOHPC::Network::ClientGameConnection::tick(uint64_t deltaTime, uint64_t currentTime)
{
	if (timeoutTime > 0)
	{
		using namespace std::chrono;
		time_point<steady_clock> clockTime = steady_clock::now();
		if (clockTime >= nextTimeoutTime)
		{
			// The server or the client timed out
			onTimedOut();
		}
	}

	IUdpSocket* socket = getNetchan()->getRawSocket();
	if (socket->wait(0))
	{
		std::vector<uint8_t> data(65536);
		FixedDataMessageStream stream(data.data(), data.size());

		netadr_t from;
		socket->receive(data.data(), data.size(), from);

		// Prepare for reading
		MSG msg(stream, msgMode_e::Reading);
		receive(from, msg);
	}
}

void MOHPC::Network::ClientGameConnection::setTimeout(size_t inTimeoutTime, Callbacks::ClientGameTimeout&& callback)
{
	timeoutTime = inTimeoutTime;
	timeoutCallback = std::move(callback);
}

MOHPC_EXPORTS void MOHPC::Network::ClientGameConnection::setErrorHandler(Callbacks::ErrorHandler&& handler)
{

}

const MOHPC::Network::INetchanPtr& MOHPC::Network::ClientGameConnection::getNetchan() const
{
	return netchan;
}

void MOHPC::Network::ClientGameConnection::onTimedOut()
{
	// Call the timeout callback
	if (timeoutCallback) timeoutCallback();
}

void MOHPC::Network::ClientGameConnection::onError(const NetworkException& exception)
{
	if (errorCallback) errorCallback(exception);
}

void Network::ClientGameConnection::receive(const netadr_t& from, MSG& msg)
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
			parseServerMessage(msg, serverMessageSequence);
		}
		catch (NetworkException& e)
		{
			MOHPC_LOG(Error, "got exception of type %s: \"%s\"", typeid(e).name(), e.what().c_str());

			// Call the handler
			onError(e);
		}

		using namespace std::chrono;
		nextTimeoutTime = steady_clock::now() + milliseconds(timeoutTime);

		// Build and send client commands
		sendCmd();
		writePacket(serverMessageSequence);
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

void Network::ClientGameConnection::parseServerMessage(MSG& msg, uint32_t serverMessageSequence)
{
	msg.SetCodec(MessageCodecs::Bit);

	// Read the ack
	reliableAcknowledge = msg.ReadInteger();
	if (reliableAcknowledge < reliableSequence - (int32_t)MAX_RELIABLE_COMMANDS) {
		reliableAcknowledge = reliableSequence;
	}

	for (;;)
	{
		uint8_t cmdNum = msg.ReadByte();

		svc_ops_e cmd = (svc_ops_e)cmdNum;
		if (cmd == svc_ops_e::eof) {
			break;
		}

		switch (cmd)
		{
		case svc_ops_e::nop:
			break;
		case svc_ops_e::serverCommand:
			parseCommandString(msg);
			break;
		case svc_ops_e::gamestate:
			parseGameState(msg);
			break;
		case svc_ops_e::snapshot:
			parseSnapshot(msg, serverMessageSequence);
			break;
		case svc_ops_e::download:
			parseDownload(msg);
			break;
		case svc_ops_e::centerprint:
			parseCenterprint(msg);
			break;
		case svc_ops_e::locprint:
			parseLocprint(msg);
			break;
		case svc_ops_e::cgameMessage:
			parseCGMessage(msg);
			return;
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
	nullState->parent = entityState_t::ENTITYNUM_NONE;
	nullState->tag_num = -1;
	nullState->constantLight = -1;
	nullState->renderfx = 16;
	nullState->bone_tag[4] = -1;
	nullState->bone_tag[3] = -1;
	nullState->bone_tag[2] = -1;
	nullState->bone_tag[1] = -1;
	nullState->bone_tag[0] = -1;
}

void Network::ClientGameConnection::parseCommandString(MSG& msg)
{
	const uint32_t seq = msg.ReadUInteger();

	StringMessage s = readStringMessage(msg);

	// check if it is already stored
	if (serverCommandSequence >= seq) {
		return;
	}

	serverCommandSequence = seq;

	const uint32_t index = seq & (MAX_RELIABLE_COMMANDS - 1);

	serverCommands[index] = &serverCmdStrings[MAX_STRING_CHARS * index];
	strncpy(serverCommands[index], s, sizeof(serverCmdStrings[0]) * MAX_STRING_CHARS);
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
		if (cmd == svc_ops_e::eof) {
			break;
		}

		switch (cmd)
		{
		case svc_ops_e::configstring:
		{
			const uint16_t stringNum = msg.ReadUShort();

			if (stringNum > MAX_CONFIGSTRINGS) {
				throw MaxConfigStringException(stringNum);
			}

			const StringMessage stringValue = readStringMessage(msg);
			const size_t sz = strlen(stringValue) + 1;

			const size_t newSz = gameState.dataCount + sz;
			if (newSz > gameState.MAX_GAMESTATE_CHARS) {
				throw MaxGameStateCharsException(newSz);
			}

			gameState.stringOffsets[stringNum] = gameState.dataCount;
			memcpy(gameState.stringData + gameState.dataCount, stringValue, sz);
			gameState.dataCount += sz;
		}
		break;
		case svc_ops_e::baseline:
		{
			const uint16_t newNum = readEntityNum(msgHelper);
			if (newNum >= MAX_GENTITIES) {
				throw BaselineOutOfRangeException(newNum);
			}

			entityState_t nullState;
			GetNullEntityState(&nullState);

			entityState_t& es = entityBaselines[newNum];
			readDeltaEntity(msg, &nullState, &es);
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
}

void Network::ClientGameConnection::parseSnapshot(MSG& msg, uint32_t serverMessageSequence)
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
	readDeltaPlayerstate(msg, old ? &old->ps : nullptr, &newSnap.ps);

	// Read all entities in this snap
	parsePacketEntities(msg, old, &newSnap);

	// Parse sounds
	parseSounds(msg);

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

	if (currentSnap.valid && (newSnap.snapFlags ^ newSnap.snapFlags) & ClientSnapshot::SNAPFLAG_SERVERCOUNT) {
		serverStartTime = newSnap.serverTime;
	}

	currentSnap = newSnap;
	// FIXME: ping ?

	// FIXME: calculate ping time
	uint16_t ping = 0;
	for (size_t i = 0; i < PACKET_BACKUP; ++i)
	{
		const uintptr_t packetNum = (getNetchan()->getOutgoingSequence() - 1 - i) & PACKET_MASK;
		if (currentSnap.ps.commandTime >= outPackets[packetNum].p_serverTime) {
			//ping = cls.realtime - cl.outPackets[packetNum].p_realtime;
			break;
		}
	}

	snapshots[currentSnap.messageNum & PACKET_MASK] = currentSnap;
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
	else {
		if (oldIndex >= oldFrame->numEntities) {
			oldNum = 99999;
		}
		else {
			oldState = &parseEntities[(oldFrame->parseEntitiesNum + oldIndex) & (MAX_PARSE_ENTITIES - 1)];
			oldNum = oldState->number;
		}
	}

	for (;;)
	{
		// SH/BT use this
		const uint16_t newNum = readEntityNum(msgHelper);

		if (newNum == (MAX_GENTITIES - 1)) {
			break;
		}

		// FIXME: throw if end of message

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
		else {
			oldState = &parseEntities[(oldFrame->parseEntitiesNum + oldIndex) & (MAX_PARSE_ENTITIES - 1)];
			oldNum = oldState->number;
		}
	}
}

void Network::ClientGameConnection::parseDeltaEntity(MSG& msg, ClientSnapshot* frame, uint32_t newNum, entityState_t* old, bool unchanged)
{
	entityState_t* state = &parseEntities[parseEntitiesNum & (MAX_PARSE_ENTITIES - 1)];

	if (unchanged) {
		*state = *old;
	}
	else
	{
		state->number = newNum;
		readDeltaEntity(msg, old, state);
	}

	if (state->number == (MAX_GENTITIES - 1))
	{
		// entity was delta removed
		return;
	}

	++parseEntitiesNum;
	frame->numEntities++;
}

void Network::ClientGameConnection::parseSounds(MSG& msg)
{
	const bool hasSounds = msg.ReadBool();
	if (!hasSounds) {
		return;
	}

	uint8_t numSounds = 0;
	msg.ReadBits(&numSounds, 7);
	if (numSounds > 64) {
		return;
	}

	MsgTypesHelper msgHelper(msg);

	// FIXME: set number of sounds and assign them
	for (size_t i = 0; i < numSounds; ++i)
	{
		const bool stopped = msg.ReadBool();

		if (stopped)
		{
			uint16_t entityNum = 0;
			msg.ReadBits(&entityNum, 10);

			uint8_t channel;
			msg.ReadBits(&channel, 7);
		}
		else
		{
			const bool streamed = msg.ReadBool();
			const bool is3D = msg.ReadBool();
			if (is3D) {
				const Vector origin = msgHelper.ReadVectorCoord();
			}
			else {
				// FIXME
			}

			uint16_t entityNum = 0;
			msg.ReadBits(&entityNum, 11);

			uint8_t channel = 0;
			msg.ReadBits(&channel, 7);

			uint16_t soundIndex = 0;
			msg.ReadBits(&soundIndex, 9);

			const bool hasVolume = msg.ReadBool();
			if (hasVolume) {
				float volume = msg.ReadFloat();
			}
			else {
				// FIXME : volume = -1.f
			}

			const bool hasDist = msg.ReadBool();
			if (hasDist) {
				float minDist = msg.ReadFloat();
			}
			else {
				// FIXME : min_dist = -1.f
			}

			const bool hasPitch = msg.ReadBool();
			if (hasPitch)
			{
				float pitch = msg.ReadFloat();
			}
			else {
				// FIXME : pitch = -1.f
			}

			float maxDist = msg.ReadFloat();
		}
	}
}

void Network::ClientGameConnection::parseCenterprint(MSG& msg)
{
	StringMessage s = readStringMessage(msg);

	// FIXME
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

void Network::ClientGameConnection::parseLocprint(MSG& msg)
{
	const uint16_t x = msg.ReadUShort();
	const uint16_t y = msg.ReadUShort();

	StringMessage string = readStringMessage(msg);

	// FIXME: print on the screen
}

void Network::ClientGameConnection::parseCGMessage(MSG& msg)
{
	// FIXME: Should be fixed for SH/BT
	uint8_t temp;
	uint8_t count;
	uint8_t index;
	bool large = false;
	bool hasMessage = false;
	StringMessage strVal;
	Vector vecTmp;
	Vector vecStart, vecEnd;
	Vector vecArray[64];
	MsgTypesHelper msgHelper(msg);

	do
	{
		uint8_t msgType = 0;
		msg.ReadBits(&msgType, 6);

		switch (msgType)
		{
		case 1:
			vecTmp = msgHelper.ReadVectorCoord();
		case 2:
		case 5:
		{
			vecStart = msgHelper.ReadVectorCoord();
			vecTmp = vecStart;
			vecArray[0] = msgHelper.ReadVectorCoord();
			large = msg.ReadBool();

			// FIXME: create bullet tracer
			break;
		}
		case 3:
		{
			vecTmp = msgHelper.ReadVectorCoord();
			temp = 0;
			msg.ReadBits(&temp, 6);
		}
		case 4:
			if (msgType == 4) {
				temp = 0;
			}

			vecStart = msgHelper.ReadVectorCoord();
			large = msg.ReadBool();
			msg.ReadBits(&count, 6);

			for (size_t i = 0; i < count; ++i) {
				vecArray[i] = msgHelper.ReadVectorCoord();
			}
			// FIXME: bullet tracer
			break;
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			vecStart = msgHelper.ReadVectorCoord();
			vecEnd = msgHelper.ReadDir();
			large = msg.ReadBool();
			// FIXME do it
			break;
		case 11:
			vecStart = msgHelper.ReadVectorCoord();
			vecEnd = msgHelper.ReadVectorCoord();
			// FIXME: melee impact
			break;
		case 12:
		case 13:
			vecStart = msgHelper.ReadVectorCoord();
			// FIXME: grenade explosion
			break;
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
			vecStart = msgHelper.ReadVectorCoord();
			vecEnd = msgHelper.ReadDir();
			// FIXME: make effect
			break;
		case 23:
		case 24:
			vecStart = msgHelper.ReadVectorCoord();
			temp = msg.ReadByte();
			// FIXME: spawn effect
			break;
		case 25:
			vecTmp = msgHelper.ReadVectorCoord();
			vecStart = msgHelper.ReadVectorCoord();
			vecArray[0] = msgHelper.ReadVectorCoord();
			large = msg.ReadBool();
			// FIXME: make bullet tracer
			break;
		case 26:
			vecTmp = vec_zero;
			vecStart = msgHelper.ReadVectorCoord();
			vecArray[0] = msgHelper.ReadVectorCoord();
			large = msg.ReadBool();
			// FIXME: make bullet tracer
			break;
		case 27:
			index = msg.ReadByte();
			strVal = readStringMessage(msg);
			// FIXME: hud draw
			break;
		case 28:
		{
			index = msg.ReadByte();
			uint8_t hAlign = 0, vAlign = 0;
			msg.ReadBits(&hAlign, 2);
			msg.ReadBits(&vAlign, 2);
			// FIXME: hud draw
			break;
		}
		case 29:
		{
			index = msg.ReadByte();

			const uint16_t x = msg.ReadUShort();
			const uint16_t y = msg.ReadUShort();
			const uint16_t width = msg.ReadUShort();
			const uint16_t height = msg.ReadUShort();
			// FIXME: hud draw
			break;
		}
		case 30:
		{
			index = msg.ReadByte();
			const bool virtualScreen = msg.ReadBool();
			// FIXME: hud draw
			break;
		}
		case 31:
		{
			index = msg.ReadByte();
			const uint8_t col[3] =
			{
				msg.ReadByte(),
				msg.ReadByte(),
				msg.ReadByte()
			};

			// Divide by 255 to get float color
			// FIXME: hud draw
			break;
		}
		case 32:
		{
			index = msg.ReadByte();
			const uint8_t alpha = msg.ReadByte();
			// FIXME: hud draw
			break;
		}
		case 33:
		{
			index = msg.ReadByte();
			strVal = readStringMessage(msg);
			// FIXME: hud draw
			break;
		}
		case 34:
		{
			index = msg.ReadByte();
			strVal = readStringMessage(msg);
			// FIXME: hud draw
			break;
		}
		case 35:
		case 36:
			// FIXME: play sound
			break;
		case 37:
			vecStart = msgHelper.ReadVectorCoord();

			msg.ReadBits(&temp, 1);
			msg.ReadBits(&index, 6);
			strVal = readStringMessage(msg);

			// FIXME: play sound
			break;

		}
		hasMessage = msg.ReadBool();
	} while (hasMessage);
}


// an info string with all the serverinfo cvars
static constexpr size_t CS_SERVERINFO = 0;
// an info string for server system to client system configuration (timescale, etc)
static constexpr size_t CS_SYSTEMINFO = 1;
// game can't modify below this, only the system can
static constexpr size_t RESERVED_CONFIGSTRINGS = 2;

void Network::ClientGameConnection::systemInfoChanged()
{
	const char* systemInfo = gameState.stringData + gameState.stringOffsets[CS_SYSTEMINFO];

	ReadOnlyInfo info(systemInfo);

	serverId = atoi(info.ValueForKey("sv_serverid"));
}

void Network::ClientGameConnection::createNewCommands()
{
	++cmdNumber;
	const uint32_t cmdNum = cmdNumber & CMD_MASK;
	cmds[cmdNum] = createCmd();
}

usercmd_t Network::ClientGameConnection::createCmd()
{
	usercmd_t cmd;

	cmd.buttons = 4;
	cmd.forwardmove = 127;
	//cmd.upmove = 127;

	cmd.serverTime = currentSnap.serverTime;

	return cmd;
}

void Network::ClientGameConnection::sendCmd()
{
	if (!serverId) {
		return;
	}

	createNewCommands();
}

void Network::ClientGameConnection::writePacket(uint32_t serverMessageSequence)
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
	//msg.WriteUInteger(-63);

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
		uint8_t cmdNum;
		//if (currentSnap.valid && serverMessageSequence == currentSnap.messageNum) {
		//	cmdNum = clc_ops_e::Move;
		//}
		//else {
		cmdNum = clc_ops_e::MoveNoDelta;
		//}

		msg.WriteByte(cmdNum);

		uint8_t numCmd = (uint8_t)count;
		msg.WriteByte(numCmd);

		usereyes_t eyeInfo;
		msg.WriteDeltaClass(&SerializableUserEyes(outPackets[oldPacketNum].p_eyeinfo), &SerializableUserEyes(eyeInfo));

		uint32_t key = checksumFeed;
		// also use the message acknowledge
		key ^= serverMessageSequence;
		// also use the last acknowledged server command in the key
		key ^= hashKey(serverCommands[serverCommandSequence & (MAX_RELIABLE_COMMANDS - 1)], 32);

		// write all the commands, including the predicted command
		for (size_t i = 0; i < count; i++)
		{
			size_t j = (cmdNumber - count + i + 1) & CMD_MASK;
			usercmd_t* cmd = &cmds[j];
			msg.WriteDeltaClass(&SerializableUsercmd(*oldcmd), &SerializableUsercmd(*cmd), key);
			oldcmd = cmd;
		}
	}

	const uint32_t packetNum = getNetchan()->getOutgoingSequence() & PACKET_MASK;
	outPackets[packetNum].p_realtime = 0;
	outPackets[packetNum].p_serverTime = oldcmd->serverTime;
	outPackets[packetNum].p_cmdNumber = cmdNumber;
	//outPackets[packetNum].p_eyeinfo = realTime;

	// Write end of command
	msg.WriteByte(clc_ops_e::eof);

	// End of transmission
	msg.WriteByte(clc_ops_e::eof);

	msg.Flush();

	encoder->encode(msg.stream(), msg.stream());
	
	// Transmit the encoded message
	getNetchan()->transmit(adr, msg.stream());
}

StringMessage MOHPC::Network::ClientGameConnection::readStringMessage(MSG& msg)
{
	return (this->*readStringMessage_pf)(msg);
}

void MOHPC::Network::ClientGameConnection::writeStringMessage(MSG& msg, const char* s)
{
	return (this->*writeStringMessage_pf)(msg, s);
}

uint32_t MOHPC::Network::ClientGameConnection::hashKey(const char* string, size_t maxlen)
{
	return (this->*hashKey_pf)(string, maxlen);
}

uint32_t MOHPC::Network::ClientGameConnection::readEntityNum(MsgTypesHelper& msgHelper)
{
	return (this->*readEntityNum_pf)(msgHelper);
}

void MOHPC::Network::ClientGameConnection::readDeltaPlayerstate(MSG& msg, playerState_t* from, playerState_t* to)
{
	(this->*readDeltaPlayerstate_pf)(msg, from, to);
}

void MOHPC::Network::ClientGameConnection::readDeltaEntity(MSG& msg, entityState_t* from, entityState_t* to)
{
	(this->*readDeltaEntity_pf)(msg, from, to);
}

MOHPC::StringMessage MOHPC::Network::ClientGameConnection::readStringMessage_normal(MSG& msg)
{
	return msg.ReadString();
}

void MOHPC::Network::ClientGameConnection::writeStringMessage_normal(MSG& msg, const char* s)
{
	msg.WriteString(s);
}

MOHPC::StringMessage MOHPC::Network::ClientGameConnection::readStringMessage_scrambled(MSG& msg)
{
	return msg.ReadScrambledString(byteCharMapping);
}

void MOHPC::Network::ClientGameConnection::writeStringMessage_scrambled(MSG& msg, const char* s)
{
	msg.WriteScrambledString(s, charByteMapping);
}

uint32_t MOHPC::Network::ClientGameConnection::hashKey_ver8(const char* string, size_t maxlen)
{
	uint32_t hash = 0;

	for (size_t i = 0; i < maxlen && string[i]; i++) {
		hash += string[i] * (119 + (uint32_t)i);
	}

	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	return hash;
}

uint32_t MOHPC::Network::ClientGameConnection::hashKey_ver17(const char* string, size_t maxlen)
{
	uint32_t hash = 0;

	for (size_t i = 0; i < maxlen && string[i]; i++) {
		hash += string[i] * (119 + (uint32_t)i);
	}

	hash = (hash ^ ((hash ^ (hash >> 10)) >> 10));
	return hash;
}

uint32_t MOHPC::Network::ClientGameConnection::readEntityNum_ver8(MsgTypesHelper& msgHelper)
{
	return msgHelper.ReadEntityNum();
}

uint32_t MOHPC::Network::ClientGameConnection::readEntityNum_ver17(MsgTypesHelper& msgHelper)
{
	return msgHelper.ReadEntityNum2();
}

void MOHPC::Network::ClientGameConnection::parseGameState_ver8(MSG& msg)
{
}

void MOHPC::Network::ClientGameConnection::parseGameState_ver17(MSG& msg)
{
	// FIXME: Should it be stored somewhere?
	const float deltaTime = msg.ReadFloat();
}

void MOHPC::Network::ClientGameConnection::readDeltaPlayerstate_ver8(MSG& msg, playerState_t* from, playerState_t* to)
{
	msg.ReadDeltaClass(from ? &SerializablePlayerState(*from) : nullptr, &SerializablePlayerState(*to));
}

void MOHPC::Network::ClientGameConnection::readDeltaPlayerstate_ver17(MSG& msg, playerState_t* from, playerState_t* to)
{
	msg.ReadDeltaClass(from ? &SerializablePlayerState_ver17(*from) : nullptr, &SerializablePlayerState_ver17(*to));
}

void MOHPC::Network::ClientGameConnection::readDeltaEntity_ver8(MSG& msg, entityState_t* from, entityState_t* to)
{
	msg.ReadDeltaClass(from ? &SerializableEntityState(*from) : nullptr, &SerializableEntityState(*to));
}

void MOHPC::Network::ClientGameConnection::readDeltaEntity_ver17(MSG& msg, entityState_t* from, entityState_t* to)
{
	msg.ReadDeltaClass(from ? &SerializableEntityState_ver17(*from) : nullptr, &SerializableEntityState_ver17(*to));
}
