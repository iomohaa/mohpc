#include <MOHPC/Network/Client/ClientGame.h>
#include <MOHPC/Network/Client/CGModule.h>
#include <MOHPC/Network/Client/ProtocolParsing.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>
#include <MOHPC/Utility/Misc/MSG/Codec.h>
#include <MOHPC/Network/Channel.h>
#include <MOHPC/Network/SerializableTypes.h>
#include <MOHPC/Utility/Info.h>
#include <MOHPC/Utility/TokenParser.h>
#include <MOHPC/Common/Log.h>

#include <typeinfo>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cstring>

using namespace MOHPC;
using namespace MOHPC::Network;

static constexpr size_t MAX_MSGLEN = 49152;
static constexpr size_t MINIMUM_RECEIVE_BUFFER_SIZE = 1024;
static constexpr char MOHPC_LOG_NAMESPACE[] = "network_cgame";

class ClientSequenceTemplate_ver6 : public ClientSequenceTemplate<1024> {};
class ClientRemoteCommandSequenceTemplate_ver6 : public ClientRemoteCommandSequenceTemplate<1024> {};
class ClientSequenceTemplate_ver15 : public ClientSequenceTemplate<2048> {};
class ClientRemoteCommandSequenceTemplate_ver15 : public ClientRemoteCommandSequenceTemplate<2048> {};

outPacket_t::outPacket_t()
	: p_cmdNumber(0)
	, p_serverTime(0)
	, p_currentTime(0)
{

}

UserInput::UserInput()
	: cmdNumber(0)
{
}

void UserInput::reset()
{
	cmdNumber = 0;
}

void UserInput::createCommand(uint64_t currentTime, uint64_t remoteTime, usercmd_t*& outCmd, usereyes_t*& outEyes)
{
	++cmdNumber;
	const uint32_t cmdNum = cmdNumber % CMD_BACKUP;

	usercmd_t& cmd = cmds[cmdNum];
	cmd = usercmd_t((uint32_t)remoteTime);
	eyeinfo = usereyes_t();

	outCmd = &cmd;
	outEyes = &eyeinfo;
}

uint32_t UserInput::getCurrentCmdNumber() const
{
	return cmdNumber;
}

const MOHPC::usercmd_t& UserInput::getCommand(size_t index) const
{
	return cmds[index % CMD_BACKUP];
}

const usercmd_t& UserInput::getCommandFromLast(size_t index) const
{
	const size_t elem = cmdNumber - index;
	return getCommand(elem);
}

const usereyes_t& UserInput::getEyeInfo() const
{
	return eyeinfo;
}

usereyes_t& UserInput::getEyeInfo()
{
	return eyeinfo;
}

const usercmd_t& UserInput::getLastCommand() const
{
	return cmds[cmdNumber % CMD_BACKUP];
}

usercmd_t& UserInput::getLastCommand()
{
	return cmds[cmdNumber % CMD_BACKUP];
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

ClientTime::ClientTime()
	: realTimeStart(0)
	, serverStartTime(0)
	, serverTime(0)
	, oldServerTime(0)
	, oldFrameServerTime(0)
	, oldRealTime(0)
	, timeNudge(0)
{
}

ClientTime::~ClientTime()
{

}

void ClientTime::setTimeNudge(int32_t timeNudgeValue)
{
	timeNudge = timeNudgeValue;
}

void ClientTime::initRemoteTime(uint64_t currentTime, uint64_t remoteTimeValue)
{
	serverStartTime = remoteTimeValue;
	//serverTime = (uint32_t)(currentSnap.serverTime - currentTime);
	serverTime = serverStartTime + (currentTime - realTimeStart);
}

void ClientTime::setStartTime(uint64_t currentTime)
{
	realTimeStart = currentTime;
}

void ClientTime::setRemoteStartTime(uint64_t remoteTimeValue)
{
	serverStartTime = remoteTimeValue;
}

uint64_t ClientTime::getStartTime() const
{
	return realTimeStart;
}

uint64_t ClientTime::getRemoteStartTime() const
{
	return serverStartTime;
}

uint64_t ClientTime::getRemoteTime() const
{
	return serverTime;
}

void ClientTime::setTime(uint64_t newTime, uint64_t remoteTime, uint64_t remoteDeltaTime, bool adjust)
{
	// FIXME: throw if snap remote time went backward

	oldFrameServerTime = remoteTime;

	const uint64_t realTime = newTime;
	const uint64_t realServerTime = realTime - realTimeStart + oldServerTime;
	serverTime = realServerTime - timeNudge;
	oldRealTime = newTime;

	// guarantee that time will never flow backwards, even if
	// serverTimeDelta made an adjustment or cl_timeNudge was changed
	if (serverTime < oldServerTime) {
		serverTime = oldServerTime;
	}

	if (realServerTime + 5 >= remoteTime) {
		extrapolatedSnapshot = true;
	}

	if (adjust) {
		adjustTimeDelta(realTime, remoteTime, remoteDeltaTime);
	}
}

void ClientTime::adjustTimeDelta(uint64_t realTime, uint64_t remoteTime, uint64_t remoteDeltaTime)
{
	constexpr size_t BASE_RESET_TIME = 400;

	const uint64_t maxDeltaTime = remoteDeltaTime * 2;
	const uint64_t resetTime = BASE_RESET_TIME + maxDeltaTime;
	const uint64_t deltaDelta = getTimeDelta(serverTime, remoteTime);

	if (deltaDelta > resetTime)
	{
		realTimeStart = realTime;
		oldServerTime = remoteTime;
		serverTime = remoteTime;
	}
	else if (deltaDelta > maxDeltaTime)
	{
		// fast adjust
		realTimeStart += maxDeltaTime - deltaDelta;
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

uint64_t ClientTime::getTimeDelta(uint64_t time, uint64_t remoteTime) const
{
	if (remoteTime < time) {
		return time - remoteTime;
	}
	else {
		return remoteTime - time;
	}
}

MOHPC_OBJECT_DEFINITION(ClientGameConnection);

ClientGameConnection::ClientGameConnection(const INetchanPtr& inNetchan, const IRemoteIdentifierPtr& inAdr, uint32_t challengeResponse, const protocolType_c& protoType, const ClientInfoPtr& cInfo)
	: netchan(inNetchan)	
	, clientProto(getProtocol(protoType.getProtocolVersionNumber()))
	, adr(inAdr)
	, lastPacketSendTime(0)
	, timeout(std::chrono::milliseconds(60000))
	, parseEntitiesNum(0)
	, lastSnapFlags(0)
	, newSnapshots(false)
	, extrapolatedSnapshot(false)
	, isActive(false)
	, isReady(false)
	, userInfo(cInfo)
{
	ClientImports imports;
	fillClientImports(imports);

	downloadState.setImports(imports);

	const IGameStateInstancier* gameStateInstancier = IGameStateInstancier::get(protoType.getProtocolVersionNumber());
	if (gameStateInstancier)
	{
		// create the game state
		gameStatePtr = gameStateInstancier->createInstance();
	}

	switch (protoType.getProtocolVersion())
	{
	case protocolVersion_e::ver005:
	case protocolVersion_e::ver100:
	case protocolVersion_e::ver111:
		cgameModule = new CGameModule6(imports);
		reliableCommands = new ClientSequenceTemplate_ver6();
		serverCommands = new ClientRemoteCommandSequenceTemplate_ver6();
		break;
	case protocolVersion_e::ver200:
	case protocolVersion_e::ver211_demo:
	case protocolVersion_e::ver211:
		cgameModule = new CGameModule15(imports);
		reliableCommands = new ClientSequenceTemplate_ver15();
		serverCommands = new ClientRemoteCommandSequenceTemplate_ver15();
		break;
	default:
		throw ClientError::BadProtocolVersionException((uint8_t)protoType.getProtocolVersion());
		break;
	}

	encoder = std::make_shared<Encoding>(challengeResponse, *reliableCommands, *serverCommands);

	using namespace std::chrono;
	const steady_clock::time_point currentTime = steady_clock::now();
	clientTime.setStartTime(currentTime.time_since_epoch().count());

	timeout.update();
}

ClientGameConnection::~ClientGameConnection()
{
	disconnect();
	delete gameStatePtr;
	delete reliableCommands;
	delete serverCommands;
}

const IClientGameProtocol& ClientGameConnection::getProtocol(uint32_t protocolNum) const
{
	for(const IClientGameProtocol* proto = IClientGameProtocol::getHead(); proto; proto = proto->getNext())
	{
		if (proto->getProtocolVersion() == protocolNum)
		{
			// found it
			return *proto;
		}
	}

	// unsupported protocol
	throw ClientError::BadProtocolVersionException(protocolNum);
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

	if (timeout.hasTimedOut())
	{
		// The server or the client has timed out
		handlerList.timeoutHandler.broadcast();

		// Disconnect from server
		serverDisconnected(nullptr);
		return;
	}

	size_t count = 0;

	ICommunicator* socket = getNetchan()->getRawSocket();
	// Loop until there is no valid data
	// or the max number of processed packets has reached the limit
	while(isChannelValid() && socket->getIncomingSize() && count++ < settings.getMaxTickPackets())
	{
		DynamicDataMessageStream stream;
		// reserve a minimum amount to avoid reallocating each time
		stream.reserve(MINIMUM_RECEIVE_BUFFER_SIZE);

		uint32_t sequenceNum;
		IRemoteIdentifierPtr from;

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
		if (cgameModule) {
			cgameModule->tick(deltaTime, currentTime, clientTime.getRemoteTime());
		}
	}
	catch (NetworkException& e)
	{
		// call the handler
		handlerList.errorHandler.broadcast(e);

		MOHPC_LOG(Error, "ClientGameModule exception: %s", e.what().c_str());
	}
}

TimeoutTimer& ClientGameConnection::getTimeoutTimer()
{
	return timeout;
}

const TimeoutTimer& ClientGameConnection::getTimeoutTimer() const
{
	return timeout;
}

const INetchanPtr& ClientGameConnection::getNetchan() const
{
	return netchan;
}

void ClientGameConnection::receive(const IRemoteIdentifierPtr& from, MSG& msg, uint64_t currentTime, uint32_t sequenceNum)
{
	serverMessageSequence = (uint32_t)sequenceNum;

	msg.SetCodec(MessageCodecs::Bit);

	// Read the ack
	reliableCommands->updateAcknowledge(msg.ReadUInteger());

	static constexpr size_t decodeStart = sizeof(uint32_t) + sizeof(uint32_t);

	// decode the stream itself
	IMessageStream& stream = msg.stream();
	stream.Seek(decodeStart, IMessageStream::SeekPos::Begin);
	encoder->setReliableAcknowledge(reliableCommands->getReliableAcknowledge());
	encoder->setSecretKey(serverMessageSequence);
	encoder->decode(stream, stream);
	// seek after sequence number
	stream.Seek(sizeof(uint32_t));

	// needs to be reset as the stream has been decoded
	msg.Reset();
	// read again to get the right bits
	msg.ReadInteger();

	// as data has been received, update the last timeout time
	timeout.update();

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

void ClientGameConnection::receiveConnectionLess(const IRemoteIdentifierPtr& from, MSG& msg)
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
	return reliableCommands->addCommand(cmd);
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
			clientProto.readGameState(msg, *gameStatePtr);
			break;
		case svc_ops_e::Snapshot:
			parseSnapshot(msg, currentTime);
			break;
		case svc_ops_e::Download:
			downloadState.processDownload(msg);
			break;
		case svc_ops_e::Centerprint:
			parseCenterprint(msg);
			break;
		case svc_ops_e::Locprint:
			parseLocprint(msg);
			break;
		case svc_ops_e::CGameMessage:
			cgameModule->parseCGMessage(msg);
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
	clientProto.readGameState(msg, *gameStatePtr);

#if 0
	MOHPC_LOG(Debug, "Received gamestate");

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

	// save the server id for later
	// it may be changed when parsing the system info
	const uint32_t oldServerId = serverId;

	systemInfoChanged();

	(this->*parseGameState_pf)(msg);

	// now notify about all received config strings
	notifyAllConfigStringChanges();

	const bool isDiff = isDifferentServer(oldServerId);
	if (isDiff)
	{
		// new map/server, reset important data
		clearState();
	} else {
		MOHPC_LOG(Warn, "Server has resent gamestate while in-game");
	}

	// notify about the new game state
	getHandlerList().gameStateParsedHandler.broadcast(getGameState(), isDiff);
#endif
}

void ClientGameConnection::parseSnapshot(MSG& msg, uint64_t currentTime)
{
	ClientSnapshot newSnap;
	newSnap.serverCommandNum = serverCommands->getCommandSequence();

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
	clientProto.readDeltaPlayerstate(msg, oldps, &newSnap.ps);
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
		// server time starts from here
		clientTime.setStartTime(currentTime);
		clientTime.setRemoteStartTime(newSnap.serverTime);
	}

	// set the new snap and calculate the ping
	setNewSnap(newSnap);
	calculatePing(currentTime);

	// read and unpack radar info on SH/BT
	//readNonPVSClient(currentSnap.ps.getRadarInfo());
	radarUnpacked_t unpacked;
	if (clientProto.readNonPVSClient(currentSnap.ps.getRadarInfo(), currentSnap, settings.getRadarRange(), unpacked))
	{
		handlerList.readNonPVSClientHandler.broadcast(unpacked);
	}


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

	for (;;)
	{
		const uint16_t newNum = clientProto.readEntityNum(msg);

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
		clientProto.readDeltaEntity(msg, *gameStatePtr, old, state, newNum);
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
			const uint16_t entityNum = clientProto.readEntityNum(msg);
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
				throw SerializableErrors::BadEntityNumberException("sound", entityNum);
			}

			const uint8_t channel = msg.ReadNumber<uint8_t>(7);
			sound.channel = channel;

			const uint16_t soundIndex = msg.ReadNumber<uint16_t>(9);

			if (soundIndex < MAX_SOUNDS)
			{
				// Get the sound name from configstrings
				sound.soundName = gameState.getConfigstringManager().getConfigString(soundIndex);
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
		throw ClientError::UnexpectedDownloadException();
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
			imports.addReliableCommand("stopdl");
			throw ClientError::DownloadException(imports.readStringMessage(msg));
		}

		downloadBlock = 0;

		MOHPC_LOG(Debug, "downloading file of size %d", fileSize);
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

		MOHPC_LOG(Debug, "downloaded block %d size %d", block, size);

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
	const StringMessage s = clientProto.readString(msg);

	serverCommands->addCommand(s, seq);

	// check if it is already stored
	if (serverCommands->getCommandSequence() >= seq) {
		return;
	}

	Event ev;
	TokenParser parser;
	parser.Parse(s, strlen(s));

	const str commandName = parser.GetToken(false);
#if _DEBUG

	if (!str::icmp(commandName, "stufftext"))
	{
		// Warn about stufftext
		MOHPC_LOG(Warn, "Stufftext command detected. Handle it with high precautions. Arguments : %s", s.getData() + 10);
	}
#endif

	if (!str::icmp(commandName, "disconnect"))
	{
		// Server kicking out the client

		// Wipe the channel as it has been closed on server already
		wipeChannel();
		// Disconnect the client
		disconnect();
	}

	// notify about the new command
	handlerList.serverCommandHandler.broadcast(commandName.c_str(), parser);
}

void ClientGameConnection::parseCenterprint(MSG& msg)
{
	const StringMessage s = clientProto.readString(msg);
	handlerList.centerPrintHandler.broadcast(const_cast<const char*>(s.getData()));
}

void ClientGameConnection::parseLocprint(MSG& msg)
{
	const uint16_t x = msg.ReadUShort();
	const uint16_t y = msg.ReadUShort();

	const StringMessage string = clientProto.readString(msg);
	handlerList.locationPrintHandler.broadcast(x, y, const_cast<const char*>(string.getData()));
}

bool ClientGameConnection::sendCmd(uint64_t currentTime)
{
	if (cgameModule && canCreateCommand())
	{
		// only create commands if the client is completely ready

		usercmd_t* newCmd;
		usereyes_t* newEyes;
		input.createCommand(currentTime, clientTime.getRemoteTime(), newCmd, newEyes);

		// all movement will happen through the event notification
		// the callee is responsible for making user input
		getHandlerList().userInputHandler.broadcast(*newCmd, *newEyes);
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
	uint32_t key = gameStatePtr->checksumFeed;
	// also use the message acknowledge
	key ^= serverMessageSequence;
	// also use the last acknowledged server command in the key
	key ^= clientProto.hashKey(serverCommands->getLastSequence(), 32);

	return key;
}

uint8_t ClientGameConnection::getNumCommandsToWrite(uint32_t oldPacketNum) const
{
	const uint32_t cmdNumber = input.getCurrentCmdNumber();
	if (cmdNumber > outPackets[oldPacketNum].p_cmdNumber + MAX_PACKET_USERCMDS)
	{
		MOHPC_LOG(Warn, "MAX_PACKET_USERCMDS");
		return MAX_PACKET_USERCMDS;
	}

	return cmdNumber - outPackets[oldPacketNum].p_cmdNumber;
}

void ClientGameConnection::writePacket(uint64_t currentTime)
{
	handlerList.preWritePacketHandler.broadcast();

	DynamicDataMessageStream stream;
	MSG msg(stream, msgMode_e::Writing);

	// a packet is between 10 and 30 bytes
	stream.reserve(32);

	// write the packet header
	PacketHeaderWriter headerWriter(*gameStatePtr, *serverCommands, serverMessageSequence);
	headerWriter.write(msg);
	// write commands the server didn't acknowledge
	ReliableCommandsWriter cmdWriter(*reliableCommands, clientProto);
	cmdWriter.write(msg);
	// write user commands
	writeUserInput(msg, currentTime);

	// end of packet
	msg.WriteByte(clc_ops_e::eof);

	// flush out pending data
	msg.Flush();

	static constexpr size_t encodeStart = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);

	stream.Seek(encodeStart, IMessageStream::SeekPos::Begin);
	encoder->setSecretKey(gameStatePtr->serverId);
	encoder->setMessageAcknowledge(serverMessageSequence);
	encoder->setReliableAcknowledge(serverCommands->getCommandSequence());
	encoder->encode(stream, stream);
	stream.Seek(0, IMessageStream::SeekPos::Begin);
	
	// transmit the encoded message
	getNetchan()->transmit(*adr, stream);

	lastPacketSendTime = currentTime;
}

void ClientGameConnection::writeUserInput(MSG& msg, uint64_t currentTime)
{
	const uint32_t oldPacketNum = (getNetchan()->getOutgoingSequence() - 1) % PACKET_BACKUP;
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
		const SerializableUserEyes oldEyeInfo(outPackets[oldPacketNum].p_eyeinfo);
		SerializableUserEyes userEyesWrite(input.getEyeInfo());
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
		const usercmd_t& cmd = input.getCommandFromLast(count + i + 1);
		// write a delta of the command by using the old
		SerializableUsercmd oldCmdRead(*const_cast<usercmd_t*>(oldcmd));
		SerializableUsercmd inputCmd(const_cast<usercmd_t&>(cmd));
		msg.WriteDeltaClass(&oldCmdRead, &inputCmd, key);

		oldcmd = &cmd;
	}
}

void ClientGameConnection::storeOutputPacket(uint64_t currentTime, uint32_t serverTime)
{
	const uint32_t packetNum = getNetchan()->getOutgoingSequence() & PACKET_MASK;
	outPackets[packetNum].p_currentTime = currentTime;
	outPackets[packetNum].p_serverTime = serverTime;
	outPackets[packetNum].p_cmdNumber = input.getCurrentCmdNumber();
	outPackets[packetNum].p_eyeinfo = input.getEyeInfo();
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
		MOHPC_LOG(Warn, "Delta from invalid frame (not supposed to happen!).");
	}
	else if (old->messageNum != snap.deltaNum) {
		// The frame that the server did the delta from
		// is too old, so we can't reconstruct it properly.
		// FIXME: throw?
		MOHPC_LOG(Warn, "Delta frame too old.");
	}
	else if (parseEntitiesNum > old->parseEntitiesNum + (MAX_PARSE_ENTITIES - 128)) {
		// FIXME: throw?
		MOHPC_LOG(Warn, "Delta parseEntitiesNum too old.");
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

CGameModuleBase* ClientGameConnection::getCGModule()
{
	return cgameModule;
}

const IRemoteIdentifierPtr& ClientGameConnection::getRemoteAddress() const
{
	return adr;
}

IGameState& ClientGameConnection::getGameState() const
{
	return *gameStatePtr;
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

const ClientTime& ClientGameConnection::getClientTime() const
{
	return clientTime;
}

const UserInput& ClientGameConnection::getUserInput() const
{
	return input;
}

IReliableSequence& ClientGameConnection::getClientCommands() const
{
	assert(reliableCommands);
	return *reliableCommands;
}

ICommandSequence& ClientGameConnection::getServerCommands() const
{
	assert(serverCommands);
	return *serverCommands;
}

uint64_t ClientGameConnection::getServerFrameTime() const
{
	return gameStatePtr->serverDeltaTime;
}

uint32_t ClientGameConnection::getClientNum() const
{
	return gameStatePtr->clientNum;
}

bool ClientGameConnection::getUserCmd(uintptr_t cmdNum, usercmd_t& outCmd) const
{
	// the usercmd has been overwritten in the wrapping
	// buffer because it is too far out of date
	const uint32_t cmdNumber = input.getCurrentCmdNumber();
	if (cmdNum + CMD_BACKUP < cmdNumber) {
		return false;
	}

	outCmd = input.getCommand(cmdNum);
	return true;
}

bool ClientGameConnection::getServerCommand(rsequence_t serverCommandNumber, TokenParser& tokenized)
{
	const char* cmdString = serverCommands->getSequence(serverCommandNumber);
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
		gameStatePtr->configStringModified(num, csString, true);

		if (num == CS_SYSTEMINFO || num == CS_SERVERINFO)
		{
			// Notify when system info has changed
			gameStatePtr->systemInfoChanged();
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
	return serverCommands->getCommandSequence();
}

uint32_t Network::ClientGameConnection::getReliableSequence() const
{
	return reliableCommands->getReliableSequence();
}

uint32_t Network::ClientGameConnection::getReliableAcknowledge() const
{
	return reliableCommands->getReliableAcknowledge();
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
	gameStatePtr->serverId = 0;

	if (cgameModule)
	{
		handlerList.disconnectHandler.broadcast(reason);

		// Delete the CG module
		delete cgameModule;
		cgameModule = nullptr;
	}
}

void ClientGameConnection::wipeChannel()
{
	netchan = nullptr;
}

bool ClientGameConnection::isChannelValid() const
{
	return netchan != nullptr && cgameModule != nullptr;
}

void ClientGameConnection::clearState()
{
	parseEntitiesNum = 0;
	input.reset();
	isActive = false;
	isReady = false;
}

bool ClientGameConnection::isDifferentServer(uint32_t id)
{
	return id != gameStatePtr->serverId;
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
		clientTime.setStartTime(currentTime);
		return;
	}

	// Check when server has restarted
	if ((currentSnap.snapFlags ^  lastSnapFlags) & SNAPFLAG_SERVERCOUNT) {
		serverRestarted();
	}

	// FIXME: throw if snap server time went backward
	clientTime.setTime(currentTime, currentSnap.serverTime, gameStatePtr->serverDeltaTime, newSnapshots);
}

void ClientGameConnection::firstSnapshot(uint64_t currentTime)
{
	if (currentSnap.snapFlags & SNAPFLAG_NOT_ACTIVE) {
		return;
	}

	updateSnapFlags();

	clientTime.initRemoteTime(currentSnap.serverTime, currentTime);
	isActive = true;

	cgameModule->init(serverMessageSequence, serverCommands->getCommandSequence());

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
		// it happens if disconnected from server
		return false;
	}

	if (!gameStatePtr->serverId || !canCreateCommand())
	{
		// allow one packet per second when not entered
		return currentTime >= lastPacketSendTime + 1000;
	}

	const size_t oldPacketNum = getNetchan()->getOutgoingSequence() % PACKET_BACKUP;
	const uint64_t delta = currentTime - outPackets[oldPacketNum].p_currentTime;
	if (delta < 1000 / settings.getMaxPackets()) {
		return false;
	}

	return true;
}

void ClientGameConnection::fillClientImports(ClientImports& imports)
{
	using namespace std::placeholders;
	imports.getClientNum				= std::bind(&ClientGameConnection::getClientNum, this);
	imports.getCurrentSnapshotNumber	= std::bind(&ClientGameConnection::getCurrentSnapshotNumber, this);
	imports.getSnapshot					= std::bind(&ClientGameConnection::getSnapshot, this, _1, _2);
	imports.getClientTime				= std::bind(&ClientGameConnection::getClientTime, this);
	imports.getUserCmd					= std::bind(&ClientGameConnection::getUserCmd, this, _1, _2);
	imports.getUserInput				= std::bind(&ClientGameConnection::getUserInput, this);
	imports.getServerCommand			= std::bind(&ClientGameConnection::getServerCommand, this, _1, _2);
	imports.getGameState				= std::bind(&ClientGameConnection::getGameState, this);
	imports.readStringMessage			= std::bind(&IClientGameProtocol::readString, &clientProto, _1);
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

ClientError::BadProtocolVersionException::BadProtocolVersionException(uint32_t inProtocolVersion)
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

PacketHeaderWriter::PacketHeaderWriter(const IGameState& gameStateRef, const ICommandSequence& serverCommandsRef, uint32_t serverMessageSequenceValue)
	: gameState(gameStateRef)
	, serverCommands(serverCommandsRef)
	, serverMessageSequence(serverMessageSequenceValue)
{
}

void PacketHeaderWriter::write(MSG& msg)
{
	// write the server id (given with the gameState)
	msg.WriteUInteger(gameState.serverId);
	// write the server sequence number (packet number)
	msg.WriteUInteger(serverMessageSequence);
	// write the command sequence acknowledge
	msg.WriteUInteger(serverCommands.getCommandSequence());
}

ReliableCommandsWriter::ReliableCommandsWriter(const IReliableSequence& reliableCommandsRef, const IClientGameProtocol& clientProtoRef)
	: reliableCommands(reliableCommandsRef)
	, clientProto(clientProtoRef)
{
}

void ReliableCommandsWriter::write(MSG& msg)
{
	const rsequence_t reliableAcknowledge = reliableCommands.getReliableAcknowledge();
	const rsequence_t reliableSequence = reliableCommands.getReliableSequence();
	for (uint32_t i = reliableAcknowledge + 1; i <= reliableSequence; ++i)
	{
		msg.WriteByte(clc_ops_e::ClientCommand);
		msg.WriteInteger(i);
		clientProto.writeString(msg, reliableCommands.getSequence(i));
	}
}
