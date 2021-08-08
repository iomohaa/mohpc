#pragma once

#include "Endian.h"
#include "../../Common/SimpleVector.h"

namespace MOHPC
{
	class IEndian;
	class Vector;

	namespace EndianHelpers
	{
		void BigVector(const IEndian& endian, const vec3r_t value, vec3_t& out);
		void LittleVector(const IEndian& endian, const vec3r_t value, vec3_t& out);
	}
}
