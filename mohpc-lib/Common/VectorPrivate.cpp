#include <MOHPC/Common/Vector.h>

using namespace MOHPC;

void MOHPC::VectorFromString(const char* value, vec3r_t out)
{
	std::sscanf(value, "%f %f %f", &out[0], &out[1], &out[2]);
}
