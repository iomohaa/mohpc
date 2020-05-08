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

uint8_t MOHPC::SerializableAngle8::AngleToByte(float v)
{
	return (uint8_t)(v * 256.f / 360.f) & 255;
}

float MOHPC::SerializableAngle8::ByteToAngle(uint8_t v)
{
	return (v) * (360.f / 255.f);
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

uint16_t MOHPC::SerializableAngle16::AngleToShort(float v)
{
	return (uint16_t)((v) * 65536.f / 360.f) & 65535;
}

float MOHPC::SerializableAngle16::ShortToAngle(uint16_t v)
{
	return (v) * (360.f / 65536.f);
}
