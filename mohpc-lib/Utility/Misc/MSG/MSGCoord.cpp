#include <MOHPC/Utility/Misc/MSG/MSGCoord.h>
#include <MOHPC/Common/Vector.h>

using namespace MOHPC;

MsgCoordHelper::MsgCoordHelper(MSG& inMsg)
	: MsgBaseHelper(inMsg)
{
}

float MsgCoordHelper::ReadCoord()
{
	uint32_t read = msg.ReadNumber<uint32_t>(19);

	float sign = 1.0f;
	if (read & 262144)
	{
		// the 19th bit is the sign
		sign = -1.0f;
	}

	read &= ~262144; //  uint=4294705151
	return sign * (read / 16.f);
}

float MsgCoordHelper::ReadCoordSmall()
{
	uint32_t read = msg.ReadNumber<uint32_t>(17);

	float sign = 1.0f;
	if (read & 65536)
	{
		// the 17th bit is the sign
		sign = -1.0f;
	}

	read &= ~65536; //  uint=4294705151
	return sign * (read / 8.f);
}

int32_t MsgCoordHelper::ReadDeltaCoord(uint32_t offset)
{
	int32_t result = 0;

	const bool isSmall = msg.ReadBool();
	if (isSmall)
	{
		const uint8_t byteValue = msg.ReadNumber<uint8_t>(8);
		result = (byteValue >> 1) + 1;
		if (byteValue & 1) result = -result;

		result += offset;

	}
	else {
		result = msg.ReadNumber<uint16_t>(16);
	}

	return result;
}

int32_t MsgCoordHelper::ReadDeltaCoordExtra(uint32_t offset)
{
	int32_t result = 0;

	const bool isSmall = msg.ReadBool();
	if (isSmall)
	{
		const uint16_t shortValue = msg.ReadNumber<uint16_t>(10);
		result = (shortValue >> 1) + 1;
		if (shortValue & 1) result = -result;

		result += offset;
	}
	else {
		result = msg.ReadNumber<uint32_t>(18);
	}

	return result;
}

void MsgCoordHelper::ReadVectorCoord(vec3_t& out)
{
	out[0] = ReadCoord();
	out[1] = ReadCoord();
	out[2] = ReadCoord();
}

void MsgCoordHelper::ReadVectorFloat(vec3_t& out)
{
	out[0] = msg.ReadFloat();
	out[1] = msg.ReadFloat();
	out[2] = msg.ReadFloat();
}

void MsgCoordHelper::ReadDir(vec3_t& out)
{
	const uint8_t byteValue = msg.ReadByte();
	ByteToDir(byteValue, out);
}

void MsgCoordHelper::WriteCoord(float value)
{
	int32_t bits = int32_t(value * 16.0f);
	if (value < 0) {
		bits = ((-bits) & 262143) | 262144;
	}
	else {
		bits = bits & 262143;
	}

	msg.WriteNumber(bits, 19);
}

void MsgCoordHelper::WriteCoordSmall(float value)
{
	int32_t bits = (uint32_t)(value * 8.0f);
	if (value < 0) {
		bits = ((-bits) & 65535) | 65536;
	}
	else {
		bits = bits & 65535;
	}

	msg.WriteNumber(bits, 17);
}

void MsgCoordHelper::WriteDeltaCoord(uint32_t from, uint32_t to)
{
	const int32_t delta = to - from;
	const uint32_t deltaAbs = abs(delta);
	if (deltaAbs > 128)
	{
		// not small
		msg.WriteBool(false);
		msg.WriteNumber<uint16_t>(to, 16);
	}
	else
	{
		// small
		msg.WriteBool(true);
		if (delta < 0) {
			msg.WriteNumber<uint8_t>(deltaAbs * 2 - 1, 8);
		}
		else {
			msg.WriteNumber<uint8_t>(deltaAbs * 2 - 2, 8);
		}
	}
}

void MsgCoordHelper::WriteDeltaCoordExtra(uint32_t from, uint32_t to)
{
	const int32_t delta = to - from;
	const uint32_t deltaAbs = abs(delta);
	if (deltaAbs > 128)
	{
		// not small
		msg.WriteBool(false);
		msg.WriteNumber<uint32_t>(to, 18);
	}
	else
	{
		// small
		msg.WriteBool(true);
		if (delta < 0) {
			msg.WriteNumber<uint16_t>(deltaAbs * 2 - 1, 10);
		}
		else {
			msg.WriteNumber<uint16_t>(deltaAbs * 2 - 2, 10);
		}
	}
}

void MsgCoordHelper::WriteVectorCoord(const_vec3r_t value)
{
	WriteCoord(value[0]);
	WriteCoord(value[1]);
	WriteCoord(value[2]);
}

void MsgCoordHelper::WriteDir(const_vec3r_t dir)
{
	vec3_t newDir;
	VectorCopy(dir, newDir);

	uint8_t byteValue = 0;
	ByteToDir(byteValue, newDir);
	msg.WriteByte(byteValue);
}
