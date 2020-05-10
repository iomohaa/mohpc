#include <MOHPC/Global.h>
#include <MOHPC/Misc/MSG/Serializable.h>
#include <MOHPC/Misc/MSG/MSG.h>

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

void MOHPC::SerializableAngle8::Serialize(MSG& msg)
{
	if (!msg.IsReading())
	{
		uint8_t byteValue = AngleToByte(value);
		msg.SerializeByte(byteValue);
	}
	else
	{
		uint8_t byteValue;
		msg.SerializeByte(byteValue);
		value = ByteToAngle(byteValue);
	}
}

MOHPC::SerializableAngle16::SerializableAngle16(float inValue)
	: SerializableAngle(inValue)
{
}

void MOHPC::SerializableAngle16::Serialize(MSG& msg)
{
	if (!msg.IsReading())
	{
		uint16_t shortValue = AngleToShort(value);
		msg.SerializeUShort(shortValue);
	}
	else
	{
		uint16_t shortValue;
		msg.SerializeUShort(shortValue);
		value = ShortToAngle(shortValue);
	}
}

