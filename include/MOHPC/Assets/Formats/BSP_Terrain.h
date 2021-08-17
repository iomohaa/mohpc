#pragma once

#include <cstdint>

namespace MOHPC
{
	using terraInt = int32_t;

	struct worknode_t {
		int32_t i0;
		int32_t j0;
		int32_t i1;
		int32_t j1;
		int32_t i2;
		int32_t j2;
	};

	namespace BSPData
	{
		struct varnodeIndex {
			short unsigned int iTreeAndMask;
			short unsigned int iNode;
		};
	}
}
