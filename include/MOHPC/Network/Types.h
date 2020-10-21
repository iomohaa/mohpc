#pragma once

#include <stdint.h>
#include <exception>
#include "../Utilities/SharedPtr.h"
#include "../Utilities/WeakPtr.h"
#include "../Script/str.h"
#include "Socket.h"

namespace MOHPC
{
	namespace Network
	{
		static constexpr unsigned long MAX_RELIABLE_COMMANDS = 64;
		static constexpr unsigned long MAX_STRING_CHARS = 1024;
		static constexpr unsigned long MAX_UDP_DATA_SIZE = 65507;
		static constexpr char CLIENT_VERSION[] = "3.00";

		class MOHPC_EXPORTS NetworkException
		{
		public:
			virtual ~NetworkException() = default;

			virtual str what() const { return str(); };
		};

		class MOHPC_EXPORTS RuntPacketNetException : public NetworkException
		{
		};

		enum class netadrtype_t : uint8_t
		{
			Bot,
			// an address lookup failed
			Bad,
			Loopback,
			Broadcast,
			IP,
			IPX,
			BroadcastIPX
		};

		// FIXME: TODO
		class INetAddr
		{
		public:
			virtual void getAddress(void* data);
			virtual size_t getAddressSize() const = 0;
		};

		// FIXME: use an abstract interface for netadr_t instead (IPv4 and IPv6)
		// however mohaa uses IPv4 only so it's not really important here
		struct netadr_t
		{
			netadrtype_t type;
			uint8_t ipx[10];
			uint8_t ip[4];
			uint16_t port;

		public:
			MOHPC_EXPORTS netadr_t();

			MOHPC_EXPORTS bool operator==(const netadr_t& other) const;
			MOHPC_EXPORTS bool operator!=(const netadr_t& other) const;
		};

		struct bindv4_t
		{
			uint8_t ip[4];
			uint16_t port;

		public:
			MOHPC_EXPORTS bindv4_t();
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

			/** MOHAA 1.0 -> not (yet) supported. */
			ver100 = 6,

			/** MOHAA 1.1. */
			ver111 = 8,

			/** Spearhead 2.0 -> not (yet) supported. */
			ver200 = 15,

			/** Spearhead 2.11 and above. */
			ver211 = 17
		};

		inline protocolVersion_e getProtocolEnumVersion(const uint32_t value)
		{
			switch (value)
			{
			case 6:
				return protocolVersion_e::ver100;
			case 8:
				return protocolVersion_e::ver111;
			case 15:
				return protocolVersion_e::ver200;
			case 17:
				return protocolVersion_e::ver211;
			default:
				return protocolVersion_e::bad;
			}
		}

		class MOHPC_EXPORTS protocolType_c
		{
		public:
			uint8_t serverType;
			protocolVersion_e protocolVersion;

		public:
			explicit protocolType_c();
			protocolType_c(uint8_t inServerType, protocolVersion_e inProtocolVersion);

			uint8_t getServerType() const;
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
	}
}
