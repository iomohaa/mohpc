#include <Shared.h>
#include <MOHPC/Assets/Formats/BSP.h>
#include "Polylib.h"
#include "BSP_Curve.h"

#include "../../../Common/VectorPrivate.h"

#include <cassert>

using namespace MOHPC;
using namespace BSPData;

static constexpr unsigned long MAX_FACETS = 1024;
static constexpr unsigned long MAX_PATCH_PLANES = 4096;
static constexpr unsigned long SUBDIVIDE_DISTANCE = 16;
static constexpr double PLANE_TRI_EPSILON = 0.1;
static constexpr double WRAP_POINT_EPSILON = 0.1;

struct cGrid_t {
public:
	cGrid_t();

public:
	int32_t width;
	int32_t height;
	bool wrapWidth;
	bool wrapHeight;
	vec3_t points[MAX_GRID_SIZE][MAX_GRID_SIZE];
};

cGrid_t::cGrid_t()
	: width(0)
	, height(0)
	, wrapWidth(false)
	, wrapHeight(false)
	, points{ 0 }
{}

static bool ValidateFacet(patchWork_t& pw, BSPData::Facet *facet) {
	float		plane[4];
	int			j;
	winding_t	*w;
	vec3_t		bounds[2];

	if (facet->surfacePlane == -1) {
		return false;
	}

	Vector4Copy(pw.planes[facet->surfacePlane].plane, plane);
	w = BaseWindingForPlane(plane, plane[3]);
	for (j = 0; j < facet->numBorders && w; j++)
	{
		if (facet->borderPlanes[j] == -1)
		{
			FreeWinding(w);
			return false;
		}
		Vector4Copy(pw.planes[facet->borderPlanes[j]].plane, plane);
		if (!facet->borderInward[j])
		{
			castVector(plane) = -castVector(plane);
			plane[3] = -plane[3];
		}
		ChopWindingInPlace(&w, plane, plane[3], 0.1f);
	}

	if (!w)
	{
		// winding was completely chopped away
		return false;
	}

	// see if the facet is unreasonably large
	WindingBounds(w, bounds[0], bounds[1]);
	FreeWinding(w);

	for (j = 0; j < 3; j++) {
		if (bounds[1][j] - bounds[0][j] > MAX_MAP_BOUNDS) {
			return false;		// we must be missing a plane
		}
		if (bounds[0][j] >= MAX_MAP_BOUNDS) {
			return false;
		}
		if (bounds[1][j] <= -MAX_MAP_BOUNDS) {
			return false;
		}
	}
	return true;		// winding is fine
}

static int SignbitsForNormal(vec3_t normal) {
	int	bits, j;

	bits = 0;
	for (j = 0; j < 3; j++) {
		if (normal[j] < 0) {
			bits |= 1 << j;
		}
	}
	return bits;
}

#define	NORMAL_EPSILON	0.0001
#define	DIST_EPSILON	0.02

int PlaneEqual(BSPData::PatchPlane *p, float plane[4], int *flipped) {
	float invplane[4];

	if (
		fabs(p->plane[0] - plane[0]) < NORMAL_EPSILON
		&& fabs(p->plane[1] - plane[1]) < NORMAL_EPSILON
		&& fabs(p->plane[2] - plane[2]) < NORMAL_EPSILON
		&& fabs(p->plane[3] - plane[3]) < DIST_EPSILON)
	{
		*flipped = false;
		return true;
	}

	castVector(invplane) = -castVector(plane);
	invplane[3] = -plane[3];

	if (
		fabs(p->plane[0] - invplane[0]) < NORMAL_EPSILON
		&& fabs(p->plane[1] - invplane[1]) < NORMAL_EPSILON
		&& fabs(p->plane[2] - invplane[2]) < NORMAL_EPSILON
		&& fabs(p->plane[3] - invplane[3]) < DIST_EPSILON)
	{
		*flipped = true;
		return true;
	}

	return false;
}

bool BSP::PlaneFromPoints(vec4_t plane, vec3_t a, vec3_t b, vec3_t c)
{
	Vector3 d1, d2;

	d1 = castVector(b) - castVector(a);
	d2 = castVector(c) - castVector(a);
	castVector(plane) = d2.cross(d1);
	castVector(plane).normalize();
	if (castVector(plane).squaredNorm() == 0) {
		return false;
	}

	plane[3] = castVector(a).dot(castVector(plane));
	return true;
}

int FindPlane2(patchWork_t& pw, float plane[4], int *flipped) {
	// see if the points are close enough to an existing plane
	for (uint32_t i = 0; i < pw.numPlanes; i++) {
		if (PlaneEqual(&pw.planes[i], plane, flipped)) return i;
	}

	// add a new plane
	if (pw.numPlanes == MAX_PATCH_PLANES) {
		assert(!"MAX_PATCH_PLANES");
	}

	Vector4Copy(plane, pw.planes[pw.numPlanes].plane);
	pw.planes[pw.numPlanes].signbits = SignbitsForNormal(plane);

	pw.numPlanes++;

	*flipped = false;

	return pw.numPlanes - 1;
}

static int FindPlane(patchWork_t& pw, float *p1, float *p2, float *p3) {
	float	plane[4];
	float	d;

	if (!BSP::PlaneFromPoints(plane, p1, p2, p3)) {
		return -1;
	}

	// see if the points are close enough to an existing plane
	for (uint32_t i = 0; i < pw.numPlanes; i++)
	{
		if (castVector(plane).dot(castVector(pw.planes[i].plane)) < 0) {
			continue;	// allow backwards planes?
		}

		d = castVector(p1).dot(castVector(pw.planes[i].plane)) - pw.planes[i].plane[3];
		if (d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON) {
			continue;
		}

		d = castVector(p2).dot(castVector(pw.planes[i].plane)) - pw.planes[i].plane[3];
		if (d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON) {
			continue;
		}

		d = castVector(p3).dot(castVector(pw.planes[i].plane)) - pw.planes[i].plane[3];
		if (d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON) {
			continue;
		}

		// found it
		return i;
	}

	// add a new plane
	if (pw.numPlanes == MAX_PATCH_PLANES) {
		return -1;
	}

	Vector4Copy(plane, pw.planes[pw.numPlanes].plane);
	pw.planes[pw.numPlanes].signbits = SignbitsForNormal(plane);

	pw.numPlanes++;

	return pw.numPlanes - 1;
}

static bool	NeedsSubdivision(vec3_t a, vec3_t b, vec3_t c, float subdivisions) {
	Vector3 cmid;
	Vector3 lmid;
	Vector3 delta;
	float dist;
	int i;

	// calculate the linear midpoint
	for (i = 0; i < 3; i++) {
		lmid[i] = 0.5f*(a[i] + c[i]);
	}

	// calculate the exact curve midpoint
	for (i = 0; i < 3; i++) {
		cmid[i] = 0.5f * (0.5f*(a[i] + b[i]) + 0.5f*(b[i] + c[i]));
	}

	// see if the curve is far enough away from the linear mid
	delta = cmid - lmid;
	dist = delta.norm();

	return dist >= subdivisions;
}

static void Subdivide(vec3_t a, vec3_t b, vec3_t c, vec3_t out1, vec3_t out2, vec3_t out3) {
	int		i;

	for (i = 0; i < 3; i++) {
		out1[i] = 0.5f * (a[i] + b[i]);
		out3[i] = 0.5f * (b[i] + c[i]);
		out2[i] = 0.5f * (out1[i] + out3[i]);
	}
}

static void TransposeGrid(cGrid_t *grid) {
	int i, j, l;
	vec3_t temp;
	bool tempWrap;

	if (grid->width > grid->height) {
		for (i = 0; i < grid->height; i++) {
			for (j = i + 1; j < grid->width; j++) {
				if (j < grid->height) {
					// swap the value
					VectorCopy(grid->points[i][j], temp);
					VectorCopy(grid->points[j][i], grid->points[i][j]);
					VectorCopy(temp, grid->points[j][i]);
				}
				else {
					// just copy
					VectorCopy(grid->points[j][i], grid->points[i][j]);
				}
			}
		}
	}
	else {
		for (i = 0; i < grid->width; i++) {
			for (j = i + 1; j < grid->height; j++) {
				if (j < grid->width) {
					// swap the value
					VectorCopy(grid->points[j][i], temp);
					VectorCopy(grid->points[i][j], grid->points[j][i]);
					VectorCopy(temp, grid->points[i][j]);
				}
				else {
					// just copy
					VectorCopy(grid->points[i][j], grid->points[j][i]);
				}
			}
		}
	}

	l = grid->width;
	grid->width = grid->height;
	grid->height = l;

	tempWrap = grid->wrapWidth;
	grid->wrapWidth = grid->wrapHeight;
	grid->wrapHeight = tempWrap;
}

static void SetGridWrapWidth(cGrid_t *grid) {
	int		i, j;
	float	d;

	for (i = 0; i < grid->height; i++) {
		for (j = 0; j < 3; j++) {
			d = grid->points[0][i][j] - grid->points[grid->width - 1][i][j];
			if (d < -WRAP_POINT_EPSILON || d > WRAP_POINT_EPSILON) {
				break;
			}
		}
		if (j != 3) {
			break;
		}
	}
	if (i == grid->height) {
		grid->wrapWidth = true;
	}
	else {
		grid->wrapWidth = false;
	}
}

static void SubdivideGridColumns(cGrid_t *grid, float subdivisions) {
	int		i, j, k;

	for (i = 0; i < grid->width - 2; ) {
		// grid->points[i][x] is an interpolating control point
		// grid->points[i+1][x] is an aproximating control point
		// grid->points[i+2][x] is an interpolating control point

		//
		// first see if we can collapse the aproximating collumn away
		//
		for (j = 0; j < grid->height; j++) {
			if (NeedsSubdivision(grid->points[i][j], grid->points[i + 1][j], grid->points[i + 2][j], subdivisions)) {
				break;
			}
		}
		if (j == grid->height) {
			// all of the points were close enough to the linear midpoints
			// that we can collapse the entire column away
			for (j = 0; j < grid->height; j++) {
				// remove the column
				for (k = i + 2; k < grid->width; k++) {
					VectorCopy(grid->points[k][j], grid->points[k - 1][j]);
				}
			}

			grid->width--;

			// go to the next curve segment
			i++;
			continue;
		}

		//
		// we need to subdivide the curve
		//
		for (j = 0; j < grid->height; j++) {
			vec3_t	prev, mid, next;

			// save the control points now
			VectorCopy(grid->points[i][j], prev);
			VectorCopy(grid->points[i + 1][j], mid);
			VectorCopy(grid->points[i + 2][j], next);

			// make room for two additional columns in the grid
			// columns i+1 will be replaced, column i+2 will become i+4
			// i+1, i+2, and i+3 will be generated
			for (k = grid->width - 1; k > i + 1; k--) {
				VectorCopy(grid->points[k][j], grid->points[k + 2][j]);
			}

			// generate the subdivided points
			Subdivide(prev, mid, next, grid->points[i + 1][j], grid->points[i + 2][j], grid->points[i + 3][j]);
		}

		grid->width += 2;

		// the new aproximating point at i+1 may need to be removed
		// or subdivided farther, so don't advance i
	}
}

#define	POINT_EPSILON	0.1
static bool ComparePoints(float *a, float *b) {
	float d;

	d = a[0] - b[0];
	if (d < -POINT_EPSILON || d > POINT_EPSILON) {
		return false;
	}
	d = a[1] - b[1];
	if (d < -POINT_EPSILON || d > POINT_EPSILON) {
		return false;
	}
	d = a[2] - b[2];
	if (d < -POINT_EPSILON || d > POINT_EPSILON) {
		return false;
	}
	return true;
}

static void RemoveDegenerateColumns(cGrid_t *grid) {
	int		i, j, k;

	for (i = 0; i < grid->width - 1; i++) {
		for (j = 0; j < grid->height; j++) {
			if (!ComparePoints(grid->points[i][j], grid->points[i + 1][j])) {
				break;
			}
		}

		if (j != grid->height) {
			continue;	// not degenerate
		}

		for (j = 0; j < grid->height; j++) {
			// remove the column
			for (k = i + 2; k < grid->width; k++) {
				VectorCopy(grid->points[k][j], grid->points[k - 1][j]);
			}
		}
		grid->width--;

		// check against the next column
		i--;
	}
}

static int PointOnPlaneSide(patchWork_t& pw, float *p, int planeNum) {
	float	*plane;
	float	d;

	if (planeNum == -1) {
		return SIDE_ON;
	}
	plane = pw.planes[planeNum].plane;

	d = castVector(p).dot(castVector(plane)) - plane[3];

	if (d > PLANE_TRI_EPSILON) {
		return SIDE_FRONT;
	}

	if (d < -PLANE_TRI_EPSILON) {
		return SIDE_BACK;
	}

	return SIDE_ON;
}

static int GridPlane(int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], int i, int j, int tri) {
	int		p;

	p = gridPlanes[i][j][tri];
	if (p != -1) {
		return p;
	}
	p = gridPlanes[i][j][!tri];
	if (p != -1) {
		return p;
	}

	// should never happen
	return -1;
}

static int EdgePlaneNum(patchWork_t& pw, cGrid_t *grid, int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2], int i, int j, int k) {
	float	*p1, *p2;
	vec3_t		up;
	int			p;

	switch (k) {
	case 0:	// top border
		p1 = grid->points[i][j];
		p2 = grid->points[i + 1][j];
		p = GridPlane(gridPlanes, i, j, 0);
		VectorMA(p1, 4, pw.planes[p].plane, up);
		return FindPlane(pw, p1, p2, up);

	case 2:	// bottom border
		p1 = grid->points[i][j + 1];
		p2 = grid->points[i + 1][j + 1];
		p = GridPlane(gridPlanes, i, j, 1);
		VectorMA(p1, 4, pw.planes[p].plane, up);
		return FindPlane(pw, p2, p1, up);

	case 3: // left border
		p1 = grid->points[i][j];
		p2 = grid->points[i][j + 1];
		p = GridPlane(gridPlanes, i, j, 1);
		VectorMA(p1, 4, pw.planes[p].plane, up);
		return FindPlane(pw, p2, p1, up);

	case 1:	// right border
		p1 = grid->points[i + 1][j];
		p2 = grid->points[i + 1][j + 1];
		p = GridPlane(gridPlanes, i, j, 0);
		VectorMA(p1, 4, pw.planes[p].plane, up);
		return FindPlane(pw, p1, p2, up);

	case 4:	// diagonal out of triangle 0
		p1 = grid->points[i + 1][j + 1];
		p2 = grid->points[i][j];
		p = GridPlane(gridPlanes, i, j, 0);
		VectorMA(p1, 4, pw.planes[p].plane, up);
		return FindPlane(pw, p1, p2, up);

	case 5:	// diagonal out of triangle 1
		p1 = grid->points[i][j];
		p2 = grid->points[i + 1][j + 1];
		p = GridPlane(gridPlanes, i, j, 1);
		VectorMA(p1, 4, pw.planes[p].plane, up);
		return FindPlane(pw, p1, p2, up);

	}

	return -1;
}

static void SetBorderInward(patchWork_t& pw, BSPData::Facet *facet, cGrid_t *grid, int gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2],
	int i, int j, int which) {
	int		k, l;
	float	*points[4];
	int		numPoints;

	switch (which) {
	case -1:
		points[0] = grid->points[i][j];
		points[1] = grid->points[i + 1][j];
		points[2] = grid->points[i + 1][j + 1];
		points[3] = grid->points[i][j + 1];
		numPoints = 4;
		break;
	case 0:
		points[0] = grid->points[i][j];
		points[1] = grid->points[i + 1][j];
		points[2] = grid->points[i + 1][j + 1];
		numPoints = 3;
		break;
	case 1:
		points[0] = grid->points[i + 1][j + 1];
		points[1] = grid->points[i][j + 1];
		points[2] = grid->points[i][j];
		numPoints = 3;
		break;
	default:
		numPoints = 0;
		break;
	}

	for (k = 0; k < facet->numBorders; k++) {
		int		front, back;

		front = 0;
		back = 0;

		for (l = 0; l < numPoints; l++) {
			int		side;

			side = PointOnPlaneSide(pw, points[l], facet->borderPlanes[k]);
			if (side == SIDE_FRONT) {
				front++;
			} if (side == SIDE_BACK) {
				back++;
			}
		}

		if (front && !back) {
			facet->borderInward[k] = true;
		}
		else if (back && !front) {
			facet->borderInward[k] = false;
		}
		else if (!front && !back) {
			// flat side border
			facet->borderPlanes[k] = -1;
		}
		else {
			// bisecting side border
			facet->borderInward[k] = false;
		}
	}
}

void AddFacetBevels(patchWork_t& pw, BSPData::Facet *facet) {

	int i, j, k, l;
	int axis, dir, order, flipped;
	float plane[4], d, newplane[4];
	winding_t *w, *w2;
	Vector3 mins, maxs, vec, vec2;

	Vector4Copy(pw.planes[facet->surfacePlane].plane, plane);

	w = BaseWindingForPlane(plane, plane[3]);
	for (j = 0; j < facet->numBorders && w; j++)
	{
		if (facet->borderPlanes[j] == facet->surfacePlane) continue;
		Vector4Copy(pw.planes[facet->borderPlanes[j]].plane, plane);

		if (!facet->borderInward[j])
		{
			castVector(plane) = -castVector(plane);
			plane[3] = -plane[3];
		}

		ChopWindingInPlace(&w, plane, plane[3], 0.1f);
	}
	if (!w) {
		return;
	}

	WindingBounds(w, uncastVector(mins), uncastVector(maxs));

	// add the axial pw.planes
	order = 0;
	for (axis = 0; axis < 3; axis++)
	{
		for (dir = -1; dir <= 1; dir += 2, order++)
		{
			VectorClear(plane);
			plane[axis] = (float)dir;
			if (dir == 1) {
				plane[3] = maxs[axis];
			}
			else {
				plane[3] = -mins[axis];
			}
			//if it's the surface plane
			if (PlaneEqual(&pw.planes[facet->surfacePlane], plane, &flipped)) {
				continue;
			}
			// see if the plane is allready present
			for (i = 0; i < facet->numBorders; i++) {
				if (PlaneEqual(&pw.planes[facet->borderPlanes[i]], plane, &flipped))
					break;
			}

			if (i == facet->numBorders) {
				//if (facet->numBorders > 4 + 6 + 16) Com_Printf("ERROR: too many bevels\n");
				facet->borderPlanes[facet->numBorders] = FindPlane2(pw, plane, &flipped);
				facet->borderNoAdjust[facet->numBorders] = 0;
				facet->borderInward[facet->numBorders] = flipped;
				facet->numBorders++;
			}
		}
	}
	//
	// add the edge bevels
	//
	// test the non-axial plane edges
	for (j = 0; j < w->numpoints; j++)
	{
		k = (j + 1) % w->numpoints;
		vec = castVector(w->p[j]) - castVector(w->p[k]);
		//if it's a degenerate edge
		if (vec.squaredNorm() < 0.5) {
			continue;
		}

		vec.normalize();

		SnapVector(uncastVector(vec));
		for (k = 0; k < 3; k++)
			if (vec[k] == -1 || vec[k] == 1)
				break;	// axial
		if (k < 3)
			continue;	// only test non-axial edges

						// try the six possible slanted axials from this edge
		for (axis = 0; axis < 3; axis++)
		{
			for (dir = -1; dir <= 1; dir += 2)
			{
				// construct a plane
				vec2 = Vector3();
				vec2[axis] = (float)dir;
				castVector(plane) = vec.cross(vec2);
				if (castVector(plane).squaredNorm() < 0.5) {
					continue;
				}

				castVector(plane).normalize();
				plane[3] = castVector(w->p[j]).dot(castVector(plane));

				// if all the points of the facet winding are
				// behind this plane, it is a proper edge bevel
				for (l = 0; l < w->numpoints; l++)
				{
					d = castVector(w->p[l]).dot(castVector(plane)) - plane[3];
					if (d > 0.1)
						break;	// point in front
				}
				if (l < w->numpoints)
					continue;

				//if it's the surface plane
				if (PlaneEqual(&pw.planes[facet->surfacePlane], plane, &flipped)) {
					continue;
				}
				// see if the plane is allready present
				for (i = 0; i < facet->numBorders; i++) {
					if (PlaneEqual(&pw.planes[facet->borderPlanes[i]], plane, &flipped)) {
						break;
					}
				}

				if (i == facet->numBorders) {
					//if (facet->numBorders > 4 + 6 + 16) Com_Printf("ERROR: too many bevels\n");
					facet->borderPlanes[facet->numBorders] = FindPlane2(pw, plane, &flipped);

					for (k = 0; k < facet->numBorders; k++) {
						//if (facet->borderPlanes[facet->numBorders] ==
						//	facet->borderPlanes[k]) Com_Printf("WARNING: bevel plane already used\n");
					}

					facet->borderNoAdjust[facet->numBorders] = 0;
					facet->borderInward[facet->numBorders] = flipped;
					//
					w2 = CopyWinding(w);
					Vector4Copy(pw.planes[facet->borderPlanes[facet->numBorders]].plane, newplane);
					if (!facet->borderInward[facet->numBorders])
					{
						castVector(newplane) = -castVector(newplane);
						newplane[3] = -newplane[3];
					} //end if
					ChopWindingInPlace(&w2, newplane, newplane[3], 0.1f);
					if (!w2) {
						//if (developer->integer == 2) {
						//	Com_DPrintf("WARNING: AddFacetBevels... invalid bevel\n");
						//}
						continue;
					}
					else {
						FreeWinding(w2);
					}
					//
					facet->numBorders++;
					//already got a bevel
					//					break;
				}
			}
		}
	}
	FreeWinding(w);

#ifndef BSPC
	//add opposite plane
	facet->borderPlanes[facet->numBorders] = facet->surfacePlane;
	facet->borderNoAdjust[facet->numBorders] = 0;
	facet->borderInward[facet->numBorders] = true;
	facet->numBorders++;
#endif //BSPC

}

typedef enum {
	EN_TOP,
	EN_RIGHT,
	EN_BOTTOM,
	EN_LEFT
} edgeName_t;

static void PatchCollideFromGrid(patchWork_t& pw, cGrid_t *grid, BSPData::PatchCollide *pf) {
	int				i, j;
	float			*p1, *p2, *p3;
	int				gridPlanes[MAX_GRID_SIZE][MAX_GRID_SIZE][2];
	BSPData::Facet		*facet;
	int				borders[4];
	int				noAdjust[4];

	pw.numPlanes = 0;
	pw.numFacets = 0;

	// find the pw.planes for each triangle of the grid
	for (i = 0; i < grid->width - 1; i++) {
		for (j = 0; j < grid->height - 1; j++) {
			p1 = grid->points[i][j];
			p2 = grid->points[i + 1][j];
			p3 = grid->points[i + 1][j + 1];
			gridPlanes[i][j][0] = FindPlane(pw, p1, p2, p3);

			p1 = grid->points[i + 1][j + 1];
			p2 = grid->points[i][j + 1];
			p3 = grid->points[i][j];
			gridPlanes[i][j][1] = FindPlane(pw, p1, p2, p3);
		}
	}

	// create the borders for each facet
	for (i = 0; i < grid->width - 1; i++) {
		for (j = 0; j < grid->height - 1; j++) {

			borders[EN_TOP] = -1;
			if (j > 0) {
				borders[EN_TOP] = gridPlanes[i][j - 1][1];
			}
			else if (grid->wrapHeight) {
				borders[EN_TOP] = gridPlanes[i][grid->height - 2][1];
			}
			noAdjust[EN_TOP] = (borders[EN_TOP] == gridPlanes[i][j][0]);
			if (borders[EN_TOP] == -1 || noAdjust[EN_TOP]) {
				borders[EN_TOP] = EdgePlaneNum(pw, grid, gridPlanes, i, j, 0);
			}

			borders[EN_BOTTOM] = -1;
			if (j < grid->height - 2) {
				borders[EN_BOTTOM] = gridPlanes[i][j + 1][0];
			}
			else if (grid->wrapHeight) {
				borders[EN_BOTTOM] = gridPlanes[i][0][0];
			}
			noAdjust[EN_BOTTOM] = (borders[EN_BOTTOM] == gridPlanes[i][j][1]);
			if (borders[EN_BOTTOM] == -1 || noAdjust[EN_BOTTOM]) {
				borders[EN_BOTTOM] = EdgePlaneNum(pw, grid, gridPlanes, i, j, 2);
			}

			borders[EN_LEFT] = -1;
			if (i > 0) {
				borders[EN_LEFT] = gridPlanes[i - 1][j][0];
			}
			else if (grid->wrapWidth) {
				borders[EN_LEFT] = gridPlanes[grid->width - 2][j][0];
			}
			noAdjust[EN_LEFT] = (borders[EN_LEFT] == gridPlanes[i][j][1]);
			if (borders[EN_LEFT] == -1 || noAdjust[EN_LEFT]) {
				borders[EN_LEFT] = EdgePlaneNum(pw, grid, gridPlanes, i, j, 3);
			}

			borders[EN_RIGHT] = -1;
			if (i < grid->width - 2) {
				borders[EN_RIGHT] = gridPlanes[i + 1][j][1];
			}
			else if (grid->wrapWidth) {
				borders[EN_RIGHT] = gridPlanes[0][j][1];
			}
			noAdjust[EN_RIGHT] = (borders[EN_RIGHT] == gridPlanes[i][j][0]);
			if (borders[EN_RIGHT] == -1 || noAdjust[EN_RIGHT]) {
				borders[EN_RIGHT] = EdgePlaneNum(pw, grid, gridPlanes, i, j, 1);
			}

			if (pw.numFacets == MAX_FACETS) {
				return;
			}
			facet = &pw.facets[pw.numFacets];
			memset(facet, 0, sizeof(*facet));

			if (gridPlanes[i][j][0] == gridPlanes[i][j][1]) {
				if (gridPlanes[i][j][0] == -1) {
					continue;		// degenrate
				}
				facet->surfacePlane = gridPlanes[i][j][0];
				facet->numBorders = 4;
				facet->borderPlanes[0] = borders[EN_TOP];
				facet->borderNoAdjust[0] = noAdjust[EN_TOP];
				facet->borderPlanes[1] = borders[EN_RIGHT];
				facet->borderNoAdjust[1] = noAdjust[EN_RIGHT];
				facet->borderPlanes[2] = borders[EN_BOTTOM];
				facet->borderNoAdjust[2] = noAdjust[EN_BOTTOM];
				facet->borderPlanes[3] = borders[EN_LEFT];
				facet->borderNoAdjust[3] = noAdjust[EN_LEFT];
				SetBorderInward(pw, facet, grid, gridPlanes, i, j, -1);
				if (ValidateFacet(pw, facet)) {
					AddFacetBevels(pw, facet);
					pw.numFacets++;
				}
			}
			else {
				// two seperate triangles
				facet->surfacePlane = gridPlanes[i][j][0];
				facet->numBorders = 3;
				facet->borderPlanes[0] = borders[EN_TOP];
				facet->borderNoAdjust[0] = noAdjust[EN_TOP];
				facet->borderPlanes[1] = borders[EN_RIGHT];
				facet->borderNoAdjust[1] = noAdjust[EN_RIGHT];
				facet->borderPlanes[2] = gridPlanes[i][j][1];
				if (facet->borderPlanes[2] == -1) {
					facet->borderPlanes[2] = borders[EN_BOTTOM];
					if (facet->borderPlanes[2] == -1) {
						facet->borderPlanes[2] = EdgePlaneNum(pw, grid, gridPlanes, i, j, 4);
					}
				}
				SetBorderInward(pw, facet, grid, gridPlanes, i, j, 0);
				if (ValidateFacet(pw, facet)) {
					AddFacetBevels(pw, facet);
					pw.numFacets++;
				}

				if (pw.numFacets == MAX_FACETS) {
					return;
				}

				facet = &pw.facets[pw.numFacets];
				memset(facet, 0, sizeof(*facet));

				facet->surfacePlane = gridPlanes[i][j][1];
				facet->numBorders = 3;
				facet->borderPlanes[0] = borders[EN_BOTTOM];
				facet->borderNoAdjust[0] = noAdjust[EN_BOTTOM];
				facet->borderPlanes[1] = borders[EN_LEFT];
				facet->borderNoAdjust[1] = noAdjust[EN_LEFT];
				facet->borderPlanes[2] = gridPlanes[i][j][0];
				if (facet->borderPlanes[2] == -1) {
					facet->borderPlanes[2] = borders[EN_TOP];
					if (facet->borderPlanes[2] == -1) {
						facet->borderPlanes[2] = EdgePlaneNum(pw, grid, gridPlanes, i, j, 5);
					}
				}
				SetBorderInward(pw, facet, grid, gridPlanes, i, j, 1);
				if (ValidateFacet(pw, facet)) {
					AddFacetBevels(pw, facet);
					pw.numFacets++;
				}
			}
		}
	}

	// copy the results out
	pf->numPlanes = pw.numPlanes;
	pf->numFacets = pw.numFacets;
	pf->facets = new BSPData::Facet[pw.numFacets];
	memcpy(pf->facets, pw.facets, pw.numFacets * sizeof(*pf->facets));
	pf->planes = new BSPData::PatchPlane[pw.numPlanes];
	memcpy(pf->planes, pw.planes, pw.numPlanes * sizeof(*pf->planes));
}

BSPData::PatchCollide* BSP::GeneratePatchCollide(int32_t width, int32_t height, const Vertice *points, float subdivisions)
{
	PatchCollide	*pf;
	cGrid_t			grid;
	int				i, j;

	if (width <= 2 || height <= 2 || !points) {
		return nullptr;
	}

	if (!(width & 1) || !(height & 1)) {
		return nullptr;
	}

	if (width > MAX_GRID_SIZE || height > MAX_GRID_SIZE) {
		return nullptr;
	}

	patchWork_t pw;
	// build a grid
	grid.width = width;
	grid.height = height;
	grid.wrapWidth = false;
	grid.wrapHeight = false;
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			VectorCopy(points[j*width + i].xyz, grid.points[i][j]);
		}
	}

	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			FindPlane(pw, grid.points[i][j], grid.points[i + 1][j], grid.points[i + 1][j + 1]);
			FindPlane(pw, grid.points[i + 1][j + 1], grid.points[i][j + 1], grid.points[i][j]);
		}
	}

	// subdivide the grid
	SetGridWrapWidth(&grid);
	SubdivideGridColumns(&grid, subdivisions);
	RemoveDegenerateColumns(&grid);

	TransposeGrid(&grid);

	SetGridWrapWidth(&grid);
	SubdivideGridColumns(&grid, subdivisions);
	RemoveDegenerateColumns(&grid);

	// we now have a grid of points exactly on the curve
	// the aproximate surface defined by these points will be
	// collided against
	pf = new PatchCollide;
	ClearBounds(pf->bounds[0], pf->bounds[1]);
	for (i = 0; i < grid.width; i++) {
		for (j = 0; j < grid.height; j++) {
			AddPointToBounds(grid.points[i][j], pf->bounds[0], pf->bounds[1]);
		}
	}

	// generate a bsp tree for the surface
	PatchCollideFromGrid(pw, &grid, pf);

	// expand by one unit for epsilon purposes
	pf->bounds[0][0] -= 1;
	pf->bounds[0][1] -= 1;
	pf->bounds[0][2] -= 1;

	pf->bounds[1][0] += 1;
	pf->bounds[1][1] += 1;
	pf->bounds[1][2] += 1;

	return pf;
}
