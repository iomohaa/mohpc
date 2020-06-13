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
		static constexpr size_t MAX_RELIABLE_COMMANDS = 64;
		static constexpr size_t MAX_STRING_CHARS = 2048;
		static constexpr char CLIENT_VERSION[] = "3.00";

		class NetworkException
		{
		public:
			virtual ~NetworkException() = default;

			virtual str what() { return str(); };
		};

		class RuntPacketNetException : public NetworkException
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

		struct netadr_t
		{
			netadrtype_t type;
			uint8_t ip[4];
			uint8_t ipx[10];
			uint16_t port;

			netadr_t()
				: type(netadrtype_t::Bad)
				, ip{ 0 }
				, ipx{ 0 }
				, port(0)
			{}

			bool operator==(const netadr_t& other) const
			{
				return *(uint32_t*)ip == *(uint32_t*)other.ip;
			}

			bool operator!=(const netadr_t& other) const
			{
				return *(uint32_t*)ip != *(uint32_t*)other.ip;
			}
		};
		
		//==
		// Protocol versions
		// 6	=> MOHAA 1.0
		// 8	=> MOHAA 1.11
		// 15	=> MOHSH 2.0
		// 17	=> MOHSH 2.11 / 2.15 | MOHBT 2.30 / 2.40b
		// Breakthrough adds a "clientType breakthrough" when connecting

		// Changes of protocol 17 since version 8
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

			/** MOHAA 1.11. */
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
	}
}
