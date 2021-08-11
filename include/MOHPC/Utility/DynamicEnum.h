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

	/**
	 * Class for making enum at runtime.
	 */
	template<typename T, typename CountType = uint32_t, CountType start = 0, typename Counter = EnumBasicCounter<CountType>>
	class DynamicEnum
	{
	public:
		using IntType = CountType;
	public:
		DynamicEnum()
			: index(Counter()(count++))
		{
		}

		IntType getIndex() const
		{
			return index;
		}

		static IntType getCount()
		{
			return count;
		}

		IntType getNumElements() const
		{
			return 1;
		}

		bool operator==(const DynamicEnum& other) const { return index == other.index; }
		bool operator!=(const DynamicEnum& other) const { return index != other.index; }

	protected:
		IntType index;
		static IntType count;
	};

	template<typename T, typename CountType, CountType start, typename Counter>
	CountType DynamicEnum<T, CountType, start, Counter>::count = start;

	template<typename T, typename CountType = uint32_t, CountType start = 0, typename Counter = EnumBasicCounter<CountType>>
	class DynamicRangeEnum : public DynamicEnum<T, CountType, start, Counter>
	{
	public:
		DynamicRangeEnum()
			: num(1)
		{}

		DynamicRangeEnum(CountType range)
			: DynamicEnum()
			, num(range)
		{
			// increase the count by the range so that it's consistent
			count += num - 1;
		}

		CountType getNumElements() const
		{
			return num;
		}

	private:
		CountType num;
	};

	template<typename DynamicType>
	class DynamicEnumRef
	{
		using IntType = typename DynamicType::IntType;

	public:
		DynamicEnumRef()
			: index(0)
		{}

		DynamicEnumRef(const DynamicType& typeRef)
			: index(typeRef.getIndex())
		{
		}

		DynamicEnumRef(const DynamicType& typeRef, size_t indexVal)
			: index(typeRef.getIndex() + indexVal)
		{
			assert(index - indexVal < typeRef.getNumElements());
		}

		DynamicEnumRef(IntType value)
			: index(value)
		{}


		/** Return an index in the enum. */
		IntType getIndex() const
		{
			return index;
		}

	private:
		IntType index;
	};
}
