#include "VectorPrivate.h"

using namespace MOHPC;

Eigen::Vector3f MOHPC::makeVector(const_vec3r_t vec)
{
	return Eigen::Vector3f(vec[0], vec[1], vec[2]);
}

const Eigen::Vector3f& MOHPC::castVector(const_vec3r_t vec)
{
	return (const Eigen::Vector3f&)(*vec);
}

Eigen::Vector3f& MOHPC::castVector(vec3r_t vec)
{
	return (Eigen::Vector3f&)(*vec);
}

const_vec3p_t MOHPC::uncastVector(const Eigen::Vector3f& vec)
{
	return (const_vec3p_t)(&vec);
}

vec3p_t MOHPC::uncastVector(Eigen::Vector3f& vec)
{
	return (vec3p_t)(&vec);
}
