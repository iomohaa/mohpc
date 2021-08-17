#pragma once

#include "../../Global.h"
#include "../SimpleVector.h"

namespace MOHPC
{
	MOHPC_EXPORTS int BoundingBoxToInteger(const_vec3r_t mins, const_vec3r_t maxs);
	MOHPC_EXPORTS void IntegerToBoundingBox(int num, vec3r_t mins, vec3r_t maxs);
}
