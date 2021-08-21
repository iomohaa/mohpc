#include "FixedAllocator.h"

#include <cassert>

using namespace MOHPC;

FixedAllocator::FixedAllocator(void* bufferPtr, size_t maxSize)
	: buffer(bufferPtr)
	, length(maxSize)
	, allocated(0)
{
}

void* FixedAllocator::allocate(size_t n)
{
	assert(allocated + n <= length);

	char* p = (char*)buffer + allocated;
	allocated += n;
	return p;
}

size_t FixedAllocator::size()
{
	return allocated;
}

void* operator new(size_t sz, FixedAllocator& allocator)
{
	return allocator.allocate(sz);
}

void* operator new[](size_t sz, FixedAllocator& allocator)
{
	return allocator.allocate(sz);
}
