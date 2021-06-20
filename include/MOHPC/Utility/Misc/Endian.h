#pragma once

#include "../UtilityGlobal.h"
#include <cstdint>
#include <cstddef>

namespace MOHPC
{
	void CopyShortSwap(void* dest, const void* src);
	void CopyLongSwap(void* dest, const void* src);
	void PointerSwap(void* p, size_t size);
	short ShortSwap(short l);
	int IntegerSwap(int l);
	long LongSwap(long l);
	long long Long64Swap(long long ll);
	float FloatSwap(const float f);

	class IEndian
	{
	public:
		virtual short BigShort(short l) const = 0;
		virtual short LittleShort(short l) const = 0;
		virtual int BigInteger(int l) const = 0;
		virtual int LittleInteger(int l) const = 0;
		virtual long BigLong(long l) const = 0;
		virtual long LittleLong(long l) const = 0;
		virtual long long BigLong64(long long l) const = 0;
		virtual long long LittleLong64(long long l) const = 0;
		virtual float BigFloat(float l) const = 0;
		virtual float LittleFloat(float l) const = 0;
		virtual void BigPointer(void* p, size_t size) const = 0;
		virtual void LittlePointer(void* p, size_t size) const = 0;

	public:
		MOHPC_UTILITY_EXPORTS void CopyLittle(void* dest, const void* src, size_t size) const;
		MOHPC_UTILITY_EXPORTS void CopyBig(void* dest, const void* src, size_t size) const;
	};

	extern MOHPC_UTILITY_EXPORTS const IEndian& Endian;
};
