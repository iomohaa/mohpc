#pragma once

#include <MOHPC/Global.h>

#include <cstdint>
#include <cstddef>
#include <memory>

namespace MOHPC
{
	class FixedAllocator
	{
	public:
		FixedAllocator()
			: buffer(nullptr)
			, length(0)
			, allocated(0)
		{}
		FixedAllocator(void* bufferPtr, size_t maxSize);
		void* allocate(size_t n);
		size_t size();

		template<typename T>
		T* newObj()
		{
			void* ptr = allocate(sizeof(T));
			return new (ptr) T;
		}

		template<typename T>
		T* newObj(size_t count)
		{
			void* ptr = allocate(sizeof(T) * count);
			return new (ptr) T[count];
		}

	private:
		void* buffer;
		size_t length;
		size_t allocated;
	};

	template<typename T>
	class FixedSTLAllocator : public std::allocator<T>
	{
	public:
		using value_type = T;

	public:
		constexpr FixedSTLAllocator() {}
		constexpr FixedSTLAllocator(void* bufferPtr, size_t maxSize)
			: allocator(bufferPtr, maxSize)
		{}

		constexpr FixedSTLAllocator(const FixedSTLAllocator& other) {}

		template<typename U>
		constexpr FixedSTLAllocator(const FixedSTLAllocator<U>& other) {};

		void deallocate(const T* ptr, const size_t count)
		{
		}

		T* allocate(std::size_t count, const void* hint)
		{
			return (T*)allocator.allocate(count);
		}

		T* allocate(std::size_t count)
		{
			return (T*)allocator.allocate(count);
		}

	private:
		FixedAllocator allocator;
	};
}

MOHPC_EXPORTS extern void* operator new(size_t sz, MOHPC::FixedAllocator& allocator);
MOHPC_EXPORTS extern void* operator new[](size_t sz, MOHPC::FixedAllocator& allocator);
