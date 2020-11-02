#include <MOHPC/Misc/EndianHelpers.h>
#include <MOHPC/Misc/Endian.h>

#include <MOHPC/Vector.h>

using namespace MOHPC;

Vector EndianHelpers::BigVector(const IEndian& endian, const Vector& value)
{
	Vector result;

	for (size_t i = 0; i < 3; i++)
	{
		result[i] = endian.BigFloat(value[i]);
	}

	return result;
}

Vector EndianHelpers::LittleVector(const IEndian& endian, const Vector& value)
{
	Vector result;

	for (size_t i = 0; i < 3; i++)
	{
		result[i] = endian.LittleFloat(value[i]);
	}

	return result;
}
