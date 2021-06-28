#include <MOHPC/Network/Serializable/EntityField.h>
#include <MOHPC/Network/Serializable/NetField.h>
#include <MOHPC/Network/Serializable/Coord.h>

#include <MOHPC/Common/Log.h>

#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/Endian.h>

#include <cstring>

using namespace MOHPC;
using namespace MOHPC::Network;

static constexpr char MOHPC_LOG_NAMESPACE[] = "net_field_entity";

static constexpr intptr_t FLOAT_INT_BITS = 13;
static constexpr size_t FLOAT_INT_BIAS = (1 << (FLOAT_INT_BITS - 1));

template<typename T>
static T packType(T val)
{
	T packed = (val << 1) - 0x7A000000;
	if (val < 0) packed |= 1;
	return packed;
}

template<typename T>
static T unpackType(T packed)
{
	if (packed & 1) {
		return ((packed + 0x7A000000) >> 1) | 0x80000000;
	}
	else {
		return (packed + 0x7A000000) >> 1;
	}
}

bool EntityField::DeltaNeeded(const void* fromField, const void* toField, const netField_t* field)
{
	if (!memcmp(fromField, toField, field->getSize()))
	{
		// same values, not needed
		return false;
	}

	switch (fieldType_ver6_e(field->getType()))
	{
	case fieldType_ver6_e::regular:
	{
		if (!field->getBits() || field->getBits() == 32) {
			return true;
		}

		const uint32_t maxValue = (1 << field->getBits()) - 1;

		uint32_t xoredValue = 0;
		for (size_t i = 0; i < field->getSize(); ++i)
		{
			uint8_t fromVal = ((const uint8_t*)fromField)[i];
			uint8_t toVal = ((const uint8_t*)toField)[i];
			xoredValue += fromVal ^ toVal;
		}

		return (xoredValue & maxValue) != 0;
	}
	case fieldType_ver6_e::angle:
		return EntityField::PackAngle(*(float*)fromField, field->getBits()) != EntityField::PackAngle(*(float*)toField, field->getBits());
	case fieldType_ver6_e::animTime:
		return EntityField::PackAnimTime(*(float*)fromField, field->getBits()) != EntityField::PackAnimTime(*(float*)toField, field->getBits());
	case fieldType_ver6_e::animWeight:
		return EntityField::PackAnimWeight(*(float*)fromField, field->getBits()) != EntityField::PackAnimWeight(*(float*)toField, field->getBits());
	case fieldType_ver6_e::scale:
		return EntityField::PackScale(*(float*)fromField, field->getBits()) != EntityField::PackScale(*(float*)toField, field->getBits());
	case fieldType_ver6_e::alpha:
		return EntityField::PackAlpha(*(float*)fromField, field->getBits()) != EntityField::PackAlpha(*(float*)toField, field->getBits());
	case fieldType_ver6_e::coord:
	case fieldType_ver6_e::velocity:
		return true;
	default:
		return true;
	}
}

bool EntityField::DeltaNeeded_ver15(const void* fromField, const void* toField, const netField_t* field)
{
	if (!memcmp(fromField, toField, field->getSize()))
	{
		// same values, not needed
		return false;
	}

	switch (fieldType_ver15_e(field->getType()))
	{
	case fieldType_ver15_e::regular:
	{
		if (!field->getBits() || field->getBits() == 32) {
			return true;
		}

		const uint32_t maxValue = (1 << field->getBits()) - 1;

		uint32_t xoredValue = 0;
		for (size_t i = 0; i < field->getSize(); ++i)
		{
			uint8_t fromVal = ((const uint8_t*)fromField)[i];
			uint8_t toVal = ((const uint8_t*)toField)[i];
			xoredValue += fromVal ^ toVal;
		}

		return (xoredValue & maxValue) != 0;
	}
	case fieldType_ver15_e::angle:
		return EntityField::PackAngle2(*(float*)fromField, field->getBits()) != EntityField::PackAngle2(*(float*)toField, field->getBits());
	case fieldType_ver15_e::animTime:
		return EntityField::PackAnimTime(*(float*)fromField, field->getBits()) != EntityField::PackAnimTime(*(float*)toField, field->getBits());
	case fieldType_ver15_e::animWeight:
		return EntityField::PackAnimWeight(*(float*)fromField, field->getBits()) != EntityField::PackAnimWeight(*(float*)toField, field->getBits());
	case fieldType_ver15_e::scale:
		return EntityField::PackScale(*(float*)fromField, field->getBits()) != EntityField::PackScale(*(float*)toField, field->getBits());
	case fieldType_ver15_e::alpha:
		return EntityField::PackAlpha(*(float*)fromField, field->getBits()) != EntityField::PackAlpha(*(float*)toField, field->getBits());
	case fieldType_ver15_e::coord:
		return EntityField::PackCoord(*(float*)fromField) != EntityField::PackCoord(*(float*)toField);
	case fieldType_ver15_e::coordExtra:
		return EntityField::PackCoordExtra(*(float*)fromField) != EntityField::PackCoordExtra(*(float*)toField);
	case fieldType_ver15_e::velocity:
	case fieldType_ver15_e::simple:
		return true;
	default:
		return true;
	}
}

void EntityField::ReadNumberPlayerStateField(MSG& msg, intptr_t bits, void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		const bool isFullFloat = msg.ReadBool();
		if (!isFullFloat)
		{
			// integral float
			int32_t truncFloat = msg.ReadNumber<int32_t>(FLOAT_INT_BITS);
			// bias to allow equal parts positive and negative
			truncFloat -= FLOAT_INT_BIAS;
			*(float*)toF = (float)truncFloat;
		}
		else
		{
			// full floating point value
			*(float*)toF = msg.ReadFloat();
		}
	}
	else
	{
		// integer
		std::memset(toF, 0, size);
		msg.ReadBits(toF, bits);
		Endian.LittlePointer(toF, size);
	}
}

void EntityField::WriteNumberPlayerStateField(MSG& msg, intptr_t bits, const void* toF, size_t size)
{
	if (bits == 0)
	{
		float floatVal = *(float*)toF;
		int truncFloat = (int)floatVal;

		const bool isFullFloat =
			truncFloat == floatVal && truncFloat + FLOAT_INT_BIAS >= 0 &&
			truncFloat + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS);

		msg.WriteBool(isFullFloat);

		if (!isFullFloat)
		{
			int truncated = truncFloat + FLOAT_INT_BIAS;
			msg.WriteNumber(truncated, FLOAT_INT_BITS);
		}
		else
		{
			// Send a full float value
			msg.WriteFloat(floatVal);
		}
	}
	else
	{
		// Integer
		uint32_t tmp = 0;
		// FIXME: might cause a buffer overflow
		std::memcpy(&tmp, toF, size);
		Endian.LittlePointer(&tmp, size);

		msg.WriteBits(&tmp, bits);
	}
}

void EntityField::ReadRegular(MSG& msg, intptr_t bits, void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		bool hasValue = msg.ReadBool();

		if (!hasValue) {
			*(float*)toF = 0.0f;
		}
		else
		{
			const bool isFullFloat = msg.ReadBool();
			if (!isFullFloat)
			{
				// integral float
				int32_t truncFloat = msg.ReadNumber<int32_t>(FLOAT_INT_BITS);
				// bias to allow equal parts positive and negative
				truncFloat -= FLOAT_INT_BIAS;
				*(float*)toF = (float)truncFloat;
			}
			else
			{
				// full floating point value
				*(float*)toF = msg.ReadFloat();
			}
		}
	}
	else
	{
		memset(toF, 0, size);

		const bool hasValue = msg.ReadBool();
		if (hasValue)
		{
			// integer
			msg.ReadBits(toF, bits);
			Endian.LittlePointer(toF, size);
		}
	}
}

void EntityField::WriteNumberEntityField(MSG& msg, intptr_t bits, const void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		const float floatVal = *(float*)toF;
		const int32_t truncFloat = (int)floatVal;

		const bool hasValue = floatVal != 0.f;
		msg.WriteBool(hasValue);

		if (hasValue)
		{
			const bool isPartial = truncFloat == floatVal && truncFloat + FLOAT_INT_BIAS >= 0 &&
				truncFloat + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS);

			msg.WriteBool(!isPartial);

			if (isPartial)
			{
				// send as small integer
				int newVal = truncFloat + FLOAT_INT_BIAS;
				msg.WriteNumber(newVal, FLOAT_INT_BITS);
			}
			else
			{
				// send as full floating point value
				msg.WriteFloat(*(float*)toF);
			}
		}
	}
	else
	{
		bool hasValue = false;
		for (size_t i = 0; i < size; ++i)
		{
			if (((uint8_t*)toF)[i] != 0)
			{
				hasValue = true;
				break;
			}
		}

		msg.WriteBool(hasValue);

		if (hasValue)
		{
			uint32_t tmp = 0;
			// FIXME: might cause a buffer overflow
			std::memcpy(&tmp, toF, size);
			Endian.LittlePointer(&tmp, size);
			// integer
			msg.WriteBits(&tmp, bits);
		}
	}
}

void EntityField::ReadRegular2(MSG& msg, intptr_t bits, void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		const bool hasValue = msg.ReadBool();

		if (!hasValue) {
			*(float*)toF = 0.0f;
		}
		else
		{
			const bool isFullFloat = msg.ReadBool();
			if (!isFullFloat)
			{
				// integral float
				int32_t truncFloat = msg.ReadNumber<int32_t>(FLOAT_INT_BITS);
				unshiftValue(&truncFloat, sizeof(int32_t));

				*(float*)toF = (float)truncFloat;
			}
			else
			{
				// full floating point value
				uint32_t packedFloat = msg.ReadInteger();

				*(uint32_t*)toF = unpackType(packedFloat);
			}
		}
	}
	else
	{
		memset(toF, 0, size);

		const bool hasValue = msg.ReadBool();
		if (hasValue)
		{
			// integer
			msg.ReadBits(toF, bits);
			Endian.LittlePointer(toF, size);
		}

		if (bits < 0)
		{
			size_t nbits = -bits;
			unshiftValue(toF, size);
		}
	}
}

void EntityField::WriteRegular2(MSG& msg, intptr_t bits, const void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		const float floatVal = *(float*)toF;
		const int32_t truncFloat = (int32_t)floatVal;

		const bool hasValue = floatVal != 0.f;
		msg.WriteBool(hasValue);

		if (hasValue)
		{
			const bool isPartial = truncFloat == floatVal && truncFloat + FLOAT_INT_BIAS >= 0 &&
				truncFloat + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS);

			msg.WriteBool(!isPartial);

			if (isPartial)
			{
				// send as small integer
				msg.WriteNumber(shiftValue(truncFloat), FLOAT_INT_BITS);
			}
			else
			{
				// send as full floating point value
				const uint32_t packedValue = packType(*(int32_t*)toF);
				msg.WriteUInteger(packedValue);
			}
		}
	}
	else
	{
		bool hasValue = false;
		for (size_t i = 0; i < size; ++i)
		{
			if (((uint8_t*)toF)[i] != 0)
			{
				hasValue = true;
				break;
			}
		}

		msg.WriteBool(hasValue);

		if (hasValue)
		{
			uint32_t tmp = 0;
			// FIXME: might cause a buffer overflow
			std::memcpy(&tmp, toF, size);
			Endian.LittlePointer(&tmp, size);

			if (bits < 0)
			{
				size_t nbits = -bits;
				shiftValue(&tmp, size);
			}

			// integer
			msg.WriteBits(&tmp, bits);
		}
	}
}

float EntityField::ReadAngleField(MSG& msg, size_t bits)
{
	if (bits < 0)
	{
		const bool isNeg = msg.ReadBool();
		const uint32_t packedValue = msg.ReadNumber<uint32_t>(~bits);
		return EntityField::UnpackAngle(packedValue, ~bits, isNeg);
	}
	else
	{
		const uint32_t packedValue = msg.ReadNumber<uint32_t>(bits);
		return EntityField::UnpackAngle(packedValue, bits, false);
	}
}

void EntityField::WriteAngleField(MSG& msg, size_t bits, float angle)
{
	const uint32_t packedValue = EntityField::PackAngle(angle, bits);
	if (bits < 0)
	{
		msg.WriteBool(angle < 0);
		msg.WriteNumber<uint32_t>(packedValue, ~bits);
	}
	else
	{
		msg.WriteNumber<uint32_t>(packedValue, bits);
	}
}

float EntityField::ReadTimeField(MSG& msg, size_t bits)
{
	int result = msg.ReadNumber<int>(15);
	return EntityField::UnpackAnimTime(result);
}

void EntityField::WriteTimeField(MSG& msg, float time)
{
	const uint32_t packedValue = EntityField::PackAnimTime(time, 15);
	msg.WriteNumber(packedValue, 15);
}

float EntityField::ReadScaleField(MSG& msg, size_t bits)
{
	int16_t result = msg.ReadNumber<int16_t>(10);
	return EntityField::UnpackScale(result);
}

void EntityField::WriteScaleField(MSG& msg, float time)
{
	float tmp = time;
	intptr_t value = intptr_t(tmp * 100.0f);
	if (value < 0) value = 0;
	else if (value > 1023) value = 1023;

	msg.WriteNumber(value, 10);
}

int32_t EntityField::PackCoord(float val)
{
	return StandardCoord.pack(val);
}

int32_t EntityField::PackCoordExtra(float val)
{
	const int32_t packedValue = ExtraCoord.pack(val);
	if (!(packedValue & ExtraCoord.coordMask)) {
		MOHPC_LOG(Warn, "Illegal XYZ coordinates for an entity, information lost in transmission");
	}

	return packedValue;
}

float EntityField::UnpackCoord(int32_t val)
{
	return StandardCoord.unpack(val);
}

float EntityField::UnpackCoordExtra(int32_t val)
{
	return ExtraCoord.unpack(val);
}

uint32_t EntityField::PackAnimWeight(float weight, size_t bits)
{
	const int32_t max = (1 << bits) - 1;
	int32_t packedValue = (uint32_t)roundf(weight * max);

	if (packedValue < 0) packedValue = 0;
	else if (packedValue > max) packedValue = max;

	return packedValue;
}

float EntityField::UnpackAnimWeight(int result, intptr_t bits)
{
	const int32_t max = (1 << bits) - 1;
	const float tmp = (float)result / (float)max;

	if (tmp < 0.0f) return 0.f;
	else if (tmp > 1.0f) return 1.f;
	else return tmp;
}

uint32_t EntityField::PackScale(float scale, size_t bits)
{
	int32_t value = (uint32_t)roundf(scale * 100.0f);
	const int32_t max = (1 << bits) - 1;

	if (value < 0) value = 0;
	else if (value > max) value = max;

	return value;
}

float EntityField::UnpackScale(int16_t result)
{
	return result / 100.f;
}

uint32_t EntityField::PackAlpha(float alpha, size_t bits)
{
	const int32_t max = (1 << bits) - 1;
	int32_t packedValue = (uint32_t)roundf(alpha * max);

	if (packedValue < 0) packedValue = 0;
	else if (packedValue > max) packedValue = max;

	return packedValue;
}

float EntityField::UnpackAlpha(int result, intptr_t bits)
{
	const int32_t max = (1 << bits) - 1;
	const float tmp = (float)result / (float)max;

	if (tmp < 0.0f) return 0.f;
	else if (tmp > 1.0f) return 1.f;

	else return tmp;
}

uint32_t EntityField::PackAngle(float angle, intptr_t bits)
{
	float tmp = angle;
	if (bits < 0)
	{
		if (tmp < 0) tmp = -tmp;
		bits = ~bits;
	}

	if (bits == 12)
	{
		tmp = tmp * 4096.0f / 360.0f;
		return (uint32_t)tmp & 4095;
	}
	else if (bits == 8)
	{
		tmp = tmp * 256.0f / 360.0f;
		return (uint32_t)tmp & 255;
	}
	else if (bits == 16)
	{
		tmp = tmp * 65536.0f / 360.0f;
		return (uint32_t)tmp & 65535;
	}
	else
	{
		tmp = tmp * (1 << (uint8_t)bits) / 360.0f;
		return ((uint32_t)tmp) & ((1 << (uint8_t)bits) - 1);
	}
}

uint32_t EntityField::PackAngle2(float angle, intptr_t bits)
{
	uint32_t flags = 0;
	float tmp = angle;

	if (bits < 0)
	{
		bits = ~bits;
		if (tmp < 0)
		{
			tmp = -tmp;
			flags = 1 << bits;
		}
	}

	if (bits == 12)
	{
		tmp = floorf(tmp * 4096.0f / 360.0f);
		return flags | ((uint32_t)tmp & 4095);
	}
	else if (bits == 8)
	{
		tmp = floorf(tmp * 256.0f / 360.0f);
		return flags | ((uint32_t)tmp & 255);
	}
	else if (bits == 16)
	{
		tmp = floorf(tmp * 65536.0f / 360.0f);
		return flags | ((uint32_t)tmp & 65535);
	}
	else
	{
		tmp = floorf(tmp * (1 << (uint8_t)bits) / 360.0f);
		return flags | (((uint32_t)tmp) & ((1 << (uint8_t)bits) - 1));
	}
}

float EntityField::UnpackAngle(int result, intptr_t bits, bool isNeg)
{
	float tmp = isNeg ? -1.f : 1.f;
	if (bits < 0) bits = ~bits;
	else bits = bits;

	if (bits == 12) return result * (360.f / 4096.f) * tmp;
	else if (bits == 8) return result * (360.f / 256.f) * tmp;
	else if (bits == 16) return result * (360.f / 65536.f) * tmp;
	else return result * (1 << bits) * tmp / 360.0f;
}

float EntityField::UnpackAngle2(uint32_t packedValue, intptr_t bits)
{
	const int32_t max = (1 << bits);
	uint32_t packedAngle = packedValue;

	float tmp = 1.f;
	if (bits < 0)
	{
		bits = ~bits;
		if (packedValue & max)
		{
			packedAngle = packedValue & ~max;
			tmp = -1.f;
		}
	}

	if (bits == 12) return packedAngle * (360.f / 4096.f) * tmp;
	else if (bits == 8) return packedAngle * (360.f / 256.f) * tmp;
	else if (bits == 16) return packedAngle * (360.f / 65536.f) * tmp;
	else return packedAngle * (1 << bits) * tmp / 360.0f;
}

uint32_t EntityField::PackAnimTime(float time, size_t bits)
{
	int32_t value = (uint32_t)roundf(time * 100.0f);
	const int32_t max = (1 << bits) - 1;

	if (value < 0) value = 0;
	else if (value > max) value = max;

	return value;
}

float EntityField::UnpackAnimTime(int result)
{
	return result / 100.f;
}

void EntityField::ReadSimple(MSG& msg, intptr_t bits, void* toF, size_t size)
{
	if (msg.ReadBool())
	{
		// has a new value
		msg.ReadBits(toF, bits);
	}
	else
	{
		memset(toF, 0, size);
	}
}

void EntityField::WriteSimple(MSG& msg, intptr_t bits, const void* toF, size_t size)
{
	const uint32_t packedValue = *(uint32_t*)toF >> 24;
	if (packedValue & 0xFF)
	{
		// write true for delta change
		msg.WriteBool(true);
		msg.WriteNumber<uint32_t>(packedValue & 0xFF, bits);
	}
	else
	{
		// no delta
		msg.WriteBool(false);
	}
}

SerializableErrors::BadEntityNumberException::BadEntityNumberException(const char* inName, size_t inBadNumber)
	: name(inName)
	, badNumber(inBadNumber)
{}

const char* SerializableErrors::BadEntityNumberException::getName() const
{
	return name;
}

size_t SerializableErrors::BadEntityNumberException::getNumber() const
{
	return badNumber;
}
str SerializableErrors::BadEntityNumberException::what() const
{
	return str(badNumber);
}

SerializableErrors::BadEntityFieldException::BadEntityFieldException(uint8_t inFieldType, const char* inFieldName)
	: fieldType(inFieldType)
	, fieldName(inFieldName)
{}

uint8_t SerializableErrors::BadEntityFieldException::getFieldType() const
{
	return fieldType;
}

const char* SerializableErrors::BadEntityFieldException::getFieldName() const
{
	return fieldName;
}

str SerializableErrors::BadEntityFieldException::what() const
{
	return str::printf("%s: %d", getFieldName(), getFieldType());
}

SerializableErrors::BadEntityFieldCountException::BadEntityFieldCountException(uint8_t inCount)
	: count(inCount)
{}

uint8_t SerializableErrors::BadEntityFieldCountException::getCount() const
{
	return count;
}

str SerializableErrors::BadEntityFieldCountException::what() const
{
	return str((int)getCount());
}
