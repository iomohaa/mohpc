#include <MOHPC/Network/Client/ServerConnection.h>
#include <MOHPC/Network/Client/TimeManager.h>
#include <MOHPC/Network/Client/ChainEncode.h>
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

#include <cassert>
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

ProtocolClassInstancier_Template<SequenceTemplate_ver8, IReliableSequence, 5, 8> sequenceTemplateInstancier8;
ProtocolClassInstancier_Template<SequenceTemplate_ver17, IReliableSequence, 15, 17> sequenceTemplateInstancier17;

ProtocolClassInstancier_Template<RemoteCommandSequenceTemplate_ver8, ICommandSequence, 5, 8> remoteSequenceTemplateInstancier8;
ProtocolClassInstancier_Template<RemoteCommandSequenceTemplate_ver17, ICommandSequence, 15, 17> remoteSequenceTemplateInstancier17;

void ClientConnectionlessHandler::handle(MSG& msg)
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

	if (!strHelpers::icmp(cmd, "disconnect"))
	{
		// disconnect message, may happen during map loading
		throw ClientError::DisconnectException();
	}
	else if (!strHelpers::icmp(cmd, "droperror"))
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

ServerChannel::ServerChannel(const INetchanPtr& netchanPtr, const IRemoteIdentifierPtr& adrPtr)
	: netchan(netchanPtr)
	, adr(adrPtr)
	, maxPacketsAtOnce(60)
{
}

void ServerChannel::setMaxPackets(uint32_t inMaxPackets)
{
	maxPacketsAtOnce = inMaxPackets;
	if (maxPacketsAtOnce < 1)
	{
		// wouldn't make sense ¯\_(°)_/¯
		maxPacketsAtOnce = 1;
	}
}

uint32_t ServerChannel::getMaxPackets() const
{
	return maxPacketsAtOnce;
}

void ServerChannel::wipeChannel()
{
	netchan = nullptr;
}

bool ServerChannel::isChannelValid() const
{
	return netchan != nullptr;
}

const MOHPC::Network::INetchanPtr& ServerChannel::getNetchan() const
{
	return netchan;
}

const MOHPC::IRemoteIdentifierPtr& ServerChannel::getAddress() const
{
	return adr;
}

void ServerChannel::process(const ChannelCallback& callback)
{
	if (!isChannelValid())
	{
		// invalid channel so don't process anything
		return;
	}

	size_t count = 0;

	ICommunicator* const socket = getNetchan()->getRawSocket();
	// Loop until there is no valid data
	// or the max number of processed packets has reached the limit
	while (isChannelValid() && socket->getIncomingSize() && count++ < maxPacketsAtOnce)
	{
		// don't process packets from other IPs otherwise the state may get corrupted
		IRemoteIdentifierPtr receivedFrom = adr;

		DynamicDataMessageStream stream;
		// reserve a minimum amount to reduce the amount of reallocations
		stream.reserve(MINIMUM_RECEIVE_BUFFER_SIZE);

		uint32_t sequenceNum;
		if (getNetchan()->receive(receivedFrom, stream, sequenceNum))
		{
			// got the sequence
			callback(stream, sequenceNum);
		}
	}
}

void ServerChannel::transmit(IMessageStream& stream)
{
	getNetchan()->transmit(*adr, stream);
}

MOHPC_OBJECT_DEFINITION(ServerConnection);

ServerConnection::ServerConnection(const INetchanPtr& inNetchan, const IRemoteIdentifierPtr& inAdr, uint32_t challengeResponse, const protocolType_c& protoType, const UserInfoPtr& cInfo)
	: serverChannel(inNetchan, inAdr)
	, timeout(std::chrono::milliseconds(60000))
	, isActive(false)
	, userInfo(cInfo)
	, clGameState(makeShared<ServerGameState>(protoType, &clientTime))
	, clSnapshotManager(protoType)
	, disconnectHandler(*this)
	, serverMessageSequence(0)
	, protocolType(protoType)
{
	if (!userInfo)
	{
		// not existing so create one
		userInfo = UserInfo::create();
	}

	const uint32_t protocol = protoType.getProtocolVersionNumber();

	hashParser = Parsing::IHash::get(protocol);
	packetHeaderStream = Parsing::IPacketHeader::get(protocol);
	remoteCommandStream = Parsing::IRemoteCommand::get(protocol);
	stringParser = Parsing::IString::get(protocol);

	const IProtocolClassInstancier<ICommandSequence>* remoteCommandInstancier = IProtocolClassInstancier<ICommandSequence>::get(protocol);
	const IProtocolClassInstancier<IReliableSequence>* reliableSequenceInstancier = IProtocolClassInstancier<IReliableSequence>::get(protocol);

	if (!remoteCommandInstancier || !reliableSequenceInstancier) {
		throw ClientError::BadProtocolVersionException(protocol);
	}

	// use protocol-specific command length
	// otherwise it may cause trouble if the string length doesn't match
	reliableCommands = reliableSequenceInstancier->createInstance();
	serverCommands = remoteCommandInstancier->createInstance();

	// set default chain as encoding/decoding
	setChain(ClientEncoding::create(clGameState->get(), reliableCommands, serverCommands, challengeResponse));

	// no input module by default
	inputModule = nullptr;

	cgameModule = new CGame::ModuleBase();
	cgameModule->setProtocol(protoType);

	CGame::Imports cgImports{
		clSnapshotManager,
		clientTime,
		*serverCommands
	};
	cgameModule->setImports(cgImports);

	// setup the download manager
	downloadState.setReliableSequence(reliableCommands);

	// register remote commands
	clGameState->registerCommands(remoteCommandManager);

	using namespace std::placeholders;
	remoteCommandManager.add("disconnect", &disconnectHandler);

	using namespace std::chrono;
	const steady_clock::time_point currentTime = steady_clock::now();
	clientTime.setStartTime(currentTime);

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

void ServerConnection::tick(deltaTime_t deltaTime, tickTime_t currentTime)
{
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
		using namespace std::placeholders;
		serverChannel.process(std::bind(&ServerConnection::sequenceReceived, this, currentTime, _1, _2));

		if (cgameModule)
		{
			setCGameTime(currentTime, serverMessageSequence);

			if (inputModule)
			{
				inputModule->process(clientTime);
			}

			// build and send client commands
			//if (sendCmd(currentTime))
			{
				// send packets if there are new commands
				if (readyToSendPacket(currentTime))
				{
					// time to write a packet to server
					writePacket(currentTime, serverMessageSequence);
				}
			}

			cgameModule->tick(deltaTime, currentTime, clientTime.getSimulatedRemoteTime());
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

void ServerConnection::sequenceReceived(tickTime_t currentTime, IMessageStream& stream, uint32_t sequenceNum)
{
	// Prepare for reading
	MSG msg(stream, msgMode_e::Reading);
	if (sequenceNum != -1)
	{
		// received connection packet
		receive(msg, currentTime, sequenceNum);
	}
	else
	{
		// can only happen when disconnected
		ClientConnectionlessHandler handler;
		handler.handle(msg);
	}
}

const protocolType_c& ServerConnection::getProtocolType() const
{
	return protocolType;
}

TimeoutTimer& ServerConnection::getTimeoutTimer()
{
	return timeout;
}

const TimeoutTimer& ServerConnection::getTimeoutTimer() const
{
	return timeout;
}

const ServerChannel& ServerConnection::getServerChannel() const
{
	return serverChannel;
}

ServerChannel& ServerConnection::getServerChannel()
{
	return serverChannel;
}

void ServerConnection::receive(MSG& msg, tickTime_t currentTime, uint32_t sequenceNum)
{
	if (sequenceNum < serverMessageSequence)
	{
		// that's an out of order packet
		MOHPC_LOG(Warn, "out of order packet received, got %d, was %d", sequenceNum, serverMessageSequence);
	}

	serverMessageSequence = sequenceNum;

	msg.SetCodec(MessageCodecs::Bit);

	// update the last acknowledge command from server
	reliableCommands->updateAcknowledge(msg.ReadUInteger());

	// handle decode/decrypt
	chain->handleReceive(msg.stream());

	// needs to be reset as the stream has been decoded
	msg.Reset();
	// read again to get the right bits
	msg.ReadInteger();

	// as data has been received, update the last timeout time
	timeout.update();

	try
	{
		// All messages will be parsed from there
		parseServerMessage(msg, currentTime, sequenceNum);
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

void ServerConnection::parseServerMessage(MSG& msg, tickTime_t currentTime, uint32_t sequenceNum)
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
			clGameState->parseGameState(msg, serverCommands);
			break;
		case svc_ops_e::Snapshot:
			clSnapshotManager.parseSnapshot(
				msg,
				*clGameState,
				clientTime,
				serverCommands,
				currentTime,
				sequenceNum,
				getServerChannel().getNetchan()->getOutgoingSequence()
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

/*
bool ServerConnection::sendCmd(uint64_t currentTime)
{
	if (inputModule)
	{
		// only create commands if the client is completely ready
		usercmd_t& newCmd = input.createCommand(clientTime);
		usereyes_t& newEyes = input.createEyes();

		// all movement will happen through the event notification
		// the callee is responsible for filling the user input
		getHandlerList().userInputHandler.broadcast(newCmd, newEyes);
	}

	return true;
}
*/

void ServerConnection::writePacket(tickTime_t currentTime, uint32_t sequenceNum)
{
	handlerList.preWritePacketHandler.broadcast();

	DynamicDataMessageStream stream;
	MSG msg(stream, msgMode_e::Writing);

	// a packet is between 10 and 30 bytes
	stream.reserve(32);

	// write the packet header
	packetHeaderStream->writeHeader(msg, clGameState->get().getMapInfo(), *serverCommands, sequenceNum);
	// write commands that the server didn't acknowledge
	remoteCommandStream->writeCommands(msg, *reliableCommands, *stringParser);

	if (inputModule)
	{
		bool deltaMove = false;
		if (clSnapshotManager.isSnapshotValid() && sequenceNum == clSnapshotManager.getCurrentSnapNumber()) {
			deltaMove = true;
		}

		inputModule->write(msg, getCommandHashKey(sequenceNum), deltaMove);
	}

	// end of packet
	msg.WriteByte(clc_ops_e::eof);

	// flush out pending data
	msg.Flush();

	// encode the stream
	chain->handleTransmit(stream);
	
	// transmit the encoded message
	getServerChannel().transmit(stream);

	lastPacketSendTime = currentTime;
}

CGame::ModuleBase* ServerConnection::getCGModule()
{
	return cgameModule;
}

ServerGameState& ServerConnection::getGameState()
{
	return *clGameState;
}

const ServerGameState& ServerConnection::getGameState() const
{
	return *clGameState;
}

const ServerGameStatePtr& ServerConnection::getGameStatePtr()
{
	return clGameState;
}

ConstServerGameStatePtr ServerConnection::getGameStatePtr() const
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
	reliableCommands->addCommand((str("userinfo ") + '"' + str(info.GetString()) + '"').c_str());
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

ClientTime& ServerConnection::getClientTime()
{
	return clientTime;
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
	if(getServerChannel().isChannelValid())
	{
		reliableCommands->addCommand("disconnect");

		for (size_t i = 0; i < 3; ++i) {
			writePacket(tickTime_t(), serverMessageSequence);
		}

		// Network channel is not needed anymore
		getServerChannel().wipeChannel();
	}

	terminateConnection(nullptr);
}

void ServerConnection::disconnectCommand(TokenParser& tokenized)
{
	// Server kicking out the client
	throw ClientError::DisconnectException();
}

void ServerConnection::serverDisconnected(const char* reason)
{
	getServerChannel().wipeChannel();

	terminateConnection(reason);
}

void ServerConnection::terminateConnection(const char* reason)
{
	// Clear the server client data
	clGameState->reset();

	if (cgameModule)
	{
		handlerList.disconnectHandler.broadcast(reason);

		// Delete the CG module
		delete cgameModule;
		cgameModule = nullptr;
	}
}

void ServerConnection::setCGameTime(tickTime_t currentTime, uint32_t sequenceNum)
{
	ClientTimeManager timeManager(clientTime, clSnapshotManager);
	timeManager.setTime(currentTime);

	if (timeManager.hasEntered())
	{
		initSnapshot(sequenceNum);
	}
}

void ServerConnection::initSnapshot(uint32_t sequenceNum)
{
	// make the game active
	isActive = true;

	cgameModule->init(sequenceNum, serverCommands->getCommandSequence());

	// update the userinfo to not let the server hold garbage fields like the challenge
	updateUserInfo();
}

bool ServerConnection::readyToSendPacket(tickTime_t currentTime) const
{
	using namespace std::chrono;

	if (!clGameState->get().getMapInfo().getServerId() || !inputModule)
	{
		// allow one packet per second when not entered
		return currentTime >= lastPacketSendTime + milliseconds(1000);
	}

	//const size_t oldPacketNum = getNetchan()->getOutgoingSequence() % PACKET_BACKUP;
	//const uint64_t delta = currentTime - outPackets[oldPacketNum].p_currentTime;
	const deltaTime_t delta = currentTime - lastPacketSendTime;
	if (delta < milliseconds(1000 / settings.getMaxPackets())) {
		return false;
	}

	return true;
}

CommandManager& ServerConnection::getRemoteCommandManager()
{
	return remoteCommandManager;
}

uint32_t ServerConnection::getCommandHashKey(uint32_t serverMessageSequence) const
{
	uint32_t key = clGameState->get().getMapInfo().getChecksumFeed();
	// also use the message acknowledge
	key ^= serverMessageSequence;
	// also use the last acknowledged server command in the key
	key ^= hashParser->hashKey(serverCommands->getLastSequence(), 32);

	return key;
}

const IChainPtr& ServerConnection::getChain()
{
	return chain;
}

void ServerConnection::setChain(const IChainPtr& chainPtr)
{
	chain = chainPtr;
}

const IUserInputModulePtr& ServerConnection::getInputModule() const
{
	return inputModule;
}

void ServerConnection::setInputModule(const IUserInputModulePtr& inputModulePtr)
{
	inputModule = inputModulePtr;
}

uint32_t ServerConnection::getCurrentServerMessageSequence() const
{
	return serverMessageSequence;
}

clientGameSettings_t::clientGameSettings_t()
	: maxPackets(30)
{

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
	return std::to_string(getProtocolVersion());
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
	return std::to_string(getLength());
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
	return std::to_string(getBaselineNum());
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
