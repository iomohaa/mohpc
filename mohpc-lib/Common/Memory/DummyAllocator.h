#pragma once

#include <MOHPC/Global.h>

#include <cstdint>
#include <cstddef>
#include <memory>

namespace MOHPC
{
	class DummyAllocator
	{
	public:
		DummyAllocator();
		void* allocate(size_t n);
		size_t size();

	private:
		size_t dummy;
	};

	template<typename T>
	class DummySTLAllocator
	{
	public:
		using value_type = T;

	public:
		constexpr DummySTLAllocator() {}
		constexpr DummySTLAllocator(const DummySTLAllocator& other) {}

		template<typename U>
		constexpr DummySTLAllocator(const DummySTLAllocator<U>& other) {};

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

		size_t size()
		{
			return allocator.size();
		}

	private:
		DummyAllocator allocator;
	};
}

MOHPC_EXPORTS extern void* operator new(size_t sz, MOHPC::DummyAllocator& allocator);
MOHPC_EXPORTS extern void* operator new[](size_t sz, MOHPC::DummyAllocator& allocator);
