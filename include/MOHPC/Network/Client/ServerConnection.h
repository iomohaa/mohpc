#pragma once

#include "../NetGlobal.h"
#include "../NetObject.h"

#include "../Exception.h"
#include "../Remote/Channel.h"
#include "../Remote/Encoding.h"
#include "../Remote/Ops.h"
#include "../Configstring.h"
#include "../Parsing/String.h"
#include "../Parsing/Hash.h"
#include "../Types/Snapshot.h"
#include "../Types/Reliable.h"
#include "../../Utility/HandlerList.h"
#include "../../Utility/Info.h"
#include "../../Utility/PropertyMap.h"
#include "../../Utility/TokenParser.h"
#include "../../Utility/RemoteIdentifier.h"
#include "../../Utility/Tick.h"
#include "../../Utility/Timeout.h"
#include "../../Utility/Misc/MSG/MSG.h"
#include "UserInfo.h"
#include "Imports.h"
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

		class Event;
		class IClientGameProtocol;

		/** Previously 128, the number has been doubled. */
		static constexpr unsigned long MAX_PACKET_USERCMDS = 32;

		class INetchan;
		struct gameState_t;

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
			 * Called when the server sent a command. The library doesn't process any commands.
			 *
			 * @param	command		Command to process.
			 */
			struct ServerCommand : public HandlerNotifyBase<void(const char* command, const TokenParser& ev)> {};

			/**
			 * This callback is used to modify the player input before sending.
			 *
			 * @param	cmd		User command structure.
			 * @param	eyes	Eyes information.
			 */
			struct UserInput : public HandlerNotifyBase<void(usercmd_t& cmd, usereyes_t& eyes)> {};

			/**
			 * Called just before writing a packet and sending it.
			 */
			struct PreWritePacket : public HandlerNotifyBase<void()> {};
		}

#if 0
		struct gameState_t
		{
			static constexpr size_t MAX_GAMESTATE_CHARS = 40000;

		public:
			MOHPC_NET_EXPORTS gameState_t();

			/**
			 * Return the configstring at the specified number.
			 *
			 * @param	num		Config string ID. Must be < MAX_CONFIGSTRINGS
			 * @return	The configstring. NULL if num is greater than MAX_CONFIGSTRINGS
			 */
			MOHPC_NET_EXPORTS const char* getConfigString(csNum_t num) const;
			const char* getConfigStringChecked(csNum_t num) const;

			/**
			 * Return the configstring at the specified number.
			 *
			 * @param	num				Config string ID. Must be < MAX_CONFIGSTRINGS
			 * @param	configString	The value to put in.
			 * @param	sz				Size of the config string
			 */
			MOHPC_NET_EXPORTS void setConfigString(csNum_t num, const char* configString, size_t sz);

		public:
			size_t dataCount;
			size_t stringOffsets[MAX_CONFIGSTRINGS];
			// could have made stringData a dynamic buffer
			char stringData[MAX_GAMESTATE_CHARS];
		};
#endif

		class ClientInfoHelper
		{
		public:
			static void fillInfoString(const ClientInfo& clientInfo, Info& info);
		};

		class DownloadManager
		{
		public:
			using startCallback_f = Function<bool(const char*)>;
			using receiveCallback_f = Function<bool(const uint8_t*, size_t)>;

		public:
			DownloadManager();

			/**
			 * Set client imports API
			 */
			void setImports(ClientImports& inImports) noexcept;

			/**
			 * Called to process a download message
			 */
			void processDownload(MSG& msg, const Parsing::IString& stringParser);

			/**
			 * Start downloading from server.
			 *
			 * @param downloadName Name of the download.
			 */
			void startDownload(const char* downloadName);

			/**
			 * Cancel downloading.
			 */
			void cancelDownload();

			/**
			 * Set the callback to be used when starting downloads.
			 */
			void setDownloadStartedCallback(startCallback_f&& callback);

			/**
			 * Set the callback to be used when receiving data.
			 */
			void setReceiveCallback(receiveCallback_f&& callback);

		private:
			void clearDownload();
			void nextDownload();
			void downloadsComplete();
			bool receive(const uint8_t* data, const size_t size);

		private:
			ClientImports imports;
			startCallback_f startCallback;
			Function<bool(const uint8_t*, size_t)> receiveCallback;
			size_t downloadSize;
			str downloadName;
			uint32_t downloadBlock;
			bool downloadRequested;
		};

		template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
		class ClientSequenceTemplate : public IReliableSequence
		{
		public:
			void set(rsequence_t index, const char* command) override;
			const char* get(rsequence_t index) const override;
			size_t getMaxElements() const override;

		private:
			char reliableCommands[MAX_RELIABLE_COMMANDS][RELIABLE_COMMAND_SIZE];
		};

		template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
		class ClientRemoteCommandSequenceTemplate : public ICommandSequence
		{
		public:
			void set(rsequence_t index, const char* command) override;
			const char* get(rsequence_t index) const override;
			size_t getMaxElements() const override;

		private:
			char serverCommands[MAX_RELIABLE_COMMANDS][RELIABLE_COMMAND_SIZE];
		};

		class UserPacket
		{
		public:
			UserPacket();

			void sendPacket(uint64_t currentTime);
		};

		class UserModule
		{
		private:
			struct HandlerList
			{
				FunctionList<ClientHandlers::UserInput> userInputHandler;
			};

		public:
			UserModule();

			void write(MSG& msg, uint64_t currentTime);
			HandlerList& getHandlerList() const;

		private:
			clc_ops_e getClientOperation() const;
			uint32_t getCommandHashKey() const;
			uint8_t getNumCommandsToWrite(uint32_t oldPacketNum) const;
			void writeAllCommands(MSG& msg, const usercmd_t*& oldcmd, size_t count, uint32_t key);

		private:
			HandlerList handlerList;
			UserInput input;
			outPacket_t outPackets[CMD_BACKUP];
		};

		class PacketHeaderWriter
		{
		public:
			PacketHeaderWriter(const ServerGameState& clGameStateRef, const ICommandSequence& serverCommandsRef, uint32_t serverMessageSequenceValue);

			void write(MSG& msg);

		private:
			const ICommandSequence& serverCommands;
			const ServerGameState& clGameState;
			uint32_t serverMessageSequence;
		};

		class ReliableCommandsWriter
		{
		public:
			ReliableCommandsWriter(const IReliableSequence& reliableCommandsRef, const Parsing::IString& stringParserRef);

			void write(MSG& msg);

		private:
			const IReliableSequence& reliableCommands;
			const Parsing::IString& stringParser;
		};

		class UserInputWriter
		{
		public:
			UserInputWriter(
				const UserInput& userInputRef,
				OutgoingPackets& outPacketsRef,
				const ServerSnapshotManager& clSnapshotManRef,
				const Parsing::IHash& hasherRef,
				const ICommandSequence* serverCommandsPtr,
				const ServerGameState& clGameStateRef
			);

			void write(MSG& msg, uint64_t currentTime, uint32_t outgoingSequence, uint32_t serverMessageSequence);

		private:
			clc_ops_e getClientOperation(uint32_t serverMessageSequence) const;
			uint32_t getCommandHashKey(uint32_t serverMessageSequence) const;
			uint8_t getNumCommandsToWrite(uint32_t oldPacketNum) const;
			void writeAllCommands(MSG& msg, const usercmd_t*& oldcmd, size_t count, uint32_t key);
			void storeOutputPacket(uint64_t currentTime, uint32_t serverTime, uint32_t outgoingSequence);

		private:
			const UserInput& userInput;
			OutgoingPackets& outPackets;
			const ServerSnapshotManager& clSnapshotMan;
			const Parsing::IHash& hasher;
			const ICommandSequence* serverCommands;
			const ServerGameState& clGameState;
		};

		class MOHPC_NET_EXPORTS clientGameSettings_t
		{
		public:
			clientGameSettings_t();

			/** Return the maximum number of packets that can be sent per second. */
			uint32_t getMaxPackets() const;

			/** Set the maximum number of packets that can be sent per second, in the range of [1, 125]. */
			void setMaxPackets(uint32_t inMaxPackets);

			/** Return the maximum number of packets that can be processed at once in once tick. */
			uint32_t getMaxTickPackets() const;

			/** Set the maximum number of packets that can be sent processed at once in one tick, in the range of [1, 1000]. */
			void setMaxTickPackets(uint32_t inMaxPackets);

			/** Set the maximum radar bounds. The default value is 1024. */
			void setRadarRange(float value);

			/** Return the radar range. */
			float getRadarRange() const;

			void setTimeNudge(uint32_t value);
			uint32_t getTimeNudge() const;

		private:
			uint32_t maxPackets;
			uint32_t maxTickPackets;
			float radarRange;
			uint32_t timeNudge;
		};

		/**
		 * Client game connection class.
		 *
		 * Maintains a connection to a server.
		 *
		 * Call markReady() method to allow client to send user input to server, making it enter the game.
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
				FunctionList<ClientHandlers::ServerCommand> serverCommandManager;
				FunctionList<ClientHandlers::UserInput> userInputHandler;
				FunctionList<ClientHandlers::PreWritePacket> preWritePacketHandler;
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
			ServerConnection(const INetchanPtr& netchan, const IRemoteIdentifierPtr& inAdr, uint32_t challengeResponse, const protocolType_c& protoType, const ClientInfoPtr& cInfo);
			~ServerConnection();

			// ITickable
			// ~
			void tick(uint64_t deltaTime, uint64_t currentTime) override;
			// ~

			/**
			 * Get the timeout timer for the client.
			 */
			MOHPC_NET_EXPORTS TimeoutTimer& getTimeoutTimer();
			MOHPC_NET_EXPORTS const TimeoutTimer& getTimeoutTimer() const;

			/** Return the client time that manages client and remote time. */
			MOHPC_NET_EXPORTS const ClientTime& getClientTime() const;

			/** Return the client time that manages client and remote time. */
			MOHPC_NET_EXPORTS const UserInput& getUserInput() const;

			/** Return the client command object for managing commands. */
			MOHPC_NET_EXPORTS IReliableSequence& getClientCommands() const;

			/** Return the client command object for managing commands. */
			MOHPC_NET_EXPORTS ICommandSequence& getServerCommands() const;

			/** Return the handler list. */
			MOHPC_NET_EXPORTS HandlerListClient& getHandlerList();

			/** Return the IP address of the remote server. */
			const IRemoteIdentifierPtr& getRemoteAddress() const;

			/** Retrieve the current game state. */
			MOHPC_NET_EXPORTS ServerGameState& getGameState();
			MOHPC_NET_EXPORTS const ServerGameState& getGameState() const;

			/** Retrieve the CGame module. */
			MOHPC_NET_EXPORTS CGame::ModuleBase* getCGModule();

			/** Return read-only user info. */
			MOHPC_NET_EXPORTS ConstClientInfoPtr getUserInfo() const;

			/** Return modifiable user info. */
			MOHPC_NET_EXPORTS const ClientInfoPtr& getUserInfo();

			/** Send the server a new user info string. Must be called when done modifying the userinfo. */
			MOHPC_NET_EXPORTS void updateUserInfo();

			/** Return the settings that the connection is using. */
			MOHPC_NET_EXPORTS clientGameSettings_t& getSettings();
			MOHPC_NET_EXPORTS const clientGameSettings_t& getSettings() const;

			/** Return the current snapshot number. */
			MOHPC_NET_EXPORTS ServerSnapshotManager& getSnapshotManager();
			MOHPC_NET_EXPORTS const ServerSnapshotManager& getSnapshotManager() const;

			/** Return the current server message sequence (the latest packet number). */
			MOHPC_NET_EXPORTS uint32_t getCurrentServerMessageSequence() const;

			/** Return the current server command sequence. */
			MOHPC_NET_EXPORTS const ICommandSequence* getCommandSequence() const;

			/** Return the current reliable sequence (the latest command number on the client). */
			MOHPC_NET_EXPORTS const IReliableSequence* getReliableSequence() const;

			/**
			 * Disconnect the client.
			 * Can also be called from the server.
			 */
			MOHPC_NET_EXPORTS void disconnect();

			/**
			 * Return whether or not an user cmd can be created.
			 */
			MOHPC_NET_EXPORTS bool canCreateCommand() const;

			/**
			 * Mark the client as ready (to send commands). Useful for loading maps.
			 */
			MOHPC_NET_EXPORTS void markReady();

			/**
			 * Make the client not ready (won't send new commands anymore).
			 */
			MOHPC_NET_EXPORTS void unmarkReady();

			/**
			 * Send a command to server.
			 *
			 * @param command Command to send.
			 */
			MOHPC_NET_EXPORTS void sendCommand(const char* command);

		private:
			const INetchanPtr& getNetchan() const;
			void receive(const IRemoteIdentifierPtr& from, MSG& msg, uint64_t currentTime, uint32_t sequenceNum);
			void receiveConnectionLess(const IRemoteIdentifierPtr& from, MSG& msg);
			void wipeChannel();
			bool isChannelValid() const;
			void serverDisconnected(const char* reason);
			void terminateConnection(const char* reason);
			void addReliableCommand(const char* command);

			void parseServerMessage(MSG& msg, uint64_t currentTime);
			void parseCommandString(MSG& msg);
			void parseCenterprint(MSG& msg);
			void parseLocprint(MSG& msg);
			void parseClientCommand(const char* arguments);

			void clearState();
			void setCGameTime(uint64_t currentTime);
			void initSnapshot(uint64_t currentTime);

			bool readyToSendPacket(uint64_t currentTime) const;
			bool sendCmd(uint64_t currentTime);

			void writePacket(uint64_t currentTime);

			void fillClientImports(ClientImports& imports);

		private:
			HandlerListClient handlerList;
			CGame::ModuleBase* cgameModule;
			const Parsing::IString* stringParser;
			const Parsing::IHash* hashParser;
			IRemoteIdentifierPtr adr;
			EncodingPtr encoder;
			INetchanPtr netchan;
			ClientInfoPtr userInfo;
			IReliableSequence* reliableCommands;
			ICommandSequence* serverCommands;
			ClientTime clientTime;
			UserInput input;
			TimeoutTimer timeout;
			ServerGameState clGameState;
			ServerSnapshotManager clSnapshotManager;
			clientGameSettings_t settings;
			DownloadManager downloadState;
			OutgoingPackets outPackets;
			uint64_t lastPacketSendTime;
			uint32_t serverMessageSequence;
			bool isActive : 1;
			bool isReady : 1;
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

			/**
			 * Server error while downloading.
			 */
			class DownloadException : public Base
			{
			public:
				DownloadException(StringMessage&& inError);

				/** Return the error the server sent. */
				MOHPC_NET_EXPORTS const char* getError() const;
				MOHPC_NET_EXPORTS str what() const override;

			private:
				StringMessage error;
			};

			class DownloadSizeException : public Base
			{
			public:
				DownloadSizeException(uint16_t inSize);

				MOHPC_NET_EXPORTS uint16_t getSize() const;

			private:
				uint16_t size;
			};

			class BadDownloadBlockException : public Base
			{
			public:
				BadDownloadBlockException(uint16_t block, uint16_t expectedBlock);

				MOHPC_NET_EXPORTS uint16_t getBlock() const noexcept;
				MOHPC_NET_EXPORTS uint16_t getExpectedBlock() const noexcept;

			private:
				uint16_t block;
				uint16_t expectedBlock;

			};

			class DisconnectException : public Base
			{
			};

			/**
			 * When the server is sending a download but it hasn't been requested by the client
			 */
			class MOHPC_NET_EXPORTS UnexpectedDownloadException : public NetworkException
			{
			};

			class BadSoundNumberException : public Base
			{

			};
		}

		template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
		void ClientSequenceTemplate<MAX_RELIABLE_COMMANDS, RELIABLE_COMMAND_SIZE>::set(rsequence_t index, const char* command)
		{
			str::copyn(reliableCommands[index], command, RELIABLE_COMMAND_SIZE);
		}

		template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
		const char* ClientSequenceTemplate<MAX_RELIABLE_COMMANDS, RELIABLE_COMMAND_SIZE>::get(rsequence_t index) const
		{
			return reliableCommands[index];
		}

		template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
		size_t ClientSequenceTemplate<MAX_RELIABLE_COMMANDS, RELIABLE_COMMAND_SIZE>::getMaxElements() const
		{
			return RELIABLE_COMMAND_SIZE;
		}

		template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
		void ClientRemoteCommandSequenceTemplate<MAX_RELIABLE_COMMANDS, RELIABLE_COMMAND_SIZE>::set(rsequence_t index, const char* command)
		{
			str::copyn(serverCommands[index], command, RELIABLE_COMMAND_SIZE);
		}

		template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
		const char* ClientRemoteCommandSequenceTemplate<MAX_RELIABLE_COMMANDS, RELIABLE_COMMAND_SIZE>::get(rsequence_t index) const
		{
			return serverCommands[index];
		}

		template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
		size_t ClientRemoteCommandSequenceTemplate<MAX_RELIABLE_COMMANDS, RELIABLE_COMMAND_SIZE>::getMaxElements() const
		{
			return RELIABLE_COMMAND_SIZE;
		}
	}
}