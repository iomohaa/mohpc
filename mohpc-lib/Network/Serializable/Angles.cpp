#include <MOHPC/Network/Serializable/Angles.h>
#include <MOHPC/Network/Types/Angles.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>

using namespace MOHPC;
using namespace MOHPC::Network;

SerializableAngle::SerializableAngle(float inValue)
	: value(inValue)
{
}

SerializableAngle::operator float()
{
	return value;
}

SerializableAngle::operator float() const
{
	return value;
}

SerializableAngle::operator float* ()
{
	return &value;
}

SerializableAngle::operator const float* () const
{
	return &value;
}

SerializableAngle8::SerializableAngle8(float inValue)
	: SerializableAngle(inValue)
{
}

void SerializableAngle8::Load(MSG& msg)
{
	const uint8_t byteValue = msg.ReadByte();
	value = ByteToAngle(byteValue);
}

void SerializableAngle8::Save(MSG& msg) const
{
	const uint8_t byteValue = AngleToByte(value);
	msg.WriteByte(byteValue);
}

SerializableAngle16::SerializableAngle16(float inValue)
	: SerializableAngle(inValue)
{
}

void SerializableAngle16::Load(MSG& msg)
{
	const uint16_t shortValue = msg.ReadUShort();
	value = ShortToAngle(shortValue);
}

void SerializableAngle16::Save(MSG& msg) const
{
	const uint16_t shortValue = AngleToShort(value);
	msg.WriteUShort(shortValue);
}
