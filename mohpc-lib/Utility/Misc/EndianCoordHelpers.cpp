#include <MOHPC/Utility/Misc/EndianCoordHelpers.h>
#include <MOHPC/Utility/Misc/Endian.h>

#include <MOHPC/Common/Vector.h>

using namespace MOHPC;

void EndianHelpers::BigVector(const IEndian& endian, const vec3r_t value, vec3_t& out)
{
	for (size_t i = 0; i < 3; i++)
	{
		out[i] = endian.BigFloat(value[i]);
	}
}

void EndianHelpers::LittleVector(const IEndian& endian, const vec3r_t value, vec3_t& out)
{
	for (size_t i = 0; i < 3; i++)
	{
		out[i] = endian.LittleFloat(value[i]);
	}
}
