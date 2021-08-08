#pragma once

#include "../Types/UserInput.h"
#include "../../Utility/WarpArray.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
namespace Network
{
	static constexpr unsigned long PACKET_BACKUP = (1 << 5);
	static constexpr unsigned long PACKET_MASK = PACKET_BACKUP - 1;

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

	template<typename T>
	using PacketBackup = StaticWarpArray<T, PACKET_BACKUP>;

	using OutgoingPackets = outPacket_t[PACKET_BACKUP];

	class OutClientPackets
	{
	public:
		/** eyeInfo when packet was sent */
		PacketBackup<usereyes_t> eyeInfo;

		/** cl.cmdNumber when packet was sent */
		PacketBackup<uint32_t> cmdNumber;
	};
}
}
