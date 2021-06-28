#pragma once

#include "../NetGlobal.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	//==
	// Supported protocol versions
	// 6	=> MOHAA 1.0
	// 8	=> MOHAA 1.1
	// 15	=> Spearhead 2.0
	// 16	=> Spearhead 2.11 Demo | Breakthrough 0.30 Demo
	// 17	=> Spearhead 2.11 / 2.15 | Breakthrough / 2.40b
	// Unsupported protocol versions
	// 5	=> MOHAA 0.05 (SP Demo)
	// 21	=> MOHPA 1.0
	// 23	=> MOHPA 1.1
	// 24	=> MOHPA 1.2
	// Breakthrough clients specifies that its a breakthrough client when connecting to a server

	// Changes of protocol 15 since version 8
	// - Each time a client connects to a server, the server sends a GameSpy authorization challenge
	//    So the client have to send back a response with his key that will be checked by GameSpy auth server
	// - Server sends the frametime with the game state
	// - New field type for entity for reading medium coords
	// - Coords are packed into a small integer
	// - Changed coordinates bits
	// - Chars can be scrambled using a table map of bytes
	// - Playerstate has 2 new fields called 'radarInfo' and 'bVoted', and pm_runtime is not sent anymore
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

		/** Spearhead 2.11 demo and Breakthrough demo 0.30. */
		ver211_demo = 16,

		/** Spearhead 2.11 and above. */
		ver211 = 17
	};

	enum class serverType_e : unsigned char
	{
		/** Allied Assault server type (unspecified on AA). */
		normal = 0,
		/** Spearhead server type (this is the default on SH/BT). */
		spearhead = 1,
		/** Breakthrough server. */
		breakthrough = 2
	};

	inline protocolVersion_e getProtocolEnumVersion(const uint32_t value)
	{
		return protocolVersion_e(value);
	}

	inline unsigned int getProtocolVersionNumber(const protocolVersion_e version)
	{
		return (unsigned int)version;
	}

	class MOHPC_NET_EXPORTS protocolType_c
	{
	public:
		serverType_e serverType;
		protocolVersion_e protocolVersion;

	public:
		explicit protocolType_c();
		protocolType_c(serverType_e inServerType, protocolVersion_e inProtocolVersion);

		serverType_e getServerType() const;
		protocolVersion_e getProtocolVersion() const;
		unsigned int getProtocolVersionNumber() const;
	};
}
}