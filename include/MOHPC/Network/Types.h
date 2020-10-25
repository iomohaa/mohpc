#pragma once

#include <stdint.h>
#include <exception>
#include "../Utilities/SharedPtr.h"
#include "../Utilities/WeakPtr.h"
#include "../Common/str.h"
#include "../Object.h"

namespace MOHPC
{
	namespace Network
	{
		static constexpr unsigned long MAX_RELIABLE_COMMANDS	= 64u;
		static constexpr unsigned long MAX_INFO_STRING			= 1350u;
		static constexpr unsigned long MAX_INFO_KEY				= 1024u;
		static constexpr unsigned long MAX_INFO_VALUE			= 1024u;
		static constexpr unsigned long MAX_STRING_CHARS			= 2048u;
		static constexpr unsigned long MAX_UDP_DATA_SIZE		= 65507u;

		// max size of a network packet
		static constexpr unsigned long MAX_PACKETLEN	= 1400u;

		static constexpr unsigned long FRAGMENT_SIZE	= (MAX_PACKETLEN - 100u);
		// two ints and a short
		static constexpr unsigned long PACKET_HEADER	= 10u;
		static constexpr unsigned long FRAGMENT_BIT		= (1u << 31u);

		static constexpr unsigned long MAX_MSGLEN		= 49152u;

		extern const char CLIENT_VERSION[];

		enum class netsrc_e : uint8_t {
			Client = 1,
			Server
		};

		//==
		// Protocol versions
		// 6	=> MOHAA 1.0
		// 8	=> MOHAA 1.1
		// 15	=> MOHSH 2.0
		// 17	=> MOHSH 2.11 / 2.15 | MOHBT 2.30 / 2.40b
		// Breakthrough adds a "clientType breakthrough" when connecting

		// Changes of protocol 17 since version 8
		// - Each time a client connects to a server, the server sends a GameSpy authorization challenge
		//   So the client have to send back a response with his key that will be checked by GameSpy auth server
		// - Server sends the frametime with the game state
		// - New field type for entity for reading medium coords
		// - Coords are packed into a small integer
		// - Changed coordinates bits
		// - Chars can be scrambled using a table map of bytes
		// - Playerstate has 2 new fields called 'radarInfo' and 'bVoted' + pm_runtime is not sent anymore
		//==

		enum class protocolVersion_e : unsigned char
		{
			/** Bad protocol. */
			bad = 0,

			/** MOHAA 0.05 (Demo SP version). */
			ver005 = 5,

			/** MOHAA 1.0. */
			ver100 = 6,

			/** MOHAA >= 1.1. */
			ver111 = 8,

			/** Spearhead 2.0. */
			ver200 = 15,

			/** Spearhead 2.11 and Breakthrough demo 0.30. */
			ver211_demo = 16,

			/** Spearhead 2.11 and above. */
			ver211 = 17
		};

		enum class serverType_e : unsigned char
		{
			/** Regular server type (the default). */
			none = 1,
			/** Breakthrough server. */
			breakthrough = 2
		};

		inline protocolVersion_e getProtocolEnumVersion(const uint32_t value)
		{
			switch (value)
			{
			case 5:
				return protocolVersion_e::ver005;
			case 6:
				return protocolVersion_e::ver100;
			case 8:
				return protocolVersion_e::ver111;
			case 15:
				return protocolVersion_e::ver200;
			case 16:
				return protocolVersion_e::ver211_demo;
			case 17:
				return protocolVersion_e::ver211;
			default:
				return protocolVersion_e::bad;
			}
		}

		class MOHPC_EXPORTS protocolType_c
		{
		public:
			serverType_e serverType;
			protocolVersion_e protocolVersion;

		public:
			explicit protocolType_c();
			protocolType_c(serverType_e inServerType, protocolVersion_e inProtocolVersion);

			serverType_e getServerType() const;
			protocolVersion_e getProtocolVersion() const;
		};

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
			/** [short] download block. */
			Download,
			/** Snapshot parsing. */
			Snapshot,
			/** [string] Centerprint command. */
			Centerprint,
			/** [short] [short] [string] Locationprint command. */
			Locprint,
			/** [byte6] Message for client game module. */
			CGameMessage,
			/** Indicate the end of message (no more commands). */
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

		/**
		 * Abstract interface for internet address.
		 */
		struct NetAddr
		{
			uint16_t port;

		public:
			MOHPC_EXPORTS NetAddr();

			/**
			 * Set the address port.
			 *
			 * @param value Port.
			 */
			MOHPC_EXPORTS void setPort(uint16_t value);
			/** Return this address port. */
			MOHPC_EXPORTS uint16_t getPort() const;

			/** Return whether or not this address equals to another one. */
			MOHPC_EXPORTS bool operator==(const NetAddr& other) const;
			/** Return whether or not this address differs from another one. */
			MOHPC_EXPORTS bool operator!=(const NetAddr& other) const;

		public:
			/** Return the address size. */
			virtual size_t getAddrSize() const = 0;
			/** Return the address array. */
			virtual const uint8_t* getAddress() const = 0;
			/** Return the address parsed as a string. */
			virtual str asString() const = 0;
		};
		using NetAddrPtr = SharedPtr<NetAddr>;

		/**
		 * IPv4 internet address.
		 */
		struct NetAddr4 : public NetAddr
		{
			MOHPC_OBJECT_DECLARATION(NetAddr4);
		public:
			MOHPC_EXPORTS NetAddr4();

			/** Set the IPv4 address. */
			MOHPC_EXPORTS void setIp(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

		public:
			size_t getAddrSize() const override;
			const uint8_t* getAddress() const override;
			virtual str asString() const override;

		public:
			uint8_t ip[4];
		};
		using NetAddr4Ptr = SharedPtr<NetAddr4>;

		/**
		 * IPv6 internet address.
		 */
		struct NetAddr6 : public NetAddr
		{
			MOHPC_OBJECT_DECLARATION(NetAddr6);

		public:
			MOHPC_EXPORTS NetAddr6();

			/** Set the IPv6 address. */
			MOHPC_EXPORTS void setIp(uint16_t a, uint16_t b, uint16_t c, uint16_t d, uint16_t e, uint16_t f, uint16_t g, uint16_t h);

		public:
			size_t getAddrSize() const override;
			const uint8_t* getAddress() const override;
			virtual str asString() const override;

		public:
			uint8_t ip[16];
		};
		using NetAddr6Ptr = SharedPtr<NetAddr6>;

		class MOHPC_EXPORTS NetworkException
		{
		public:
			virtual ~NetworkException() = default;
			virtual str what() const { return str(); };
		};

		class MOHPC_EXPORTS RuntPacketNetException : public NetworkException {};
	}
}
