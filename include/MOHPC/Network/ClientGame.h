#pragma once

#include <stdint.h>
#include "Types.h"
#include "InfoTypes.h"
#include "Client.h"
#include <functional>

namespace MOHPC
{
	class MSG;

	namespace Network
	{
		// these are sent over the net as 8 bits
		static constexpr unsigned int MAX_MODELS = 1024;
		// so they cannot be blindly increased
		static constexpr unsigned int MAX_SOUNDS = 512;
		static constexpr unsigned int MAX_OBJECTIVES = 20;
		static constexpr unsigned int MAX_LIGHTSTYLES = 32;
		static constexpr unsigned int MAX_WEAPONS = 48;
		static constexpr unsigned int MAX_CLIENTS = 64;

		static constexpr unsigned int MAX_CONFIGSTRINGS = 2736;

		// from the map worldspawn's message field
		static constexpr unsigned int CS_MESSAGE = 2;
		// current save
		static constexpr unsigned int CS_SAVENAME = 3;
		// g_motd string for server message of the day
		static constexpr unsigned int CS_MOTD = 4;
		// server time when the match will be restarted
		static constexpr unsigned int CS_WARMUP = 5;

		// MUSIC_NewSoundtrack(cs)
		static constexpr unsigned int CS_MUSIC = 8;
		// cg.farplane_cull cg.farplane_distance cg.farplane_color[3]
		static constexpr unsigned int CS_FOGINFO = 9;
		// cg.sky_alpha cg.sky_portal
		static constexpr unsigned int CS_SKYINFO = 10;

		static constexpr unsigned int CS_GAME_VERSION = 11;
		// so the timer only shows the current level cgs.levelStartTime
		static constexpr unsigned int CS_LEVEL_START_TIME = 12;

		static constexpr unsigned int CS_CURRENT_OBJECTIVE = 13;

		// cg.rain
		static constexpr unsigned int CS_RAIN_DENSITY = 14;
		static constexpr unsigned int CS_RAIN_SPEED = 15;
		static constexpr unsigned int CS_RAIN_SPEEDVARY = 16;
		static constexpr unsigned int CS_RAIN_SLANT = 17;
		static constexpr unsigned int CS_RAIN_LENGTH = 18;
		static constexpr unsigned int CS_RAIN_MINDIST = 19;
		static constexpr unsigned int CS_RAIN_WIDTH = 20;
		static constexpr unsigned int CS_RAIN_SHADER = 21;
		static constexpr unsigned int CS_RAIN_NUMSHADERS = 22;

		// cgs.matchEndTime
		static constexpr unsigned int CS_MATCHEND = 26;

		static constexpr unsigned int CS_MODELS = 32;
		static constexpr unsigned int CS_OBJECTIVES = (CS_MODELS + MAX_MODELS);
		static constexpr unsigned int CS_SOUNDS = (CS_OBJECTIVES + MAX_OBJECTIVES);

		static constexpr unsigned int CS_IMAGES = (CS_SOUNDS + MAX_SOUNDS);
		static constexpr unsigned int MAX_IMAGES = 64;

		static constexpr unsigned int CS_LIGHTSTYLES(CS_IMAGES + MAX_IMAGES);
		static constexpr unsigned int CS_PLAYERS = (CS_LIGHTSTYLES + MAX_LIGHTSTYLES);

		static constexpr unsigned int CS_WEAPONS = (CS_PLAYERS + MAX_CLIENTS);
		static constexpr unsigned int CS_TEAMS = 1876;
		static constexpr unsigned int CS_GENERAL_STRINGS = 1877;
		static constexpr unsigned int CS_SPECTATORS = 1878;
		static constexpr unsigned int CS_ALLIES = 1879;
		static constexpr unsigned int CS_AXIS = 1880;
		static constexpr unsigned int CS_SOUNDTRACK = 1881;

		static constexpr unsigned int CS_TEAMINFO = 1;

		//static constexpr unsigned int CS_MAX = (CS_PARTICLES + MAX_LOCATIONS);

		namespace Callbacks
		{
			using ClientGameTimeout = std::function<void()>;
			using ErrorHandler = std::function<void(const NetworkException& exception)>;
		}

		enum class svc_ops_e : uint8_t
		{
			bad,
			nop,
			gamestate,
			// [short] [string] only in gamestate messages
			configstring,
			// only in gamestate messages
			baseline,
			// [string] to be executed by client game module
			serverCommand,
			// [short] size [size bytes]
			download,
			snapshot,
			centerprint,
			locprint,
			cgameMessage,
			eof
		};

		enum clc_ops_e : uint8_t {
			Bad,
			Nop,
			// [usercmd_t]
			Move,
			// [usercmd_t]
			MoveNoDelta,
			// [string] message
			ClientCommand,
			eof
		};

		struct outPacket_t {
			// cl.cmdNumber when packet was sent
			uint32_t p_cmdNumber;
			// usercmd->serverTime when packet was sent
			uint32_t p_serverTime;
			// cls.realtime when packet was sent
			uint32_t p_realtime;
			// eyeInfo when packet was sent
			usereyes_t p_eyeinfo;
		};

		class BadCommandByteException : public NetworkException
		{
		private:
			uint8_t cmdNum;

		public:
			BadCommandByteException(uint8_t inCmdNum)
				: cmdNum(inCmdNum)
			{}

			uint8_t getLength() const { return cmdNum; }
			virtual str what() { return str((int)getLength()); }
		};

		class BadProtocolVersionException : public NetworkException
		{
		private:
			uint32_t protocolVersion;

		public:
			BadProtocolVersionException(uint8_t inProtocolVersion)
				: protocolVersion(inProtocolVersion)
			{}

			uint32_t getProtocolVersion() const { return protocolVersion; }
			virtual str what() { return str((int)getProtocolVersion()); }
		};

		class IllegibleServerMessageException : public NetworkException
		{
		private:
			uint8_t cmdNum;

		public:
			IllegibleServerMessageException(uint8_t inCmdNum)
				: cmdNum(inCmdNum)
			{}

			uint8_t getLength() const { return cmdNum; }
			virtual str what() { return str((int)getLength()); }
		};

		class BaselineOutOfRangeException : public NetworkException
		{
		private:
			uint16_t baselineNum;

		public:
			BaselineOutOfRangeException(uint16_t inBaselineNum)
				: baselineNum(inBaselineNum)
			{}

			uint16_t getBaselineNum() const { return baselineNum; }
			virtual str what() { return str((int)getBaselineNum()); }
		};

		class MaxConfigStringException : public NetworkException
		{
		private:
			uint16_t configStringNum;

		public:
			MaxConfigStringException(uint16_t inConfigStringNum)
				: configStringNum(inConfigStringNum)
			{}

			uint16_t GetConfigstringNum() const { return configStringNum; }
			virtual str what() { return str((int)GetConfigstringNum()); }
		};

		class MaxGameStateCharsException : public NetworkException
		{
		private:
			size_t stringLen;

		public:
			MaxGameStateCharsException(size_t inStringLen)
				: stringLen(inStringLen)
			{}

			size_t GetStringLength() const { return stringLen; }
			virtual str what() { return str((int)GetStringLength()); }
		};

		class AreaMaskBadSize : public NetworkException
		{
		private:
			uint8_t size;

		public:
			AreaMaskBadSize(uint8_t inSize)
				: size(inSize)
			{}

			uint8_t getSize() const { return size; }
			virtual str what() { return str((int)getSize()); }
		};

		class DownloadException : public NetworkException
		{
		private:
			StringMessage error;

		public:
			DownloadException(StringMessage&& inError);

			const char* getError() const { return error; }
			virtual str what() { return str(getError()); }
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
		};

		class ClientGameConnection : public ITickableNetwork
		{
		private:
			using parse_f = void (ClientGameConnection::*)(MSG& msg);
			using readString_f = StringMessage(ClientGameConnection::*)(MSG& msg);
			using writeString_f = void (ClientGameConnection::*)(MSG& msg, const char* s);
			using hashKey_f = uint32_t(ClientGameConnection::*)(const char* string, size_t maxlen);
			using readEntityNum_f = uint32_t(ClientGameConnection::*)(MsgTypesHelper& msgHelper);
			using readDeltaPlayerstate_f = void(ClientGameConnection::*)(MSG& msg, playerState_t* from, playerState_t* to);
			using readDeltaEntity_f = void(ClientGameConnection::*)(MSG& msg, entityState_t* from, entityState_t* to);

		private:
			INetchanPtr netchan;
			netadr_t adr;
			Callbacks::ClientGameTimeout timeoutCallback;
			Callbacks::ErrorHandler errorCallback;
			size_t timeoutTime;
			std::chrono::time_point<std::chrono::steady_clock> nextTimeoutTime;
			IEncodingPtr encoder;
			uint32_t parseEntitiesNum;
			uint32_t serverStartTime;
			uint32_t serverCommandSequence;
			uint32_t serverMessageSequence;
			uint32_t cmdNumber;
			uint32_t clientNum;
			uint32_t checksumFeed;
			uint32_t serverId;
			uint32_t downloadedBlock;
			int32_t reliableSequence;
			int32_t reliableAcknowledge;
			char* reliableCommands[MAX_RELIABLE_COMMANDS];
			char* serverCommands[MAX_RELIABLE_COMMANDS];
			char reliableCmdStrings[MAX_STRING_CHARS * MAX_RELIABLE_COMMANDS];
			char serverCmdStrings[MAX_STRING_CHARS * MAX_RELIABLE_COMMANDS];
			ClientSnapshot currentSnap;
			ClientSnapshot snapshots[PACKET_BACKUP];
			gameState_t gameState;
			usercmd_t cmds[CMD_BACKUP];
			outPacket_t outPackets[CMD_BACKUP];
			entityState_t entityBaselines[MAX_GENTITIES];
			entityState_t parseEntities[MAX_PARSE_ENTITIES];
			parse_f parseGameState_pf;
			readString_f readStringMessage_pf;
			writeString_f writeStringMessage_pf;
			hashKey_f hashKey_pf;
			readEntityNum_f readEntityNum_pf;
			readDeltaPlayerstate_f readDeltaPlayerstate_pf;
			readDeltaEntity_f readDeltaEntity_pf;

		public:
			/**
			 * Construct the game client connection. This should not be called outside of server code.
			 *
			 * @param	netchan				Channel used to send/receive data
			 * @param	inAdr				Address of the server.
			 * @param	challengeResponse	Challenge used to XOR data.
			 * @param	protocolVersion		Version of the protocol to use.
			 */
			ClientGameConnection(const INetchanPtr& netchan, const netadr_t& inAdr, uint32_t challengeResponse, protocolVersion_e protocolVersion);
			~ClientGameConnection();

			// ITickableNetwork
			// ~
			virtual void tick(uint64_t deltaTime, uint64_t currentTime) override;
			// ~

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
			 * @param	callback		Target callback to set.
			 */
			MOHPC_EXPORTS void setTimeout(size_t timeoutTime, Callbacks::ClientGameTimeout&& callback = Callbacks::ClientGameTimeout());

			/**
			 * Set the callback to be called when a serious error happens (such as illegible server command).
			 *
			 * @param	handler		Handler called on error.
			 */
			MOHPC_EXPORTS void setErrorHandler(Callbacks::ErrorHandler&& handler);

			// FIXME: entity callback
			// FIXME: server command callback
			// FIXME: Usercmd builder
			// FIXME: sound callback
			// FIXME: gamestate parsed callback

		private:
			const INetchanPtr& getNetchan() const;
			void receive(const netadr_t& from, MSG& msg);

			void onTimedOut();
			void onError(const NetworkException& exception);

			void parseServerMessage(MSG& msg, uint32_t serverMessageSequence);
			void parseGameState(MSG& msg);
			void parseCommandString(MSG& msg);
			void parseSnapshot(MSG& msg, uint32_t serverMessageSequence);
			void parsePacketEntities(MSG& msg, ClientSnapshot* oldFrame, ClientSnapshot* newFrame);
			void parseDeltaEntity(MSG& msg, ClientSnapshot* frame, uint32_t newNum, entityState_t* old, bool unchanged);
			void parseSounds(MSG& msg);
			void parseCenterprint(MSG& msg);
			void parseDownload(MSG& msg);
			void parseLocprint(MSG& msg);
			void parseCGMessage(MSG& msg);

			void systemInfoChanged();
			void createNewCommands();
			usercmd_t createCmd();
			void sendCmd();
			void writePacket(uint32_t serverMessageSequence);

			StringMessage readStringMessage(MSG& msg);
			void writeStringMessage(MSG& msg, const char* s);
			uint32_t hashKey(const char* string, size_t maxlen);
			uint32_t readEntityNum(MsgTypesHelper& msgHelper);
			void readDeltaPlayerstate(MSG& msg, playerState_t* from, playerState_t* to);
			void readDeltaEntity(MSG& msg, entityState_t* from, entityState_t* to);

		private:
			StringMessage readStringMessage_normal(MSG& msg);
			void writeStringMessage_normal(MSG& msg, const char* s);
			StringMessage readStringMessage_scrambled(MSG& msg);
			void writeStringMessage_scrambled(MSG& msg, const char* s);
			uint32_t hashKey_ver8(const char* string, size_t maxlen);
			uint32_t hashKey_ver17(const char* string, size_t maxlen);
			uint32_t readEntityNum_ver8(MsgTypesHelper& msgHelper);
			uint32_t readEntityNum_ver17(MsgTypesHelper& msgHelper);
			void parseGameState_ver8(MSG& msg);
			void parseGameState_ver17(MSG& msg);
			void readDeltaPlayerstate_ver8(MSG& msg, playerState_t* from, playerState_t* to);
			void readDeltaPlayerstate_ver17(MSG& msg, playerState_t* from, playerState_t* to);
			void readDeltaEntity_ver8(MSG& msg, entityState_t* from, entityState_t* to);
			void readDeltaEntity_ver17(MSG& msg, entityState_t* from, entityState_t* to);
		};

		using ClientGameConnectionPtr = SharedPtr<ClientGameConnection>;
	}
}