#pragma once

#include "Endian.h"

namespace MOHPC
{
	class IEndian;
	class Vector;

	namespace EndianHelpers
	{
		Vector BigVector(const IEndian& endian, const Vector& value);
		Vector LittleVector(const IEndian& endian, const Vector& value);
	}
}
