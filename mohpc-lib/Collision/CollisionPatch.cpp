#include <MOHPC/Collision/Collision.h>
#include <MOHPC/Managers/ShaderManager.h>

using namespace MOHPC;

#define	MAX_FACETS			1024
#define	MAX_PATCH_PLANES	2048
#define	SURFACE_CLIP_EPSILON (0.125f)

/*
================================================================================

TRACE TESTING

================================================================================
*/

/*
====================
CollisionWorld::CM_TracePointThroughPatchCollide

  special case for point traces because the patch collide "brushes" have no volume
====================
*/
void CollisionWorld::CM_TracePointThroughPatchCollide(traceWork_t* tw, const patchCollide_t* pc) {
	bool	frontFacing[MAX_PATCH_PLANES];
	float		intersection[MAX_PATCH_PLANES];
	float		intersect;
	const patchPlane_t* planes;
	const facet_t* facet;
	int			i, j, k;
	float		offset;
	float		d1, d2;

	if (!tw->isPoint) {
		return;
	}

	// determine the trace's relationship to all planes
	planes = pc->planes;
	for (i = 0; i < pc->numPlanes; i++, planes++) {
		offset = DotProduct(tw->offsets[planes->signbits], planes->plane);
		d1 = DotProduct(tw->start, planes->plane) - planes->plane[3] + offset;
		d2 = DotProduct(tw->end, planes->plane) - planes->plane[3] + offset;
		if (d1 <= 0) {
			frontFacing[i] = false;
		}
		else {
			frontFacing[i] = true;
		}
		if (d1 == d2) {
			intersection[i] = 99999;
		}
		else {
			intersection[i] = d1 / (d1 - d2);
			if (intersection[i] <= 0) {
				intersection[i] = 99999;
			}
		}
	}


	// see if any of the surface planes are intersected
	facet = pc->facets;
	for (i = 0; i < pc->numFacets; i++, facet++) {
		if (!frontFacing[facet->surfacePlane]) {
			continue;
		}
		intersect = intersection[facet->surfacePlane];
		if (intersect < 0) {
			continue;		// surface is behind the starting point
		}
		if (intersect > tw->trace.fraction) {
			continue;		// already hit something closer
		}
		for (j = 0; j < facet->numBorders; j++) {
			k = facet->borderPlanes[j];
			if (frontFacing[k] ^ facet->borderInward[j]) {
				if (intersection[k] > intersect) {
					break;
				}
			}
			else {
				if (intersection[k] < intersect) {
					break;
				}
			}
		}
		if (j == facet->numBorders) {
			// we hit this facet
			planes = &pc->planes[facet->surfacePlane];

			// calculate intersection with a slight pushoff
			offset = DotProduct(tw->offsets[planes->signbits], planes->plane);
			d1 = DotProduct(tw->start, planes->plane) - planes->plane[3] + offset;
			d2 = DotProduct(tw->end, planes->plane) - planes->plane[3] + offset;
			tw->trace.fraction = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);

			if (tw->trace.fraction < 0) {
				tw->trace.fraction = 0;
			}

			VecCopy(planes->plane, tw->trace.plane.normal);
			tw->trace.plane.dist = planes->plane[3];
		}
	}
}

/*
====================
CollisionWorld::CM_CheckFacetPlane
====================
*/
int CollisionWorld::CM_CheckFacetPlane(float* plane, vec3_t start, vec3_t end, float* enterFrac, float* leaveFrac, int* hit) {
	float d1, d2, f;

	*hit = false;

	d1 = DotProduct(start, plane) - plane[3];
	d2 = DotProduct(end, plane) - plane[3];

	// if completely in front of face, no intersection with the entire facet
	if (d1 > 0 && (d2 >= SURFACE_CLIP_EPSILON || d2 >= d1)) {
		return false;
	}

	// if it doesn't cross the plane, the plane isn't relevent
	if (d1 <= 0 && d2 <= 0) {
		return true;
	}

	// crosses face
	if (d1 > d2) {	// enter
		f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
		if (f < 0) {
			f = 0;
		}
		//always favor previous plane hits and thus also the surface plane hit
		if (f > * enterFrac) {
			*enterFrac = f;
			*hit = true;
		}
	}
	else {	// leave
		f = (d1 + SURFACE_CLIP_EPSILON) / (d1 - d2);
		if (f > 1) {
			f = 1;
		}
		if (f < *leaveFrac) {
			*leaveFrac = f;
		}
	}
	return true;
}

/*
====================
CollisionWorld::CM_TraceThroughPatchCollide
====================
*/
void CollisionWorld::CM_TraceThroughPatchCollide(traceWork_t* tw, const patchCollide_t* pc) {
	int i, j, hit, hitnum;
	float enterFrac, leaveFrac;
	patchPlane_t* planes;
	facet_t* facet;
	float plane[4] = { 0, 0, 0, 0 }, bestplane[4] = { 0, 0, 0, 0 };

	if (tw->isPoint) {
		CollisionWorld::CM_TracePointThroughPatchCollide(tw, pc);
		return;
	}

	facet = pc->facets;
	for (i = 0; i < pc->numFacets; i++, facet++) {
		enterFrac = -1.0;
		leaveFrac = 1.0;
		hitnum = -1;
		//
		planes = &pc->planes[facet->surfacePlane];
		VecCopy(planes->plane, plane);
		plane[3] = planes->plane[3];
		plane[3] -= DotProduct(tw->offsets[planes->signbits], plane);

		if (!CollisionWorld::CM_CheckFacetPlane(plane, tw->start, tw->end, &enterFrac, &leaveFrac, &hit)) {
			continue;
		}
		if (hit) {
			Vec4Copy(plane, bestplane);
		}

		for (j = 0; j < facet->numBorders; j++) {
			planes = &pc->planes[facet->borderPlanes[j]];
			if (facet->borderInward[j]) {
				VecNegate(planes->plane, plane);
				plane[3] = -planes->plane[3];
			}
			else {
				VecCopy(planes->plane, plane);
				plane[3] = planes->plane[3];
			}

			// NOTE: this works even though the plane might be flipped because the bbox is centered
			plane[3] += ::fabsf(DotProduct(tw->offsets[planes->signbits], plane));

			if (!CollisionWorld::CM_CheckFacetPlane(plane, tw->start, tw->end, &enterFrac, &leaveFrac, &hit)) {
				break;
			}
			if (hit) {
				hitnum = j;
				Vec4Copy(plane, bestplane);
			}
		}
		if (j < facet->numBorders) continue;
		//never clip against the back side
		if (hitnum == facet->numBorders - 1) continue;

		if (enterFrac < leaveFrac && enterFrac >= 0) {
			if (enterFrac < tw->trace.fraction) {
				if (enterFrac < 0) {
					enterFrac = 0;
				}

				tw->trace.fraction = enterFrac;
				VecCopy(bestplane, tw->trace.plane.normal);
				tw->trace.plane.dist = bestplane[3];
			}
		}
	}
}


/*
=======================================================================

POSITION TEST

=======================================================================
*/

/*
====================
CollisionWorld::CM_PositionTestInPatchCollide
====================
*/
bool CollisionWorld::CM_PositionTestInPatchCollide(traceWork_t* tw, const patchCollide_t* pc) {
	int i, j;
	float offset, t;
	patchPlane_t* planes;
	facet_t* facet;
	float plane[4];
	vec3_t startp;

	if (tw->isPoint) {
		return false;
	}
	//
	facet = pc->facets;
	for (i = 0; i < pc->numFacets; i++, facet++) {
		planes = &pc->planes[facet->surfacePlane];
		VecCopy(planes->plane, plane);
		plane[3] = planes->plane[3];
		if (sphere.use) {
			// adjust the plane distance apropriately for radius
			plane[3] += sphere.radius;

			// find the closest point on the capsule to the plane
			t = DotProduct(plane, sphere.offset);
			if (t > 0) {
				VecSubtract(tw->start, sphere.offset, startp);
			}
			else {
				VecAdd(tw->start, sphere.offset, startp);
			}
		}
		else {
			offset = DotProduct(tw->offsets[planes->signbits], plane);
			plane[3] -= offset;
			VecCopy(tw->start, startp);
		}

		if (DotProduct(plane, startp) - plane[3] > 0.0f) {
			continue;
		}

		for (j = 0; j < facet->numBorders; j++) {
			planes = &pc->planes[facet->borderPlanes[j]];
			if (facet->borderInward[j]) {
				VecNegate(planes->plane, plane);
				plane[3] = -planes->plane[3];
			}
			else {
				VecCopy(planes->plane, plane);
				plane[3] = planes->plane[3];
			}
			if (sphere.use) {
				// adjust the plane distance apropriately for radius
				plane[3] += sphere.radius;

				// find the closest point on the capsule to the plane
				t = DotProduct(plane, sphere.offset);
				if (t > 0.0f) {
					VecSubtract(tw->start, sphere.offset, startp);
				}
				else {
					VecAdd(tw->start, sphere.offset, startp);
				}
			}
			else {
				// NOTE: this works even though the plane might be flipped because the bbox is centered
				offset = DotProduct(tw->offsets[planes->signbits], plane);
				plane[3] += ::fabsf(offset);
				VecCopy(tw->start, startp);
			}

			if (DotProduct(plane, startp) - plane[3] > 0.0f) {
				break;
			}
		}
		if (j < facet->numBorders) {
			continue;
		}
		// inside this patch facet
		return true;
	}
	return false;
}
