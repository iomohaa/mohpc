#include <MOHPC/Common/Vector.h>
#include <Eigen/Geometry>

namespace MOHPC
{
	Eigen::Vector3f makeVector(const_vec3r_t vec);
	const Eigen::Vector3f& castVector(const_vec3r_t vec);
	Eigen::Vector3f& castVector(vec3r_t vec);
	const_vec3p_t uncastVector(const Eigen::Vector3f& vec);
	vec3p_t uncastVector(Eigen::Vector3f& vec);

	static_assert(sizeof(Eigen::Vector3f) == sizeof(vec3_t), "Eigen vector must be the same size as vec3_t");
}