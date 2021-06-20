#include <MOHPC/Utility/Misc/MSG/Serializable.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>

using namespace MOHPC;

MOHPC::SerializableAngle::SerializableAngle(float inValue)
	: value(inValue)
{
}

MOHPC::SerializableAngle::operator float()
{
	return value;
}

MOHPC::SerializableAngle::operator float() const
{
	return value;
}

MOHPC::SerializableAngle::operator float* ()
{
	return &value;
}

MOHPC::SerializableAngle::operator const float* () const
{
	return &value;
}

MOHPC::SerializableAngle8::SerializableAngle8(float inValue)
	: SerializableAngle(inValue)
{
}

void MOHPC::SerializableAngle8::Load(MSG& msg)
{
	const uint8_t byteValue = msg.ReadByte();
	value = ByteToAngle(byteValue);
}

void MOHPC::SerializableAngle8::Save(MSG& msg) const
{
	const uint8_t byteValue = AngleToByte(value);
	msg.WriteByte(byteValue);
}

MOHPC::SerializableAngle16::SerializableAngle16(float inValue)
	: SerializableAngle(inValue)
{
}

void MOHPC::SerializableAngle16::Load(MSG& msg)
{
	const uint16_t shortValue = msg.ReadUShort();
	value = ShortToAngle(shortValue);
}

void MOHPC::SerializableAngle16::Save(MSG& msg) const
{
	const uint16_t shortValue = AngleToShort(value);
	msg.WriteUShort(shortValue);
}
