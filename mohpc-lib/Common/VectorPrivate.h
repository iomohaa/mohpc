#include <MOHPC/Common/Vector.h>
#include <Eigen/Geometry>

namespace MOHPC
{
	using Vector3 = Eigen::Vector3f;

	Vector3 makeVector(const_vec3r_t vec);
	const Vector3& castVector(const_vec3r_t vec);
	Vector3& castVector(vec3r_t vec);
	const_vec3p_t uncastVector(const Vector3& vec);
	vec3p_t uncastVector(Vector3& vec);

	static_assert(sizeof(Vector3) == sizeof(vec3_t), "Eigen vector must be the same size as vec3_t");
}