#include <Shared.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/MSG/Codec.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>
#include <MOHPC/Utility/Misc/MSG/Serializable.h>
#include <MOHPC/Utility/Misc/MSG/HuffmanTree.h>
#include <MOHPC/Utility/Misc/Endian.h>
#include <cstring>
#include <cassert>
#include <algorithm>

using namespace MOHPC;

const uint8_t charByteMapping[256] =
{
	254, 120, 89, 13, 27, 73, 103, 78, 74, 102, 21, 117, 76, 86, 238, 96, 88, 62, 59, 60,
	40, 84, 52, 119, 251, 51, 75, 121, 192, 85, 44, 54, 114, 87, 25, 53, 35, 224, 67, 31,
	82, 41, 45, 99, 233, 112, 255, 11, 46, 115, 8, 32, 19, 100, 110, 95, 116, 48, 58, 107,
	70, 91, 104, 81, 118, 109, 36, 24, 17, 39, 43, 65, 49, 83, 56, 57, 33, 64, 80, 28,
	184, 160, 18, 105, 42, 20, 194, 38, 29, 26, 61, 50, 9, 90, 37, 128, 79, 2, 108, 34,
	4, 0, 47, 12, 101, 10, 92, 15, 5, 7, 22, 55, 23, 14, 3, 1, 66, 16, 63, 30,
	6, 97, 111, 248, 72, 197, 191, 122, 176, 245, 250, 68, 195, 77, 232, 106, 228, 93, 240, 98,
	208, 69, 164, 144, 186, 222, 94, 246, 148, 170, 244, 190, 205, 234, 252, 202, 230, 239, 174, 225,
	226, 209, 236, 216, 237, 151, 149, 231, 129, 188, 200, 172, 204, 154, 168, 71, 133, 217, 196, 223,
	134, 253, 173, 177, 219, 235, 214, 182, 132, 227, 183, 175, 137, 152, 158, 221, 243, 150, 210, 136,
	167, 211, 179, 193, 218, 124, 140, 178, 213, 249, 185, 113, 127, 220, 180, 145, 138, 198, 123, 162,
	189, 203, 166, 126, 159, 156, 212, 207, 146, 181, 247, 139, 142, 169, 242, 241, 171, 187, 153, 135,
	201, 155, 161, 125, 163, 130, 229, 206, 165, 157, 141, 147, 143, 199, 215, 131
};

const uint8_t byteCharMapping[256] =
{
	101, 115, 97, 114, 100, 108, 120, 109, 50, 92, 105, 47, 103, 3, 113, 107, 117, 68, 82, 52,
	85, 10, 110, 112, 67, 34, 89, 4, 79, 88, 119, 39, 51, 76, 99, 36, 66, 94, 87, 69,
	20, 41, 84, 70, 30, 42, 48, 102, 57, 72, 91, 25, 22, 35, 31, 111, 74, 75, 58, 18,
	19, 90, 17, 118, 77, 71, 116, 38, 131, 141, 60, 175, 124, 5, 8, 26, 12, 133, 7, 96,
	78, 63, 40, 73, 21, 29, 13, 33, 16, 2, 93, 61, 106, 137, 146, 55, 15, 121, 139, 43,
	53, 104, 9, 6, 62, 83, 135, 59, 98, 65, 54, 122, 45, 211, 32, 49, 56, 11, 64, 23,
	1, 27, 127, 218, 205, 243, 223, 212, 95, 168, 245, 255, 188, 176, 180, 239, 199, 192, 216, 231,
	206, 250, 232, 252, 143, 215, 228, 251, 148, 166, 197, 165, 193, 238, 173, 241, 225, 249, 194, 224,
	81, 242, 219, 244, 142, 248, 222, 200, 174, 233, 149, 236, 171, 182, 158, 191, 128, 183, 207, 202,
	214, 229, 187, 190, 80, 210, 144, 237, 169, 220, 151, 126, 28, 203, 86, 132, 178, 125, 217, 253,
	170, 240, 155, 221, 172, 152, 247, 227, 140, 161, 198, 201, 226, 208, 186, 254, 163, 177, 204, 184,
	213, 195, 145, 179, 37, 159, 160, 189, 136, 246, 156, 167, 134, 44, 153, 185, 162, 164, 14, 157,
	138, 235, 234, 196, 150, 129, 147, 230, 123, 209, 130, 24, 154, 181, 0, 46
};

namespace MOHPC
{
	// Unscrambled char mapping
	class charCharMapping_c
	{
	private:
		char mapping[256];

	public:
		constexpr charCharMapping_c()
			: mapping{ 0 }
		{
			for (uint16_t i = 0; i < 256; ++i) {
				mapping[i] = (uint8_t)i;
			}
		}

		constexpr operator const char* () const { return mapping; }
	};
	charCharMapping_c charCharMapping;
}

void MOHPC::unshiftValue(void* val, size_t size)
{
	Endian.BigPointer(val, size);

	uint8_t* valB = (uint8_t*)val;
	const bool shouldComp = valB[size - 1] & 1;
	uint8_t carry = 0;

	for (size_t i = 0; i < size; ++i)
	{
		const uint8_t saved = valB[i] & 1;

		valB[i] = valB[i] >> 1;
		if (carry) {
			valB[i] |= 1 << 7;
		}

		carry = saved;
	}

	if (shouldComp)
	{
		for (size_t i = 0; i < size; ++i) {
			valB[i] = ~valB[i];
		}
	}

	Endian.BigPointer(val, size);
}

void MOHPC::shiftValue(void* val, size_t size)
{
	Endian.LittlePointer(val, size);

	uint8_t* valB = (uint8_t*)val;
	uint8_t carry = 0;

	for (size_t i = 0; i < size; ++i)
	{
		const uint8_t saved = valB[i] & (1 << 7);

		valB[i] = valB[i] << 1;
		if (carry) {
			valB[i] |= 1;
		}

		carry = saved;
	}

	if (valB[0] & 1)
	{
		for (size_t i = 0; i < size; ++i) {
			valB[i] = ~valB[i];
		}

		valB[size - 1] |= 1;
	}

	Endian.LittlePointer(val, size);
}

MSG::MSG(IMessageStream& stream, msgMode_e inMode) noexcept
	: msgStream(stream)
	, msgCodec(&MessageCodecs::Bit)
	, mode(inMode)
	, bit(0)
	, bitData{ 0 }
{
	if(mode == msgMode_e::Reading)
	{
		// Make sure it's readable
		Reset();
	}
}

MSG::~MSG()
{
	Flush();
}

void MSG::Flush()
{
	if (bit && IsWriting())
	{
		const size_t sz = ((bit + 7) & ~7) >> 3;
		stream().Write(bitData, sz);
		bit = 0;
	}
}

void MSG::SetCodec(IMessageCodec& inCodec) noexcept
{
	msgCodec = &inCodec;
}

void MSG::SetMode(msgMode_e inMode)
{
	mode = inMode;
}

void MSG::Reset()
{
	bit = 0;
	memset(bitData, 0, sizeof(bitData));
	stream().Read(bitData, std::min(stream().GetLength() - stream().GetPosition(), sizeof(bitData)));
}

void MSG::SerializeBits(void* value, intptr_t bits)
{
	if (!IsReading()) {
		WriteBits(value, bits);
	}
	else {
		ReadBits(value, bits);
	}
}

MSG& MSG::Serialize(void* data, size_t length)
{
	for (size_t i = 0; i < length; i++) {
		SerializeByte(((uint8_t*)data)[i]);
	}
	return *this;
}

MSG& MSG::SerializeDelta(const void* a, void* b, size_t bits)
{
	bool isSame = !memcmp(a, b, std::max(bits >> 3, size_t(1)));
	SerializeBits(&isSame, 1);

	if (!isSame) {
		SerializeBits(b, bits);
	}
	return *this;
}

MSG& MSG::SerializeDelta(const void* a, void* b, intptr_t key, size_t bits)
{
	if (!IsReading())
	{
		bool isSame = !memcmp(a, b, std::max(bits >> 3, size_t(1)));
		SerializeBits(&isSame, 1);

		if (!isSame)
		{
			const size_t sz = std::max(bits >> 3, size_t(1));
			uint8_t* buffer = new uint8_t[sz];
			std::memcpy(buffer, b, sz);

			for (size_t i = 0; i < sz; ++i) {
				buffer[i] ^= key;
			}

			SerializeBits(b, bits);
			delete[] buffer;
		}
	}
	else
	{
		bool isSame;
		SerializeBits(&isSame, 1);

		if (!isSame)
		{
			SerializeBits(b, bits);

			uint8_t* p2 = (uint8_t*)b;
			const size_t sz = std::max(bits >> 3, size_t(1));
			for (size_t i = 0; i < sz; ++i) {
				p2[i] ^= key;
			}
		}
	}
	return *this;
}

MSG& MSG::SerializeDeltaClass(const ISerializableMessage* a, ISerializableMessage* b)
{
	if (!IsReading()) b->SaveDelta(*this, a);
	else b->LoadDelta(*this, a);
	return *this;
}

MSG& MSG::SerializeDeltaClass(const ISerializableMessage* a, ISerializableMessage* b, intptr_t key)
{
	if (!IsReading()) b->SaveDelta(*this, a,key);
	else b->LoadDelta(*this, a, key);
	return *this;
}

template<>
MSG& MSG::SerializeDeltaType<bool>(const bool& a, bool& b, intptr_t key)
{
	if (!IsReading())
	{
		bool isSame = !((a & 1) ^ (b & 1));
		SerializeBits(&isSame, 1);

		if (!isSame)
		{
			bool val = b ^ bool(key);
			SerializeBits(&val, 1);
		}
	}
	else
	{
		bool isSame;
		SerializeBits(&isSame, 1);

		if (!isSame)
		{
			SerializeBits(&b, 1);
			b ^= bool(key);
		}
	}
	return *this;
}

MSG& MSG::SerializeBool(bool& value)
{
	if (!IsReading()) {
		WriteBool(value);
	}
	else {
		value = ReadBool();
	}
	return *this;
}

MSG& MSG::SerializeByteBool(bool& value)
{
	uint8_t bval;
	SerializeByte(bval);
	value = (bool)bval;
	return *this;
}

MSG& MSG::SerializeChar(char& value)
{
	if (!IsReading()) {
		WriteChar(value);
	}
	else {
		value = ReadChar();
	}
	return *this;
}

MSG& MSG::SerializeByte(unsigned char& value)
{
	if (!IsReading()) {
		WriteByte(value);
	}
	else {
		value = ReadByte();
	}
	return *this;
}

MSG& MSG::SerializeShort(short& value)
{
	if (!IsReading()) {
		WriteShort(value);
	}
	else {
		value = ReadShort();
	}
	return *this;
}

MSG& MSG::SerializeUShort(unsigned short& value)
{
	if (!IsReading()) {
		WriteUShort(value);
	}
	else {
		value = ReadUShort();
	}
	return *this;
}

MSG& MSG::SerializeInteger(int& value)
{
	if (!IsReading()) {
		WriteInteger(value);
	}
	else {
		value = ReadInteger();
	}
	return *this;
}

MSG& MSG::SerializeUInteger(unsigned int& value)
{
	if (!IsReading()) {
		WriteUInteger(value);
	}
	else {
		value = ReadUInteger();
	}
	return *this;
}

MSG& MSG::SerializeFloat(float& value)
{
	if (!IsReading()) {
		WriteFloat(value);
	}
	else {
		value = ReadFloat();
	}
	return *this;
}

MSG& MSG::SerializeClass(ISerializableMessage* value)
{
	if (!IsReading()) value->Save(*this);
	else value->Load(*this);
	return *this;
}

MSG& MSG::SerializeClass(ISerializableMessage& value)
{
	SerializeClass(&value);
	return *this;
}

void MSG::SerializeString(StringMessage& s)
{
	if (!IsReading()) {
		WriteString(s);
	}
	else {
		s = std::move(ReadString());
	}
}

void MSG::SerializeStringWithMapping(StringMessage& s)
{
	if (!IsReading())
	{
		const size_t len = strlen(s) + 1;
		for (size_t i = 0; i < len; ++i)
		{
			uint8_t byteValue = charByteMapping[s[i]];
			SerializeByte(byteValue);
		}
	}
	else
	{
		s.preAlloc(1024);

		// FIXME: temporary solution
		uint8_t val = 0;
		char c = 0;
		size_t i = 0;
		do
		{
			SerializeByte(val);
			c = byteCharMapping[val];
			s.writeChar(c, i++);
		} while (c > 0);
	}
}

size_t MSG::Size() const
{
	return stream().GetLength();
}

size_t MSG::GetPosition() const
{
	const size_t pos = stream().GetPosition();
	if(pos >= sizeof(bitData)) {
		return stream().GetPosition() - sizeof(bitData) + (bit >> 3);
	}
	else
	{
		// if the stream is in a position below the buffer size, return read bytes read instead
		return (bit >> 3);
	}
}

size_t MSG::GetBitPosition() const
{
	return bit;
}

bool MSG::IsReading() noexcept
{
	return mode == msgMode_e::Reading || mode == msgMode_e::Both;
}

bool MSG::IsWriting() noexcept
{
	return mode == msgMode_e::Writing || mode == msgMode_e::Both;
}

void MSG::ReadData(void* data, size_t length)
{
	for (size_t i = 0; i < length; i++) {
		((uint8_t*)data)[i] = ReadByte();
	}
}

void MSG::ReadBits(void* value, intptr_t bits)
{
	// Read the first n bytes
	if (!stream().GetPosition()) {
		stream().Read(bitData, std::min(stream().GetLength(), sizeof(bitData)));
	}

	if (bits < 0) bits = -bits;

	codec().Decode(value, bits, bit, stream(), bitData, sizeof(bitData));
}

bool MSG::ReadBool()
{
	assert(IsReading());
	bool val = false;
	ReadBits(&val, 1);
	return val;
}

bool MSG::ReadByteBool()
{
	assert(IsReading());
	uint8_t val = 0;
	ReadBits(&val, 8);
	return (bool)val;
}

char MSG::ReadChar()
{
	assert(IsReading());
	char val = 0;
	ReadBits(&val, 8);
	return val;
}

unsigned char MSG::ReadByte()
{
	assert(IsReading());
	unsigned char val = 0;
	ReadBits(&val, 8);
	return val;
}

short MSG::ReadShort()
{
	assert(IsReading());
	short val = 0;
	ReadBits(&val, 16);
	return Endian.LittleShort(val);
}

unsigned short MSG::ReadUShort()
{
	assert(IsReading());
	unsigned short val = 0;
	ReadBits(&val, 16);
	return Endian.LittleShort(val);
}

int MSG::ReadInteger()
{
	assert(IsReading());
	int val = 0;
	ReadBits(&val, 32);
	return Endian.LittleInteger(val);
}

unsigned int MSG::ReadUInteger()
{
	assert(IsReading());
	unsigned int val = 0;
	ReadBits(&val, 32);
	return Endian.LittleInteger(val);
}

float MSG::ReadFloat()
{
	assert(IsReading());
	float val = 0;
	ReadBits(&val, 32);
	return Endian.LittleFloat(val);
}

StringMessage MSG::ReadString()
{
	return ReadStringInternal(charCharMapping);
}

StringMessage MSG::ReadScrambledString(const char* byteCharMapping)
{
	return ReadStringInternal(byteCharMapping);
}

template<>
uint8_t MSG::ReadNumber<uint8_t>(size_t bits)
{
	uint8_t value = 0;
	ReadBits(&value, bits);
	return value;
}

template<>
uint16_t MSG::ReadNumber<uint16_t>(size_t bits)
{
	uint16_t value = 0;
	ReadBits(&value, bits);
	return Endian.LittleShort(value);
}

template<>
uint32_t MSG::ReadNumber<uint32_t>(size_t bits)
{
	uint32_t value = 0;
	ReadBits(&value, bits);
	return Endian.LittleInteger(value);
}

template<>
uint64_t MSG::ReadNumber<uint64_t>(size_t bits)
{
	uint64_t value = 0;
	ReadBits(&value, bits);
	return Endian.LittleLong64(value);
}

template<>
float MSG::ReadNumber<float>(size_t bits)
{
	float value = 0;
	ReadBits(&value, bits);
	return Endian.LittleFloat(value);
}

template<> int8_t MSG::ReadNumber<int8_t>(size_t bits) { return ReadNumber<uint8_t>(bits); }
template<> int16_t MSG::ReadNumber<int16_t>(size_t bits) { return ReadNumber<uint16_t>(bits); }
template<> int32_t MSG::ReadNumber<int32_t>(size_t bits) { return ReadNumber<uint32_t>(bits); }
template<> int64_t MSG::ReadNumber<int64_t>(size_t bits) { return ReadNumber<uint64_t>(bits); }

MSG& MSG::ReadClass(ISerializableMessage& value)
{
	assert(IsReading());
	value.Load(*this);
	return *this;
}

MSG& MSG::ReadDeltaClass(const ISerializableMessage* a, ISerializableMessage* b)
{
	assert(IsReading());
	b->LoadDelta(*this, a);
	return *this;
}

MSG& MSG::ReadDeltaClass(const ISerializableMessage* a, ISerializableMessage* b, intptr_t key)
{
	assert(IsReading());
	b->LoadDelta(*this, a, key);
	return *this;
}

MSG& MSG::WriteData(const void* data, uintptr_t size)
{
	for (size_t i = 0; i < size; i++) {
		WriteByte(((const uint8_t*)data)[i]);
	}
	return *this;
}

MSG& MSG::WriteBits(const void* value, intptr_t bits)
{
	assert(IsWriting());

	if (bits < 0) bits = -bits;
	codec().Encode(value, bits, bit, stream(), bitData, sizeof(bitData));

	return *this;
}

MSG& MSG::WriteBool(bool value)
{
	assert(IsWriting());
	WriteBits(&value, 1);
	return *this;
}

MSG& MSG::WriteByteBool(bool value)
{
	assert(IsWriting());
	WriteBits(&value, 8);
	return *this;
}

MSG& MSG::WriteChar(char value)
{
	assert(IsWriting());
	WriteBits(&value, 8);
	return *this;
}

MSG& MSG::WriteByte(unsigned char value)
{
	assert(IsWriting());
	WriteBits(&value, 8);
	return *this;
}

MSG& MSG::WriteShort(short value)
{
	assert(IsWriting());
	short newValue = Endian.LittleShort(value);
	WriteBits(&newValue, 16);
	return *this;
}

MSG& MSG::WriteUShort(unsigned short value)
{
	assert(IsWriting());
	unsigned short newValue = Endian.LittleShort(value);
	WriteBits(&newValue, 16);
	return *this;
}

MSG& MSG::WriteInteger(int value)
{
	assert(IsWriting());
	int newValue = Endian.LittleInteger(value);
	WriteBits(&newValue, 32);
	return *this;
}

MSG& MSG::WriteUInteger(unsigned int value)
{
	assert(IsWriting());
	unsigned int newValue = Endian.LittleInteger(value);
	WriteBits(&newValue, 32);
	return *this;
}

MSG& MSG::WriteFloat(float value)
{
	assert(IsWriting());
	float newValue = Endian.LittleFloat(value);
	WriteBits(&newValue, 32);
	return *this;
}

MSG& MSG::WriteString(const StringMessage& s)
{
	assert(IsWriting());

	static char emptyString = 0;

	if (!s) {
		WriteData(&emptyString, 1);
	}
	else
	{
		WriteData((const void*)s.getData(), strlen(s) + 1);
	}

	return *this;
}

MSG& MSG::WriteScrambledString(const StringMessage& s, const uint8_t* charByteMapping)
{
	const char emptyString = charByteMapping[0];

	if (s)
	{
		const char* p = s.getData();
		while(*p)
		{
			const uint8_t c = *p++;
			const uint8_t val = charByteMapping[c];
			WriteData(&val, 1);
		}
	}

	WriteData(&emptyString, 1);

	return *this;
}

template<>
MSG& MOHPC::MSG::WriteNumber<uint8_t>(uint8_t value, size_t bits)
{
	return WriteBits(&value, bits);
}

template<>
MSG& MOHPC::MSG::WriteNumber<uint16_t>(uint16_t value, size_t bits)
{
	const uint16_t bytes = Endian.LittleShort(value);
	return WriteBits(&bytes, bits);
}

template<>
MSG& MOHPC::MSG::WriteNumber<uint32_t>(uint32_t value, size_t bits)
{
	const uint32_t bytes = Endian.LittleInteger(value);
	return WriteBits(&bytes, bits);
}

template<>
MSG& MOHPC::MSG::WriteNumber<uint64_t>(uint64_t value, size_t bits)
{
	const uint64_t bytes = Endian.LittleLong64(value);
	return WriteBits(&bytes, bits);
}

template<> MSG& MSG::WriteNumber<float>(float value, size_t bits)
{
	const float bytes = Endian.LittleFloat(value);
	return WriteBits(&bytes, bits);
}

template<> MSG& MSG::WriteNumber<int8_t>(int8_t value, size_t bits) { return WriteNumber<uint8_t>(value, bits); }
template<> MSG& MSG::WriteNumber<int16_t>(int16_t value, size_t bits) { return WriteNumber<uint16_t>(value, bits); }
template<> MSG& MSG::WriteNumber<int32_t>(int32_t value, size_t bits) { return WriteNumber<uint32_t>(value, bits); }
template<> MSG& MSG::WriteNumber<int64_t>(int64_t value, size_t bits) { return WriteNumber<uint64_t>(value, bits); }

MSG& MSG::WriteClass(const ISerializableMessage& value)
{
	assert(IsWriting());
	value.Save(*this);
	return *this;
}

MSG& MSG::WriteDeltaClass(const ISerializableMessage* a, const ISerializableMessage* b)
{
	assert(IsWriting());
	b->SaveDelta(*this, a);
	return *this;
}

MSG& MSG::WriteDeltaClass(const ISerializableMessage* a, const ISerializableMessage* b, intptr_t key)
{
	assert(IsWriting());
	b->SaveDelta(*this, a, key);
	return *this;
}

template<>
float MSG::XORType(const float& b, intptr_t key)
{
	int xored = *(int*)&b ^ int(key);
	return *(float*)&xored;
}

StringMessage MSG::ReadStringInternal(const char* byteCharMapping)
{
	size_t startBit = bit;
	size_t startPos = stream().GetPosition();
	uint8_t oldBits[sizeof(bitData)];

	// Copy bits for later restoring
	memcpy(oldBits, bitData, sizeof(bitData));

	// Calculate the length of strings in the message
	size_t len = 0;
	uint8_t val = 0;
	char c = 0;
	do
	{
		val = ReadByte();
		c = byteCharMapping[val];
		++len;
	} while (c);

	StringMessage s;

	if (len > 0)
	{
		bit = startBit;
		stream().Seek(startPos);
		memcpy(bitData, oldBits, sizeof(bitData));

		s.preAlloc(len);
		len--; // Skip null-terminated character

		for (size_t i = 0; i < len; ++i)
		{
			val = ReadByte();

			c = byteCharMapping[val];
			assert(c);
			s.writeChar(c, i);
		}

		// null-terminating character
		ReadByte();
	}

	return s;
}

template<>
double MSG::XORType(const double& b, intptr_t key)
{
	if constexpr (sizeof(double) == 4)
	{
		int xored = *(int*)&b ^ int(key);
		return *(double*)&xored;
	}
	else if constexpr (sizeof(double) == 8)
	{
		long long xored = *(long long*)&b ^ key;
		return *(double*)&xored;
	}
	else {
		// don't know what to do in this case
		return b;
	}
}

StringMessage::StringMessage() noexcept
	: strData(NULL)
	, isAlloced(false)
{
}

StringMessage::StringMessage(StringMessage&& str) noexcept
{
	strData = str.strData;
	isAlloced = str.isAlloced;
	str.isAlloced = false;
}

StringMessage& StringMessage::operator=(StringMessage&& str) noexcept
{
	if (isAlloced) delete[] strData;
	strData = str.strData;
	isAlloced = str.isAlloced;
	str.isAlloced = false;

	return *this;
}

StringMessage::StringMessage(const str& str) noexcept
{
	strData = new char[str.length() + 1];
	memcpy(strData, str.c_str(), str.length() + 1);
	isAlloced = true;
}

StringMessage::StringMessage(const char* str) noexcept
	: strData((char*)str)
	, isAlloced(false)
{
}

StringMessage::~StringMessage() noexcept
{
	if (isAlloced) {
		delete[] strData;
	}
}

char* StringMessage::getData() noexcept
{
	return strData;
}

char* StringMessage::getData() const noexcept
{
	return strData;
}

const char* StringMessage::c_str() const noexcept
{
	return strData;
}

void StringMessage::preAlloc(size_t len) noexcept
{
	if (isAlloced) delete[] strData;
	strData = new char[len];
	strData[len - 1] = 0;
	isAlloced = true;
}

void StringMessage::writeChar(char c, size_t i) noexcept
{
	strData[i] = c;
}

StringMessage::operator const char* () const noexcept
{
	return strData;
}

void CompressedMessage::Compress(size_t offset, size_t len)
{
	const size_t size = std::min(input().GetLength(), len);

	if (size <= 0) {
		return;
	}

	Huff huff;

	uint8_t bitData[8]{ 0 };
	// store the original size
	bitData[0] = (uint8_t)(size >> 8);
	bitData[1] = (uint8_t)size & 0xff;

	// start position
	size_t bloc = 16;

	// Seek at the specified offset
	input().Seek(offset);
	MessageCodecs::FlushBits(bloc, output(), bitData, sizeof(bitData));

	uint8_t buffer[8]{ 0 };

	for (uintptr_t i = 0; i < size; i += sizeof(buffer))
	{
		const size_t bufSize = std::min(sizeof(buffer), size - i);

		input().Read(buffer, bufSize);
		compressBuf(huff, bloc, buffer, bufSize, bitData, sizeof(bitData));
	}

	if (bloc)
	{
		// write remaining bits
		output().Write(bitData, (bloc >> 3) + 1);
	}
}

void CompressedMessage::Decompress(size_t offset, size_t len)
{
	const size_t size = input().GetLength() - offset;

	if (size <= 0) {
		return;
	}

	Huff huff;

	// Seek at the specified offset
	input().Seek(offset);

	uint8_t bitData[8]{ 0 };
	input().Read(bitData, sizeof(bitData));

	size_t cch = bitData[0] * 256 + bitData[1];
	// don't overflow with bad messages
	if (cch > len) {
		cch = len;
	}
	size_t bloc = 16;

	output().Seek(0);

	uint8_t buffer[8];
	size_t pos = 0;

	for (uintptr_t j = 0; j < cch; ++j)
	{
		MessageCodecs::ReadBits(bloc, input(), bitData, sizeof(bitData));

		// don't overflow reading from the messages
		// FIXME: would it be better to have a overflow check in get_bit ?
		if (((size_t)bloc >> 3) > len)
		{
			// write the NUL character
			output().Write("", 1);
			break;
		}

		// Get a character
		uintptr_t ch = huff.receive(bitData, bloc);
		if (ch == Huff::NYT)
		{
			// We got a NYT, get the symbol associated with it
			ch = 0;
			for (uintptr_t i = 0; i < 8; i++) {
				ch = (ch << 1) + Huff::getBit(bitData, bloc);
			}
		}

		// insert the character
		buffer[pos] = (uint8_t)ch;

		// Increment node
		huff.addRef((uint8_t)ch);

		pos++;
		if (pos == sizeof(buffer))
		{
			output().Write(buffer, sizeof(buffer));
			pos = 0;
		}
	}

	if (pos > 0)
	{
		// write remaining bytes
		output().Write(buffer, pos);
	}
}

void CompressedMessage::compressBuf(Huff& huff, size_t& bloc, uint8_t* buffer, size_t bufSize, uint8_t* bitData, size_t bitDataSize)
{
	for (size_t i = 0; i < bufSize; ++i)
	{
		const uint8_t ch = buffer[i];
		// Transmit symbol
		huff.transmit(ch, bitData, bloc);
		// Do update
		huff.addRef(ch);

		MessageCodecs::FlushBits(bloc, output(), bitData, bitDataSize);
	}
}

float MsgTypesHelper::ReadCoord()
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

float MsgTypesHelper::ReadCoordSmall()
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

int32_t MsgTypesHelper::ReadDeltaCoord(uint32_t offset)
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

int32_t MsgTypesHelper::ReadDeltaCoordExtra(uint32_t offset)
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

Vector MsgTypesHelper::ReadVectorCoord()
{
	const Vector vec =
	{
		ReadCoord(),
		ReadCoord(),
		ReadCoord()
	};
	return vec;
}

Vector MsgTypesHelper::ReadVectorFloat()
{
	const Vector vec =
	{
		msg.ReadFloat(),
		msg.ReadFloat(),
		msg.ReadFloat()
	};
	return vec;
}

Vector MsgTypesHelper::ReadDir()
{
	Vector dir;

	const uint8_t byteValue = msg.ReadByte();
	ByteToDir(byteValue, dir);

	return dir;
}

void MsgTypesHelper::WriteCoord(float value)
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

void MsgTypesHelper::WriteCoordSmall(float value)
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

void MsgTypesHelper::WriteDeltaCoord(uint32_t from, uint32_t to)
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

void MsgTypesHelper::WriteDeltaCoordExtra(uint32_t from, uint32_t to)
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

void MsgTypesHelper::WriteVectorCoord(Vector& value)
{
	WriteCoord(value.x);
	WriteCoord(value.y);
	WriteCoord(value.z);
}

void MsgTypesHelper::WriteDir(Vector& dir)
{
	uint8_t byteValue = 0;
	ByteToDir(byteValue, dir);
	msg.WriteByte(byteValue);
}
