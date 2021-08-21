#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <cstddef>

namespace MOHPC
{
	template<typename T, typename ArrayType = T[1]>
	class WarpArray
	{
	public:
		using Type = T;

	public:
		const T& get(size_t index) const
		{
			return data[index % count()];
		}

		T& get(size_t index)
		{
			return data[index % count()];
		}

		size_t count() const
		{
			return data.size();
		}

	private:
		ArrayType data;
	};

	template<typename T, size_t count>
	using StaticWarpArray = WarpArray<T, std::array<T, count>>;

	template<typename WarpArrayType>
	class WarpArrayCounter
	{
	public:
		WarpArrayType& getArray()
		{
			return warpArray;
		}

		const WarpArrayType& getArray() const
		{
			return warpArray;
		}

		typename WarpArrayType::Type& getFromLast(size_t index)
		{
			const size_t elem = number - index;
			return warpArray.get(elem);
		}

		typename WarpArrayType::Type& push()
		{
			return warpArray.get(number++);
		}

		void reset()
		{
			number = 0;
		}

	private:
		size_t number;
		WarpArrayType warpArray;
	};
}
