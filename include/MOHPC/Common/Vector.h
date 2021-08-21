#pragma once

#include "SimpleVector.h"

#include <cstdio>
#include <cmath>

namespace MOHPC
{
	static void VectorFromString(const char* value, vec3r_t out)
	{
		sscanf(value, "%f %f %f", &out[0], &out[1], &out[2]);
	}

	static void VectorClear(vec3r_t vec)
	{
		vec[0] = vec[1] = vec[2] = 0.f;
	}

	static void VectorCopy(const_vec3r_t in, vec3r_t out)
	{
		out[0] = in[0];
		out[1] = in[1];
		out[2] = in[2];
	}

	static void VectorMA(const_vec3r_t start, float scale, const_vec3r_t dir, vec3r_t dest)
	{
		dest[0] = start[0] + dir[0] * scale;
		dest[1] = start[1] + dir[1] * scale;
		dest[2] = start[2] + dir[2] * scale;
	}

	template<typename Type>
	static void Vector4Clear(Type& vec)
	{
		vec[0] = vec[1] = vec[2] = vec[3] = 0.f;
	}

	static void Vector4Copy(const_vec4r_t in, vec4r_t out)
	{
		out[0] = in[0];
		out[1] = in[1];
		out[2] = in[2];
		out[3] = in[3];
	}

	static void AddPointToBounds(const_vec3r_t v, vec3r_t mins, vec3r_t maxs)
	{
		for (int i = 0; i < 3; i++)
		{
			float val = (float)v[i];
			if (val < mins[i])
				mins[i] = val;
			if (val > maxs[i])
				maxs[i] = val;
		}
	}

	static void ClearBounds(vec3r_t mins, vec3r_t maxs)
	{
		mins[0] = mins[1] = mins[2] = 99999;
		maxs[0] = maxs[1] = maxs[2] = -99999;
	}

	static void SnapVector(vec3r_t normal)
	{
		static constexpr float NORMAL_EPSILON = 0.0001f;
		static constexpr float DIST_EPSILON = 0.02f;

		for (int i = 0; i < 3; i++)
		{
			if (::fabs(normal[i] - 1) < NORMAL_EPSILON)
			{
				VectorClear(normal);
				normal[i] = 1;
				break;
			}
			if (::fabs(normal[i] - -1) < NORMAL_EPSILON)
			{
				VectorClear(normal);
				normal[i] = -1;
				break;
			}
		}
	}
}
