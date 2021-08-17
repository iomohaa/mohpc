#include <MOHPC/Network/Types/Angles.h>

using namespace MOHPC;
using namespace MOHPC::Network;

netAngle8_t Network::AngleToByte(float v)
{
	return (uint8_t)(v * 256.f / 360.f) & 255;
}

float Network::ByteToAngle(netAngle8_t v)
{
	return (v) * (360.f / 255.f);
}

netAngle16_t Network::AngleToShort(float v)
{
	return (uint16_t)((v) * 65536.f / 360.f) & 65535;
}

float Network::ShortToAngle(netAngle16_t v)
{
	return (v) * (360.f / 65536.f);
}
