#pragma once

#include <stdint.h>
#include "Types.h"
#include "InfoTypes.h"
#include "Channel.h"
#include "Encoding.h"
#include "Event.h"
#include "../Utilities/HandlerList.h"
#include "../Utilities/Info.h"
#include "../Utilities/PropertyMap.h"
#include "../Utilities/TokenParser.h"
#include "../Object.h"
#include "Configstring.h"
#include "../Misc/MSG/MSG.h"
#include "../Managers/NetworkManager.h"
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

		using cs_t = uint16_t;

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
			 * Called when an entity has been modified or added.
			 *
			 * @param	old		The entity before modification.
			 * @param	state	The new entity.
			 */
			struct EntityRead : public HandlerNotifyBase<void(const entityState_t* old, const entityState_t* state)> {};

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
			struct Configstring : public HandlerNotifyBase<void(size_t csNum, const char* configString)> {};

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
			struct ServerCommand : public HandlerNotifyBase<void(const char* command, const Event& ev)> {};

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
			 * Called when a snap was received.
			 *
			 * @param	snap	The snapshot that was received.
			 */
			struct SnapReceived : public HandlerNotifyBase<void(const ClientSnapshot& snap)> {};

			/**
			 * Called the game state was parsed. Can be called multiple times.
			 * This is the callback to use for map change/loading.
			 *
			 * @param	gameState	New game state.
			 */
			struct GameStateParsed : public HandlerNotifyBase<void(const gameState_t& gameState, bool differentMap)> {};
		}

		enum class svc_ops_e : uint8_t
		{
			/** Bad operation. */
			Bad,
			/** Operation that should be ignored. */
			Nop,
			/** Gamestate parsing. */
			Gamestate,
			/** [short] [string] only in gamestate messages. */
			Configstring,
			/** [short] [entity] Baseline entity parsing (only in gamestate messages). */
			Baseline,
			/** [string] server command to execute on client. */
			ServerCommand,
			/** [short] size [size bytes] */
			Download,
			/** Snapshot parsing. */
			Snapshot,
			/** [string] Centerprint command. */
			Centerprint,
			/** [short] [short] [string] Locationprint command. */
			Locprint,
			/** [byte6] Message for cgame. */
			CGameMessage,
			/** Indicate the end of message. */
			Eof
		};

		enum clc_ops_e : uint8_t
		{
			/** Bad operation. */
			Bad,
			Nop,
			/** [usercmd_t] Movement. */
			Move,
			/** [usercmd_t] Movement without delta. */
			MoveNoDelta,
			/** [string] Command to execute on server. */
			ClientCommand,
			/** Indicate the end of client message. */
			eof
		};

		struct outPacket_t
		{
			/** cl.cmdNumber when packet was sent */
			uint32_t p_cmdNumber;

			/** usercmd->serverTime when packet was sent */
			uint32_t p_serverTime;

			/** time when packet was sent */
			uint64_t p_currentTime;

			/** eyeInfo when packet was sent */
			usereyes_t p_eyeinfo;
		};

		/**
		 * Invalid command while parsing game state.
		 */
		class BadCommandByteException : public NetworkException
		{
		private:
			uint8_t cmdNum;

		public:
			BadCommandByteException(uint8_t inCmdNum)
				: cmdNum(inCmdNum)
			{}

			uint8_t getLength() const { return cmdNum; }
			str what() const override { return str((int)getLength()); }
		};

		/**
		 * The protocol version does not exist.
		 */
		class BadProtocolVersionException : public NetworkException
		{
		private:
			uint32_t protocolVersion;

		public:
			BadProtocolVersionException(uint8_t inProtocolVersion)
				: protocolVersion(inProtocolVersion)
			{}

			uint32_t getProtocolVersion() const { return protocolVersion; }
			str what() const override { return str((int)getProtocolVersion()); }
		};

		/**
		 * Invalid server operation.
		 */
		class IllegibleServerMessageException : public NetworkException
		{
		private:
			uint8_t cmdNum;

		public:
			IllegibleServerMessageException(uint8_t inCmdNum)
				: cmdNum(inCmdNum)
			{}

			uint8_t getLength() const { return cmdNum; }
			str what() const override { return str((int)getLength()); }
		};

		/**
		 * Invalid baseline entity number while parsing gamestate.
		 */
		class BaselineOutOfRangeException : public NetworkException
		{
		private:
			uint16_t baselineNum;

		public:
			BaselineOutOfRangeException(uint16_t inBaselineNum)
				: baselineNum(inBaselineNum)
			{}

			uint16_t getBaselineNum() const { return baselineNum; }
			str what() const override { return str((int)getBaselineNum()); }
		};

		/**
		 * Bad configstring number.
		 */
		class MaxConfigStringException : public NetworkException
		{
		private:
			uint16_t configStringNum;

		public:
			MaxConfigStringException(uint16_t inConfigStringNum)
				: configStringNum(inConfigStringNum)
			{}

			uint16_t GetConfigstringNum() const { return configStringNum; }
			str what() const override { return str((int)GetConfigstringNum()); }
		};

		/**
		 * MAX_GAMESTATE_CHARS was reached while parsing a configstring.
		 */
		class MaxGameStateCharsException : public NetworkException
		{
		private:
			size_t stringLen;

		public:
			MaxGameStateCharsException(size_t inStringLen)
				: stringLen(inStringLen)
			{}

			size_t GetStringLength() const { return stringLen; }
			str what() const override { return str((int)GetStringLength()); }
		};

		/**
		 * Bad area mask size while parsing snapshot.
		 */
		class AreaMaskBadSize : public NetworkException
		{
		private:
			uint8_t size;

		public:
			AreaMaskBadSize(uint8_t inSize)
				: size(inSize)
			{}

			uint8_t getSize() const { return size; }
			str what() const override { return str((int)getSize()); }
		};

		/**
		 * Server error while downloading.
		 */
		class DownloadException : public NetworkException
		{
		private:
			StringMessage error;

		public:
			DownloadException(StringMessage&& inError);

			const char* getError() const { return error; }
			str what() const override { return str(getError()); }
		};

		struct gameState_t
		{
			static constexpr size_t MAX_GAMESTATE_CHARS = 40000;

		public:
			size_t dataCount;
			size_t stringOffsets[MAX_CONFIGSTRINGS];
			char stringData[MAX_GAMESTATE_CHARS];


		public:
			MOHPC_EXPORTS gameState_t();

			/**
			 * Return the configstring at the specified number.
			 *
			 * @param	num		Config string ID. Must be < MAX_CONFIGSTRINGS
			 * @return	The configstring. NULL if num is greater than MAX_CONFIGSTRINGS
			 */
			MOHPC_EXPORTS const char* getConfigString(size_t num) const;

			/**
			 * Return the configstring at the specified number.
			 *
			 * @param	num				Config string ID. Must be < MAX_CONFIGSTRINGS
			 * @param	configString	The value to put in.
			 * @param	sz				Size of the config string
			 */
			MOHPC_EXPORTS void setConfigString(size_t num, const char* configString, size_t sz);
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

		/** Class for handling client specifing settings. */
		class ClientInfo
		{
			MOHPC_OBJECT_DECLARATION(ClientInfo);

		private:
			str name;
			uint32_t rate;
			uint32_t snaps;
			PropertyObject properties;

		private:
			MOHPC_EXPORTS ClientInfo();
			ClientInfo(ClientInfo&& other) = default;
			ClientInfo& operator=(ClientInfo&& other) = default;
			ClientInfo(const ClientInfo& other) = delete;
			ClientInfo& operator=(const ClientInfo& other) = delete;
			~ClientInfo() = default;

		public:
			/**
			 * Set/get the client rate, in kbps. Common rates are :
			 * - 2500 : 28.8k modem
			 * - 3000 : 33.6k modem
			 * - 4000 : 56k modem
			 * - 5000 : ISDN
			 * - 20000 : Cable
			 * - 25000 : xDSL
			 * - 30000 : LAN
			 */
			MOHPC_EXPORTS void setRate(uint32_t inRate);
			MOHPC_EXPORTS uint32_t getRate() const;

			/** Set/get the number of processed snapshots per second. */
			MOHPC_EXPORTS void setSnaps(uint32_t inSnaps);
			MOHPC_EXPORTS uint32_t getSnaps() const;

			/** Set/get the client name. */
			MOHPC_EXPORTS void setName(const char* newName);
			MOHPC_EXPORTS const char* getName() const;

			/** Set/get the client deathmatch allied model. */
			MOHPC_EXPORTS void setPlayerAlliedModel(const char* newModel);
			MOHPC_EXPORTS const char* getPlayerAlliedModel() const;

			/** Set/get the client deathmatch german model. */
			MOHPC_EXPORTS void setPlayerGermanModel(const char* newModel);
			MOHPC_EXPORTS const char* getPlayerGermanModel() const;

			/** Set/get an user value. */
			MOHPC_EXPORTS void setUserKeyValue(const char* key, const char* value);
			MOHPC_EXPORTS const char* getUserKeyValue(const char* key) const;

			/** Build info string from properties. */
			MOHPC_EXPORTS void fillInfoString(Info& info) const;
		};
		using ClientInfoPtr = SharedPtr<ClientInfo>;
		using ConstClientInfoPtr = SharedPtr<const ClientInfo>;
		
		// FIXME: Store server config/properties
		class ClientGameConnection : public ITickableNetwork
		{
			MOHPC_OBJECT_DECLARATION(ClientGameConnection);

		private:
			struct HandlerListClient : public HandlerList
			{
			public:
				MOHPC_HANDLERLIST_DEFINITIONS();

				MOHPC_HANDLERLIST_NOTIFY0();
				MOHPC_HANDLERLIST_NOTIFY1(const char*);

				MOHPC_HANDLERLIST_HANDLER0_NODEF(ClientHandlers::Timeout, timeoutHandler);
				MOHPC_HANDLERLIST_HANDLER1_NODEF(ClientHandlers::Disconnect, disconnectHandler, const char*);
				MOHPC_HANDLERLIST_HANDLER1(ClientHandlers::Error, errorHandler, const NetworkException&);
				MOHPC_HANDLERLIST_HANDLER2(ClientHandlers::EntityRead, entityReadHandler, const entityState_t*, const entityState_t*);
				MOHPC_HANDLERLIST_HANDLER2(ClientHandlers::PlayerstateRead, playerStateReadHandler, const playerState_t*, const playerState_t*);
				MOHPC_HANDLERLIST_HANDLER2(ClientHandlers::Configstring, configStringHandler, uint16_t, const char*);
				MOHPC_HANDLERLIST_HANDLER1(ClientHandlers::Sound, soundHandler, const sound_t&);
				MOHPC_HANDLERLIST_HANDLER1_NODEF(ClientHandlers::CenterPrint, centerPrintHandler, const char*);
				MOHPC_HANDLERLIST_HANDLER3(ClientHandlers::LocationPrint, locationPrintHandler, uint16_t, uint16_t, const char*);
				MOHPC_HANDLERLIST_HANDLER2(ClientHandlers::ServerCommand, serverCommandHandler, const char*, const Event&);
				MOHPC_HANDLERLIST_HANDLER2(ClientHandlers::UserInput, userInputHandler, usercmd_t&, usereyes_t&);

				MOHPC_HANDLERLIST_NOTIFY1(const ClientSnapshot&);
				MOHPC_HANDLERLIST_HANDLER1_NODEF(ClientHandlers::FirstSnapshot, firstSnapshotHandler, const ClientSnapshot&);
				MOHPC_HANDLERLIST_HANDLER1_NODEF(ClientHandlers::SnapReceived, snapshotReceivedHandler, const ClientSnapshot&);
				MOHPC_HANDLERLIST_HANDLER2(ClientHandlers::GameStateParsed, gameStateParsedHandler, const gameState_t&, bool);
			};

		private:
			using parse_f = void (ClientGameConnection::*)(MSG& msg);
			using readString_f = StringMessage(*)(MSG& msg);
			using writeString_f = void (*)(MSG& msg, const char* s);
			using hashKey_f = uint32_t(ClientGameConnection::*)(const char* string, size_t maxlen);
			using readEntityNum_f = entityNum_t (ClientGameConnection::*)(MsgTypesHelper& msgHelper);
			using readDeltaPlayerstate_f = void(ClientGameConnection::*)(MSG& msg, const playerState_t* from, playerState_t* to);
			using readDeltaEntity_f = void(ClientGameConnection::*)(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum);
			using getNormalizedConfigstring_f = cs_t(ClientGameConnection::*)(cs_t num);

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
			CGameModuleBase* cgameModule;
			IEncodingPtr encoder;
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
			uint32_t maxPackets;
			uint32_t maxTickPackets;
			uint32_t parseEntitiesNum;
			uint32_t serverCommandSequence;
			uint32_t serverMessageSequence;
			uint32_t cmdNumber;
			uint32_t clientNum;
			uint32_t checksumFeed;
			uint32_t serverId;
			uint32_t downloadedBlock;
			int32_t reliableSequence;
			int32_t reliableAcknowledge;
			netadr_t adr;
			bool newSnapshots : 1;
			bool extrapolatedSnapshot : 1;
			bool isActive : 1;
			char* reliableCommands[MAX_RELIABLE_COMMANDS];
			char* serverCommands[MAX_RELIABLE_COMMANDS];
			char reliableCmdStrings[MAX_STRING_CHARS * MAX_RELIABLE_COMMANDS];
			char serverCmdStrings[MAX_STRING_CHARS * MAX_RELIABLE_COMMANDS];
			usereyes_t userEyes;
			gameState_t gameState;
			ClientSnapshot currentSnap;
			ClientSnapshot snapshots[PACKET_BACKUP];
			usercmd_t cmds[CMD_BACKUP];
			outPacket_t outPackets[CMD_BACKUP];
			entityState_t entityBaselines[MAX_GENTITIES];
			entityState_t parseEntities[MAX_PARSE_ENTITIES];
			std::bitset<MAX_GENTITIES> validEntities;

		public:
			/**
			 * Construct the game client connection. This should not be called outside of server code.
			 *
			 * @param	netchan				Channel used to send/receive data
			 * @param	inAdr				Address of the server.
			 * @param	challengeResponse	Challenge used to XOR data.
			 * @param	protocolVersion		Version of the protocol to use.
			 */
			ClientGameConnection(const NetworkManagerPtr& inNetworkManager, const INetchanPtr& netchan, const netadr_t& inAdr, uint32_t challengeResponse, const protocolType_c& protoType, const ClientInfoPtr& cInfo);
			~ClientGameConnection();

			// ITickableNetwork
			// ~
			virtual void tick(uint64_t deltaTime, uint64_t currentTime) override;
			// ~

			/** Called to set the current time on start. */
			void initTime(uint64_t currentTime);

			/**
			 * Send a command to server.
			 *
			 * @param	cmd		Command to send.
			 */
			MOHPC_EXPORTS void addReliableCommand(const char* command);

			/**
			 * Set the timeout and the callback to be called when the client/server timeouts.
			 *
			 * @param	timeoutTime		Timeout in milliseconds before the game considers it timed out.
			 */
			MOHPC_EXPORTS void setTimeout(size_t timeoutTime);

			/** Return the handler list. */
			MOHPC_EXPORTS HandlerListClient& getHandlerList();
	
			/** Generic callback function. See Callbacks above for valid callbacks. */
			template<typename T>
			fnHandle_t setCallback(typename T::Type&& handler)
			{
				return handlerList.set<T>(std::forward<T::Type>(handler));
			}

			/**
			 * Unset a previously set callback.
			 *
			 * @param	handle	The returned handle when registering a callback.
			 */
			template<typename T>
			void unsetCallback(fnHandle_t handle)
			{
				handlerList.unset<T>(handle);
			}

			/** Retrieve the current game state. */
			MOHPC_EXPORTS const gameState_t& getGameState() const;

			/** Retrieve the CGame module. */
			MOHPC_EXPORTS CGameModuleBase& getCGModule();

			/** Retrieve the server system info such as the serverid, the timescale, cheats allowed, ... */
			MOHPC_EXPORTS ReadOnlyInfo getServerSystemInfo() const;

			/** Retrieve the server game configuration, such as the hostname, gametype, force respawn, timelimit, ... */
			MOHPC_EXPORTS ReadOnlyInfo getServerGameInfo() const;

			/** Return read-only user info. */
			MOHPC_EXPORTS ConstClientInfoPtr getUserInfo() const;

			/** Return modifiable user info. */
			MOHPC_EXPORTS const ClientInfoPtr& getUserInfo();

			/** Send the server a new user info string. Must be called after having finished modifying the userinfo. */
			MOHPC_EXPORTS void updateUserInfo();

			/** Return the maximum number of packets that can be sent per second. */
			MOHPC_EXPORTS uint32_t getMaxPackets() const;

			/** Set the maximum number of packets that can be sent per second, in the range of [1, 125]. */
			MOHPC_EXPORTS void setMaxPackets(uint32_t inMaxPackets);

			/** Return the maximum number of packets that can be processed at once in once tick. */
			MOHPC_EXPORTS uint32_t getMaxTickPackets() const;

			/** Set the maximum number of packets that can be sent processed at once in one tick, in the range of [1, 1000]. */
			MOHPC_EXPORTS void setMaxTickPackets(uint32_t inMaxPackets);

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

			/** Return th current user input number. */
			MOHPC_EXPORTS uintptr_t getCurrentCmdNumber();

			/**
			 * Return user input data.
			 *
			 * @param	cmdNum		The cmd number to get data from.
			 * @param	outCmd		Output data.
			 * @return	true if the command is valid.
			 */
			MOHPC_EXPORTS bool getUserCmd(uintptr_t cmdNum, usercmd_t& outCmd);

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

		private:
			const INetchanPtr& getNetchan() const;
			void receive(const netadr_t& from, MSG& msg, uint64_t currentTime);
			void wipeChannel();
			bool isChannelValid() const;
			void serverDisconnected(const char* reason);
			void terminateConnection(const char* reason);

			void parseServerMessage(MSG& msg, uint32_t serverMessageSequence, uint64_t currentTime);
			void parseGameState(MSG& msg);
			void parseSnapshot(MSG& msg, uint32_t serverMessageSequence, uint64_t currentTime);
			void parsePacketEntities(MSG& msg, ClientSnapshot* oldFrame, ClientSnapshot* newFrame);
			void parseDeltaEntity(MSG& msg, ClientSnapshot* frame, uint32_t newNum, entityState_t* old, bool unchanged);
			void parseSounds(MSG& msg, ClientSnapshot* newFrame);
			void parseDownload(MSG& msg);
			void parseCommandString(MSG& msg);
			void parseCenterprint(MSG& msg);
			void parseLocprint(MSG& msg);
			void parseCGMessage(MSG& msg);

			void clearState();
			bool isDifferentServer(uint32_t id);
			void setCGameTime(uint64_t currentTime);
			void adjustTimeDelta(uint64_t realTime);
			void firstSnapshot(uint64_t currentTime);

			void configStringModified(uint16_t num, const char* newString);

			void systemInfoChanged();
			bool readyToSendPacket(uint64_t currentTime) const;
			void createNewCommands();
			void createCmd(usercmd_t& outcmd);
			bool sendCmd(uint64_t currentTime);
			void writePacket(uint32_t serverMessageSequence, uint64_t currentTime);

			StringMessage readStringMessage(MSG& msg);
			void writeStringMessage(MSG& msg, const char* s);
			uint32_t hashKey(const char* string, size_t maxlen);
			entityNum_t readEntityNum(MsgTypesHelper& msgHelper);
			void readDeltaPlayerstate(MSG& msg, const playerState_t* from, playerState_t* to);
			void readDeltaEntity(MSG& msg, const entityState_t* from, entityState_t* to, uint16_t newNum);
			cs_t getNormalizedConfigstring(cs_t num);

		private:
			static StringMessage readStringMessage_normal(MSG& msg);
			static void writeStringMessage_normal(MSG& msg, const char* s);
			static StringMessage readStringMessage_scrambled(MSG& msg);
			static void writeStringMessage_scrambled(MSG& msg, const char* s);
			uint32_t hashKey_ver6(const char* string, size_t maxlen);
			uint32_t hashKey_ver15(const char* string, size_t maxlen);
			entityNum_t readEntityNum_ver6(MsgTypesHelper& msgHelper);
			entityNum_t readEntityNum_ver15(MsgTypesHelper& msgHelper);
			void parseGameState_ver6(MSG& msg);
			void parseGameState_ver15(MSG& msg);
			void readDeltaPlayerstate_ver6(MSG& msg, const playerState_t* from, playerState_t* to);
			void readDeltaPlayerstate_ver15(MSG& msg, const playerState_t* from, playerState_t* to);
			void readDeltaEntity_ver6(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum);
			void readDeltaEntity_ver15(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum);
			cs_t getNormalizedConfigstring_ver6(cs_t num);
			cs_t getNormalizedConfigstring_ver15(cs_t num);
		};

		using ClientGameConnectionPtr = SharedPtr<ClientGameConnection>;
	}
}