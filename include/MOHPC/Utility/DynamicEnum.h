#pragma once

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
	template<typename CountType>
	class EnumBasicCounter
	{
	public:
		constexpr CountType operator()(CountType current) { return current; }
	};

	template<typename CountType>
	class EnumShiftCounter
	{
	public:
		constexpr CountType operator()(CountType current) { return (1 << current); }
	};

	template<typename T, typename CountType = uint32_t, CountType start = 0, typename Counter = EnumBasicCounter<CountType>>
	class DynamicEnum
	{
	public:
		DynamicEnum()
			: index(Counter()(count++))
		{
		}

		CountType getIndex() const
		{
			return index;
		}

		static CountType getCount()
		{
			return count;
		}

	private:
		CountType index;
		static CountType count;
	};

	template<typename T, typename CountType, CountType start, typename Counter>
	CountType DynamicEnum<T, CountType, start, Counter>::count = start;
}
