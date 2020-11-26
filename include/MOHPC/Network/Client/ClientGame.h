#pragma once

#include "../Types.h"
#include "../InfoTypes.h"
#include "../Channel.h"
#include "../Encoding.h"
#include "../Event.h"
#include "../Configstring.h"
#include "../../Utilities/HandlerList.h"
#include "../../Utilities/Info.h"
#include "../../Utilities/PropertyMap.h"
#include "../../Utilities/TokenParser.h"
#include "../../Utilities/RemoteIdentifier.h"
#include "../../Utilities/Tick.h"
#include "../../Object.h"
#include "../../Misc/MSG/MSG.h"
#include "../../Managers/NetworkManager.h"
#include "UserInfo.h"
#include "Imports.h"
#include <stdint.h>
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

		static constexpr unsigned long PACKET_BACKUP = (1 << 5); // 32
		static constexpr unsigned long PACKET_MASK = PACKET_BACKUP - 1;
		static constexpr unsigned long MAX_PARSE_ENTITIES = 2048;
		static constexpr unsigned long CMD_BACKUP = (1 << 8); // increased to 256
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
			 * @param	message		String message to print.
			 */
			struct CenterPrint : public HandlerNotifyBase<void(const char* message)> {};

			/**
			 * Called when the server sent a string to print at a 2D location of the screen.
			 *
			 * @param	message		String message to print.
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
			struct GameStateParsed : public HandlerNotifyBase<void(const gameState_t& gameState, bool differentLevel)> {};

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

		/**
		 * When the server is sending a download but it hasn't been requested by the client
		 */
		class MOHPC_EXPORTS UnexpectedDownloadException : public NetworkException
		{
		};

		struct gameState_t
		{
			static constexpr size_t MAX_GAMESTATE_CHARS = 40000;

		public:
			MOHPC_EXPORTS gameState_t();

			/**
			 * Return the configstring at the specified number.
			 *
			 * @param	num		Config string ID. Must be < MAX_CONFIGSTRINGS
			 * @return	The configstring. NULL if num is greater than MAX_CONFIGSTRINGS
			 */
			MOHPC_EXPORTS const char* getConfigString(csNum_t num) const;
			const char* getConfigStringChecked(csNum_t num) const;

			/**
			 * Return the configstring at the specified number.
			 *
			 * @param	num				Config string ID. Must be < MAX_CONFIGSTRINGS
			 * @param	configString	The value to put in.
			 * @param	sz				Size of the config string
			 */
			MOHPC_EXPORTS void setConfigString(csNum_t num, const char* configString, size_t sz);

		public:
			size_t dataCount;
			size_t stringOffsets[MAX_CONFIGSTRINGS];
			// could have made stringData a dynamic buffer
			char stringData[MAX_GAMESTATE_CHARS];
		};

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

		class MOHPC_EXPORTS clientGameSettings_t
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
		 * Call markReady() method to allow client to send user input to server, thus making it enter the game.
		 */
		class ClientGameConnection : public ITickable
		{
			MOHPC_OBJECT_DECLARATION(ClientGameConnection);

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

		private:
			using parse_f = void (ClientGameConnection::*)(MSG& msg);
			using readString_f = StringMessage(*)(MSG& msg);
			using writeString_f = void (*)(MSG& msg, const char* s);
			using hashKey_f = uint32_t (ClientGameConnection::*)(const char* string, size_t maxlen) const;
			using readEntityNum_f = entityNum_t (ClientGameConnection::*)(MsgTypesHelper& msgHelper);
			using readDeltaPlayerstate_f = void(ClientGameConnection::*)(MSG& msg, const playerState_t* from, playerState_t* to);
			using readDeltaEntity_f = void(ClientGameConnection::*)(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum);
			using getNormalizedConfigstring_f = csNum_t(ClientGameConnection::*)(csNum_t num);
			using readNonPVSClient_f = void(ClientGameConnection::*)(radarInfo_t radarInfo);
			using getMaxCommandSize_f = size_t(ClientGameConnection::*)() const;

		private:
			std::chrono::steady_clock::time_point lastTimeoutTime;
			HandlerListClient handlerList;
			parse_f parseGameState_pf;
			readString_f readStringMessage_pf;
			writeString_f writeStringMessage_pf;
			hashKey_f hashKey_pf;
			readEntityNum_f readEntityNum_pf;
			readDeltaPlayerstate_f readDeltaPlayerstate_pf;
			readDeltaEntity_f readDeltaEntity_pf;
			getNormalizedConfigstring_f getNormalizedConfigstring_pf;
			readNonPVSClient_f readNonPVSClient_pf;
			getMaxCommandSize_f getMaxCommandSize_pf;
			CGameModuleBase* cgameModule;
			EncodingPtr encoder;
			INetchanPtr netchan;
			ClientInfoPtr userInfo;
			uint64_t realTimeStart;
			uint64_t serverStartTime;
			uint64_t serverTime;
			uint64_t oldServerTime;
			uint64_t oldFrameServerTime;
			uint64_t oldRealTime;
			uint64_t lastPacketSendTime;
			uint64_t serverDeltaFrequency;
			std::chrono::milliseconds timeoutTime;
			uint32_t parseEntitiesNum;
			uint32_t serverCommandSequence;
			uint32_t serverMessageSequence;
			uint32_t cmdNumber;
			uint32_t clientNum;
			uint32_t checksumFeed;
			uint32_t serverId;
			uint32_t reliableSequence;
			uint32_t reliableAcknowledge;
			uint32_t lastSnapFlags;
			clientGameSettings_t settings;
			IRemoteIdentifierPtr adr;
			bool newSnapshots : 1;
			bool extrapolatedSnapshot : 1;
			bool isActive : 1;
			bool isReady : 1;
			char* reliableCommands[MAX_RELIABLE_COMMANDS];
			char* serverCommands[MAX_RELIABLE_COMMANDS];
			char reliableCmdStrings[MAX_STRING_CHARS * MAX_RELIABLE_COMMANDS];
			char serverCmdStrings[MAX_STRING_CHARS * MAX_RELIABLE_COMMANDS];
			usereyes_t userEyes;
			gameState_t gameState;
			DownloadManager downloadState;
			ClientSnapshot currentSnap;
			ClientSnapshot snapshots[PACKET_BACKUP];
			usercmd_t cmds[CMD_BACKUP];
			outPacket_t outPackets[CMD_BACKUP];
			entityState_t entityBaselines[MAX_GENTITIES];
			entityState_t parseEntities[MAX_PARSE_ENTITIES];

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
			virtual void tick(uint64_t deltaTime, uint64_t currentTime) override;
			// ~

			/** Called to set the current time on start. */
			void initTime(uint64_t currentTime);

			/**
			 * Set the timeout and the callback to be called when the client/server timeouts.
			 *
			 * @param	timeoutTime		Timeout in milliseconds before the game considers it timed out.
			 */
			MOHPC_EXPORTS void setTimeout(size_t timeoutTime);

			/** Return the handler list. */
			MOHPC_EXPORTS HandlerListClient& getHandlerList();

			/** Return the IP address of the remote server. */
			const IRemoteIdentifierPtr& getRemoteAddress() const;

			/** Retrieve the current game state. */
			MOHPC_EXPORTS const gameState_t& getGameState() const;

			/** Retrieve the CGame module. */
			MOHPC_EXPORTS CGameModuleBase* getCGModule();

			/** Retrieve the server system info such as the serverid, the timescale, cheats allowed, ... */
			MOHPC_EXPORTS ReadOnlyInfo getServerSystemInfo() const;

			/** Retrieve the server game configuration, such as the hostname, gametype, force respawn, timelimit, ... */
			MOHPC_EXPORTS ReadOnlyInfo getServerGameInfo() const;

			/** Return read-only user info. */
			MOHPC_EXPORTS ConstClientInfoPtr getUserInfo() const;

			/** Return modifiable user info. */
			MOHPC_EXPORTS const ClientInfoPtr& getUserInfo();

			/** Send the server a new user info string. Must be called when done modifying the userinfo. */
			MOHPC_EXPORTS void updateUserInfo();

			MOHPC_EXPORTS clientGameSettings_t& getSettings();

			MOHPC_EXPORTS const clientGameSettings_t& getSettings() const;

			/** Return the current snapshot number. */
			MOHPC_EXPORTS uintptr_t getCurrentSnapshotNumber() const;

			/**
			 * Return snapshot data.
			 *
			 * @param	snapshotNum		The snapshot number to get data from.
			 * @param	outSnapshot		Output data.
			 * @return	true if the snap is valid.
			 */
			MOHPC_EXPORTS bool getSnapshot(uintptr_t snapshotNum, SnapshotInfo& outSnapshot) const;

			/** Return the time at which the server started (in milliseconds). */
			MOHPC_EXPORTS uint64_t getServerStartTime() const;

			/** Return the current server time (in milliseconds). */
			MOHPC_EXPORTS uint64_t getServerTime() const;

			/** Return the frequency at which the game server is running (sv_fps). */
			MOHPC_EXPORTS uint64_t getServerFrameFrequency() const;

			/** Return the current user input number. */
			MOHPC_EXPORTS uintptr_t getCurrentCmdNumber() const;

			/** Return the current client number. */
			MOHPC_EXPORTS uint32_t getClientNum() const;

			/** Return the current server message sequence (the latest packet number). */
			MOHPC_EXPORTS uint32_t getCurrentServerMessageSequence() const;

			/** Return the current server command sequence (the latest command number). */
			MOHPC_EXPORTS uint32_t getCurrentServerCommandSequence() const;

			/** Return the current reliable sequence (the latest command number on the client). */
			MOHPC_EXPORTS uint32_t getReliableSequence() const;

			/** Return the current reliable acknowledge (the latest command number on the server). */
			MOHPC_EXPORTS uint32_t getReliableAcknowledge() const;

			/**
			 * Return user input data.
			 *
			 * @param	cmdNum		The cmd number to get data from.
			 * @param	outCmd		Output data.
			 * @return	true if the command is valid.
			 */
			MOHPC_EXPORTS bool getUserCmd(uintptr_t cmdNum, usercmd_t& outCmd) const;

			/**
			 * Return parseable server command.
			 * @param	serverCommandNumber		The command num to get.
			 * @param	tokenized				Tokenized data.
			 * @return	true if the command is valid.
			 */
			MOHPC_EXPORTS bool getServerCommand(uintptr_t serverCommandNumber, TokenParser& tokenized);

			/**
			 * Disconnect the client.
			 * Can also be called from the server.
			 */
			MOHPC_EXPORTS void disconnect();

			/**
			 * Return whether or not an user cmd can be created.
			 */
			MOHPC_EXPORTS bool canCreateCommand() const;

			/**
			 * Mark the client as ready (to send commands). Useful for loading maps.
			 */
			MOHPC_EXPORTS void markReady();

			/**
			 * Make the client not ready (won't send new commands anymore).
			 */
			MOHPC_EXPORTS void unmarkReady();

			/**
			 * Send a command to server.
			 *
			 * @param command Command to send.
			 */
			MOHPC_EXPORTS void sendCommand(const char* command);

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
			void parseCGMessage(MSG& msg);
			void parseClientCommand(const char* arguments);

			void clearState();
			bool isDifferentServer(uint32_t id);
			void setCGameTime(uint64_t currentTime);
			void adjustTimeDelta(uint64_t realTime);
			uint64_t getTimeDelta(uint64_t time) const;
			void firstSnapshot(uint64_t currentTime);
			void serverRestarted();
			void updateSnapFlags();
			void calculatePing(uint64_t currentTime);

			void configStringModified(csNum_t num, const char* newString, bool notify = true);
			void notifyConfigStringChange(csNum_t num, const char* newString);
			void notifyAllConfigStringChanges();
			void systemInfoChanged();

			bool readyToSendPacket(uint64_t currentTime) const;
			void createNewCommands();
			void createCmd(usercmd_t& outcmd);
			bool sendCmd(uint64_t currentTime);
			clc_ops_e getClientOperation() const;
			uint32_t getCommandHashKey() const;
			uint8_t getNumCommandsToWrite(uint32_t oldPacketNum) const;

			void writePacket(uint64_t currentTime);
			void writePacketHeader(MSG& msg);
			void writeReliableCommands(MSG& msg);
			void writeUserInput(MSG& msg, uint64_t currentTime);
			void writeAllCommands(MSG& msg, const usercmd_t*& oldcmd, size_t count, uint32_t key);
			void storeOutputPacket(uint64_t currentTime, uint32_t serverTime);

			void fillClientImports(ClientImports& imports);
			StringMessage readStringMessage(MSG& msg);
			void writeStringMessage(MSG& msg, const char* s);
			uint32_t hashKey(const char* string, size_t maxlen) const;
			entityNum_t readEntityNum(MsgTypesHelper& msgHelper);
			void readDeltaPlayerstate(MSG& msg, const playerState_t* from, playerState_t* to);
			void readDeltaEntity(MSG& msg, const entityState_t* from, entityState_t* to, uint16_t newNum);
			csNum_t getNormalizedConfigstring(csNum_t num);
			size_t getMaxCommandSize() const;
			void readNonPVSClient(radarInfo_t radarInfo);
			bool unpackNonPVSClient(radarInfo_t radarInfo, radarUnpacked_t& unpacked);
			const ClientSnapshot* readOldSnapshot(MSG& msg, ClientSnapshot& snap) const;
			void readAreaMask(MSG& msg, ClientSnapshot& snap);
			void setNewSnap(ClientSnapshot& newSnap);

		private:
			static StringMessage readStringMessage_normal(MSG& msg);
			static void writeStringMessage_normal(MSG& msg, const char* s);
			static StringMessage readStringMessage_scrambled(MSG& msg);
			static void writeStringMessage_scrambled(MSG& msg, const char* s);
			uint32_t hashKey_ver6(const char* string, size_t maxlen) const;
			uint32_t hashKey_ver15(const char* string, size_t maxlen) const;
			entityNum_t readEntityNum_ver6(MsgTypesHelper& msgHelper);
			entityNum_t readEntityNum_ver15(MsgTypesHelper& msgHelper);
			void parseGameState_ver6(MSG& msg);
			void parseGameState_ver15(MSG& msg);
			void readDeltaPlayerstate_ver6(MSG& msg, const playerState_t* from, playerState_t* to);
			void readDeltaPlayerstate_ver15(MSG& msg, const playerState_t* from, playerState_t* to);
			void readDeltaEntity_ver6(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum);
			void readDeltaEntity_ver15(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum);
			csNum_t getNormalizedConfigstring_ver6(csNum_t num);
			csNum_t getNormalizedConfigstring_ver15(csNum_t num);
			size_t getMaxCommandSize_ver6() const;
			size_t getMaxCommandSize_ver15() const;
			void readNonPVSClient_ver6(radarInfo_t radarInfo);
			void readNonPVSClient_ver15(radarInfo_t radarInfo);
		};

		using ClientGameConnectionPtr = SharedPtr<ClientGameConnection>;

		namespace ClientError
		{
			class Base : public NetworkException {};

			/**
			 * Invalid command while parsing game state.
			 */
			class BadCommandByteException : public Base
			{
			public:
				BadCommandByteException(uint8_t inCmdNum);

				MOHPC_EXPORTS uint8_t getLength() const;
				MOHPC_EXPORTS str what() const override;

			private:
				uint8_t cmdNum;
			};

			/**
			 * The protocol version does not exist.
			 */
			class BadProtocolVersionException : public Base
			{
			public:
				BadProtocolVersionException(uint8_t inProtocolVersion);

				MOHPC_EXPORTS uint32_t getProtocolVersion() const;
				MOHPC_EXPORTS str what() const override;

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

				MOHPC_EXPORTS uint8_t getLength() const;
				MOHPC_EXPORTS str what() const override;

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

				MOHPC_EXPORTS uint16_t getBaselineNum() const;
				MOHPC_EXPORTS str what() const override;

			private:
				uint16_t baselineNum;
			};

			/**
			 * Bad configstring number.
			 */
			class MaxConfigStringException : public Base
			{
			public:
				MaxConfigStringException(const char* inName, csNum_t inConfigStringNum);

				/** Return the name of the code that tried to access the config string. */
				MOHPC_EXPORTS const char* getName() const;
				/** Return the config string number. */
				MOHPC_EXPORTS csNum_t getConfigstringNum() const;
				MOHPC_EXPORTS str what() const override;

			private:
				const char* name;
				csNum_t configStringNum;
			};

			/**
			 * MAX_GAMESTATE_CHARS was reached while parsing a configstring.
			 */
			class MaxGameStateCharsException : public Base
			{
			public:
				MaxGameStateCharsException(size_t inStringLen);

				/** Return the length of the string that was passed. */
				MOHPC_EXPORTS size_t GetStringLength() const;
				MOHPC_EXPORTS str what() const override;

			private:
				size_t stringLen;
			};

			/**
			 * Bad area mask size while parsing snapshot.
			 */
			class AreaMaskBadSize : public Base
			{
			public:
				AreaMaskBadSize(uint8_t inSize);

				/** Return the size of the area mask. */
				MOHPC_EXPORTS uint8_t getSize() const;
				MOHPC_EXPORTS str what() const override;

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
				MOHPC_EXPORTS const char* getError() const;
				MOHPC_EXPORTS str what() const override;

			private:
				StringMessage error;
			};

			class DownloadSizeException : public Base
			{
			public:
				DownloadSizeException(uint16_t inSize);

				MOHPC_EXPORTS uint16_t getSize() const;

			private:
				uint16_t size;
			};

			class BadDownloadBlockException : public Base
			{
			public:
				BadDownloadBlockException(uint16_t block, uint16_t expectedBlock);

				MOHPC_EXPORTS uint16_t getBlock() const noexcept;
				MOHPC_EXPORTS uint16_t getExpectedBlock() const noexcept;

			private:
				uint16_t block;
				uint16_t expectedBlock;

			};

			class BadSoundNumberException : public Base
			{

			};
		}
	}
}