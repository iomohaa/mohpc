#pragma once

#include <utility>

namespace MOHPC
{
	using vec_t = float;
	using vec2_t = float[2];
	using vec2r_t = float[2];
	using const_vec2r_t = const float[2];
	using vec2p_t = float*;
	using const_vec2p_t = const float*;
	using vec3_t = float[3];
	using vec3r_t = float[3];
	using const_vec3r_t = const float[3];
	using vec3p_t = float*;
	using const_vec3p_t = const float*;
	using vec4_t = float[4];
	using vec4r_t = float[4];
	using const_vec4r_t = const float[4];
	using vec4p_t = float*;
	using const_vec4p_t = const float*;
	using quat_t = vec4_t;
	using matrix_t = float[16];

	static vec3_t vec3_origin{ 0, 0, 0 };
	static vec3_t vec3_zero{ 0, 0, 0 };
}
