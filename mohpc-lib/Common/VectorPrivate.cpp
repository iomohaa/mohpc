#include "VectorPrivate.h"

using namespace MOHPC;

Vector3 MOHPC::makeVector(const_vec3r_t vec)
{
	return Vector3(vec[0], vec[1], vec[2]);
}

const Vector3& MOHPC::castVector(const_vec3r_t vec)
{
	return (const Vector3&)(*vec);
}

Vector3& MOHPC::castVector(vec3r_t vec)
{
	return (Vector3&)(*vec);
}

const_vec3p_t MOHPC::uncastVector(const Vector3& vec)
{
	return (const_vec3p_t)(&vec);
}

vec3p_t MOHPC::uncastVector(Vector3& vec)
{
	return (vec3p_t)(&vec);
}
