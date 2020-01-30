#pragma once

#include <stdlib.h>

namespace MOHPC
{
	class MEM_TempAlloc {
		unsigned char *m_CurrentMemoryBlock;
		size_t m_CurrentMemoryPos;

	public:
		MEM_TempAlloc();

		void *Alloc(size_t len);
		void FreeAll(void);
	};
};
