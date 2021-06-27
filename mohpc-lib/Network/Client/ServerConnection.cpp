#include <MOHPC/Network/Client/ServerConnection.h>
#include <MOHPC/Network/Client/CGame/Module.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>
#include <MOHPC/Utility/Misc/MSG/Codec.h>
#include <MOHPC/Network/Remote/Channel.h>
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
using namespace Network;

static constexpr size_t MAX_MSGLEN = 49152;
static constexpr size_t MINIMUM_RECEIVE_BUFFER_SIZE = 1024;
static constexpr char MOHPC_LOG_NAMESPACE[] = "network_cgame";

class ClientSequenceTemplate_ver8 : public ClientSequenceTemplate<64, 1024> {};
class ClientRemoteCommandSequenceTemplate_ver8 : public ClientRemoteCommandSequenceTemplate<64, 1024> {};
class ClientSequenceTemplate_ver17 : public ClientSequenceTemplate<64, 2048> {};
class ClientRemoteCommandSequenceTemplate_ver17 : public ClientRemoteCommandSequenceTemplate<64, 2048> {};

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

const usercmd_t& UserInput::getCommand(size_t index) const
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

MOHPC_OBJECT_DEFINITION(ServerConnection);

ServerConnection::ServerConnection(const INetchanPtr& inNetchan, const IRemoteIdentifierPtr& inAdr, uint32_t challengeResponse, const protocolType_c& protoType, const ClientInfoPtr& cInfo)
	: netchan(inNetchan)	
	, adr(inAdr)
	, timeout(std::chrono::milliseconds(60000))
	, isActive(false)
	, isReady(false)
	, userInfo(cInfo)
	, clGameState(protoType)
	, clSnapshotManager(protoType)
{
	ClientImports imports;
	fillClientImports(imports);

	downloadState.setImports(imports);

	const uint32_t protocol = protoType.getProtocolVersionNumber();

	stringParser = Parsing::IString::get(protocol);
	hashParser = Parsing::IHash::get(protocol);

	// from now always use version 17
	// the maximum command char may vary between version
	// FIXME: would it alter the gameplay?
	//  the command size is bigger than moh:aa (protocol version 5-8)
	reliableCommands = new ClientSequenceTemplate_ver17();
	serverCommands = new ClientRemoteCommandSequenceTemplate_ver17();

	cgameModule = CGameInstancier::get(protocol)->createInstance();
	cgameModule->setProtocol(protoType);
	if (!cgameModule)
	{
		throw ClientError::BadProtocolVersionException((uint8_t)protoType.getProtocolVersion());
	}
	cgameModule->setImports(imports);

	encoder = std::make_shared<Encoding>(challengeResponse, *reliableCommands, *serverCommands);

	using namespace std::chrono;
	const steady_clock::time_point currentTime = steady_clock::now();
	clientTime.setStartTime(currentTime.time_since_epoch().count());

	timeout.update();
}

ServerConnection::~ServerConnection()
{
	disconnect();
	delete reliableCommands;
	delete serverCommands;
}

ServerConnection::HandlerListClient& ServerConnection::getHandlerList()
{
	return handlerList;
}

void ServerConnection::tick(uint64_t deltaTime, uint64_t currentTime)
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
	catch (ClientError::DisconnectException&)
	{
		// Could happen if the server sent a "disconnect" message

		// Wipe the channel as it has been closed on server already
		wipeChannel();
		// Disconnect the client
		terminateConnection(nullptr);
	}
	catch (NetworkException& e)
	{
		// call the handler
		handlerList.errorHandler.broadcast(e);

		MOHPC_LOG(Error, "ClientGameModule exception: %s", e.what().c_str());
	}
}

TimeoutTimer& ServerConnection::getTimeoutTimer()
{
	return timeout;
}

const TimeoutTimer& ServerConnection::getTimeoutTimer() const
{
	return timeout;
}

const INetchanPtr& ServerConnection::getNetchan() const
{
	return netchan;
}

void ServerConnection::receive(const IRemoteIdentifierPtr& from, MSG& msg, uint64_t currentTime, uint32_t sequenceNum)
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

void ServerConnection::receiveConnectionLess(const IRemoteIdentifierPtr& from, MSG& msg)
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

void ServerConnection::addReliableCommand(const char* cmd)
{
	return reliableCommands->addCommand(cmd);
}

void ServerConnection::parseServerMessage(MSG& msg, uint64_t currentTime)
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
			clGameState.parseGameState(msg, serverCommands, clientTime);
			break;
		case svc_ops_e::Snapshot:
			clSnapshotManager.parseSnapshot(
				msg,
				clGameState,
				clientTime,
				serverCommands,
				outPackets,
				currentTime,
				serverMessageSequence,
				getNetchan()->getOutgoingSequence()
			);
			break;
		case svc_ops_e::Download:
			downloadState.processDownload(msg, *stringParser);
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

/*
void ServerConnection::parseGameState(MSG& msg)
{
	// create a new gameState with correct version
	clGameState.get() = gameStateParser->create();

	Parsing::gameStateClient_t clientData;
	gameStateParser->parseGameState(msg, clGameState.get(), clientData);

	// update the command sequence
	serverCommands->setSequence(clientData.commandSequence);

	clGameState.clientNum = clientData.clientNum;
	// seems to be always zero, is it really useful?
	clGameState.checksumFeed = clientData.checksumFeed;

	const bool isDiff = reloadGameState();
	if (isDiff)
	{
		clearState();
	}

	// take care of each config-string
	// the user must be fully aware of the current state
	clGameState.notifyAllConfigStringChanges();

	// gameState has been parsed, notify
	clGameState.getHandlers().gameStateParsedHandler.broadcast(clGameState, isDiff);

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

			const StringMessage stringValue = stringParser->readString(msg);

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
*/

DownloadManager::DownloadManager()
	: downloadSize(0)
	, downloadBlock(0)
	, downloadRequested(false)
{}

void DownloadManager::setImports(ClientImports& inImports) noexcept
{
	imports = inImports;
}

void DownloadManager::processDownload(MSG& msg, const Parsing::IString& stringParser)
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
			throw ClientError::DownloadException(stringParser.readString(msg));
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

void ServerConnection::parseCommandString(MSG& msg)
{
	const uint32_t seq = msg.ReadUInteger();
	const StringMessage s = stringParser->readString(msg);

	serverCommands->addCommand(s, seq);

	// check if it is already stored
	if (serverCommands->getCommandSequence() >= seq) {
		return;
	}

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
		throw ClientError::DisconnectException();
	}

	// notify about the new command
	handlerList.serverCommandManager.broadcast(commandName.c_str(), parser);
}

void ServerConnection::parseCenterprint(MSG& msg)
{
	const StringMessage s = stringParser->readString(msg);
	handlerList.centerPrintHandler.broadcast(const_cast<const char*>(s.getData()));
}

void ServerConnection::parseLocprint(MSG& msg)
{
	const uint16_t x = msg.ReadUShort();
	const uint16_t y = msg.ReadUShort();

	const StringMessage string = stringParser->readString(msg);
	handlerList.locationPrintHandler.broadcast(x, y, const_cast<const char*>(string.getData()));
}

bool ServerConnection::sendCmd(uint64_t currentTime)
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

void ServerConnection::writePacket(uint64_t currentTime)
{
	handlerList.preWritePacketHandler.broadcast();

	DynamicDataMessageStream stream;
	MSG msg(stream, msgMode_e::Writing);

	// a packet is between 10 and 30 bytes
	stream.reserve(32);

	// write the packet header
	PacketHeaderWriter headerWriter(clGameState, *serverCommands, serverMessageSequence);
	headerWriter.write(msg);
	// write commands the server didn't acknowledge
	ReliableCommandsWriter cmdWriter(*reliableCommands, *stringParser);
	cmdWriter.write(msg);
	// write user commands
	UserInputWriter inputWriter(input, outPackets, clSnapshotManager, *hashParser, serverCommands, clGameState);
	inputWriter.write(msg, clientTime.getRemoteTime(), getNetchan()->getOutgoingSequence(), serverMessageSequence);

	// end of packet
	msg.WriteByte(clc_ops_e::eof);

	// flush out pending data
	msg.Flush();

	static constexpr size_t encodeStart = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);

	stream.Seek(encodeStart, IMessageStream::SeekPos::Begin);
	encoder->setSecretKey(clGameState.serverId);
	encoder->setMessageAcknowledge(serverMessageSequence);
	encoder->setReliableAcknowledge(serverCommands->getCommandSequence());
	encoder->encode(stream, stream);
	stream.Seek(0, IMessageStream::SeekPos::Begin);
	
	// transmit the encoded message
	getNetchan()->transmit(*adr, stream);

	lastPacketSendTime = currentTime;
}

CGameModuleBase* ServerConnection::getCGModule()
{
	return cgameModule;
}

const IRemoteIdentifierPtr& ServerConnection::getRemoteAddress() const
{
	return adr;
}

ServerGameState& ServerConnection::getGameState()
{
	return clGameState;
}

const ServerGameState& ServerConnection::getGameState() const
{
	return clGameState;
}

ConstClientInfoPtr ServerConnection::getUserInfo() const
{
	return userInfo;
}

const ClientInfoPtr& ServerConnection::getUserInfo()
{
	return userInfo;
}

void ServerConnection::updateUserInfo()
{
	Info info;
	ClientInfoHelper::fillInfoString(*userInfo, info);
	// send the new user info to the server
	addReliableCommand(str::printf("userinfo \"%s\"", info.GetString()));
}

clientGameSettings_t& ServerConnection::getSettings()
{
	return settings;
}

const clientGameSettings_t& ServerConnection::getSettings() const
{
	return settings;
}

ServerSnapshotManager& ServerConnection::getSnapshotManager()
{
	return clSnapshotManager;
}

const ServerSnapshotManager& ServerConnection::getSnapshotManager() const
{
	return clSnapshotManager;
}

const ClientTime& ServerConnection::getClientTime() const
{
	return clientTime;
}

const UserInput& ServerConnection::getUserInput() const
{
	return input;
}

IReliableSequence& ServerConnection::getClientCommands() const
{
	assert(reliableCommands);
	return *reliableCommands;
}

ICommandSequence& ServerConnection::getServerCommands() const
{
	assert(serverCommands);
	return *serverCommands;
}

bool ServerConnection::getUserCmd(uintptr_t cmdNum, usercmd_t& outCmd) const
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

bool ServerConnection::getServerCommand(rsequence_t serverCommandNumber, TokenParser& tokenized)
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

		//gameStatePtr->configStringModified(num, csString, true);
		// can set the config-string right now
		clGameState.get().getConfigstringManager().setConfigString(num, csString);

		// Notify about modification
		clGameState.getHandlers().configStringHandler.broadcast(num, csString);

		if (num == CS_SYSTEMINFO || num == CS_SERVERINFO)
		{
			// reload gameState settings including the sv_fps value
			clGameState.reloadGameState(clientTime);
		}

		return false;
	}

	return true;
}

uint32_t ServerConnection::getCurrentServerMessageSequence() const
{
	return serverMessageSequence;
}

const ICommandSequence* ServerConnection::getCommandSequence() const
{
	return serverCommands;
}

const IReliableSequence* ServerConnection::getReliableSequence() const
{
	return reliableCommands;
}

void ServerConnection::disconnect()
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

void ServerConnection::sendCommand(const char* command)
{
	addReliableCommand(command);
	parseClientCommand(command);
}

void ServerConnection::markReady()
{
	isReady = true;
}

void ServerConnection::unmarkReady()
{
	isReady = false;
}

bool ServerConnection::canCreateCommand() const
{
	return isReady;
}

void ServerConnection::serverDisconnected(const char* reason)
{
	wipeChannel();

	// Terminate after wiping channel
	terminateConnection(reason);
}

void ServerConnection::terminateConnection(const char* reason)
{
	// Clear out the server client data
	clGameState = ServerGameState(protocolType_c());

	if (cgameModule)
	{
		handlerList.disconnectHandler.broadcast(reason);

		// Delete the CG module
		delete cgameModule;
		cgameModule = nullptr;
	}
}

void ServerConnection::wipeChannel()
{
	netchan = nullptr;
}

bool ServerConnection::isChannelValid() const
{
	return netchan != nullptr && cgameModule != nullptr;
}

void ServerConnection::clearState()
{
	input.reset();
	isActive = false;
	isReady = false;
}

void ServerConnection::parseClientCommand(const char* arguments)
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

void ServerConnection::setCGameTime(uint64_t currentTime)
{
	if (clSnapshotManager.checkTime(clientTime))
	{
		initSnapshot(currentTime);
	}

	// if we have gotten to this point, cl.snap is guaranteed to be valid
	if (!clSnapshotManager.isSnapshotValid())
	{
		clientTime.setStartTime(currentTime);
		return;
	}

	// FIXME: throw if snap server time went backward
	clientTime.setTime(currentTime, clSnapshotManager.getServerTime(), clSnapshotManager.hasNewSnapshots());
}

void ServerConnection::initSnapshot(uint64_t currentTime)
{
	clientTime.initRemoteTime(clSnapshotManager.getServerTime(), currentTime);
	// make the game active
	isActive = true;

	cgameModule->init(serverMessageSequence, serverCommands->getCommandSequence());

	// update the userinfo to not let the server hold garbage fields like the challenge
	updateUserInfo();
}

bool ServerConnection::readyToSendPacket(uint64_t currentTime) const
{
	if (!cgameModule)
	{
		// it happens if disconnected from server
		return false;
	}

	if (!clGameState.serverId || !canCreateCommand())
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

void ServerConnection::fillClientImports(ClientImports& imports)
{
	using namespace std::placeholders;
	imports.getClientTime				= std::bind(&ServerConnection::getClientTime, this);
	imports.getUserCmd					= std::bind(&ServerConnection::getUserCmd, this, _1, _2);
	imports.getUserInput				= std::bind(&ServerConnection::getUserInput, this);
	imports.getServerCommand			= std::bind(&ServerConnection::getServerCommand, this, _1, _2);
	imports.getGameState				= std::bind(const_cast<ServerGameState& (ServerConnection::*)()>(&ServerConnection::getGameState), this);
	imports.getSnapshotManager			= std::bind(const_cast<ServerSnapshotManager& (ServerConnection::*)()>(&ServerConnection::getSnapshotManager), this);
	imports.addReliableCommand			= std::bind(&ServerConnection::addReliableCommand, this, _1);
	imports.getUserInfo					= std::bind(static_cast<const ClientInfoPtr&(ServerConnection::*)()>(&ServerConnection::getUserInfo), this);
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

PacketHeaderWriter::PacketHeaderWriter(const ServerGameState& clGameStateRef, const ICommandSequence& serverCommandsRef, uint32_t serverMessageSequenceValue)
	: clGameState(clGameStateRef)
	, serverCommands(serverCommandsRef)
	, serverMessageSequence(serverMessageSequenceValue)
{
}

void PacketHeaderWriter::write(MSG& msg)
{
	// write the server id (given with the gameState)
	msg.WriteUInteger(clGameState.serverId);
	// write the server sequence number (packet number)
	msg.WriteUInteger(serverMessageSequence);
	// write the command sequence acknowledge
	msg.WriteUInteger(serverCommands.getCommandSequence());
}

ReliableCommandsWriter::ReliableCommandsWriter(const IReliableSequence& reliableCommandsRef, const Parsing::IString& stringParserRef)
	: reliableCommands(reliableCommandsRef)
	, stringParser(stringParserRef)
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
		stringParser.writeString(msg, reliableCommands.getSequence(i));
	}
}

UserInputWriter::UserInputWriter(
	const UserInput& userInputRef,
	OutgoingPackets& outPacketsRef,
	const ServerSnapshotManager& clSnapshotManRef,
	const Parsing::IHash& hasherRef,
	const ICommandSequence* serverCommandsPtr,
	const ServerGameState& clGameStateRef
)
	: userInput(userInputRef)
	, outPackets(outPacketsRef)
	, clSnapshotMan(clSnapshotManRef)
	, hasher(hasherRef)
	, serverCommands(serverCommandsPtr)
	, clGameState(clGameStateRef)
{
}

void UserInputWriter::write(MSG& msg, uint64_t currentTime, uint32_t outgoingSequence, uint32_t serverMessageSequence)
{
	const uint32_t oldPacketNum = (outgoingSequence - 1) % PACKET_BACKUP;
	const uint8_t count = getNumCommandsToWrite(oldPacketNum);

	static const usercmd_t nullcmd;
	const usercmd_t* oldcmd = &nullcmd;

	if (count >= 1)
	{
		const clc_ops_e cmdOp = getClientOperation(serverMessageSequence);

		// write the operation type
		msg.WriteByteEnum<clc_ops_e>(cmdOp);
		// write the number of commands
		msg.WriteByte(count);
		// write delta eyes
		const SerializableUserEyes oldEyeInfo(outPackets[oldPacketNum].p_eyeinfo);
		SerializableUserEyes userEyesWrite(const_cast<usereyes_t&>(userInput.getEyeInfo()));
		msg.WriteDeltaClass(&oldEyeInfo, &userEyesWrite);

		// get the key used to xor user command data
		const uint32_t key = getCommandHashKey(serverMessageSequence);

		// now write all commands
		writeAllCommands(msg, oldcmd, count, key);
	}

	// save values for later
	storeOutputPacket(currentTime, oldcmd->serverTime, outgoingSequence);

	// Write end of user commands
	msg.WriteByte(clc_ops_e::eof);
}

clc_ops_e UserInputWriter::getClientOperation(uint32_t serverMessageSequence) const
{
	if (clSnapshotMan.isSnapshotValid() && serverMessageSequence == clSnapshotMan.getCurrentSnapNumber()) {
		return clc_ops_e::Move;
	}
	else {
		return clc_ops_e::MoveNoDelta;
	}
}

uint32_t UserInputWriter::getCommandHashKey(uint32_t serverMessageSequence) const
{
	uint32_t key = clGameState.checksumFeed;
	// also use the message acknowledge
	key ^= serverMessageSequence;
	// also use the last acknowledged server command in the key
	key ^= hasher.hashKey(serverCommands->getLastSequence(), 32);

	return key;
}

uint8_t UserInputWriter::getNumCommandsToWrite(uint32_t oldPacketNum) const
{
	const uint32_t cmdNumber = userInput.getCurrentCmdNumber();
	if (cmdNumber > outPackets[oldPacketNum].p_cmdNumber + MAX_PACKET_USERCMDS)
	{
		MOHPC_LOG(Warn, "MAX_PACKET_USERCMDS");
		return MAX_PACKET_USERCMDS;
	}

	return cmdNumber - outPackets[oldPacketNum].p_cmdNumber;
}

void UserInputWriter::writeAllCommands(MSG& msg, const usercmd_t*& oldcmd, size_t count, uint32_t key)
{
	// write all the commands, including the predicted command
	for (size_t i = 0; i < count; i++)
	{
		const usercmd_t& cmd = userInput.getCommandFromLast(count + i + 1);
		// write a delta of the command by using the old
		SerializableUsercmd oldCmdRead(*const_cast<usercmd_t*>(oldcmd));
		SerializableUsercmd inputCmd(const_cast<usercmd_t&>(cmd));
		msg.WriteDeltaClass(&oldCmdRead, &inputCmd, key);

		oldcmd = &cmd;
	}
}

void UserInputWriter::storeOutputPacket(uint64_t currentTime, uint32_t serverTime, uint32_t outgoingSequence)
{
	const uint32_t packetNum = outgoingSequence % PACKET_BACKUP;
	outPackets[packetNum].p_currentTime = currentTime;
	outPackets[packetNum].p_serverTime = serverTime;
	outPackets[packetNum].p_cmdNumber = userInput.getCurrentCmdNumber();
	outPackets[packetNum].p_eyeinfo = userInput.getEyeInfo();
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

//=============================
// EXCEPTIONS
//=============================

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
