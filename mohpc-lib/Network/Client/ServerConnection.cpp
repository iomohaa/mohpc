#include <MOHPC/Network/Client/ServerConnection.h>
#include <MOHPC/Network/Client/TimeManager.h>
#include <MOHPC/Network/Client/CGame/Module.h>
#include <MOHPC/Network/Remote/Channel.h>
#include <MOHPC/Network/Serializable/UserInput.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>
#include <MOHPC/Utility/Misc/MSG/Codec.h>
#include <MOHPC/Utility/Info.h>
#include <MOHPC/Utility/TokenParser.h>
#include <MOHPC/Common/Log.h>
#include <MOHPC/Network/Types/ReliableTemplate.h>

#include <typeinfo>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cstring>

using namespace MOHPC;
using namespace Network;

static constexpr size_t MINIMUM_RECEIVE_BUFFER_SIZE = 1024;
static constexpr char MOHPC_LOG_NAMESPACE[] = "network_cgame";

class SequenceTemplate_ver8 : public SequenceTemplate<64, 1024> {};
class RemoteCommandSequenceTemplate_ver8 : public RemoteCommandSequenceTemplate<64, 1024> {};
class SequenceTemplate_ver17 : public SequenceTemplate<64, 2048> {};
class RemoteCommandSequenceTemplate_ver17 : public RemoteCommandSequenceTemplate<64, 2048> {};

class ClientEncoding
{
public:
	static void encode(IMessageStream& stream, const ICommandSequence* serverCommands, const ServerGameState& gameState, uint32_t challenge, uint32_t serverMessageSequence)
	{
		static constexpr size_t encodeStart = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);

		stream.Seek(encodeStart, IMessageStream::SeekPos::Begin);

		XOREncoding encoder(challenge, *serverCommands);
		encoder.setSecretKey(gameState.getServerId());
		encoder.setMessageAcknowledge(serverMessageSequence);
		encoder.setReliableAcknowledge(serverCommands->getCommandSequence());
		encoder.convert(stream, stream);

		stream.Seek(0, IMessageStream::SeekPos::Begin);
	}

	static void decode(MSG& msg, const IReliableSequence* reliableCommands, const ServerGameState& gameState, uint32_t challenge, uint32_t serverMessageSequence)
	{
		static constexpr size_t decodeStart = sizeof(uint32_t) + sizeof(uint32_t);

		// decode the stream itself
		IMessageStream& stream = msg.stream();
		stream.Seek(decodeStart, IMessageStream::SeekPos::Begin);

		XOREncoding decoder(challenge, *reliableCommands);
		decoder.setReliableAcknowledge(reliableCommands->getReliableAcknowledge());
		decoder.setSecretKey(serverMessageSequence);
		decoder.convert(stream, stream);
		// seek after sequence number
		stream.Seek(sizeof(uint32_t));

		// needs to be reset as the stream has been decoded
		msg.Reset();
		// read again to get the right bits
		msg.ReadInteger();
	}
};

void ClientConnectionlessHandler::handle(const IRemoteIdentifierPtr& from, MSG& msg)
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
		throw ClientError::DisconnectException();
	}
	else if (!str::icmp(cmd, "droperror"))
	{
		// server is kicking the client due to an error

		// trim leading spaces/crlf
		parser.SkipWhiteSpace(true);
		// get the reason of the kick
		const char* reason = parser.GetCurrentScript();

		// Supply the reason when disconnecting
		throw ClientError::DisconnectException(reason);
	}
}

MOHPC_OBJECT_DEFINITION(ServerConnection);

ServerConnection::ServerConnection(const INetchanPtr& inNetchan, const IRemoteIdentifierPtr& inAdr, uint32_t challengeResponse, const protocolType_c& protoType, const UserInfoPtr& cInfo)
	: netchan(inNetchan)
	, adr(inAdr)
	, timeout(std::chrono::milliseconds(60000))
	, isActive(false)
	, isReady(false)
	, userInfo(cInfo)
	, clGameState(protoType, &clientTime)
	, clSnapshotManager(protoType)
	, disconnectHandler(*this)
	, lastPacketSendTime(0)
{
	if (!userInfo)
	{
		// not existing so create one
		userInfo = UserInfo::create();
	}

	const uint32_t protocol = protoType.getProtocolVersionNumber();

	stringParser = Parsing::IString::get(protocol);
	hashParser = Parsing::IHash::get(protocol);

	// from now always use version 17
	// the maximum command char is different across versions
	// would it alter the gameplay?
	//  the command size is bigger than moh:aa (protocol version 5-8)
	reliableCommands = new SequenceTemplate_ver17();
	serverCommands = new RemoteCommandSequenceTemplate_ver17();

	// from now always use version 17
	// the maximum command char is different across versions
	// would it alter the gameplay?
	//  the command size is bigger than moh:aa (protocol version 5-8)
	reliableCommands = new SequenceTemplate_ver17();
	serverCommands = new RemoteCommandSequenceTemplate_ver17();

	cgameModule = CGame::ModuleInstancier::get(protocol)->createInstance();
	cgameModule->setProtocol(protoType);
	if (!cgameModule)
	{
		throw ClientError::BadProtocolVersionException((uint8_t)protoType.getProtocolVersion());
	}
	CGame::Imports cgImports{
		clSnapshotManager,
		clientTime,
		input,
		*serverCommands,
		clGameState,
		userInfo
	};
	cgameModule->setImports(cgImports);

	// setup the download manager
	downloadState.setReliableSequence(reliableCommands);

	// register remote commands
	clGameState.registerCommands(remoteCommandManager);

	using namespace std::placeholders;
	remoteCommandManager.add("disconnect", &disconnectHandler);

	//encoder = std::make_shared<Encoding>(challengeResponse, *reliableCommands, *serverCommands);

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

	try
	{
		ICommunicator* socket = getNetchan()->getRawSocket();
		// Loop until there is no valid data
		// or the max number of processed packets has reached the limit
		while (isChannelValid() && socket->getIncomingSize() && count++ < settings.getMaxTickPackets())
		{
			DynamicDataMessageStream stream;
			// reserve a minimum amount to reduce the amount of reallocations
			stream.reserve(MINIMUM_RECEIVE_BUFFER_SIZE);

			uint32_t sequenceNum;
			IRemoteIdentifierPtr from;

			if (getNetchan()->receive(from, stream, sequenceNum))
			{
				// Prepare for reading
				MSG msg(stream, msgMode_e::Reading);
				if (sequenceNum != -1)
				{
					// received connection packet
					receive(from, msg, currentTime, sequenceNum);
				}
				else
				{
					// can only happen when disconnected
					ClientConnectionlessHandler handler;
					handler.handle(from, msg);
				}
			}
		}

		// Build and send client commands
		if (sendCmd(currentTime))
		{
			// Send packets if there are new commands
			writePacket(currentTime);
		}

		setCGameTime(currentTime);

		if (cgameModule) {
			cgameModule->tick(deltaTime, currentTime, clientTime.getRemoteTime());
		}
	}
	catch (ClientError::DisconnectException& exception)
	{
		// Could happen if the server sent a "disconnect" message

		// Wipe the channel as it has been closed on server already
		serverDisconnected(exception.getReason());
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
	serverMessageSequence = sequenceNum;

	msg.SetCodec(MessageCodecs::Bit);

	// Read the ack
	reliableCommands->updateAcknowledge(msg.ReadUInteger());

	ClientEncoding::decode(msg, reliableCommands, clGameState, getChallenge(), serverMessageSequence);

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
		const IMessageStream& stream = msg.stream();
		MOHPC_LOG(Error, "Tried to read past end of server message (length %d)", stream.GetLength());
	}
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
			clGameState.parseGameState(msg, serverCommands);
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
			// Client game module will handle all gameplay related messages
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

void ServerConnection::parseCommandString(MSG& msg)
{
	const uint32_t seq = msg.ReadUInteger();
	const StringMessage s = stringParser->readString(msg);

	if (serverCommands->getCommandSequence() >= seq)
	{
		// already stored so avoid processing it twice
		return;
	}

	serverCommands->addCommand(s, seq);

#if _DEBUG
	if (!str::icmpn(s.c_str(), "stufftext ", 10))
	{
		// Warn about stufftext
		MOHPC_LOG(Warn, "Stufftext command detected. Handle it with high precautions. Arguments : %s", s.getData() + 10);
	}
#endif

	// notify handlers about the new command
	remoteCommandManager.process(s.c_str());
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

	// encode the stream
	ClientEncoding::encode(stream, serverCommands, clGameState, getChallenge(), serverMessageSequence);
	
	// transmit the encoded message
	getNetchan()->transmit(*adr, stream);

	lastPacketSendTime = currentTime;
}

CGame::ModuleBase* ServerConnection::getCGModule()
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

ConstUserInfoPtr ServerConnection::getUserInfo() const
{
	return userInfo;
}

const UserInfoPtr& ServerConnection::getUserInfo()
{
	return userInfo;
}

void ServerConnection::updateUserInfo()
{
	Info info;
	ClientInfoHelper::fillInfoString(*userInfo, info);
	// send the new user info to the server
	reliableCommands->addCommand(str::printf("userinfo \"%s\"", info.GetString()));
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

uint32_t ServerConnection::getCurrentServerMessageSequence() const
{
	return serverMessageSequence;
}

const ICommandSequence* ServerConnection::getCommandSequence() const
{
	return serverCommands;
}

IReliableSequence* ServerConnection::getReliableSequence()
{
	return reliableCommands;
}

const IReliableSequence* ServerConnection::getReliableSequence() const
{
	return reliableCommands;
}

void ServerConnection::disconnect()
{
	if(getNetchan())
	{
		reliableCommands->addCommand("disconnect");

		for (size_t i = 0; i < 3; ++i) {
			writePacket(0);
		}

		// Network channel is not needed anymore
		wipeChannel();
	}

	// Terminate the connection
	terminateConnection(nullptr);
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

void ServerConnection::disconnectCommand(TokenParser& tokenized)
{
	// Server kicking out the client
	throw ClientError::DisconnectException();
}

void ServerConnection::serverDisconnected(const char* reason)
{
	wipeChannel();

	// Terminate after wiping channel
	terminateConnection(reason);
}

void ServerConnection::terminateConnection(const char* reason)
{
	// Clear the server client data
	clGameState.reset();

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

void ServerConnection::setCGameTime(uint64_t currentTime)
{
	ClientTimeManager timeManager(clientTime, clSnapshotManager);
	timeManager.setTime(currentTime);

	if (timeManager.hasEntered())
	{
		initSnapshot();
	}
}

void ServerConnection::initSnapshot()
{
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

	if (!clGameState.getServerId() || !canCreateCommand())
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

CommandManager& ServerConnection::getRemoteCommandManager()
{
	return remoteCommandManager;
}

uint32_t ServerConnection::getChallenge() const
{
	return challenge;
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
	msg.WriteUInteger(clGameState.getServerId());
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
	uint32_t key = clGameState.getChecksumFeed();
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

ClientError::DisconnectException::DisconnectException()
{
}

ClientError::DisconnectException::DisconnectException(const char* reasonValue)
	: reason(reasonValue)
{
}

ClientError::DisconnectException::DisconnectException(const str& reasonValue)
	: reason(reasonValue)
{
}

const char* ClientError::DisconnectException::getReason() const
{
	return reason.c_str();
}
