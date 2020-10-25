#include <MOHPC/Common/Memory.h>
#include <cstdlib>

using namespace MOHPC;

void* MOHPC::allocateMemory(size_t size)
{
	return malloc(size);
}

void MOHPC::freeMemory(void* ptr)
{
	free(ptr);
}