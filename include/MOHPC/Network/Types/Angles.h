#pragma once

#include "../NetGlobal.h"
#include <cstdint>

namespace MOHPC
{
namespace Network
{
	using netAngle8_t = uint8_t;
	using netAngles8_t = netAngle8_t[3];
	using netAngle16_t = uint16_t;
	using netAngles16_t = uint16_t[3];

	/** Default angle. */
	using netAngle_t = netAngle16_t;
	using netAngles_t = netAngles16_t;

	MOHPC_NET_EXPORTS netAngle8_t AngleToByte(float v);
	MOHPC_NET_EXPORTS float ByteToAngle(netAngle8_t v);
	MOHPC_NET_EXPORTS netAngle16_t AngleToShort(float v);
	MOHPC_NET_EXPORTS float ShortToAngle(netAngle16_t v);
}
}
