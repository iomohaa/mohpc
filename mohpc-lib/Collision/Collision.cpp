#include <MOHPC/Collision/Collision.h>
#include <MOHPC/Managers/ShaderManager.h>

using namespace MOHPC;

collisionModel_t box_model;
collisionPlane_t* box_planes;
collisionBrush_t* box_brush;

static constexpr float SURFACE_CLIP_EPSILON = 0.125f;
static constexpr clipHandle_t BOX_MODEL_HANDLE = 1023;
MOHPC::sphere_t::sphere_t()
	: use(false)
	, radius(0.f)
{
}

MOHPC::trace_t::trace_t()
	: allsolid(0)
	, startsolid(0)
	, fraction(0)
	, endpos()
	, surfaceFlags(0)
	, shaderNum(0)
	, contents(0)
	, entityNum(0)
	, location(0)
{
}

MOHPC::pointtrace_t::pointtrace_t()
	: tw(nullptr)
	, tc(nullptr)
	, i(0)
	, j(0)
	, fSurfaceClipEpsilon(0.f)
{
}

MOHPC::collisionPlane_t::collisionPlane_t()
	: dist(0.f)
	, type(0)
	, signbits(0)
{
}

MOHPC::collisionNode_t::collisionNode_t()
	: plane(nullptr)
	, children{ 0 }
{
}

MOHPC::collisionLeaf_t::collisionLeaf_t()
	: cluster(0)
	, area(0)
	, firstLeafBrush(0)
	, numLeafBrushes(0)
	, firstLeafSurface(0)
	, numLeafSurfaces(0)
	, firstLeafTerrain(0)
	, numLeafTerrains(0)
{
}

MOHPC::collisionFencemask_t::collisionFencemask_t()
	: iWidth(0)
	, iHeight(0)
	, pData(nullptr)
	, pNext(nullptr)
{
}

MOHPC::collisionShader_t::collisionShader_t()
	: surfaceFlags(0)
	, contentFlags(0)
	, mask(nullptr)
{
}

MOHPC::terrainCollideSquare_t::terrainCollideSquare_t()
	: eMode(0)
{
}

MOHPC::terrainCollide_t::terrainCollide_t()
{
}

MOHPC::collisionTerrain_t::collisionTerrain_t()
	: checkcount(0)
	, surfaceFlags(0)
	, contents(0)
	, shaderNum(0)
{
}

MOHPC::collisionArea_t::collisionArea_t()
	: floodnum(0)
	, floodvalid(0)
{
}

MOHPC::collisionModel_t::collisionModel_t()
{
}

MOHPC::collisionSideEq_t::collisionSideEq_t()
	: fSeq{ 0 }
	, fTeq{ 0 }
{
}

MOHPC::collisionBrushSide_t::collisionBrushSide_t()
	: plane(nullptr)
	, surfaceFlags(0)
	, shaderNum(0)
	, pEq(nullptr)
{
}

MOHPC::collisionBrush_t::collisionBrush_t()
	: shaderNum(0)
	, contents(0)
	, numsides(0)
	, sides(nullptr)
	, checkcount(0)
{
}

MOHPC::patchPlane_t::patchPlane_t()
	: plane{ 0 }
	, signbits(0)
{
}

MOHPC::facet_t::facet_t()
	: surfacePlane(0)
	, numBorders(0)
	, borderPlanes{ 0 }
	, borderInward{ false }
	, borderNoAdjust{ false }
{
}

MOHPC::patchCollide_t::patchCollide_t()
	: numPlanes(0)
	, planes(nullptr)
	, numFacets(0)
	, facets(nullptr)
{
}

MOHPC::collisionPatch_t::collisionPatch_t()
	: checkcount(0)
	, surfaceFlags(0)
	, contents(0)
	, shaderNum(0)
	, subdivisions(0)
{
}

MOHPC::traceWork_t::traceWork_t()
	: maxOffset(0.f)
	, height(0.f)
	, radius(0.f)
	, contents(0)
	, isPoint(false)
{
}

MOHPC::leafList_t::leafList_t()
	: count(0)
	, maxcount(0)
	, overflowed(false)
	, list(nullptr)
	, lastLeaf(0)
	, storeLeafs(nullptr)
{
}

static void SetPlaneSignbits(collisionPlane_t* out)
{
	int	bits, j;

	// for fast box on planeside test
	bits = 0;
	for (j = 0; j < 3; j++) {
		if (out->normal[j] < 0) {
			bits |= 1 << j;
		}
	}
	out->signbits = bits;
}

MOHPC_OBJECT_DEFINITION(CollisionWorld);

CollisionWorld::CollisionWorld()
{
	checkcount = 0;
	c_traces = 0;
	c_patch_traces = 0;
	c_brush_traces = 0;
	c_pointcontents = 0;
}

CollisionWorld::~CollisionWorld()
{
}

collisionFencemask_t* CollisionWorld::createFenceMask()
{
	return new(fencemasks) collisionFencemask_t();
}

collisionShader_t* CollisionWorld::createShader()
{
	return new(shaders) collisionShader_t();
}

collisionSideEq_t* CollisionWorld::createSideEquation()
{
	return new(sideequations) collisionSideEq_t();
}

collisionBrushSide_t* CollisionWorld::createBrushSide()
{
	return new(brushsides) collisionBrushSide_t();
}

collisionPlane_t* CollisionWorld::createPlane()
{
	return new(planes) collisionPlane_t();
}

collisionNode_t* CollisionWorld::createNode()
{
	return new(nodes) collisionNode_t();
}

collisionLeaf_t* CollisionWorld::createLeaf()
{
	return new(leafs) collisionLeaf_t();
}

void CollisionWorld::createLeafTerrain(collisionTerrain_t* terrain)
{
	leafterrains.AddObject(terrain);
}

collisionModel_t* CollisionWorld::createModel()
{
	return new(cmodels) collisionModel_t();
}

collisionBrush_t* CollisionWorld::createBrush()
{
	return new(brushes) collisionBrush_t();
}

collisionPatch_t* CollisionWorld::createPatch()
{
	return new(patchList) collisionPatch_t();
}

collisionTerrain_t* CollisionWorld::createTerrain()
{
	return new(terrain) collisionTerrain_t();
}

void MOHPC::CollisionWorld::clearAll()
{
	fencemasks.FreeObjectList();
	shaders.FreeObjectList();
	brushsides.FreeObjectList();
	planes.FreeObjectList();
	nodes.FreeObjectList();
	leafs.FreeObjectList();
	leafbrushes.FreeObjectList();
	leafsurfaces.FreeObjectList();
	leafterrains.FreeObjectList();
	cmodels.FreeObjectList();
	brushes.FreeObjectList();
	surfaces.FreeObjectList();
	terrain.FreeObjectList();
	patchList.FreeObjectList();
}

void MOHPC::CollisionWorld::reserve(
	size_t numFenceMasks,
	size_t numShaders,
	size_t numSideEquations,
	size_t numBrushSides,
	size_t numPlanes,
	size_t numNodes,
	size_t numLeafs,
	size_t numLeafBrushes,
	size_t numLeafSurfaces,
	size_t numLeafTerrains,
	size_t numCModels,
	size_t numBrushes,
	size_t numSurfaces,
	size_t numTerrains,
	size_t numPatchList
)
{
	fencemasks.Resize(numFenceMasks);
	shaders.Resize(numShaders);
	brushsides.Resize(numBrushSides);
	planes.Resize(numPlanes);
	nodes.Resize(numNodes);
	leafs.Resize(numLeafs);
	leafbrushes.Resize(numLeafBrushes);
	leafsurfaces.Resize(numLeafSurfaces);
	leafterrains.Resize(numLeafTerrains);
	cmodels.Resize(numCModels);
	brushes.Resize(numBrushes);
	surfaces.Resize(numSurfaces);
	terrain.Resize(numTerrains);
	patchList.Resize(numPatchList);
}

MOHPC::collisionFencemask_t* MOHPC::CollisionWorld::getFenceMask(uintptr_t num) const
{
	return &fencemasks[num];
}

size_t MOHPC::CollisionWorld::getNumFenceMasks() const
{
	return fencemasks.NumObjects();
}

MOHPC::collisionShader_t* MOHPC::CollisionWorld::getShader(uintptr_t num) const
{
	return &shaders[num];
}

size_t MOHPC::CollisionWorld::getNumShaders() const
{
	return shaders.NumObjects();
}

MOHPC::collisionSideEq_t* MOHPC::CollisionWorld::getSideEquation(uintptr_t num) const
{
	return &sideequations[num];
}

size_t MOHPC::CollisionWorld::getNumSideEquations() const
{
	return sideequations.NumObjects();
}

MOHPC::collisionBrushSide_t* MOHPC::CollisionWorld::getBrushSide(uintptr_t num) const
{
	return &brushsides[num];
}

size_t MOHPC::CollisionWorld::getNumBrushSides() const
{
	return brushsides.NumObjects();
}

MOHPC::collisionPlane_t* MOHPC::CollisionWorld::getPlane(uintptr_t num) const
{
	return &planes[num];
}

size_t MOHPC::CollisionWorld::getNumPlanes() const
{
	return planes.NumObjects();
}

MOHPC::collisionNode_t* MOHPC::CollisionWorld::getNode(uintptr_t num) const
{
	return &nodes[num];
}

size_t MOHPC::CollisionWorld::getNumNodes() const
{
	return nodes.NumObjects();
}

MOHPC::collisionLeaf_t* MOHPC::CollisionWorld::getLeaf(uintptr_t num) const
{
	return &leafs[num];
}

size_t MOHPC::CollisionWorld::getNumLeafs() const
{
	return leafs.NumObjects();
}

void CollisionWorld::createLeafBrush(uintptr_t num)
{
	leafbrushes.AddObject(num);
}

void CollisionWorld::createLeafSurface(uintptr_t num)
{
	leafsurfaces.AddObject(num);
}

MOHPC::collisionModel_t* MOHPC::CollisionWorld::getModel(uintptr_t num) const
{
	return &cmodels[num];
}

size_t MOHPC::CollisionWorld::getNumModels() const
{
	return cmodels.NumObjects();
}

MOHPC::collisionBrush_t* MOHPC::CollisionWorld::getBrush(uintptr_t num) const
{
	return &brushes[num];
}

size_t MOHPC::CollisionWorld::getNumBrushes() const
{
	return brushes.NumObjects();
}

MOHPC::collisionPatch_t* MOHPC::CollisionWorld::getPatch(uintptr_t num) const
{
	return &patchList[num];
}

size_t MOHPC::CollisionWorld::getNumPatches() const
{
	return patchList.NumObjects();
}

MOHPC::collisionTerrain_t* MOHPC::CollisionWorld::getTerrain(uintptr_t num) const
{
	return &terrain[num];
}


size_t MOHPC::CollisionWorld::getNumTerrains() const
{
	return terrain.NumObjects();
}

size_t MOHPC::CollisionWorld::getNumLeafTerrains() const
{
	return leafterrains.NumObjects();
}

collisionTerrain_t* MOHPC::CollisionWorld::getLeafTerrain(uintptr_t num) const
{
	return leafterrains[num];
}

size_t MOHPC::CollisionWorld::getNumLeafBrushes() const
{
	return leafbrushes.NumObjects();
}

collisionBrush_t* MOHPC::CollisionWorld::getLeafBrush(uintptr_t num) const
{
	return &brushes[leafbrushes[num]];
}

size_t MOHPC::CollisionWorld::getNumLeafSurfaces() const
{
	return leafsurfaces.NumObjects();
}

collisionPatch_t* MOHPC::CollisionWorld::getLeafSurface(uintptr_t num) const
{
	return surfaces[leafsurfaces[num]];
}

void MOHPC::CollisionWorld::createSurface(collisionPatch_t* patch)
{
	surfaces.AddObject(patch);
}

clipHandle_t MOHPC::CollisionWorld::inlineModel(uint32_t index)
{
	if (index >= cmodels.NumObjects()) {
		return 0;
	}

	return index;
}

/*
===============================================================================

BASIC MATH

===============================================================================
*/

constexpr float Square(float value)
{
	return value * value;
}

/*
================
RotatePoint
================
*/
void RotatePoint(vec3_t point, /*const*/ vec3_t matrix[3]) { // FIXME
	vec3_t tvec;

	VecCopy(point, tvec);
	point[0] = DotProduct(matrix[0], tvec);
	point[1] = DotProduct(matrix[1], tvec);
	point[2] = DotProduct(matrix[2], tvec);
}

/*
================
CreateRotationMatrix
================
*/
void CreateRotationMatrix(const vec3_t angles, vec3_t matrix[3]) {
	AngleVectorsLeft(angles, matrix[0], matrix[1], matrix[2]);
	VectorInverse(matrix[1]);
}

/*
================
CollisionWorld::CM_ProjectPointOntoVector
================
*/
void CollisionWorld::CM_ProjectPointOntoVector(vec3_t point, vec3_t vStart, vec3_t vDir, vec3_t vProj)
{
	vec3_t pVec;

	VecSubtract(point, vStart, pVec);
	// project onto the directional vector for this segment
	VectorMA(vStart, DotProduct(pVec, vDir), vDir, vProj);
}

/*
================
CollisionWorld::CM_DistanceFromLineSquared
================
*/
float CollisionWorld::CM_DistanceFromLineSquared(vec3_t p, vec3_t lp1, vec3_t lp2, vec3_t dir) {
	vec3_t proj, t;
	int j;

	CollisionWorld::CM_ProjectPointOntoVector(p, lp1, dir, proj);
	for (j = 0; j < 3; j++)
		if ((proj[j] > lp1[j] && proj[j] > lp2[j]) ||
			(proj[j] < lp1[j] && proj[j] < lp2[j]))
			break;
	if (j < 3) {
		if (::fabs(proj[j] - lp1[j]) < ::fabs(proj[j] - lp2[j]))
			VecSubtract(p, lp1, t);
		else
			VecSubtract(p, lp2, t);
		return VectorLengthSquared(t);
	}
	VecSubtract(p, proj, t);
	return VectorLengthSquared(t);
}

/*
================
CollisionWorld::CM_VectorDistanceSquared
================
*/
float CollisionWorld::CM_VectorDistanceSquared(vec3_t p1, vec3_t p2) {
	vec3_t dir;

	VecSubtract(p2, p1, dir);
	return VectorLengthSquared(dir);
}

int BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, const collisionPlane_t* p)
{
	float	dist1, dist2;
	int		sides;

	// fast axial cases
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;
		if (p->dist >= emaxs[p->type])
			return 2;
		return 3;
	}

	// general case
	switch (p->signbits)
	{
	case 0:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		break;
	case 1:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		break;
	case 2:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		break;
	case 3:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		break;
	case 4:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		break;
	}

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

	return sides;
}

/*
================
SquareRootFloat
================
*/
float SquareRootFloat(float number) {
	union {
		float f;
		int i;
	} t;
	float x, y;
	const float f = 1.5F;

	x = number * 0.5F;
	t.f = number;
	t.i = 0x5f3759df - (t.i >> 1);
	y = t.f;
	y = y * (f - (x * y * y));
	y = y * (f - (x * y * y));
	return number * y;
}

collisionModel_t* CollisionWorld::CM_ClipHandleToModel(clipHandle_t handle)
{
	if (handle < 0)
	{
		// FIXME: throw?
		return nullptr;
	}

	if (handle < cmodels.NumObjects()) {
		return &cmodels[handle];
	}
	if (handle == BOX_MODEL_HANDLE) {
		return &box_model;
	}

	// FIXME: throw?
	return nullptr;

}

void CollisionWorld::CM_ModelBounds(clipHandle_t model, Vector& mins, Vector& maxs)
{
	collisionModel_t* cmod;

	cmod = CM_ClipHandleToModel(model);
	VecCopy(cmod->mins, mins);
	VecCopy(cmod->maxs, maxs);
}

void CollisionWorld::CM_InitBoxHull()
{
	int			i;
	int			side;
	collisionPlane_t* p;
	collisionBrushSide_t* s;

	planes.SetNumObjectsUninitialized(12);
	box_planes = &planes[0];

	// Create 6 sides
	brushsides.SetNumObjectsUninitialized(6);

	box_brush = createBrush();
	box_brush->numsides = 6;
	box_brush->sides = &this->brushsides[0];
	box_brush->contents = ContentFlags::CONTENTS_BBOX;

	box_model.leaf.numLeafBrushes = 1;
	box_model.leaf.firstLeafBrush = (uint32_t)this->leafbrushes.NumObjects();
	createLeafBrush(box_brush - brushes.Data());

	for (i = 0; i < 6; i++)
	{
		side = i & 1;

		// brush sides
		s = &brushsides[i];
		s->plane = &box_planes[i * 2 + side]; // &this->planes[(this->planes.NumObjects() + i * 2 + side)];
		s->surfaceFlags = 0;

		// planes
		p = &box_planes[i * 2];
		p->type = i >> 1;
		p->signbits = 0;
		VectorClear(p->normal);
		p->normal[i >> 1] = 1;

		p = &box_planes[i * 2 + 1];
		p->type = 3 + (i >> 1);
		p->signbits = 0;
		VectorClear(p->normal);
		p->normal[i >> 1] = -1;

		SetPlaneSignbits(p);
	}
}

clipHandle_t CollisionWorld::CM_TempBoxModel(const vec3_t mins, const vec3_t maxs, int contents)
{
	box_planes[0].dist = maxs[0];
	box_planes[1].dist = -maxs[0];
	box_planes[2].dist = mins[0];
	box_planes[3].dist = -mins[0];
	box_planes[4].dist = maxs[1];
	box_planes[5].dist = -maxs[1];
	box_planes[6].dist = mins[1];
	box_planes[7].dist = -mins[1];
	box_planes[8].dist = maxs[2];
	box_planes[9].dist = -maxs[2];
	box_planes[10].dist = mins[2];
	box_planes[11].dist = -mins[2];

	VecCopy(mins, box_brush->bounds[0]);
	VecCopy(maxs, box_brush->bounds[1]);
	box_brush->contents = contents;

	return BOX_MODEL_HANDLE;
}

/*
===============================================================================

POSITION TESTING

===============================================================================
*/

/*
================
CollisionWorld::CM_TestBoxInBrush
================
*/
void CollisionWorld::CM_TestBoxInBrush(traceWork_t* tw, collisionBrush_t* brush) {
	int			i;
	const collisionPlane_t* plane;
	float		dist;
	float		d1;
	const collisionBrushSide_t* side;
	float		t;

	if (!brush->numsides) {
		return;
	}

	// special test for axial
	if (tw->bounds[0][0] > brush->bounds[1][0]
		|| tw->bounds[0][1] > brush->bounds[1][1]
		|| tw->bounds[0][2] > brush->bounds[1][2]
		|| tw->bounds[1][0] < brush->bounds[0][0]
		|| tw->bounds[1][1] < brush->bounds[0][1]
		|| tw->bounds[1][2] < brush->bounds[0][2]
		) {
		return;
	}

	if (sphere.use) {
		// the first six planes are the axial planes, so we only
		// need to test the remainder
		for (i = 6; i < brush->numsides; i++) {
			side = brush->sides + i;
			plane = side->plane;

			// find the closest point on the capsule to the plane
			t = DotProduct(plane->normal, sphere.offset);

			// adjust the plane distance apropriately for radius
			dist = t + plane->dist + sphere.radius;

			d1 = DotProduct(tw->start, plane->normal) - dist;
			// if completely in front of face, no intersection
			if (d1 > 0) {
				return;
			}
		}
	}
	else {
		// the first six planes are the axial planes, so we only
		// need to test the remainder
		for (i = 6; i < brush->numsides; i++) {
			side = brush->sides + i;
			plane = side->plane;

			// adjust the plane distance apropriately for mins/maxs
			dist = plane->dist - DotProduct(tw->offsets[plane->signbits], plane->normal);

			d1 = DotProduct(tw->start, plane->normal) - dist;

			// if completely in front of face, no intersection
			if (d1 > 0) {
				return;
			}
		}
	}

	// inside this brush
	tw->trace.startsolid = tw->trace.allsolid = true;
	tw->trace.fraction = 0;
	tw->trace.contents = brush->contents;
}



/*
================
CollisionWorld::CM_TestInLeaf
================
*/
void CollisionWorld::CM_TestInLeaf(traceWork_t* tw, collisionLeaf_t* leaf) {
	uintptr_t brushnum;
	collisionBrush_t* b;
	collisionPatch_t* patch;
	collisionTerrain_t* terrain;

	// test box position against all brushes in the leaf
	for (uintptr_t k = 0; k < leaf->numLeafBrushes; k++) {
		brushnum = this->leafbrushes[leaf->firstLeafBrush + k];
		b = &this->brushes[brushnum];
		if (b->checkcount == this->checkcount) {
			continue;	// already checked this brush in another leaf
		}
		b->checkcount = this->checkcount;

		if (!(b->contents & tw->contents)) {
			continue;
		}

		CollisionWorld::CM_TestBoxInBrush(tw, b);
		if (tw->trace.allsolid) {
			return;
		}
	}

	// test against all patches
	for (uintptr_t k = 0; k < leaf->numLeafSurfaces; k++) {
		patch = this->surfaces[this->leafsurfaces[leaf->firstLeafSurface + k]];
		if (!patch) {
			continue;
		}
		if (patch->checkcount == this->checkcount) {
			continue;	// already checked this brush in another leaf
		}
		patch->checkcount = this->checkcount;

		if (!(patch->contents & tw->contents)) {
			continue;
		}

		if (CollisionWorld::CM_PositionTestInPatchCollide(tw, &patch->pc)) {
			tw->trace.fraction = 0;
			tw->trace.startsolid = tw->trace.allsolid = true;
			return;
		}
	}

	for (uintptr_t k = 0; k < leaf->numLeafTerrains; k++) {
		terrain = this->leafterrains[leaf->firstLeafTerrain + k];
		if (!terrain) {
			continue;
		}
		if (terrain->checkcount == this->checkcount) {
			continue;
		}
		terrain->checkcount = this->checkcount;

		if (CollisionWorld::CM_PositionTestInTerrainCollide(tw, &terrain->tc)) {
			tw->trace.fraction = 0;
			tw->trace.startsolid = tw->trace.allsolid = true;
			return;
		}
	}
	}

/*
==================
CollisionWorld::CM_TestCapsuleInCapsule

capsule inside capsule check
==================
*/
void CollisionWorld::CM_TestCapsuleInCapsule(traceWork_t * tw, clipHandle_t model) {
	int i;
	Vector mins, maxs;
	vec3_t top, bottom;
	vec3_t p1, p2, tmp;
	vec3_t offset, symetricSize[2];
	float radius, halfwidth, halfheight, offs, r;

	CollisionWorld::CM_ModelBounds(model, mins, maxs);

	VecAdd(tw->start, sphere.offset, top);
	VecSubtract(tw->start, sphere.offset, bottom);
	for (i = 0; i < 3; i++) {
		offset[i] = (mins[i] + maxs[i]) * 0.5f;
		symetricSize[0][i] = mins[i] - offset[i];
		symetricSize[1][i] = maxs[i] - offset[i];
	}
	halfwidth = symetricSize[1][0];
	halfheight = symetricSize[1][2];
	radius = (halfwidth > halfheight) ? halfheight : halfwidth;
	offs = halfheight - radius;

	r = Square(sphere.radius + radius);
	// check if any of the spheres overlap
	VecCopy(offset, p1);
	p1[2] += offs;
	VecSubtract(p1, top, tmp);
	if (VectorLengthSquared(tmp) < r) {
		tw->trace.startsolid = tw->trace.allsolid = true;
		tw->trace.fraction = 0;
	}
	VecSubtract(p1, bottom, tmp);
	if (VectorLengthSquared(tmp) < r) {
		tw->trace.startsolid = tw->trace.allsolid = true;
		tw->trace.fraction = 0;
	}
	VecCopy(offset, p2);
	p2[2] -= offs;
	VecSubtract(p2, top, tmp);
	if (VectorLengthSquared(tmp) < r) {
		tw->trace.startsolid = tw->trace.allsolid = true;
		tw->trace.fraction = 0;
	}
	VecSubtract(p2, bottom, tmp);
	if (VectorLengthSquared(tmp) < r) {
		tw->trace.startsolid = tw->trace.allsolid = true;
		tw->trace.fraction = 0;
	}
	// if between cylinder up and lower bounds
	if ((top[2] >= p1[2] && top[2] <= p2[2]) ||
		(bottom[2] >= p1[2] && bottom[2] <= p2[2])) {
		// 2d coordinates
		top[2] = p1[2] = 0;
		// if the cylinders overlap
		VecSubtract(top, p1, tmp);
		if (VectorLengthSquared(tmp) < r) {
			tw->trace.startsolid = tw->trace.allsolid = true;
			tw->trace.fraction = 0;
		}
	}
}

/*
==================
CollisionWorld::CM_TestBoundingBoxInCapsule

bounding box inside capsule check
==================
*/
void CollisionWorld::CM_TestBoundingBoxInCapsule(traceWork_t * tw, clipHandle_t model) {
	Vector mins, maxs;
	vec3_t offset, size[2];
	clipHandle_t h;
	collisionModel_t* cmod;
	int i;

	// mins maxs of the capsule
	CollisionWorld::CM_ModelBounds(model, mins, maxs);

	// offset for capsule center
	for (i = 0; i < 3; i++) {
		offset[i] = (mins[i] + maxs[i]) * 0.5f;
		size[0][i] = mins[i] - offset[i];
		size[1][i] = maxs[i] - offset[i];
		tw->start[i] -= offset[i];
		tw->end[i] -= offset[i];
	}

	// replace the bounding box with the capsule
	sphere.use = true;
	sphere.radius = (size[1][0] > size[1][2]) ? size[1][2] : size[1][0];
	VecSet(sphere.offset, 0, 0, size[1][2] - sphere.radius);

	// replace the capsule with the bounding box
	h = CollisionWorld::CM_TempBoxModel(tw->size[0], tw->size[1], false);
	// calculate collision
	cmod = CollisionWorld::CM_ClipHandleToModel(h);
	CollisionWorld::CM_TestInLeaf(tw, &cmod->leaf);
}

/*
==================
CollisionWorld::CM_PositionTest
==================
*/
#define	MAX_POSITION_LEAFS	1024
void CollisionWorld::CM_PositionTest(traceWork_t * tw) {
	int		leafs[MAX_POSITION_LEAFS];
	int		i;
	leafList_t	ll;

	// identify the leafs we are touching
	VecAdd(tw->start, tw->size[0], ll.bounds[0]);
	VecAdd(tw->start, tw->size[1], ll.bounds[1]);

	for (i = 0; i < 3; i++) {
		ll.bounds[0][i] -= 1;
		ll.bounds[1][i] += 1;
	}

	ll.count = 0;
	ll.maxcount = MAX_POSITION_LEAFS;
	ll.list = leafs;
	ll.storeLeafs = &CollisionWorld::CM_StoreLeafs;
	ll.lastLeaf = 0;
	ll.overflowed = false;

	this->checkcount++;

	CollisionWorld::CM_BoxLeafnums_r(&ll, 0);

	this->checkcount++;

	// test the contents of the leafs
	for (i = 0; i < ll.count; i++) {
		CollisionWorld::CM_TestInLeaf(tw, &this->leafs[leafs[i]]);
		if (tw->trace.allsolid) {
			break;
		}
	}
}

/*
===============================================================================

TRACING

===============================================================================
*/


/*
================
CollisionWorld::CM_TraceThroughPatch
================
*/

void CollisionWorld::CM_TraceThroughPatch(traceWork_t * tw, collisionPatch_t * patch) {
	float		oldFrac;

	c_patch_traces++;

	oldFrac = tw->trace.fraction;

	CollisionWorld::CM_TraceThroughPatchCollide(tw, &patch->pc);

	if (tw->trace.fraction < oldFrac) {
		tw->trace.surfaceFlags = patch->surfaceFlags;
		tw->trace.shaderNum = patch->shaderNum;
		tw->trace.contents = patch->contents;
	}
}

/*
================
CollisionWorld::CM_TraceThroughTerrain
================
*/
void CollisionWorld::CM_TraceThroughTerrain(traceWork_t * tw, collisionTerrain_t * terrain) {
	float		oldFrac;

	oldFrac = tw->trace.fraction;

	// test against all terrain patches
	CollisionWorld::CM_TraceThroughTerrainCollide(tw, &terrain->tc);

	if (tw->trace.fraction < oldFrac) {
		tw->trace.surfaceFlags = terrain->surfaceFlags;
		tw->trace.shaderNum = terrain->shaderNum;
		tw->trace.contents = terrain->contents;
	}
}

/*
================
CollisionWorld::CM_TraceThroughBrush
================
*/
void CollisionWorld::CM_TraceThroughBrush(traceWork_t * tw, collisionBrush_t * brush) {
	int			i;
	const collisionPlane_t* plane, * clipplane, * clipplane2;
	float		dist;
	float		enterFrac, leaveFrac, leaveFrac2;
	float		d1, d2;
	bool	getout, startout;
	float		f;
	const collisionBrushSide_t* side, * leadside, * leadside2;
	float		t;

	if (!brush->numsides) {
		return;
	}

	enterFrac = -1.0;
	leaveFrac = 1.0;
	clipplane = NULL;

	c_brush_traces++;

	getout = false;
	startout = false;

	leadside = NULL;
	if (!(brush->contents & CONTENTS_FENCE) || !tw->isPoint) {
		if (sphere.use) {
			//
			// compare the trace against all planes of the brush
			// find the latest time the trace crosses a plane towards the interior
			// and the earliest time the trace crosses a plane towards the exterior
			//
			for (i = 0; i < brush->numsides; i++) {
				side = brush->sides + i;
				plane = side->plane;

				// find the closest point on the capsule to the plane
				t = DotProduct(plane->normal, sphere.offset);
				if (t < 0)
				{
					t = -t;
				}

				// adjust the plane distance apropriately for radius
				dist = t + plane->dist + sphere.radius;

				d1 = DotProduct(tw->start, plane->normal) - dist;
				d2 = DotProduct(tw->end, plane->normal) - dist;

				// if it doesn't cross the plane, the plane isn't relevent
				if (d1 <= 0 && d2 <= 0) {
					continue;
				}

				if (d2 > 0) {
					getout = true;	// endpoint is not in solid
				}
				if (d1 > 0) {
					startout = true;
				}

				// if completely in front of face, no intersection with the entire brush
				if (d1 > 0 && (d2 >= SURFACE_CLIP_EPSILON || d2 >= d1)) {
					return;
				}

				// crosses face
				if (d1 > d2) {	// enter
					f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
					if (f < 0) {
						f = 0;
					}
					if (f > enterFrac) {
						enterFrac = f;
						clipplane = plane;
						leadside = side;
					}
				}
				else {	// leave
					f = (d1 + SURFACE_CLIP_EPSILON) / (d1 - d2);
					if (f > 1) {
						f = 1;
					}
					if (f < leaveFrac) {
						leaveFrac = f;
					}
				}
			}
		}
		else {
			//
			// compare the trace against all planes of the brush
			// find the latest time the trace crosses a plane towards the interior
			// and the earliest time the trace crosses a plane towards the exterior
			//
			for (i = 0; i < brush->numsides; i++) {
				side = brush->sides + i;
				plane = side->plane;

				// adjust the plane distance apropriately for mins/maxs
				dist = plane->dist - DotProduct(tw->offsets[plane->signbits], plane->normal);

				d1 = DotProduct(tw->start, plane->normal) - dist;
				d2 = DotProduct(tw->end, plane->normal) - dist;

				// if it doesn't cross the plane, the plane isn't relevent
				if (d1 <= 0 && d2 <= 0) {
					continue;
				}

				if (d2 > 0) {
					getout = true;	// endpoint is not in solid
				}
				if (d1 > 0) {
					startout = true;

					// if completely in front of face, no intersection with the entire brush
					if (d2 >= SURFACE_CLIP_EPSILON || d2 >= d1) {
						return;
					}
				}

				// crosses face
				if (d1 > d2) { // enter
					f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
					if (f < 0) {
						f = 0;
					}
					if (f > enterFrac) {
						enterFrac = f;
						clipplane = plane;
						leadside = side;
					}
				}
				else { // leave
					f = (d1 + SURFACE_CLIP_EPSILON) / (d1 - d2);
					if (f > 1) {
						f = 1;
					}
					if (f < leaveFrac) {
						leaveFrac = f;
					}
				}
			}
		}

		//
		// all planes have been checked, and the trace was not
		// completely outside the brush
		//
		if (!startout) {	// original point was inside brush
			tw->trace.startsolid = true;
			if (!getout) {
				tw->trace.fraction = 0;
				tw->trace.allsolid = true;
			}
			return;
		}

		if (enterFrac <= leaveFrac) {
			if (enterFrac > -1 && enterFrac < tw->trace.fraction) {
				if (enterFrac < 0) {
					enterFrac = 0;
				}
				tw->trace.fraction = enterFrac;
				tw->trace.plane = *clipplane;
				tw->trace.surfaceFlags = leadside->surfaceFlags;
				tw->trace.shaderNum = leadside->shaderNum;
				tw->trace.contents = brush->contents;
			}
		}
	}
	else {
		leaveFrac2 = 1.0;
		clipplane2 = NULL;
		leadside2 = NULL;
		//
		// compare the trace against all planes of the brush
		// find the latest time the trace crosses a plane towards the interior
		// and the earliest time the trace crosses a plane towards the exterior
		//
		for (i = 0; i < brush->numsides; i++) {
			side = brush->sides + i;
			plane = side->plane;

			// adjust the plane distance apropriately for mins/maxs
			dist = plane->dist - DotProduct(tw->offsets[plane->signbits], plane->normal);

			d1 = DotProduct(tw->start, plane->normal) - dist;
			d2 = DotProduct(tw->end, plane->normal) - dist;

			// if it doesn't cross the plane, the plane isn't relevent
			if (d1 <= 0 && d2 <= 0) {
				continue;
			}

			if (d1 > 0) {
				// if completely in front of face, no intersection with the entire brush
				if (d2 >= SURFACE_CLIP_EPSILON || d2 >= d1) {
					return;
				}
			}

			// crosses face
			if (d1 > d2) {	// enter
				f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
				if (f < 0) {
					f = 0;
				}
				if (f > enterFrac) {
					enterFrac = f;
					clipplane = plane;
					leadside = side;
				}
			}
			else {	// leave
				f = (d1 + SURFACE_CLIP_EPSILON) / (d1 - d2);
				if (f > 1) {
					f = 1;
				}
				if (f < leaveFrac) {
					leaveFrac = f;
					clipplane2 = plane;
					leadside2 = side;
					leaveFrac2 = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
				}
			}
		}

		//
		// all planes have been checked, and the trace was not
		// completely outside the brush
		//
		if (enterFrac <= leaveFrac) {
			if (enterFrac > -1 && enterFrac < tw->trace.fraction) {
				if (enterFrac < 0) {
					enterFrac = 0;
				}

				if (CollisionWorld::CM_TraceThroughFence(tw, brush, leadside, enterFrac)) {
					tw->trace.fraction = enterFrac;
					tw->trace.plane = *clipplane;
					tw->trace.surfaceFlags = leadside->surfaceFlags;
					tw->trace.shaderNum = leadside->shaderNum;
					tw->trace.contents = brush->contents;
					return;
				}
			}
		}

		if ((leaveFrac2 < 1.0) && (leadside2->surfaceFlags & SURF_BACKSIDE)) {
			if (leaveFrac2 < tw->trace.fraction) {
				if (CollisionWorld::CM_TraceThroughFence(tw, brush, leadside2, leaveFrac)) {
					tw->trace.fraction = leaveFrac2;
					tw->trace.plane = *clipplane2;
					tw->trace.surfaceFlags = leadside2->surfaceFlags;
					tw->trace.shaderNum = leadside2->shaderNum;
					tw->trace.contents = brush->contents;
					return;
				}
			}
		}
	}
}

/*
================
CollisionWorld::CM_TraceToLeaf
================
*/
void CollisionWorld::CM_TraceToLeaf(traceWork_t * tw, collisionLeaf_t * leaf)
{
	collisionBrush_t* b;
	collisionPatch_t* patch;
	collisionTerrain_t* terrain;

	if(leaf->numLeafBrushes != -1)
	{
		// test box position against all brushes in the leaf
		for (uintptr_t k = 0; k < leaf->numLeafBrushes; k++) {
			b = &this->brushes[this->leafbrushes[leaf->firstLeafBrush + k]];
			if (b->checkcount == this->checkcount) {
				continue;	// already checked this brush in another leaf
			}
			b->checkcount = this->checkcount;

			if (!(b->contents & tw->contents)) {
				continue;
			}

			CollisionWorld::CM_TraceThroughBrush(tw, b);
			if (!tw->trace.fraction) {
				return;
			}
		}
	}

	if (leaf->numLeafSurfaces != -1)
	{
		// test against all patches
		for (uintptr_t k = 0; k < leaf->numLeafSurfaces; k++) {
			patch = this->surfaces[this->leafsurfaces[leaf->firstLeafSurface + k]];
			if (!patch) {
				continue;
			}
			if (patch->checkcount == this->checkcount) {
				continue;	// already checked this brush in another leaf
			}
			patch->checkcount = this->checkcount;

			if (!(patch->contents & tw->contents)) {
				continue;
			}

			CollisionWorld::CM_TraceThroughPatch(tw, patch);
			if (!tw->trace.fraction) {
				return;
			}
		}
	}

	if (leaf->numLeafTerrains != -1)
	{
		// test against all terrains
		for (uintptr_t k = 0; k < leaf->numLeafTerrains; k++) {
			terrain = this->leafterrains[leaf->firstLeafTerrain + k];
			if (!terrain) {
				continue;
			}
			if (terrain->checkcount == this->checkcount) {
				continue;
			}
			terrain->checkcount = this->checkcount;

			CollisionWorld::CM_TraceThroughTerrain(tw, terrain);
			if (!tw->trace.fraction) {
				return;
			}
		}
	}
}

#define RADIUS_EPSILON		1.0f

/*
================
CollisionWorld::CM_TraceThroughSphere

get the first intersection of the ray with the sphere
================
*/
void CollisionWorld::CM_TraceThroughSphere(traceWork_t * tw, vec3_t origin, float radius, vec3_t start, vec3_t end) {
	float l1, l2, length, scale, fraction;
	float a, b, c, d, sqrtd;
	vec3_t v1, dir, intersection;

	// if inside the sphere
	VecSubtract(start, origin, dir);
	l1 = VectorLengthSquared(dir);
	if (l1 < Square(radius)) {
		tw->trace.fraction = 0;
		tw->trace.startsolid = true;
		// test for allsolid
		VecSubtract(end, origin, dir);
		l1 = VectorLengthSquared(dir);
		if (l1 < Square(radius)) {
			tw->trace.allsolid = true;
		}
		return;
	}
	//
	VecSubtract(end, start, dir);
	length = VectorNormalize(dir);
	//
	l1 = CollisionWorld::CM_DistanceFromLineSquared(origin, start, end, dir);
	VecSubtract(end, origin, v1);
	l2 = VectorLengthSquared(v1);
	// if no intersection with the sphere and the end point is at least an epsilon away
	if (l1 >= Square(radius) && l2 > Square(radius + SURFACE_CLIP_EPSILON)) {
		return;
	}
	//
	//	| origin - (start + t * dir) | = radius
	//	a = dir[0]^2 + dir[1]^2 + dir[2]^2;
	//	b = 2 * (dir[0] * (start[0] - origin[0]) + dir[1] * (start[1] - origin[1]) + dir[2] * (start[2] - origin[2]));
	//	c = (start[0] - origin[0])^2 + (start[1] - origin[1])^2 + (start[2] - origin[2])^2 - radius^2;
	//
	VecSubtract(start, origin, v1);
	// dir is normalized so a = 1
	a = 1.0f;//dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2];
	b = 2.0f * (dir[0] * v1[0] + dir[1] * v1[1] + dir[2] * v1[2]);
	c = v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2] - (radius + RADIUS_EPSILON) * (radius + RADIUS_EPSILON);

	d = b * b - 4.0f * c;// * a;
	if (d > 0) {
		sqrtd = SquareRootFloat(d);
		// = (- b + sqrtd) * 0.5f; // / (2.0f * a);
		fraction = (-b - sqrtd) * 0.5f; // / (2.0f * a);
		//
		if (fraction < 0) {
			fraction = 0;
		}
		else {
			fraction /= length;
		}
		if (fraction < tw->trace.fraction) {
			tw->trace.fraction = fraction;
			VecSubtract(end, start, dir);
			VectorMA(start, fraction, dir, intersection);
			VecSubtract(intersection, origin, dir);
#ifdef CAPSULE_DEBUG
			l2 = VectorLength(dir);
			if (l2 < radius) {
				int bah = 1;
			}
#endif
			scale = 1 / (radius + RADIUS_EPSILON);
			VectorScale(dir, scale, dir);
			VecCopy(dir, tw->trace.plane.normal);
			tw->trace.plane.dist = DotProduct(tw->trace.plane.normal, intersection);
			tw->trace.contents = CONTENTS_BODY;
		}
	}
	else if (d == 0) {
		//t1 = (- b ) / 2;
		// slide along the sphere
	}
	// no intersection at all
}

/*
================
CollisionWorld::CM_TraceThroughVerticalCylinder

get the first intersection of the ray with the cylinder
the cylinder extends halfheight above and below the origin
================
*/
void CollisionWorld::CM_TraceThroughVerticalCylinder(traceWork_t * tw, vec3_t origin, float radius, float halfheight, vec3_t start, vec3_t end) {
	float length, scale, fraction, l1, l2;
	float a, b, c, d, sqrtd;
	vec3_t v1, dir, start2d, end2d, org2d, intersection;

	// 2d coordinates
	VecSet(start2d, start[0], start[1], 0);
	VecSet(end2d, end[0], end[1], 0);
	VecSet(org2d, origin[0], origin[1], 0);
	// if between lower and upper cylinder bounds
	if (start[2] <= origin[2] + halfheight &&
		start[2] >= origin[2] - halfheight) {
		// if inside the cylinder
		VecSubtract(start2d, org2d, dir);
		l1 = VectorLengthSquared(dir);
		if (l1 < Square(radius)) {
			tw->trace.fraction = 0;
			tw->trace.startsolid = true;
			VecSubtract(end2d, org2d, dir);
			l1 = VectorLengthSquared(dir);
			if (l1 < Square(radius)) {
				tw->trace.allsolid = true;
			}
			return;
		}
	}
	//
	VecSubtract(end2d, start2d, dir);
	length = VectorNormalize(dir);
	//
	l1 = CollisionWorld::CM_DistanceFromLineSquared(org2d, start2d, end2d, dir);
	VecSubtract(end2d, org2d, v1);
	l2 = VectorLengthSquared(v1);
	// if no intersection with the cylinder and the end point is at least an epsilon away
	if (l1 >= Square(radius) && l2 > Square(radius + SURFACE_CLIP_EPSILON)) {
		return;
	}
	//
	//
	// (start[0] - origin[0] - t * dir[0]) ^ 2 + (start[1] - origin[1] - t * dir[1]) ^ 2 = radius ^ 2
	// (v1[0] + t * dir[0]) ^ 2 + (v1[1] + t * dir[1]) ^ 2 = radius ^ 2;
	// v1[0] ^ 2 + 2 * v1[0] * t * dir[0] + (t * dir[0]) ^ 2 +
	//						v1[1] ^ 2 + 2 * v1[1] * t * dir[1] + (t * dir[1]) ^ 2 = radius ^ 2
	// t ^ 2 * (dir[0] ^ 2 + dir[1] ^ 2) + t * (2 * v1[0] * dir[0] + 2 * v1[1] * dir[1]) +
	//						v1[0] ^ 2 + v1[1] ^ 2 - radius ^ 2 = 0
	//
	VecSubtract(start, origin, v1);
	// dir is normalized so we can use a = 1
	a = 1.0f;// * (dir[0] * dir[0] + dir[1] * dir[1]);
	b = 2.0f * (v1[0] * dir[0] + v1[1] * dir[1]);
	c = v1[0] * v1[0] + v1[1] * v1[1] - (radius + RADIUS_EPSILON) * (radius + RADIUS_EPSILON);

	d = b * b - 4.0f * c;// * a;
	if (d > 0) {
		sqrtd = SquareRootFloat(d);
		// = (- b + sqrtd) * 0.5f;// / (2.0f * a);
		fraction = (-b - sqrtd) * 0.5f;// / (2.0f * a);
		//
		if (fraction < 0) {
			fraction = 0;
		}
		else {
			fraction /= length;
		}
		if (fraction < tw->trace.fraction) {
			VecSubtract(end, start, dir);
			VectorMA(start, fraction, dir, intersection);
			// if the intersection is between the cylinder lower and upper bound
			if (intersection[2] <= origin[2] + halfheight &&
				intersection[2] >= origin[2] - halfheight) {
				//
				tw->trace.fraction = fraction;
				VecSubtract(intersection, origin, dir);
				dir[2] = 0;
#ifdef CAPSULE_DEBUG
				l2 = VectorLength(dir);
				if (l2 <= radius) {
					int bah = 1;
				}
#endif
				scale = 1 / (radius + RADIUS_EPSILON);
				VectorScale(dir, scale, dir);
				VecCopy(dir, tw->trace.plane.normal);
				tw->trace.plane.dist = DotProduct(tw->trace.plane.normal, intersection);
				tw->trace.contents = CONTENTS_BODY;
			}
		}
	}
	else if (d == 0) {
		//t[0] = (- b ) / 2 * a;
		// slide along the cylinder
	}
	// no intersection at all
}

/*
================
CollisionWorld::CM_TraceCapsuleThroughCapsule

capsule vs. capsule collision (not rotated)
================
*/
void CollisionWorld::CM_TraceCapsuleThroughCapsule(traceWork_t * tw, clipHandle_t model) {
	int i;
	Vector mins, maxs;
	vec3_t top, bottom, starttop, startbottom, endtop, endbottom;
	vec3_t offset, symetricSize[2];
	float radius, halfwidth, halfheight, offs, h;

	CollisionWorld::CM_ModelBounds(model, mins, maxs);
	// test trace bounds vs. capsule bounds
	if (tw->bounds[0][0] > maxs[0] + RADIUS_EPSILON
		|| tw->bounds[0][1] > maxs[1] + RADIUS_EPSILON
		|| tw->bounds[0][2] > maxs[2] + RADIUS_EPSILON
		|| tw->bounds[1][0] < mins[0] - RADIUS_EPSILON
		|| tw->bounds[1][1] < mins[1] - RADIUS_EPSILON
		|| tw->bounds[1][2] < mins[2] - RADIUS_EPSILON
		) {
		return;
	}
	// top origin and bottom origin of each sphere at start and end of trace
	VecAdd(tw->start, sphere.offset, starttop);
	VecSubtract(tw->start, sphere.offset, startbottom);
	VecAdd(tw->end, sphere.offset, endtop);
	VecSubtract(tw->end, sphere.offset, endbottom);

	// calculate top and bottom of the capsule spheres to collide with
	for (i = 0; i < 3; i++) {
		offset[i] = (mins[i] + maxs[i]) * 0.5f;
		symetricSize[0][i] = mins[i] - offset[i];
		symetricSize[1][i] = maxs[i] - offset[i];
	}
	halfwidth = symetricSize[1][0];
	halfheight = symetricSize[1][2];
	radius = (halfwidth > halfheight) ? halfheight : halfwidth;
	offs = halfheight - radius;
	VecCopy(offset, top);
	top[2] += offs;
	VecCopy(offset, bottom);
	bottom[2] -= offs;
	// expand radius of spheres
	radius += sphere.radius;
	// if there is horizontal movement
	if (tw->start[0] != tw->end[0] || tw->start[1] != tw->end[1]) {
		// height of the expanded cylinder is the height of both cylinders minus the radius of both spheres
		h = halfheight - radius;
		// if the cylinder has a height
		if (h > 0) {
			// test for collisions between the cylinders
			CollisionWorld::CM_TraceThroughVerticalCylinder(tw, offset, radius, h, tw->start, tw->end);
		}
	}
	// test for collision between the spheres
	CollisionWorld::CM_TraceThroughSphere(tw, top, radius, startbottom, endbottom);
	CollisionWorld::CM_TraceThroughSphere(tw, bottom, radius, starttop, endtop);
}

/*
================
CollisionWorld::CM_TraceBoundingBoxThroughCapsule

bounding box vs. capsule collision
================
*/
void CollisionWorld::CM_TraceBoundingBoxThroughCapsule(traceWork_t * tw, clipHandle_t model) {
	Vector mins, maxs;
	vec3_t offset, size[2];
	clipHandle_t h;
	collisionModel_t* cmod;
	int i;

	// mins maxs of the capsule
	CollisionWorld::CM_ModelBounds(model, mins, maxs);

	// offset for capsule center
	for (i = 0; i < 3; i++) {
		offset[i] = (mins[i] + maxs[i]) * 0.5f;
		size[0][i] = mins[i] - offset[i];
		size[1][i] = maxs[i] - offset[i];
		tw->start[i] -= offset[i];
		tw->end[i] -= offset[i];
	}

	// replace the bounding box with the capsule
	sphere.use = true;
	sphere.radius = (size[1][0] > size[1][2]) ? size[1][2] : size[1][0];
	VecSet(sphere.offset, 0, 0, size[1][2] - sphere.radius);

	// replace the capsule with the bounding box
	h = CollisionWorld::CM_TempBoxModel(tw->size[0], tw->size[1], false);
	// calculate collision
	cmod = CollisionWorld::CM_ClipHandleToModel(h);
	CollisionWorld::CM_TraceToLeaf(tw, &cmod->leaf);
}

//=========================================================================================

/*
==================
CollisionWorld::CM_TraceThroughTree

Traverse all the contacted leafs from the start to the end position.
If the trace is a point, they will be exactly in order, but for larger
trace volumes it is possible to hit something in a later leaf with
a smaller intercept fraction.
==================
*/
void CollisionWorld::CM_TraceThroughTree(traceWork_t * tw, int num, float p1f, float p2f, vec3_t p1, vec3_t p2) {
	const collisionNode_t* node;
	const collisionPlane_t* plane;
	float		t1, t2, offset;
	float		frac, frac2;
	float		idist;
	vec3_t		mid;
	int			side;
	float		midf;

	if (tw->trace.fraction <= p1f) {
		return;		// already hit something nearer
	}

	// if < 0, we are in a leaf node
	if (num < 0) {
		CollisionWorld::CM_TraceToLeaf(tw, &this->leafs[-1 - num]);
		return;
	}

	//
	// find the point distances to the seperating plane
	// and the offset for the size of the box
	//
	node = &this->nodes[num];
	plane = node->plane;

	// adjust the plane distance apropriately for mins/maxs
	if (plane->type < 3) {
		t1 = p1[plane->type] - plane->dist;
		t2 = p2[plane->type] - plane->dist;
		offset = tw->extents[plane->type];
	}
	else {
		t1 = DotProduct(plane->normal, p1) - plane->dist;
		t2 = DotProduct(plane->normal, p2) - plane->dist;
		if (tw->isPoint) {
			offset = 0;
		}
		else {
			// this is silly
			offset = 2048;
		}
	}

	// see which sides we need to consider
	if (t1 >= offset + 1 && t2 >= offset + 1) {
		CollisionWorld::CM_TraceThroughTree(tw, node->children[0], p1f, p2f, p1, p2);
		return;
	}
	if (t1 < -offset - 1 && t2 < -offset - 1) {
		CollisionWorld::CM_TraceThroughTree(tw, node->children[1], p1f, p2f, p1, p2);
		return;
	}

	// put the crosspoint SURFACE_CLIP_EPSILON pixels on the near side
	if (t1 < t2) {
		idist = 1.0f / (t1 - t2);
		side = 1;
		frac2 = (t1 + offset + SURFACE_CLIP_EPSILON) * idist;
		frac = (t1 - offset + SURFACE_CLIP_EPSILON) * idist;
	}
	else if (t1 > t2) {
		idist = 1.0f / (t1 - t2);
		side = 0;
		frac2 = (t1 - offset - SURFACE_CLIP_EPSILON) * idist;
		frac = (t1 + offset + SURFACE_CLIP_EPSILON) * idist;
	}
	else {
		side = 0;
		frac = 1;
		frac2 = 0;
	}

	// move up to the node
	if (frac < 0) {
		frac = 0;
	}
	if (frac > 1) {
		frac = 1;
	}

	midf = p1f + (p2f - p1f) * frac;

	mid[0] = p1[0] + frac * (p2[0] - p1[0]);
	mid[1] = p1[1] + frac * (p2[1] - p1[1]);
	mid[2] = p1[2] + frac * (p2[2] - p1[2]);

	CollisionWorld::CM_TraceThroughTree(tw, node->children[side], p1f, midf, p1, mid);


	// go past the node
	if (frac2 < 0) {
		frac2 = 0;
	}
	if (frac2 > 1) {
		frac2 = 1;
	}

	midf = p1f + (p2f - p1f) * frac2;

	mid[0] = p1[0] + frac2 * (p2[0] - p1[0]);
	mid[1] = p1[1] + frac2 * (p2[1] - p1[1]);
	mid[2] = p1[2] + frac2 * (p2[2] - p1[2]);

	CollisionWorld::CM_TraceThroughTree(tw, node->children[side ^ 1], midf, p2f, mid, p2);
}

//======================================================================

/*
==================
CollisionWorld::CM_BoxTrace
==================
*/
void CollisionWorld::CM_BoxTrace(trace_t * results, const Vector& start, const Vector& end,
	const Vector& mins, const Vector& maxs,
	clipHandle_t model, uint32_t brushmask, bool cylinder) {
	int			i;
	traceWork_t	tw;
	vec3_t		offset;
	collisionModel_t* cmod;

	cmod = CollisionWorld::CM_ClipHandleToModel(model);

	this->checkcount++;		// for multi-check avoidance

	c_traces++;				// for statistics, may be zeroed

	// fill in a default trace
	memset(&tw, 0, sizeof(tw));
	tw.trace.fraction = 1;	// assume it goes the entire distance until shown otherwise

	// set basic parms
	tw.trace.location = -1; // clear out unneeded location
	tw.contents = brushmask;

	// adjust so that mins and maxs are always symetric, which
	// avoids some complications with plane expanding of rotated
	// bmodels
	for (i = 0; i < 3; i++) {
		offset[i] = (mins[i] + maxs[i]) * 0.5f;
		tw.size[0][i] = mins[i] - offset[i];
		tw.size[1][i] = maxs[i] - offset[i];
		tw.start[i] = start[i] + offset[i];
		tw.end[i] = end[i] + offset[i];
	}

	tw.height = tw.size[1][2];
	tw.radius = tw.size[1][0];

	if (cylinder && !sphere.use)
	{
		sphere.use = true;
		sphere.radius = (tw.size[1][0] > tw.size[1][2]) ? tw.size[1][2] : tw.size[1][0];
		VecSet(sphere.offset, 0, 0, tw.size[1][2] - sphere.radius);
	}
	tw.maxOffset = tw.size[1][0] + tw.size[1][1] + tw.size[1][2];

	// tw.offsets[signbits] = vector to apropriate corner from origin
	tw.offsets[0][0] = tw.size[0][0];
	tw.offsets[0][1] = tw.size[0][1];
	tw.offsets[0][2] = tw.size[0][2];

	tw.offsets[1][0] = tw.size[1][0];
	tw.offsets[1][1] = tw.size[0][1];
	tw.offsets[1][2] = tw.size[0][2];

	tw.offsets[2][0] = tw.size[0][0];
	tw.offsets[2][1] = tw.size[1][1];
	tw.offsets[2][2] = tw.size[0][2];

	tw.offsets[3][0] = tw.size[1][0];
	tw.offsets[3][1] = tw.size[1][1];
	tw.offsets[3][2] = tw.size[0][2];

	tw.offsets[4][0] = tw.size[0][0];
	tw.offsets[4][1] = tw.size[0][1];
	tw.offsets[4][2] = tw.size[1][2];

	tw.offsets[5][0] = tw.size[1][0];
	tw.offsets[5][1] = tw.size[0][1];
	tw.offsets[5][2] = tw.size[1][2];

	tw.offsets[6][0] = tw.size[0][0];
	tw.offsets[6][1] = tw.size[1][1];
	tw.offsets[6][2] = tw.size[1][2];

	tw.offsets[7][0] = tw.size[1][0];
	tw.offsets[7][1] = tw.size[1][1];
	tw.offsets[7][2] = tw.size[1][2];

	//
	// calculate bounds
	//
	for (i = 0; i < 3; i++) {
		if (tw.start[i] < tw.end[i]) {
			tw.bounds[0][i] = tw.start[i] + tw.size[0][i];
			tw.bounds[1][i] = tw.end[i] + tw.size[1][i];
		}
		else {
			tw.bounds[0][i] = tw.end[i] + tw.size[0][i];
			tw.bounds[1][i] = tw.start[i] + tw.size[1][i];
		}
	}

	//
	// check for position test special case
	//
	if (start[0] == end[0] && start[1] == end[1] && start[2] == end[2]) {
		if (model) {
			CollisionWorld::CM_TestInLeaf(&tw, &cmod->leaf);
		}
		else {
			CollisionWorld::CM_PositionTest(&tw);
		}
	}
	else {
		//
		// check for point special case
		//
		if (tw.size[0][0] == 0 && tw.size[0][1] == 0 && tw.size[0][2] == 0) {
			tw.isPoint = true;
			VectorClear(tw.extents);
		}
		else {
			tw.isPoint = false;
			tw.extents[0] = tw.size[1][0];
			tw.extents[1] = tw.size[1][1];
			tw.extents[2] = tw.size[1][2];
		}

		//
		// general sweeping through world
		//
		if (model) {
			CollisionWorld::CM_TraceToLeaf(&tw, &cmod->leaf);
		}
		else {
			CollisionWorld::CM_TraceThroughTree(&tw, 0, 0, 1, tw.start, tw.end);
		}
	}

	// generate endpos from the original, unmodified start/end
	if (tw.trace.fraction == 1) {
		VecCopy(end, tw.trace.endpos);
	}
	else {
		for (i = 0; i < 3; i++) {
			tw.trace.endpos[i] = start[i] + tw.trace.fraction * (end[i] - start[i]);
		}
	}

	// If allsolid is set (was entirely inside something solid), the plane is not valid.
	// If fraction == 1.0, we never hit anything, and thus the plane is not valid.
	// Otherwise, the normal on the plane should have unit length
	assert(tw.trace.allsolid ||
		tw.trace.fraction == 1.0 ||
		VectorLengthSquared(tw.trace.plane.normal) > 0.9999);
	*results = tw.trace;
	sphere.use = false;
}

/*
==================
CollisionWorld::CM_TransformedBoxTrace

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
void CollisionWorld::CM_TransformedBoxTrace(trace_t * results, const Vector& start, const Vector& end,
	const Vector& mins, const Vector& maxs,
	clipHandle_t model, uint32_t brushmask,
	const Vector& origin, const Vector& angles, bool cylinder) {
	trace_t		trace;
	vec3_t		start_l, end_l;
	vec3_t		a;
	vec3_t		forward, left, up;
	vec3_t		temp;
	bool	rotated;
	vec3_t		offset;
	vec3_t		symetricSize[2];
	int			i;
	float		halfwidth;
	float		halfheight;
	float		t;

	// adjust so that mins and maxs are always symetric, which
	// avoids some complications with plane expanding of rotated
	// bmodels
	for (i = 0; i < 3; i++) {
		offset[i] = (mins[i] + maxs[i]) * 0.5f;
		symetricSize[0][i] = mins[i] - offset[i];
		symetricSize[1][i] = maxs[i] - offset[i];
		start_l[i] = start[i] + offset[i];
		end_l[i] = end[i] + offset[i];
	}

	// subtract origin offset
	VecSubtract(start_l, origin, start_l);
	VecSubtract(end_l, origin, end_l);

	// rotate start and end into the models frame of reference
	if (angles[0] || angles[1] || angles[2]) {
		rotated = true;
	}
	else {
		rotated = false;
	}

	halfwidth = symetricSize[1][0];
	halfheight = symetricSize[1][2];

	sphere.use = cylinder;
	sphere.radius = (halfwidth > halfheight) ? halfheight : halfwidth;
	t = halfheight - sphere.radius;

	if (rotated) {
		AngleVectorsLeft(angles, forward, left, up);

		VecCopy(start_l, temp);
		start_l[0] = DotProduct(temp, forward);
		start_l[1] = DotProduct(temp, left);
		start_l[2] = DotProduct(temp, up);

		VecCopy(end_l, temp);
		end_l[0] = DotProduct(temp, forward);
		end_l[1] = DotProduct(temp, left);
		end_l[2] = DotProduct(temp, up);

		sphere.offset[0] = forward[2] * t;
		sphere.offset[1] = left[2] * t;
		sphere.offset[2] = up[2] * t;
	}
	else {
		VecSet(sphere.offset, 0, 0, t);
	}

	// sweep the box through the model
	CollisionWorld::CM_BoxTrace(&trace, start_l, end_l, symetricSize[0], symetricSize[1], model, brushmask, cylinder);

	sphere.use = false;

	// if the bmodel was rotated and there was a collision
	if (rotated && trace.fraction != 1.0) {
		// rotation of bmodel collision plane
		VecNegate(angles, a);
		AngleVectorsLeft(a, forward, left, up);

		VecCopy(trace.plane.normal, temp);
		trace.plane.normal[0] = DotProduct(temp, forward);
		trace.plane.normal[1] = DotProduct(temp, left);
		trace.plane.normal[2] = DotProduct(temp, up);
	}

	// re-calculate the end position of the trace because the trace.endpos
	// calculated by CollisionWorld::CM_Trace could be rotated and have an offset
	trace.endpos[0] = start[0] + trace.fraction * (end[0] - start[0]);
	trace.endpos[1] = start[1] + trace.fraction * (end[1] - start[1]);
	trace.endpos[2] = start[2] + trace.fraction * (end[2] - start[2]);

	*results = trace;
}

/*
================
CollisionWorld::CM_SightTraceThroughPatch
================
*/
bool CollisionWorld::CM_SightTraceThroughPatch(traceWork_t * tw, collisionPatch_t * patch)
{
	CollisionWorld::CM_TraceThroughPatchCollide(tw, &patch->pc);

	if (tw->trace.allsolid || tw->trace.startsolid || tw->trace.fraction < 1) {
		return false;
	}
	return true;
}

/*
================
CollisionWorld::CM_SightTraceThroughTerrain
================
*/
bool CollisionWorld::CM_SightTraceThroughTerrain(traceWork_t * tw, collisionTerrain_t * terrain)
{
	CollisionWorld::CM_TraceThroughTerrainCollide(tw, &terrain->tc);

	if (tw->trace.allsolid || tw->trace.startsolid || tw->trace.fraction < 1) {
		return false;
	}
	return true;
}

/*
================
CollisionWorld::CM_SightTraceThroughBrush
================
*/
bool CollisionWorld::CM_SightTraceThroughBrush(traceWork_t * tw, collisionBrush_t * brush)
{
	// They redid the fonction in mohaa
	CollisionWorld::CM_TraceThroughBrush(tw, brush);

	if (tw->trace.allsolid || tw->trace.startsolid || tw->trace.fraction < 1) {
		return false;
	}
	return true;
}

/*
================
CollisionWorld::CM_SightTraceToLeaf
================
*/
bool CollisionWorld::CM_SightTraceToLeaf(traceWork_t* tw, collisionLeaf_t* leaf) {
	collisionBrush_t* b;
	collisionPatch_t* patch;
	collisionTerrain_t* terrain;

	// test box position against all brushes in the leaf
	for (uintptr_t k = 0; k < leaf->numLeafBrushes; k++) {
		b = &this->brushes[this->leafbrushes[leaf->firstLeafBrush + k]];
		if (b->checkcount == this->checkcount) {
			continue;	// already checked this brush in another leaf
		}
		b->checkcount = this->checkcount;

		if (!(b->contents & tw->contents)) {
			continue;
		}

		if (!CollisionWorld::CM_SightTraceThroughBrush(tw, b)) {
			return false;
		}
	}

	// test against all patches
	for (uintptr_t k = 0; k < leaf->numLeafSurfaces; k++) {
		patch = this->surfaces[this->leafsurfaces[leaf->firstLeafSurface + k]];
		if (!patch) {
			continue;
		}
		if (patch->checkcount == this->checkcount) {
			continue;	// already checked this brush in another leaf
		}
		patch->checkcount = this->checkcount;

		if (!(patch->contents & tw->contents)) {
			continue;
		}

		if (!CollisionWorld::CM_SightTraceThroughPatch(tw, patch)) {
			return false;
		}
	}

	// test against all terrains
	for (uintptr_t k = 0; k < leaf->numLeafTerrains; k++) {
		terrain = this->leafterrains[leaf->firstLeafTerrain + k];
		if (!terrain) {
			continue;
		}
		if (terrain->checkcount == this->checkcount) {
			continue;
		}
		terrain->checkcount = this->checkcount;

		if (!CollisionWorld::CM_SightTraceThroughTerrain(tw, terrain)) {
			return false;
		}
	}

	return true;
	}

/*
==================
CollisionWorld::CM_SightTraceThroughTree

Traverse all the contacted leafs from the start to the end position.
If the trace is a point, they will be exactly in order, but for larger
trace volumes it is possible to hit something in a later leaf with
a smaller intercept fraction.
==================
*/
bool CollisionWorld::CM_SightTraceThroughTree(traceWork_t * tw, int num, float p1f, float p2f, vec3_t p1, vec3_t p2) {
	const collisionNode_t* node;
	const collisionPlane_t* plane;
	float		t1, t2, offset;
	float		frac, frac2;
	float		idist;
	vec3_t		mid;
	int			side;
	float		midf;

	// if < 0, we are in a leaf node
	if (num < 0) {
		return CollisionWorld::CM_SightTraceToLeaf(tw, &this->leafs[-1 - num]);
	}

	//
	// find the point distances to the seperating plane
	// and the offset for the size of the box
	//
	node = &this->nodes[num];
	plane = node->plane;

	// adjust the plane distance apropriately for mins/maxs
	if (plane->type < 3) {
		t1 = p1[plane->type] - plane->dist;
		t2 = p2[plane->type] - plane->dist;
		offset = tw->extents[plane->type];
	}
	else {
		t1 = DotProduct(plane->normal, p1) - plane->dist;
		t2 = DotProduct(plane->normal, p2) - plane->dist;
		if (tw->isPoint) {
			offset = 0;
		}
		else {
			// this is silly
			offset = 2048;
		}
	}

	// see which sides we need to consider
	if (t1 >= offset + 1 && t2 >= offset + 1) {
		return CollisionWorld::CM_SightTraceThroughTree(tw, node->children[0], p1f, p2f, p1, p2);
	}
	if (t1 < -offset - 1 && t2 < -offset - 1) {
		return CollisionWorld::CM_SightTraceThroughTree(tw, node->children[1], p1f, p2f, p1, p2);
	}

	// put the crosspoint SURFACE_CLIP_EPSILON pixels on the near side
	if (t1 < t2) {
		idist = 1.0f / (t1 - t2);
		side = 1;
		frac2 = (t1 + offset + SURFACE_CLIP_EPSILON) * idist;
		frac = (t1 - offset + SURFACE_CLIP_EPSILON) * idist;
	}
	else if (t1 > t2) {
		idist = 1.0f / (t1 - t2);
		side = 0;
		frac2 = (t1 - offset - SURFACE_CLIP_EPSILON) * idist;
		frac = (t1 + offset + SURFACE_CLIP_EPSILON) * idist;
	}
	else {
		side = 0;
		frac = 1;
		frac2 = 0;
	}

	// move up to the node
	if (frac < 0) {
		frac = 0;
	}
	if (frac > 1) {
		frac = 1;
	}

	midf = p1f + (p2f - p1f) * frac;

	mid[0] = p1[0] + frac * (p2[0] - p1[0]);
	mid[1] = p1[1] + frac * (p2[1] - p1[1]);
	mid[2] = p1[2] + frac * (p2[2] - p1[2]);

	if (!CollisionWorld::CM_SightTraceThroughTree(tw, node->children[side], p1f, midf, p1, mid)) {
		return false;
	}


	// go past the node
	if (frac2 < 0) {
		frac2 = 0;
	}
	if (frac2 > 1) {
		frac2 = 1;
	}

	midf = p1f + (p2f - p1f) * frac2;

	mid[0] = p1[0] + frac2 * (p2[0] - p1[0]);
	mid[1] = p1[1] + frac2 * (p2[1] - p1[1]);
	mid[2] = p1[2] + frac2 * (p2[2] - p1[2]);

	return CollisionWorld::CM_SightTraceThroughTree(tw, node->children[side ^ 1], midf, p2f, mid, p2);
}

/*
==================
CollisionWorld::CM_BoxSightTrace
==================
*/
bool CollisionWorld::CM_BoxSightTrace(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, clipHandle_t model, uint32_t brushmask, bool cylinder)
{
	int			i;
	traceWork_t	tw;
	vec3_t		offset;
	collisionModel_t* cmod;
	bool	bPassed;

	cmod = CollisionWorld::CM_ClipHandleToModel(model);

	this->checkcount++;		// for multi-check avoidance

	c_traces++;				// for statistics, may be zeroed

	if (!this->nodes.NumObjects()) {
		return false;
	}

	// fill in a default trace
	memset(&tw, 0, sizeof(tw));
	tw.trace.fraction = 1;	// assume it goes the entire distance until shown otherwise

	// set basic parms
	tw.contents = brushmask;

	// adjust so that mins and maxs are always symetric, which
	// avoids some complications with plane expanding of rotated
	// bmodels
	for (i = 0; i < 3; i++) {
		offset[i] = (mins[i] + maxs[i]) * 0.5f;
		tw.size[0][i] = mins[i] - offset[i];
		tw.size[1][i] = maxs[i] - offset[i];
		tw.start[i] = start[i] + offset[i];
		tw.end[i] = end[i] + offset[i];
	}

	tw.height = tw.size[1][2];
	tw.radius = tw.size[1][0];

	if (cylinder && !sphere.use)
	{
		sphere.use = true;
		sphere.radius = (tw.size[1][0] > tw.size[1][2]) ? tw.size[1][2] : tw.size[1][0];
		VecSet(sphere.offset, 0, 0, tw.size[1][2] - sphere.radius);
	}
	tw.maxOffset = tw.size[1][0] + tw.size[1][1] + tw.size[1][2];

	// tw.offsets[signbits] = vector to apropriate corner from origin
	tw.offsets[0][0] = tw.size[0][0];
	tw.offsets[0][1] = tw.size[0][1];
	tw.offsets[0][2] = tw.size[0][2];

	tw.offsets[1][0] = tw.size[1][0];
	tw.offsets[1][1] = tw.size[0][1];
	tw.offsets[1][2] = tw.size[0][2];

	tw.offsets[2][0] = tw.size[0][0];
	tw.offsets[2][1] = tw.size[1][1];
	tw.offsets[2][2] = tw.size[0][2];

	tw.offsets[3][0] = tw.size[1][0];
	tw.offsets[3][1] = tw.size[1][1];
	tw.offsets[3][2] = tw.size[0][2];

	tw.offsets[4][0] = tw.size[0][0];
	tw.offsets[4][1] = tw.size[0][1];
	tw.offsets[4][2] = tw.size[1][2];

	tw.offsets[5][0] = tw.size[1][0];
	tw.offsets[5][1] = tw.size[0][1];
	tw.offsets[5][2] = tw.size[1][2];

	tw.offsets[6][0] = tw.size[0][0];
	tw.offsets[6][1] = tw.size[1][1];
	tw.offsets[6][2] = tw.size[1][2];

	tw.offsets[7][0] = tw.size[1][0];
	tw.offsets[7][1] = tw.size[1][1];
	tw.offsets[7][2] = tw.size[1][2];

	//
	// calculate bounds
	//
	for (i = 0; i < 3; i++) {
		if (tw.start[i] < tw.end[i]) {
			tw.bounds[0][i] = tw.start[i] + tw.size[0][i];
			tw.bounds[1][i] = tw.end[i] + tw.size[1][i];
		}
		else {
			tw.bounds[0][i] = tw.end[i] + tw.size[0][i];
			tw.bounds[1][i] = tw.start[i] + tw.size[1][i];
		}
	}

	//
	// check for position test special case
	//
	if (start[0] == end[0] && start[1] == end[1] && start[2] == end[2]) {
		if (model) {
			CollisionWorld::CM_TestInLeaf(&tw, &cmod->leaf);
		}
		else {
			CollisionWorld::CM_PositionTest(&tw);
		}
		bPassed = !tw.trace.startsolid;
	}
	else {
		//
		// check for point special case
		//
		if (tw.size[0][0] == 0 && tw.size[0][1] == 0 && tw.size[0][2] == 0) {
			tw.isPoint = true;
			VectorClear(tw.extents);
		}
		else {
			tw.isPoint = false;
			tw.extents[0] = tw.size[1][0];
			tw.extents[1] = tw.size[1][1];
			tw.extents[2] = tw.size[1][2];
		}

		//
		// general sweeping through world
		//
		if (model) {
			bPassed = CollisionWorld::CM_SightTraceToLeaf(&tw, &cmod->leaf);
		}
		else {
			bPassed = CollisionWorld::CM_SightTraceThroughTree(&tw, 0, 0, 1, tw.start, tw.end);
		}
	}

	sphere.use = false;

	return bPassed;
}

/*
==================
CollisionWorld::CM_TransformedBoxSightTrace
==================
*/
bool CollisionWorld::CM_TransformedBoxSightTrace(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, clipHandle_t model, uint32_t brushmask, const Vector& origin, const Vector& angles, bool cylinder)
{
	vec3_t		start_l, end_l;
	vec3_t		forward, left, up;
	vec3_t		temp;
	bool	rotated;
	vec3_t		offset;
	vec3_t		symetricSize[2];
	int			i;
	float		halfwidth;
	float		halfheight;
	float		t;

	// adjust so that mins and maxs are always symetric, which
	// avoids some complications with plane expanding of rotated
	// bmodels
	for (i = 0; i < 3; i++) {
		offset[i] = (mins[i] + maxs[i]) * 0.5f;
		symetricSize[0][i] = mins[i] - offset[i];
		symetricSize[1][i] = maxs[i] - offset[i];
		start_l[i] = start[i] + offset[i];
		end_l[i] = end[i] + offset[i];
	}

	// subtract origin offset
	VecSubtract(start_l, origin, start_l);
	VecSubtract(end_l, origin, end_l);

	// rotate start and end into the models frame of reference
	if (angles[0] || angles[1] || angles[2]) {
		rotated = true;
	}
	else {
		rotated = false;
	}

	halfwidth = symetricSize[1][0];
	halfheight = symetricSize[1][2];

	sphere.use = cylinder;
	sphere.radius = (halfwidth > halfheight) ? halfheight : halfwidth;
	t = halfheight - sphere.radius;

	if (rotated) {
		AngleVectorsLeft(angles, forward, left, up);

		VecCopy(start_l, temp);
		start_l[0] = DotProduct(temp, forward);
		start_l[1] = DotProduct(temp, left);
		start_l[2] = DotProduct(temp, up);

		VecCopy(end_l, temp);
		end_l[0] = DotProduct(temp, forward);
		end_l[1] = DotProduct(temp, left);
		end_l[2] = DotProduct(temp, up);

		sphere.offset[0] = forward[2] * t;
		sphere.offset[1] = left[2] * t;
		sphere.offset[2] = up[2] * t;
	}
	else {
		VecSet(sphere.offset, 0, 0, t);
	}

	// sweep the box through the model
	return CollisionWorld::CM_BoxSightTrace(start_l, end_l, symetricSize[0], symetricSize[1], model, brushmask, cylinder);
}

/*
==================
CollisionWorld::CM_PointLeafnum_r

==================
*/
int CollisionWorld::CM_PointLeafnum_r(const Vector& p, int num) {
	float		d;
	const collisionNode_t* node;
	const collisionPlane_t* plane;

	while (num >= 0)
	{
		node = &this->nodes[num];
		plane = node->plane;

		if (plane->type < 3)
			d = p[plane->type] - plane->dist;
		else
			d = DotProduct(plane->normal, p) - plane->dist;
		if (d < 0)
			num = node->children[1];
		else
			num = node->children[0];
	}

	c_pointcontents++;		// optimize counter

	return -1 - num;
}

int CollisionWorld::CM_PointLeafnum(const Vector& p) {
	if (!this->nodes.NumObjects()) {	// map not loaded
		return 0;
	}
	return CollisionWorld::CM_PointLeafnum_r(p, 0);
}


/*
======================================================================

LEAF LISTING

======================================================================
*/


void CollisionWorld::CM_StoreLeafs(leafList_t* ll, int nodenum) {
	int		leafNum;

	leafNum = -1 - nodenum;

	// store the lastLeaf even if the list is overflowed
	if (this->leafs[leafNum].cluster != -1) {
		ll->lastLeaf = leafNum;
	}

	if (ll->count >= ll->maxcount) {
		ll->overflowed = true;
		return;
	}
	ll->list[ll->count++] = leafNum;
}

void CollisionWorld::CM_StoreBrushes(leafList_t* ll, int nodenum) {
	uintptr_t leafnum;
	uintptr_t brushnum;
	collisionLeaf_t* leaf;
	collisionBrush_t* b;

	leafnum = -1 - nodenum;

	leaf = &this->leafs[leafnum];

	for (uintptr_t k = 0; k < leaf->numLeafBrushes; k++) {
		brushnum = this->leafbrushes[leaf->firstLeafBrush + k];
		b = &this->brushes[brushnum];
		if (b->checkcount == this->checkcount) {
			continue;	// already checked this brush in another leaf
		}
		b->checkcount = this->checkcount;
		int i;
		for (i = 0; i < 3; i++) {
			if (b->bounds[0][i] >= ll->bounds[1][i] || b->bounds[1][i] <= ll->bounds[0][i]) {
				break;
			}
		}
		if (i != 3) {
			continue;
		}
		if (ll->count >= ll->maxcount) {
			ll->overflowed = true;
			return;
		}
		((collisionBrush_t**)ll->list)[ll->count++] = b;
	}
#if 0
	// store patches?
	for (k = 0; k < leaf->numLeafSurfaces; k++) {
		patch = this->surfaces[this->leafsurfaces[leaf->firstleafsurface + k]];
		if (!patch) {
			continue;
		}
	}
#endif
}

/*
=============
CollisionWorld::CM_BoxLeafnums

Fills in a list of all the leafs touched
=============
*/
void CollisionWorld::CM_BoxLeafnums_r(leafList_t* ll, int nodenum) {
	const collisionPlane_t* plane;
	const collisionNode_t* node;
	int			s;

	while (1) {
		if (nodenum < 0) {
			(this->*ll->storeLeafs)(ll, nodenum);
			return;
		}

		node = &this->nodes[nodenum];
		plane = node->plane;
		s = BoxOnPlaneSide(ll->bounds[0], ll->bounds[1], plane);
		if (s == 1) {
			nodenum = node->children[0];
		}
		else if (s == 2) {
			nodenum = node->children[1];
		}
		else {
			// go down both
			CollisionWorld::CM_BoxLeafnums_r(ll, node->children[0]);
			nodenum = node->children[1];
		}
	}
}

/*
==================
CollisionWorld::CM_BoxLeafnums
==================
*/
size_t	CollisionWorld::CM_BoxLeafnums(const Vector& mins, const Vector& maxs, int* list, size_t listsize, int* lastLeaf) {
	leafList_t	ll;

	this->checkcount++;

	VecCopy(mins, ll.bounds[0]);
	VecCopy(maxs, ll.bounds[1]);
	ll.count = 0;
	ll.maxcount = listsize;
	ll.list = list;
	ll.storeLeafs = &CollisionWorld::CM_StoreLeafs;
	ll.lastLeaf = 0;
	ll.overflowed = false;

	CollisionWorld::CM_BoxLeafnums_r(&ll, 0);

	*lastLeaf = ll.lastLeaf;
	return ll.count;
}

/*
==================
CollisionWorld::CM_BoxBrushes
==================
*/
size_t CollisionWorld::CM_BoxBrushes(const Vector& mins, const Vector& maxs, collisionBrush_t** list, size_t listsize) {
	leafList_t	ll;

	this->checkcount++;

	VecCopy(mins, ll.bounds[0]);
	VecCopy(maxs, ll.bounds[1]);
	ll.count = 0;
	ll.maxcount = listsize;
	ll.list = (int*)list;
	ll.storeLeafs = &CollisionWorld::CM_StoreBrushes;
	ll.lastLeaf = 0;
	ll.overflowed = false;

	CollisionWorld::CM_BoxLeafnums_r(&ll, 0);

	return ll.count;
}

/*
==================
CollisionWorld::CM_ShaderPointer
==================
*/
collisionShader_t* CollisionWorld::CM_ShaderPointer(int iShaderNum)
{
	return (collisionShader_t*)&this->shaders[iShaderNum];
}


//====================================================================


/*
==================
CollisionWorld::CM_PointContents

==================
*/
int CollisionWorld::CM_PointContents(const vec3_t p, clipHandle_t model) {
	uintptr_t leafnum;
	uintptr_t brushnum;
	collisionLeaf_t* leaf;
	collisionBrush_t* b;
	int			contents;
	float		d;
	collisionModel_t* clipm;

	if (!this->nodes.NumObjects()) {	// map not loaded
		return 0;
	}

	if (model) {
		clipm = CollisionWorld::CM_ClipHandleToModel(model);
		leaf = &clipm->leaf;
	}
	else {
		leafnum = CollisionWorld::CM_PointLeafnum_r(p, 0);
		leaf = &this->leafs[leafnum];
	}

	contents = 0;
	for (uintptr_t k = 0; k < leaf->numLeafBrushes; k++) {
		brushnum = this->leafbrushes[leaf->firstLeafBrush + k];
		b = &this->brushes[brushnum];

		if (!CollisionWorld::CM_BoundsIntersectPoint(b->bounds[0], b->bounds[1], p)) {
			continue;
		}

		// see if the point is in the brush
		uintptr_t i;
		for (i = 0; i < b->numsides; i++) {
			d = DotProduct(p, b->sides[i].plane->normal);
			// FIXME test for Cash
			//			if ( d >= b->sides[i].plane->dist ) {
			if (d > b->sides[i].plane->dist) {
				break;
			}
		}

		if (i == b->numsides) {
			contents |= b->contents;
		}
	}

	return contents;
}

/*
==================
CollisionWorld::CM_PointBrushNum
==================
*/
uintptr_t CollisionWorld::CM_PointBrushNum(const vec3_t p, clipHandle_t model) {
	uintptr_t brushnum;
	collisionLeaf_t* leaf;
	collisionBrush_t* b;
	collisionModel_t* clipm;

	if (!this->nodes.NumObjects()) {
		return 0;
	}

	if (model) {
		clipm = CollisionWorld::CM_ClipHandleToModel(model);
		leaf = &clipm->leaf;
	}
	else {
		leaf = &this->leafs[CollisionWorld::CM_PointLeafnum_r(p, 0)];
	}

	for (uintptr_t k = 0; k < leaf->numLeafBrushes; k++) {
		brushnum = this->leafbrushes[leaf->firstLeafBrush + k];
		b = &this->brushes[brushnum];

		// see if the point is in the brush
		uintptr_t i;
		for (i = 0; i < b->numsides; i++) {
			if (DotProduct(p, b->sides[i].plane->normal) > b->sides[i].plane->dist) {
				return brushnum;
			}
		}

		if (i == b->numsides) {
			return brushnum;
		}
	}

	return -1;
}

/*
==================
CollisionWorld::CM_TransformedPointContents

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
int	CollisionWorld::CM_TransformedPointContents(const Vector& p, clipHandle_t model, const Vector& origin, const Vector& angles) {
	vec3_t		p_l;
	vec3_t		temp;
	vec3_t		forward, right, up;

	// subtract origin offset
	VecSubtract(p, origin, p_l);

	// rotate start and end into the models frame of reference
	if (model != BOX_MODEL_HANDLE &&
		(angles[0] || angles[1] || angles[2]))
	{
		AngleVectors(angles, forward, right, up);

		VecCopy(p_l, temp);
		p_l[0] = DotProduct(temp, forward);
		p_l[1] = -DotProduct(temp, right);
		p_l[2] = DotProduct(temp, up);
	}

	return CollisionWorld::CM_PointContents(p_l, model);
}

/*
====================
CollisionWorld::CM_BoundsIntersect
====================
*/
bool CollisionWorld::CM_BoundsIntersect(const Vector& mins, const Vector& maxs, const Vector& mins2, const Vector& maxs2)
{
	if (maxs[0] < mins2[0] - SURFACE_CLIP_EPSILON ||
		maxs[1] < mins2[1] - SURFACE_CLIP_EPSILON ||
		maxs[2] < mins2[2] - SURFACE_CLIP_EPSILON ||
		mins[0] > maxs2[0] + SURFACE_CLIP_EPSILON ||
		mins[1] > maxs2[1] + SURFACE_CLIP_EPSILON ||
		mins[2] > maxs2[2] + SURFACE_CLIP_EPSILON)
	{
		return false;
	}

	return true;
}

/*
====================
CollisionWorld::CM_BoundsIntersectPoint
====================
*/
bool CollisionWorld::CM_BoundsIntersectPoint(const Vector& mins, const Vector& maxs, const Vector& point)
{
	if (maxs[0] < point[0] - SURFACE_CLIP_EPSILON ||
		maxs[1] < point[1] - SURFACE_CLIP_EPSILON ||
		maxs[2] < point[2] - SURFACE_CLIP_EPSILON ||
		mins[0] > point[0] + SURFACE_CLIP_EPSILON ||
		mins[1] > point[1] + SURFACE_CLIP_EPSILON ||
		mins[2] > point[2] + SURFACE_CLIP_EPSILON)
	{
		return false;
	}

	return true;
}

/*
====================
CM_TraceThroughFence
====================
*/
bool CollisionWorld::CM_TraceThroughFence(traceWork_t* tw, const collisionBrush_t* brush, const collisionBrushSide_t* side, float fTraceFraction)
{
	int				i;
	int				iMaskPos;
	float			fS;
	float			fT;
	float* vNorm;
	vec3_t			vPos;
	const collisionFencemask_t* pMask;

	// FIXME?

	pMask = shaders[side->shaderNum].mask;
	if (!pMask) {
		return true;
	}

	//if (pMask == &cmf_dummy_trans) {
	//	return false;
	//}

	if (!side->pEq) {
		return true;
	}

	for (i = 0; i < 3; i++)
	{
		vNorm = side->plane->normal;
		vPos[i] = tw->start[i] + fTraceFraction * (tw->end[i] - tw->start[i]) - SURFACE_CLIP_EPSILON * vNorm[i];
	}

	fS = DotProduct(vPos, side->pEq->fSeq) + side->pEq->fSeq[3];
	fS = fS - floorf(fS);
	fT = DotProduct(vPos, side->pEq->fTeq) + side->pEq->fTeq[3];
	fT = fT - floorf(fT);

	iMaskPos = (int)((float)pMask->iWidth * fS) + pMask->iWidth * (int)((float)pMask->iHeight * fT);
	return (1 << (iMaskPos & 7)) & pMask->pData[iMaskPos >> 3];
}
