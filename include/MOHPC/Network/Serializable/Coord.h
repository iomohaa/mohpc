#pragma once

#include <type_traits>
#include <cstdint>

namespace MOHPC
{
namespace Network
{
	static constexpr uint32_t MAX_COORDS = 1 << 19;

	template<size_t maxBits, intptr_t minCoord, intptr_t maxCoord>
	class NetCoord
	{
		static_assert(maxBits < 64, "Bits must be under 64");
	public:
		using packedint_t = std::conditional_t<maxBits <= 8, uint8_t,
			std::conditional_t<maxBits <= 16, uint16_t,
			std::conditional_t<maxBits <= 32, uint32_t,
			std::conditional_t<maxBits <= 64, uint64_t, uint64_t>>>>;

	public:
		packedint_t pack(float val)
		{
			return (packedint_t)roundf(maxBitCoordSigned + val * precision);
		}

		float unpack(packedint_t packedValue)
		{
			return (float)(int64_t)(packedValue - maxBitCoordSigned) / precision;
		}

	public:
		static constexpr uint64_t maxBitCoord = (1ull << maxBits);
		static constexpr uint64_t maxBitCoordSigned = maxBitCoord / 2;
		static constexpr uint64_t coordMask = maxBitCoord - 1;
		static constexpr uint64_t maxDelta = (maxCoord - minCoord);
		static constexpr uint64_t precision = maxBitCoord / maxDelta;
	};

	extern NetCoord<16, -8192, 8192> StandardCoord;
	extern NetCoord<18, -8192, 8192> ExtraCoord;
}
}