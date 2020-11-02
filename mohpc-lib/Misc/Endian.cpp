#include <Shared.h>
#include <MOHPC/Misc/Endian.h>

#include <cstdint>
#include <cstddef>
#include <cstring>

using namespace MOHPC;

struct uint16data_t
{
	union
	{
		uint8_t b[2];
		uint16_t value;
	};

	uint16data_t() = default;
	uint16data_t(uint16_t l) { value = l; }
};

struct uint32data_t
{
	union
	{
		uint8_t b[4];
		uint32_t value;
	};

	uint32data_t() = default;
	uint32data_t(uint32_t l) { value = l; }
};

struct uint64data_t
{
	union
	{
		uint8_t b[8];
		uint64_t value;
	};

	uint64data_t() = default;
	uint64data_t(uint64_t ll) { value = ll; }
};

struct long_data_t
{
	union
	{
		uint8_t b[sizeof(long)];
		long value;
	};

	long_data_t() = default;
	long_data_t(long l) { value = l; }
};

union float_t {
	float f;
	int i;
	unsigned int ui;
};

union endian_t {
	uint8_t value[4];
	int intValue;

	constexpr endian_t(int inValue) : intValue(inValue) {}
};

constexpr bool isLittleEndian()
{
	constexpr endian_t endianVal(0x12345678);
	// on little endian, the highest number will be the first
	return endianVal.value[0] == 0x78;
}

void IEndian::CopyLittle(void* dest, const void* src, size_t size) const
{
	memcpy(dest, src, size);
	LittlePointer(dest, size);
}

void IEndian::CopyBig(void* dest, const void* src, size_t size) const
{
	memcpy(dest, src, size);
	BigPointer(dest, size);
}

class BigEndian : public IEndian
{
public:
	short BigShort(short l) const override { return l; }
	short LittleShort(short l) const override { return ShortSwap(l); };
	int BigInteger(int l) const override { return l; }
	int LittleInteger(int l) const override { return IntegerSwap(l); }
	long BigLong(long l) const override { return l; }
	long LittleLong(long l) const override { return LongSwap(l); }
	long long BigLong64(long long l) const override { return l; }
	long long LittleLong64(long long l) const override { return Long64Swap(l); }
	float BigFloat(float l) const override { return l; }
	float LittleFloat(float l) const override { return FloatSwap(l); }
	void BigPointer(void* p, size_t size) const override {}
	void LittlePointer(void* p, size_t size) const override { return PointerSwap(p, size); }

};

class LittleEndian : public IEndian
{
public:
	short BigShort(short l) const override { return ShortSwap(l); };
	short LittleShort(short l) const override { return l; };
	int BigInteger(int l) const override { return IntegerSwap(l); }
	int LittleInteger(int l) const override { return l; }
	long BigLong(long l) const override { return LongSwap(l); }
	long LittleLong(long l) const override { return l; }
	long long BigLong64(long long l) const override { return Long64Swap(l); }
	long long LittleLong64(long long l) const override { return l; }
	float BigFloat(float l) const override { return FloatSwap(l); }
	float LittleFloat(float l) const override { return l; }
	void BigPointer(void* p, size_t size) const override { return PointerSwap(p, size); }
	void LittlePointer(void* p, size_t size) const override {}
};

BigEndian bigEndian;
LittleEndian littleEndian;

constexpr const IEndian& getEndianness()
{
	if (isLittleEndian())
	{
		return littleEndian;
	}
	else
	{
		return bigEndian;
	}
}

const IEndian& MOHPC::Endian = getEndianness();

void MOHPC::CopyShortSwap(void* dest, const void* src)
{
	uint8_t* to = (uint8_t*)dest;
	const uint8_t* from = (const uint8_t*)src;

	to[0] = from[1];
	to[1] = from[0];
}

void MOHPC::CopyLongSwap(void* dest, const void* src)
{
	uint8_t* to = (uint8_t*)dest;
	const uint8_t* from = (const uint8_t*)src;

	to[0] = from[3];
	to[1] = from[2];
	to[2] = from[1];
	to[3] = from[0];
}

void MOHPC::PointerSwap(void* p, size_t size)
{
	if(size > 1)
	{
		uint8_t* data = static_cast<uint8_t*>(p);

		// swap values
		for (size_t i = size / 2; i > 0; --i)
		{
			uint8_t* in = data + i - 1;
			uint8_t* out = data + size - i;

			const uint8_t tmp = *in;
			*in = *out;
			*out = tmp;
		}
	}
}

short MOHPC::ShortSwap(short l)
{
	const uint16data_t in = l;
	uint16data_t result;

	result.b[0] = in.b[1];
	result.b[1] = in.b[0];

	return result.value;
}

int MOHPC::IntegerSwap(int l)
{
	const uint32data_t in = l;
	uint32data_t result;

	result.b[0] = in.b[3];
	result.b[1] = in.b[2];
	result.b[2] = in.b[1];
	result.b[3] = in.b[0];

	return result.value;
}

long MOHPC::LongSwap(long l)
{
	const long_data_t in = l;
	long_data_t result;

	constexpr size_t num = sizeof(l);
	// long can be either 32 or 64-bits
	// as a consequence, a loop must be used
	for(size_t i = sizeof(l); i > 0; i--)
	{
		result.b[i - 1] = in.b[num - i];
	}

	return result.value;
}

long long MOHPC::Long64Swap(long long ll)
{
	const uint64data_t in = ll;
	uint64data_t result;

	result.b[0] = in.b[7];
	result.b[1] = in.b[6];
	result.b[2] = in.b[5];
	result.b[3] = in.b[4];
	result.b[4] = in.b[3];
	result.b[5] = in.b[2];
	result.b[6] = in.b[1];
	result.b[7] = in.b[0];

	return result.value;
}

float MOHPC::FloatSwap(const float f)
{
	float_t out;

	out.f = f;
	out.ui = IntegerSwap(out.ui);

	return out.f;
}
