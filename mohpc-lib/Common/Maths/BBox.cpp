#include <MOHPC/Common/Maths/BBox.h>
#include <MOHPC/Common/SimpleVector.h>

using namespace MOHPC;

constexpr unsigned int BBOX_XBITS = 9;
constexpr unsigned int BBOX_YBITS = 8;
constexpr unsigned int BBOX_ZBOTTOMBITS = 5;
constexpr unsigned int BBOX_ZTOPBITS = 9;

constexpr unsigned int BBOX_MAX_X = (1 << BBOX_XBITS);
constexpr unsigned int BBOX_MAX_Y = (1 << BBOX_YBITS);
constexpr unsigned int BBOX_MAX_BOTTOM_Z = (1 << (BBOX_ZBOTTOMBITS - 1));
constexpr unsigned int BBOX_REALMAX_BOTTOM_Z = (1 << BBOX_ZBOTTOMBITS);
constexpr unsigned int BBOX_MAX_TOP_Z = (1 << BBOX_ZTOPBITS);

int MOHPC::BoundingBoxToInteger(const_vec3r_t mins, const_vec3r_t maxs)
{
	int x, y, zd, zu, result;

	x = int(maxs[0]);
	if (x < 0)
		x = 0;
	if (x >= BBOX_MAX_X)
		x = BBOX_MAX_X - 1;

	y = int(maxs[1]);
	if (y < 0)
		y = 0;
	if (y >= BBOX_MAX_Y)
		y = BBOX_MAX_Y - 1;

	zd = int(mins[2]) + BBOX_MAX_BOTTOM_Z;
	if (zd < 0)
	{
		zd = 0;
	}
	if (zd >= BBOX_REALMAX_BOTTOM_Z)
	{
		zd = BBOX_REALMAX_BOTTOM_Z - 1;
	}

	zu = int(maxs[2]);
	if (zu < 0)
		zu = 0;
	if (zu >= BBOX_MAX_TOP_Z)
		zu = BBOX_MAX_TOP_Z - 1;

	result = x |
		(y << BBOX_XBITS) |
		(zd << (BBOX_XBITS + BBOX_YBITS)) |
		(zu << (BBOX_XBITS + BBOX_YBITS + BBOX_ZBOTTOMBITS));

	return result;
}

void MOHPC::IntegerToBoundingBox(int num, vec3r_t mins, vec3r_t maxs)
{
	int x, y, zd, zu;

	x = num & (BBOX_MAX_X - 1);
	y = (num >> (BBOX_XBITS)) & (BBOX_MAX_Y - 1);
	zd = (num >> (BBOX_XBITS + BBOX_YBITS)) & (BBOX_REALMAX_BOTTOM_Z - 1);
	zd -= BBOX_MAX_BOTTOM_Z;
	zu = (num >> (BBOX_XBITS + BBOX_YBITS + BBOX_ZBOTTOMBITS)) & (BBOX_MAX_TOP_Z - 1);

	mins[0] = float(-x);
	mins[1] = float(-y);
	mins[2] = float(zd);

	maxs[0] = float(x);
	maxs[1] = float(y);
	maxs[2] = float(zu);
}
