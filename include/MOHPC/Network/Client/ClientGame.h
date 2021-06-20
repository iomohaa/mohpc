#pragma once

#include "../Types.h"
#include "../InfoTypes.h"
#include "../Channel.h"
#include "../Encoding.h"
#include "../Reliable.h"
#include "../Event.h"
#include "../Configstring.h"
#include "../Types/GameState.h"
#include "../../Utility/HandlerList.h"
#include "../../Utility/Info.h"
#include "../../Utility/PropertyMap.h"
#include "../../Utility/TokenParser.h"
#include "../../Utility/RemoteIdentifier.h"
#include "../../Utility/Tick.h"
#include "../../Utility/Timeout.h"
#include "../NetObject.h"
#include "../../Utility/Misc/MSG/MSG.h"
#include "UserInfo.h"
#include "Imports.h"
#include "ProtocolParsing.h"
#include <cstdint>
#include <functional>
#include <type_traits>
#include <bitset>

namespace MOHPC
{
	class MSG;

	namespace Network
	{
		class CGameModuleBase;
		class Event;
		class ClientSnapshot;
		class IClientGameProtocol;

		static constexpr unsigned long PACKET_BACKUP = (1 << 5);
		static constexpr unsigned long PACKET_MASK = PACKET_BACKUP - 1;
		static constexpr unsigned long MAX_PARSE_ENTITIES = 2048;
		/** Previously 128, the number has been doubled. */
		static constexpr unsigned long CMD_BACKUP = (1 << 8);
		static constexpr unsigned long CMD_MASK = CMD_BACKUP - 1;
		static constexpr unsigned long MAX_PACKET_USERCMDS = 32;

		class INetchan;
		struct gameState_t;

		using ClientGameConnectionPtr = SharedPtr<class ClientGameConnection>;

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
			 * REMOVED! Entity handlers from CGame Module should be used instead.
			 *
			 * Called when an entity has been modified or added.
			 *
			 * @param	old		The entity before modification.
			 * @param	state	The new entity.
			 */
			//struct EntityRead : public HandlerNotifyBase<void(const entityState_t* old, const entityState_t* state)> {};

			/**
			 * Called when the current player state has been modified.
			 *
			 * @param	old		Previous player state (can be NULL).
			 * @param	state	Current player state.
			 */
			struct PlayerstateRead : public HandlerNotifyBase<void(const playerState_t* old, const playerState_t* state)> {};

			/**
			 * Called after the insertion/modification of a new configstring.
			 *
			 * @param	csNum			The configstring num.
			 * @param	configString	The string pointed at by the csNum.
			 */
			struct Configstring : public HandlerNotifyBase<void(csNum_t csNum, const char* configString)> {};

			/**
			 * Called when a sound started to play/stopped.
			 *
			 * @param	sound	Sound structure containing various informations.
			 */
			struct Sound : public HandlerNotifyBase<void(const sound_t& sound)> {};

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
			 * Called when receiving the first valid snapshot on each map load. Useful to get the server time at this point.
			 *
			 * @param	snap	The first snapshot received.
			 */
			struct FirstSnapshot : public HandlerNotifyBase<void(const ClientSnapshot& snap)> {};

			/**
			 * Called when the server has restarted (when a restart command has been issued on the server).
			 */
			struct ServerRestarted : public HandlerNotifyBase<void()> {};

			/**
			 * Called when a snap was received.
			 *
			 * @param	snap	The snapshot that was received.
			 */
			struct SnapReceived : public HandlerNotifyBase<void(const ClientSnapshot& snap)> {};

			/**
			 * Called the game state was parsed. Can be called multiple times.
			 * This is the callback to use for map change/loading.
			 *
			 * @param   gameState       New game state.
			 * @param   differentLevel  False if the client has to re-download game state for the current game.
			 *                          True = new game session (but doesn't necessarily mean that it is a different map file).
			 */
			struct GameStateParsed : public HandlerNotifyBase<void(const IGameState& gameState, bool differentLevel)> {};

			/**
			 * Called when a client is not visible to the player.
			 * It is used to update radar information (radar = teammate icons on the compass)
			 * to be able to keep track of players (teammates) that are not visible.
			 */
			struct ReadNonPVSClient : public HandlerNotifyBase<void(const radarUnpacked_t& radarUnpacked)> {};

			/**
			 * Called just before writing a packet and sending it.
			 */
			struct PreWritePacket : public HandlerNotifyBase<void()> {};
		}

		struct outPacket_t
		{
		public:
			/** cl.cmdNumber when packet was sent */
			uint32_t p_cmdNumber;

			/** usercmd->serverTime when packet was sent */
			uint32_t p_serverTime;

			/** time when packet was sent */
			uint64_t p_currentTime;

			/** eyeInfo when packet was sent */
			usereyes_t p_eyeinfo;

		public:
			outPacket_t();
		};

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

		class ClientSnapshot
		{
		public:
			int32_t deltaNum;
			uint32_t ping;
			uint32_t serverTime;
			uint32_t messageNum;
			uint32_t numEntities;
			uint32_t parseEntitiesNum;
			uint32_t serverCommandNum;
			uint32_t numSounds;
			uint8_t snapFlags;
			uint8_t serverTimeResidual;
			uint8_t areamask[MAX_MAP_AREA_BYTES];
			uint8_t cmdNum;
			playerState_t ps;
			bool valid;
			sound_t sounds[MAX_SERVER_SOUNDS];

		public:
			ClientSnapshot();
		};

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
			void processDownload(MSG& msg);

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

		class MOHPC_NET_EXPORTS ClientTime
		{
		public:
			ClientTime();
			~ClientTime();

			void initRemoteTime(uint64_t currentTime, uint64_t remoteTimeValue);
			void setStartTime(uint64_t currentTime);
			void setRemoteStartTime(uint64_t remoteTimeValue);
			uint64_t getStartTime() const;
			uint64_t getRemoteStartTime() const;
			uint64_t getRemoteTime() const;
			void setTime(uint64_t newTime, uint64_t remoteTime, uint64_t remoteDeltaTime, bool adjust);
			void setTimeNudge(int32_t timeNudgeValue);
		
		private:
			void adjustTimeDelta(uint64_t realTime, uint64_t remoteTime, uint64_t remoteDeltaTime);
			uint64_t getTimeDelta(uint64_t time, uint64_t remoteTime) const;
		
		private:
			uint64_t realTimeStart;
			uint64_t serverStartTime;
			uint64_t serverTime;
			uint64_t oldServerTime;
			uint64_t oldFrameServerTime;
			uint64_t oldRealTime;
			int32_t timeNudge;
			bool extrapolatedSnapshot;
		};

		template<size_t RELIABLE_COMMAND_SIZE>
		class ClientSequenceTemplate : public IReliableSequence
		{
		public:
			void set(rsequence_t index, const char* command) override;
			const char* get(rsequence_t index) const override;
			size_t getMaxElements() const override;

		private:
			char reliableCommands[MAX_RELIABLE_COMMANDS][RELIABLE_COMMAND_SIZE];
		};

		template<size_t RELIABLE_COMMAND_SIZE>
		class ClientRemoteCommandSequenceTemplate : public ICommandSequence
		{
		public:
			void set(rsequence_t index, const char* command) override;
			const char* get(rsequence_t index) const override;
			size_t getMaxElements() const override;

		private:
			char serverCommands[MAX_RELIABLE_COMMANDS][RELIABLE_COMMAND_SIZE];
		};

		class UserInput
		{
		public:
			UserInput();

			void reset();
			void createCommand(uint64_t currentTime, uint64_t remoteTime, usercmd_t*& outCmd, usereyes_t*& outEyes);
			uint32_t getCurrentCmdNumber() const;
			const usercmd_t& getCommand(size_t index) const;
			const usercmd_t& getCommandFromLast(size_t index) const;
			const usercmd_t& getLastCommand() const;
			usercmd_t& getLastCommand();
			const usereyes_t& getEyeInfo() const;
			usereyes_t& getEyeInfo();

		private:
			uint32_t cmdNumber;
			usereyes_t eyeinfo;
			usercmd_t cmds[CMD_BACKUP];
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
			PacketHeaderWriter(const IGameState& gameStateRef, const ICommandSequence& serverCommandsRef, uint32_t serverMessageSequenceValue);

			void write(MSG& msg);

		private:
			const ICommandSequence& serverCommands;
			const IGameState& gameState;
			uint32_t serverMessageSequence;
		};

		class ReliableCommandsWriter
		{
		public:
			ReliableCommandsWriter(const IReliableSequence& reliableCommandsRef, const IClientGameProtocol& clientProtoRef);

			void write(MSG& msg);

		private:
			const IReliableSequence& reliableCommands;
			const IClientGameProtocol& clientProto;
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

		class UserInputWriter
		{
		public:
			UserInputWriter(const UserInput& userInputRef, const IClientGameProtocol& clientProtoRef, const outPacket_t& oldPacketRef);

			void write(MSG& msg, uint64_t currentTime);

		private:
			clc_ops_e getClientOperation() const;
			uint32_t getCommandHashKey() const;
			uint8_t getNumCommandsToWrite(uint32_t oldPacketNum) const;
			void writeAllCommands(MSG& msg, const usercmd_t*& oldcmd, size_t count, uint32_t key);

		private:
			const UserInput& userInput;
			const IClientGameProtocol& clientProto;
			const outPacket_t& oldPacket;
		};
		
		/**
		 * Client game connection class.
		 *
		 * Maintains a connection to a server.
		 *
		 * Call markReady() method to allow client to send user input to server, thus making it enter the game.
		 */
		class ClientGameConnection : public ITickable
		{
			MOHPC_NET_OBJECT_DECLARATION(ClientGameConnection);

		private:
			struct HandlerListClient
			{
			public:
				FunctionList<ClientHandlers::Timeout> timeoutHandler;
				FunctionList<ClientHandlers::Disconnect> disconnectHandler;
				FunctionList<ClientHandlers::Error> errorHandler;
				FunctionList<ClientHandlers::PlayerstateRead> playerStateReadHandler;
				FunctionList<ClientHandlers::Configstring> configStringHandler;
				FunctionList<ClientHandlers::Sound> soundHandler;
				FunctionList<ClientHandlers::CenterPrint> centerPrintHandler;
				FunctionList<ClientHandlers::LocationPrint> locationPrintHandler;
				FunctionList<ClientHandlers::ServerCommand> serverCommandHandler;
				FunctionList<ClientHandlers::UserInput> userInputHandler;
				FunctionList<ClientHandlers::FirstSnapshot> firstSnapshotHandler;
				FunctionList<ClientHandlers::SnapReceived> snapshotReceivedHandler;
				FunctionList<ClientHandlers::ServerRestarted> serverRestartedHandler;
				FunctionList<ClientHandlers::GameStateParsed> gameStateParsedHandler;
				FunctionList<ClientHandlers::ReadNonPVSClient> readNonPVSClientHandler;
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
			ClientGameConnection(const INetchanPtr& netchan, const IRemoteIdentifierPtr& inAdr, uint32_t challengeResponse, const protocolType_c& protoType, const ClientInfoPtr& cInfo);
			~ClientGameConnection();

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
			MOHPC_NET_EXPORTS IGameState& getGameState() const;

			/** Retrieve the CGame module. */
			MOHPC_NET_EXPORTS CGameModuleBase* getCGModule();

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
			MOHPC_NET_EXPORTS uintptr_t getCurrentSnapshotNumber() const;

			/**
			 * Return snapshot data.
			 *
			 * @param	snapshotNum		The snapshot number to get data from.
			 * @param	outSnapshot		Output data.
			 * @return	true if the snap is valid.
			 */
			MOHPC_NET_EXPORTS bool getSnapshot(uintptr_t snapshotNum, SnapshotInfo& outSnapshot) const;

			/** Return the frequency at which the game server is running (1 / sv_fps). */
			MOHPC_NET_EXPORTS uint64_t getServerFrameTime() const;

			/** Return the current client number. */
			MOHPC_NET_EXPORTS uint32_t getClientNum() const;

			/** Return the current server message sequence (the latest packet number). */
			MOHPC_NET_EXPORTS uint32_t getCurrentServerMessageSequence() const;

			/** Return the current server command sequence (the latest command number). */
			MOHPC_NET_EXPORTS uint32_t getCurrentServerCommandSequence() const;

			/** Return the current reliable sequence (the latest command number on the client). */
			MOHPC_NET_EXPORTS uint32_t getReliableSequence() const;

			/** Return the current reliable acknowledge (the latest command number on the server). */
			MOHPC_NET_EXPORTS uint32_t getReliableAcknowledge() const;

			/**
			 * Return user input data.
			 *
			 * @param	cmdNum		The cmd number to get data from.
			 * @param	outCmd		Output data.
			 * @return	true if the command is valid.
			 */
			MOHPC_NET_EXPORTS bool getUserCmd(uintptr_t cmdNum, usercmd_t& outCmd) const;

			/**
			 * Return parseable server command.
			 * @param	serverCommandNumber		The command num to get.
			 * @param	tokenized				Tokenized data.
			 * @return	true if the command is valid.
			 */
			MOHPC_NET_EXPORTS bool getServerCommand(rsequence_t serverCommandNumber, TokenParser& tokenized);

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
			void parseGameState(MSG& msg);
			void parseSnapshot(MSG& msg, uint64_t currentTime);
			void parsePacketEntities(MSG& msg, const ClientSnapshot* oldFrame, ClientSnapshot* newFrame);
			void parseDeltaEntity(MSG& msg, ClientSnapshot* frame, uint32_t newNum, entityState_t* old, bool unchanged);
			void parseSounds(MSG& msg, ClientSnapshot* newFrame);
			void parseDownload(MSG& msg);
			void parseCommandString(MSG& msg);
			void parseCenterprint(MSG& msg);
			void parseLocprint(MSG& msg);
			void parseClientCommand(const char* arguments);

			void clearState();
			bool isDifferentServer(uint32_t id);
			void setCGameTime(uint64_t currentTime);
			void firstSnapshot(uint64_t currentTime);
			void serverRestarted();
			void updateSnapFlags();
			void calculatePing(uint64_t currentTime);

			bool readyToSendPacket(uint64_t currentTime) const;
			bool sendCmd(uint64_t currentTime);
			clc_ops_e getClientOperation() const;
			uint32_t getCommandHashKey() const;
			uint8_t getNumCommandsToWrite(uint32_t oldPacketNum) const;

			void writePacket(uint64_t currentTime);
			void writeUserInput(MSG& msg, uint64_t currentTime);
			void writeAllCommands(MSG& msg, const usercmd_t*& oldcmd, size_t count, uint32_t key);
			void storeOutputPacket(uint64_t currentTime, uint32_t serverTime);

			const IClientGameProtocol& getProtocol(uint32_t protocolNum) const;
			void fillClientImports(ClientImports& imports);
			void readNonPVSClient(radarInfo_t radarInfo);
			const ClientSnapshot* readOldSnapshot(MSG& msg, ClientSnapshot& snap) const;
			void readAreaMask(MSG& msg, ClientSnapshot& snap);
			void setNewSnap(ClientSnapshot& newSnap);

		private:
			HandlerListClient handlerList;
			const IClientGameProtocol& clientProto;
			CGameModuleBase* cgameModule;
			EncodingPtr encoder;
			INetchanPtr netchan;
			ClientInfoPtr userInfo;
			IGameState* gameStatePtr;
			IReliableSequence* reliableCommands;
			ICommandSequence* serverCommands;
			ClientTime clientTime;
			UserInput input;
			TimeoutTimer timeout;
			uint64_t lastPacketSendTime;
			uint32_t parseEntitiesNum;
			uint32_t serverMessageSequence;
			uint32_t lastSnapFlags;
			clientGameSettings_t settings;
			IRemoteIdentifierPtr adr;
			bool newSnapshots : 1;
			bool extrapolatedSnapshot : 1;
			bool isActive : 1;
			bool isReady : 1;
			gameState_t gameState;
			DownloadManager downloadState;
			ClientSnapshot currentSnap;
			ClientSnapshot snapshots[PACKET_BACKUP];
			outPacket_t outPackets[CMD_BACKUP];
			entityState_t entityBaselines[MAX_GENTITIES];
			entityState_t parseEntities[MAX_PARSE_ENTITIES];
		};

		using ClientGameConnectionPtr = SharedPtr<ClientGameConnection>;

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
			 * Bad area mask size while parsing snapshot.
			 */
			class AreaMaskBadSize : public Base
			{
			public:
				AreaMaskBadSize(uint8_t inSize);

				/** Return the size of the area mask. */
				MOHPC_NET_EXPORTS uint8_t getSize() const;
				MOHPC_NET_EXPORTS str what() const override;

			private:
				uint8_t size;
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

		template<size_t RELIABLE_COMMAND_SIZE>
		void ClientSequenceTemplate<RELIABLE_COMMAND_SIZE>::set(rsequence_t index, const char* command)
		{
			str::copyn(reliableCommands[index], command, RELIABLE_COMMAND_SIZE);
		}

		template<size_t RELIABLE_COMMAND_SIZE>
		const char* ClientSequenceTemplate<RELIABLE_COMMAND_SIZE>::get(rsequence_t index) const
		{
			return reliableCommands[index];
		}

		template<size_t RELIABLE_COMMAND_SIZE>
		size_t ClientSequenceTemplate<RELIABLE_COMMAND_SIZE>::getMaxElements() const
		{
			return RELIABLE_COMMAND_SIZE;
		}

		template<size_t RELIABLE_COMMAND_SIZE>
		void ClientRemoteCommandSequenceTemplate<RELIABLE_COMMAND_SIZE>::set(rsequence_t index, const char* command)
		{
			str::copyn(serverCommands[index], command, RELIABLE_COMMAND_SIZE);
		}

		template<size_t RELIABLE_COMMAND_SIZE>
		const char* ClientRemoteCommandSequenceTemplate<RELIABLE_COMMAND_SIZE>::get(rsequence_t index) const
		{
			return serverCommands[index];
		}

		template<size_t RELIABLE_COMMAND_SIZE>
		size_t ClientRemoteCommandSequenceTemplate<RELIABLE_COMMAND_SIZE>::getMaxElements() const
		{
			return RELIABLE_COMMAND_SIZE;
		}
	}
}