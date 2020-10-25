#pragma once

#include <cstdint>
#include "../Global.h"

namespace MOHPC
{
	extern "C"
	{
		MOHPC_EXPORTS void* allocateMemory(size_t size);
		MOHPC_EXPORTS void freeMemory(void* ptr);
	}
}