#pragma once

#include "../NetGlobal.h"
#include "../NetObject.h"

#include "../Exception.h"
#include "../Remote/Channel.h"
#include "../Remote/Encoding.h"
#include "../Remote/Ops.h"
#include "../Remote/Chain.h"
#include "../Configstring.h"
#include "../Parsing/String.h"
#include "../Parsing/Hash.h"
#include "../Parsing/Input.h"
#include "../Types/Snapshot.h"
#include "../../Utility/HandlerList.h"
#include "../../Utility/Info.h"
#include "../../Utility/PropertyMap.h"
#include "../../Utility/TokenParser.h"
#include "../../Utility/RemoteIdentifier.h"
#include "../../Utility/Tick.h"
#include "../../Utility/Timeout.h"
#include "../../Utility/CommandManager.h"
#include "../../Utility/Misc/MSG/MSG.h"

#include "ClientReflector.h"
#include "DownloadManager.h"
#include "UserInfo.h"
#include "InputModule.h"
#include "Packet.h"
#include "GameState.h"
#include "ServerSnapshot.h"
#include "Time.h"
#include "UserInput.h"

#include <cstdint>
#include <functional>
#include <type_traits>
#include <bitset>

namespace MOHPC
{
	class MSG;

	namespace Network
	{
		namespace CGame
		{
			class ModuleBase;
		}

		class INetchan;

		using ServerConnectionPtr = SharedPtr<class ServerConnection>;

		namespace ClientHandlers
		{
			/** Called when the client/server has timed out (data no longer received within a certain amount of time). */
			struct Timeout : public HandlerNotifyBase<void()> {};

			/** Called when the client/server is disconnecting. */
			struct Disconnect : public HandlerNotifyBase<void(const char* reason)> {};

			/**
			 * Called when an exception occurs within the game client.
			 * The callee can safely disconnect the game client.
			 *
			 * @param	exception	The exception that has occurred.
			 */
			struct Error : public HandlerNotifyBase<void(const NetworkException& exception)> {};

			/**
			 * Called when the server sent a string to print at the center of the screen.
			 *
			 * @param	message		string message to print.
			 */
			struct CenterPrint : public HandlerNotifyBase<void(const char* message)> {};

			/**
			 * Called when the server sent a string to print at a 2D location of the screen.
			 *
			 * @param	message		string message to print.
			 */
			struct LocationPrint : public HandlerNotifyBase<void(uint16_t x, uint16_t y, const char* message)> {};

			/**
			 * Called just before writing a packet and sending it.
			 */
			struct PreWritePacket : public HandlerNotifyBase<void(const ClientTime& currentTime, uint32_t sequenceNum)> {};

			/**
			 * Called after writing a packet and sending it.
			 */
			struct PostWritePacket : public HandlerNotifyBase<void(const ClientTime& currentTime, uint32_t sequenceNum)> {};
		}

		class MOHPC_NET_EXPORTS clientGameSettings_t
		{
		public:
			clientGameSettings_t();

			/** Return the maximum number of packets that can be sent per second. */
			uint32_t getMaxPackets() const;

			/** Set the maximum number of packets that can be sent per second, in the range of [1, 125]. */
			void setMaxPackets(uint32_t inMaxPackets);

		private:
			uint32_t maxPackets;
		};

		class ClientConnectionlessHandler
		{
		public:
			void handle(MSG& msg);
		};

		using ChannelCallback = std::function<void(IMessageStream& stream, uint32_t sequenceNum)>;
		class ServerChannel
		{
		public:
			ServerChannel(const INetchanPtr& netchanPtr, const IRemoteIdentifierPtr& adrPtr);

			/**
			 * Set the maximum number of packets that can be sent processed at once.
			 * @param maxPackets the maximum number of packets to process.
			 *  Use a value that isn't too high to avoid high processing time.
			 */
			MOHPC_NET_EXPORTS void setMaxPackets(uint32_t maxPackets);

			/** Return the maximum number of packets that can be processed at once. */
			MOHPC_NET_EXPORTS uint32_t getMaxPackets() const;

			/**
			 * Return whether or not the current channel is valid.
			 * An invalid channel means no more connection to the server.
			 */
			MOHPC_NET_EXPORTS bool isChannelValid() const;

			/** Return the network channel associated with the server channel. */
			const INetchanPtr& getNetchan() const;

			/** Return the IP address of the channel connection. */
			const IRemoteIdentifierPtr& getAddress() const;

			/**
			 * Fetch and process pending packets.
			 * @param from Packets to process from
			 * @param callback The function to call for each received packets.
			 */
			void process(const ChannelCallback& callback);

			/** Transmit data to the remote server. */
			void transmit(IMessageStream& stream);

			/** This clear the network channel, which will wipe the connection to the server if there are no more references to the channel. */
			void wipeChannel();

		private:
			INetchanPtr netchan;
			IRemoteIdentifierPtr adr;
			uint32_t maxPacketsAtOnce;
		};

		/**
		 * Maintains a connection to a server. Keeps track of the game state and commands.
		 *
		 * There is no input module by default, it should be created once the game state has been parsed and the map loaded.
		 */
		class ServerConnection : public ITickable
		{
			MOHPC_NET_OBJECT_DECLARATION(ServerConnection);

		private:
			struct HandlerListClient
			{
			public:
				FunctionList<ClientHandlers::Timeout> timeoutHandler;
				FunctionList<ClientHandlers::Disconnect> disconnectHandler;
				FunctionList<ClientHandlers::Error> errorHandler;
				FunctionList<ClientHandlers::CenterPrint> centerPrintHandler;
				FunctionList<ClientHandlers::LocationPrint> locationPrintHandler;
				FunctionList<ClientHandlers::PreWritePacket> preWritePacketHandler;
				FunctionList<ClientHandlers::PostWritePacket> postWritePacketHandler;
			};

		public:
			/**
			 * Construct the game client connection. This should not be called outside of server code.
			 *
			 * @param	netchan				Channel used to send/receive data
			 * @param	inAdr				Address of the server.
			 * @param	challengeResponse	Challenge used to XOR data.
			 * @param	protocolVersion		Version of the protocol to use.
			 */
			ServerConnection(const INetchanPtr& netchan, const IRemoteIdentifierPtr& inAdr, uint32_t challengeResponse, const protocolType_c& protoType, const UserInfoPtr& cInfo);
			~ServerConnection();

			// ITickable
			// ~
			void tick(deltaTime_t deltaTime, tickTime_t currentTime) override;
			// ~

			/**
			 * Return the protocol type for this server connection.
			 */
			MOHPC_NET_EXPORTS const protocolType_c& getProtocolType() const;

			/** Get the timeout timer for the client. */
			MOHPC_NET_EXPORTS TimeoutTimer& getTimeoutTimer();
			MOHPC_NET_EXPORTS const TimeoutTimer& getTimeoutTimer() const;

			/** Return the server channel associated with the connection. */
			MOHPC_NET_EXPORTS const ServerChannel& getServerChannel() const;
			MOHPC_NET_EXPORTS ServerChannel& getServerChannel();

			/** Return the client time that manages client and remote time. */
			MOHPC_NET_EXPORTS const ClientTime& getClientTime() const;
			MOHPC_NET_EXPORTS ClientTime& getClientTime();

			/**
			 * Return a reference to the class that manages remote command.
			 * Use this to register a server command.
			 */
			MOHPC_NET_EXPORTS CommandManager& getRemoteCommandManager();

			/** Return the client command object for managing commands. */
			MOHPC_NET_EXPORTS IReliableSequence& getClientCommands() const;

			/** Return the client command object for managing commands. */
			MOHPC_NET_EXPORTS ICommandSequence& getServerCommands() const;

			/** Return the handler list. */
			MOHPC_NET_EXPORTS HandlerListClient& getHandlerList();

			/** Retrieve the current game state. */
			MOHPC_NET_EXPORTS ServerGameState& getGameState();
			MOHPC_NET_EXPORTS const ServerGameState& getGameState() const;
			MOHPC_NET_EXPORTS const ServerGameStatePtr& getGameStatePtr();
			MOHPC_NET_EXPORTS ConstServerGameStatePtr getGameStatePtr() const;

			/** Retrieve the CGame module. */
			MOHPC_NET_EXPORTS CGame::ModuleBase* getCGModule();

			/** Return the chain for transmission/receive.
			 * Handle post-receive and pre-transmission.
			 */
			MOHPC_NET_EXPORTS const IChainPtr& getChain();
			MOHPC_NET_EXPORTS void setChain(const IChainPtr& chain);

			/** Return read-only user info. */
			MOHPC_NET_EXPORTS ConstUserInfoPtr getUserInfo() const;

			/** Return modifiable user info. */
			MOHPC_NET_EXPORTS const UserInfoPtr& getUserInfo();

			/** Send the server a new user info string. Must be called after modifying the userinfo. */
			MOHPC_NET_EXPORTS void updateUserInfo();

			/** Return the settings that the connection is using. */
			MOHPC_NET_EXPORTS clientGameSettings_t& getSettings();
			MOHPC_NET_EXPORTS const clientGameSettings_t& getSettings() const;

			/** Return the snapshot manager instance. */
			MOHPC_NET_EXPORTS ServerSnapshotManager& getSnapshotManager();
			MOHPC_NET_EXPORTS const ServerSnapshotManager& getSnapshotManager() const;

			/** Return the current server message sequence (the latest packet number). */
			MOHPC_NET_EXPORTS uint32_t getCurrentServerMessageSequence() const;

			/** Return the current server command sequence. */
			MOHPC_NET_EXPORTS const ICommandSequence* getCommandSequence() const;

			/**
			 * Return the current reliable sequence (the latest command number on the client).
			 * Can be used to enqueue a new (reliable) command.
			 */
			MOHPC_NET_EXPORTS IReliableSequence* getReliableSequence();
			MOHPC_NET_EXPORTS const IReliableSequence* getReliableSequence() const;

			/** Get/set the input module associated with the server connection. */
			MOHPC_NET_EXPORTS const IUserInputModulePtr& getInputModule() const;
			MOHPC_NET_EXPORTS void setInputModule(const IUserInputModulePtr& inputModulePtr);

			/**
			 * Disconnect the client.
			 * Can also be called from the server.
			 */
			MOHPC_NET_EXPORTS void disconnect();

		private:
			void receive(MSG& msg, tickTime_t currentTime, uint32_t sequenceNum);
			void sequenceReceived(tickTime_t currentTime, IMessageStream& stream, uint32_t sequenceNum);
			void disconnectCommand(TokenParser& tokenized);
			void serverDisconnected(const char* reason);
			void terminateConnection(const char* reason);

			void parseServerMessage(MSG& msg, tickTime_t currentTime, uint32_t sequenceNum);
			void parseCommandString(MSG& msg);
			void parseCenterprint(MSG& msg);
			void parseLocprint(MSG& msg);

			void setCGameTime(tickTime_t currentTime, uint32_t sequenceNum);
			void initSnapshot(uint32_t sequenceNum);

			bool readyToSendPacket(tickTime_t currentTime) const;
			//bool sendCmd(uint64_t currentTime);

			void writePacket(tickTime_t currentTime, uint32_t sequenceNum);
			uint32_t getCommandHashKey(uint32_t serverMessageSequence) const;

		private:
			CommandTemplate<ServerConnection, &ServerConnection::disconnectCommand> disconnectHandler;

			HandlerListClient handlerList;
			CGame::ModuleBase* cgameModule;
			const Parsing::IString* stringParser;
			const Parsing::IHash* hashParser;
			const Parsing::IPacketHeader* packetHeaderStream;
			const Parsing::IRemoteCommand* remoteCommandStream;
			IUserInputModulePtr inputModule;
			CommandManager remoteCommandManager;
			ServerChannel serverChannel;
			UserInfoPtr userInfo;
			IReliableSequence* reliableCommands;
			ICommandSequence* serverCommands;
			IChainPtr chain;
			ClientTime clientTime;
			TimeoutTimer timeout;
			ServerGameStatePtr clGameState;
			ServerSnapshotManager clSnapshotManager;
			ClientReflector reflector;
			clientGameSettings_t settings;
			DownloadManager downloadState;
			OutgoingPackets outPackets;
			tickTime_t lastPacketSendTime;
			uint32_t serverMessageSequence;
			protocolType_c protocolType;
		};

		using ServerConnectionPtr = SharedPtr<ServerConnection>;

		namespace ClientError
		{
			class Base : public NetworkException {};

			/**
			 * The protocol version does not exist.
			 */
			class BadProtocolVersionException : public Base
			{
			public:
				BadProtocolVersionException(uint32_t inProtocolVersion);

				MOHPC_NET_EXPORTS uint32_t getProtocolVersion() const;
				MOHPC_NET_EXPORTS str what() const override;

			private:
				uint32_t protocolVersion;
			};

			/**
			 * Invalid server operation.
			 */
			class IllegibleServerMessageException : public Base
			{
			public:
				IllegibleServerMessageException(uint8_t inCmdNum);

				MOHPC_NET_EXPORTS uint8_t getLength() const;
				MOHPC_NET_EXPORTS str what() const override;

			private:
				uint8_t cmdNum;
			};

			/**
			 * Invalid baseline entity number while parsing gamestate.
			 */
			class BaselineOutOfRangeException : public Base
			{
			public:
				BaselineOutOfRangeException(uint16_t inBaselineNum);

				MOHPC_NET_EXPORTS uint16_t getBaselineNum() const;
				MOHPC_NET_EXPORTS str what() const override;

			private:
				uint16_t baselineNum;
			};

			class DisconnectException : public Base
			{
			public:
				DisconnectException();
				DisconnectException(const char* reasonValue);
				DisconnectException(const str& reasonValue);

				const char* getReason() const;

			private:
				str reason;
			};

			class BadSoundNumberException : public Base
			{

			};
		}
	}

	MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::Timeout);
	MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::Disconnect);
	MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::Error);
	MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::CenterPrint);
	MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::LocationPrint);
	MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::PreWritePacket);
	MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::PostWritePacket);
}