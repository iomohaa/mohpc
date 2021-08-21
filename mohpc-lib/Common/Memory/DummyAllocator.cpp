#include "DummyAllocator.h"

using namespace MOHPC;

DummyAllocator::DummyAllocator()
	: dummy(0)
{}

void* DummyAllocator::allocate(size_t n)
{
	char* p = (char*)dummy;
	dummy += n;
	return nullptr;

}

size_t DummyAllocator::size()
{
	return dummy;
}

void* operator new(size_t sz, DummyAllocator& allocator)
{
	return allocator.allocate(sz);
}

void* operator new[](size_t sz, DummyAllocator& allocator)
{
	return allocator.allocate(sz);
}
