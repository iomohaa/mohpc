#pragma once

#include "../UtilityGlobal.h"
#include "../UtilityObject.h"
#include "../../Common/Math.h"
#include "../../Common/Vector.h"
#include "../../Common/str.h"
#include "../SharedPtr.h"

#include <vector>
#include <cstdint>

namespace MOHPC
{
	class CollisionWorld;

	using clipHandle_t = unsigned int;

	struct MOHPC_UTILITY_EXPORTS collisionPlane_t
	{
	public:
		collisionPlane_t();

		const_vec3p_t getNormal() const;
		float getDist() const;
		uint8_t getType() const;
		uint8_t getSignBits() const;

	public:
		vec3_t normal;
		float dist;
		union {
			struct {
				uint8_t type : 2;
				uint8_t signbits : 3;
			};
			uint8_t typedata;
		};
	};

	struct MOHPC_UTILITY_EXPORTS collisionNode_t
	{
		const collisionPlane_t* plane;
		// negative numbers are leafs
		int32_t children[2];

	public:
		collisionNode_t();
	};

	struct MOHPC_UTILITY_EXPORTS collisionLeaf_t
	{
		int32_t cluster;
		int32_t area;
		uint32_t firstLeafBrush;
		uint32_t numLeafBrushes;
		uint32_t firstLeafSurface;
		uint32_t numLeafSurfaces;
		uint32_t firstLeafTerrain;
		uint32_t numLeafTerrains;

	public:
		collisionLeaf_t();
	};

	struct collisionFencemask_t
	{
		str name;
		uint32_t iWidth;
		uint32_t iHeight;
		uint8_t* pData;
		collisionFencemask_t* pNext;

	public:
		collisionFencemask_t();
	};

	struct collisionShader_t
	{
	public:
		collisionShader_t();

	public:
		str shader;
		uint32_t surfaceFlags;
		uint32_t contentFlags;
		const collisionFencemask_t* mask;
	};

	struct terrainCollideSquare_t
	{
	public:
		terrainCollideSquare_t();

	public:
		vec4_t plane[2];
		int32_t eMode;
	};

	struct terrainCollide_t
	{
	public:
		terrainCollide_t();

	public:
		vec3_t vBounds[2];
		terrainCollideSquare_t squares[8][8];
	};

	struct collisionTerrain_t
	{
	public:
		collisionTerrain_t();

	public:
		size_t checkcount;
		int32_t surfaceFlags;
		int32_t contents;
		uintptr_t shaderNum;
		terrainCollide_t tc;
	};

	struct collisionArea_t
	{
		int32_t floodnum;
		int32_t floodvalid;

	public:
		collisionArea_t();
	};

	struct collisionModel_t
	{
	public:
		collisionModel_t();

	public:
		vec3_t mins;
		vec3_t maxs;
		// submodels don't reference the main tree
		collisionLeaf_t leaf;
	};

	struct collisionSideEq_t
	{
		float fSeq[4];
		float fTeq[4];

	public:
		collisionSideEq_t();
	};

	struct collisionBrushSide_t
	{
		collisionPlane_t* plane;
		int32_t surfaceFlags;
		uintptr_t shaderNum;
		collisionSideEq_t* pEq;

	public:
		collisionBrushSide_t();
	};

	struct collisionBrush_t
	{
	public:
		collisionBrush_t();

	public:
		// the shader that determined the contents
		uintptr_t shaderNum;
		int32_t contents;
		vec3_t bounds[2];
		size_t numsides;
		collisionBrushSide_t* sides;
		// to avoid repeated testings
		size_t checkcount;
	};

	struct patchPlane_t
	{
		vec4_t plane;
		// signx + (signy<<1) + (signz<<2), used as lookup during collision
		uint8_t signbits;

	public:
		patchPlane_t();
	};

	struct facet_t
	{
		int32_t surfacePlane;
		// 3 or four + 6 axial bevels + 4 or 3 * 4 edge bevels
		uint32_t numBorders;
		int32_t borderPlanes[4 + 6 + 16];
		bool borderInward[4 + 6 + 16];
		bool borderNoAdjust[4 + 6 + 16];

	public:
		facet_t();
	};

	struct patchCollide_t
	{

	public:
		patchCollide_t();
		~patchCollide_t();

	public:
		vec3_t bounds[2];
		// surface planes plus edge planes
		uint32_t numPlanes;
		patchPlane_t* planes;
		uint32_t numFacets;
		facet_t* facets;
	};

	struct collisionPatch_t
	{
		// to avoid repeated testings
		size_t checkcount;
		int32_t surfaceFlags;
		int32_t contents;
		uintptr_t shaderNum;
		int32_t subdivisions;
		patchCollide_t pc;

	public:
		collisionPatch_t();
	};

	// a trace is returned when a box is swept through the world
	struct MOHPC_UTILITY_EXPORTS trace_t
	{
		uintptr_t shaderNum;
		// time completed, 1.0 = didn't hit anything
		float fraction;
		// final position
		vec3_t endpos;
		// surface hit
		uint32_t surfaceFlags;
		// contents on other side of surface hit
		uint32_t contents;
		// surface normal at impact, transformed to world space
		collisionPlane_t plane;
		// entity the contacted surface is a part of
		uint16_t entityNum;
		// Bone location when using traceDeep
		uint8_t location;
		// if true, plane is not valid
		bool allsolid;
		// if true, the initial point was in a solid area
		bool startsolid;

	public:
		trace_t();
	};

	// Used for oriented capsule collision detection
	struct sphere_t
	{
		vec3_t offset;
		float radius;
		bool use;

	public:
		sphere_t();
	};

	struct traceWork_t
	{
		vec3_t start;
		vec3_t end;
		// size of the box being swept through the model
		vec3_t size[2];
		// [signbits][x] = either size[0][x] or size[1][x]
		vec3_t offsets[8];
		// longest corner length from origin
		float maxOffset;
		// greatest of abs(size[0]) and abs(size[1])
		vec3_t extents;
		// enclosing box of start and end surrounding by size
		vec3_t bounds[2];
		float height;
		float radius;
		// ored contents of the model tracing through
		uint32_t contents;
		// returned from trace call
		trace_t trace;
		// optimized case
		bool isPoint;

	public:
		traceWork_t();
	};

	struct leafList_t
	{
		size_t count;
		size_t maxcount;
		bool overflowed;
		int* list;
		vec3_t bounds[2];
		int lastLeaf;		// for overflows where each leaf can't be stored individually
		void (CollisionWorld::*storeLeafs)(leafList_t* ll, int nodenum);

	public:
		leafList_t();
	};

	struct pointtrace_t
	{
		traceWork_t* tw;
		const terrainCollide_t* tc;
		vec3_t vStart;
		vec3_t vEnd;
		int i;
		int j;
		float fSurfaceClipEpsilon;

	public:
		pointtrace_t();
	};

	/**
	 * Class used to be able to trace through world full of planes
	 */
	class CollisionWorld
	{
		MOHPC_UTILITY_OBJECT_DECLARATION(CollisionWorld);

	public:
		MOHPC_UTILITY_EXPORTS CollisionWorld();
		~CollisionWorld();

		MOHPC_UTILITY_EXPORTS void clearAll();
		MOHPC_UTILITY_EXPORTS void reserve(
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
			size_t numPatches
		);

		MOHPC_UTILITY_EXPORTS collisionFencemask_t* createFenceMask();
		MOHPC_UTILITY_EXPORTS collisionFencemask_t* getFenceMask(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS collisionFencemask_t* getFenceMasks();
		MOHPC_UTILITY_EXPORTS size_t getNumFenceMasks() const;
		MOHPC_UTILITY_EXPORTS collisionShader_t* createShader();
		MOHPC_UTILITY_EXPORTS collisionShader_t* getShader(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS size_t getNumShaders() const;
		MOHPC_UTILITY_EXPORTS collisionSideEq_t* createSideEquation();
		MOHPC_UTILITY_EXPORTS collisionSideEq_t* getSideEquation(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS collisionSideEq_t* getSideEquations();
		MOHPC_UTILITY_EXPORTS size_t getNumSideEquations() const;
		MOHPC_UTILITY_EXPORTS collisionBrushSide_t* createBrushSide();
		MOHPC_UTILITY_EXPORTS collisionBrushSide_t* getBrushSide(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS collisionBrushSide_t* getBrushSides();
		MOHPC_UTILITY_EXPORTS size_t getNumBrushSides() const;
		MOHPC_UTILITY_EXPORTS collisionPlane_t* createPlane();
		MOHPC_UTILITY_EXPORTS collisionPlane_t* getPlane(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS collisionPlane_t* getPlanes();
		MOHPC_UTILITY_EXPORTS size_t getNumPlanes() const;
		MOHPC_UTILITY_EXPORTS collisionNode_t* createNode();
		MOHPC_UTILITY_EXPORTS collisionNode_t* getNode(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS size_t getNumNodes() const;
		MOHPC_UTILITY_EXPORTS collisionLeaf_t* createLeaf();
		MOHPC_UTILITY_EXPORTS collisionLeaf_t* getLeaf(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS size_t getNumLeafs() const;
		MOHPC_UTILITY_EXPORTS collisionModel_t* createModel();
		MOHPC_UTILITY_EXPORTS collisionModel_t* getModel(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS size_t getNumModels() const;
		MOHPC_UTILITY_EXPORTS collisionBrush_t* createBrush();
		MOHPC_UTILITY_EXPORTS collisionBrush_t* getBrush(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS size_t getNumBrushes() const;
		MOHPC_UTILITY_EXPORTS collisionPatch_t* createPatch();
		MOHPC_UTILITY_EXPORTS collisionPatch_t* getPatch(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS collisionPatch_t* getPatches();
		MOHPC_UTILITY_EXPORTS size_t getNumPatches() const;
		MOHPC_UTILITY_EXPORTS collisionTerrain_t* createTerrain();
		MOHPC_UTILITY_EXPORTS collisionTerrain_t* getTerrain(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS collisionTerrain_t* getTerrains();
		MOHPC_UTILITY_EXPORTS size_t getNumTerrains() const;
		MOHPC_UTILITY_EXPORTS void createLeafTerrain(collisionTerrain_t* terrain);
		MOHPC_UTILITY_EXPORTS size_t getNumLeafTerrains() const;
		MOHPC_UTILITY_EXPORTS collisionTerrain_t* getLeafTerrain(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS void createLeafBrush(uintptr_t num);
		MOHPC_UTILITY_EXPORTS size_t getNumLeafBrushes() const;
		MOHPC_UTILITY_EXPORTS intptr_t getLeafBrush(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS void createLeafSurface(uintptr_t num);
		MOHPC_UTILITY_EXPORTS size_t getNumLeafSurfaces() const;
		MOHPC_UTILITY_EXPORTS intptr_t getLeafSurface(uintptr_t num) const;
		MOHPC_UTILITY_EXPORTS void createSurface(collisionPatch_t* patch);
		MOHPC_UTILITY_EXPORTS collisionPatch_t* getSurface(uintptr_t num);
		MOHPC_UTILITY_EXPORTS size_t getNumSurfaces() const;

	public:
		MOHPC_UTILITY_EXPORTS void ModelBounds(clipHandle_t model, vec3r_t mins, vec3r_t maxs);
		MOHPC_UTILITY_EXPORTS void InitBoxHull();
		MOHPC_UTILITY_EXPORTS int PointContents(const vec3r_t p, clipHandle_t model);
		MOHPC_UTILITY_EXPORTS clipHandle_t inlineModel(uint32_t index);
		MOHPC_UTILITY_EXPORTS clipHandle_t TempBoxModel(const vec3r_t mins, const vec3r_t maxs, int contents);
		MOHPC_UTILITY_EXPORTS uintptr_t PointBrushNum(const vec3r_t p, clipHandle_t model);
		MOHPC_UTILITY_EXPORTS int TransformedPointContents(const vec3r_t p, clipHandle_t model, const vec3r_t origin, const vec3r_t angles);

		MOHPC_UTILITY_EXPORTS bool BoxSightTrace(const vec3r_t start, const vec3r_t end,
			const vec3r_t mins, const vec3r_t maxs,
			clipHandle_t model, uint32_t brushmask, bool cylinder);

		MOHPC_UTILITY_EXPORTS bool TransformedBoxSightTrace(const vec3r_t start, const vec3r_t end,
			const vec3r_t mins, const vec3r_t maxs,
			clipHandle_t model, uint32_t brushmask, const vec3r_t origin, const vec3r_t angles, bool cylinder);

		MOHPC_UTILITY_EXPORTS void BoxTrace(trace_t* results, const vec3r_t start, const vec3r_t end,
			const vec3r_t mins, const vec3r_t maxs,
			clipHandle_t model, uint32_t brushmask, bool cylinder);

		MOHPC_UTILITY_EXPORTS void TransformedBoxTrace(trace_t* results, const vec3r_t start, const vec3r_t end,
			const vec3r_t mins, const vec3r_t maxs,
			clipHandle_t model, uint32_t brushmask,
			const vec3r_t origin, const vec3r_t angles, bool cylinder);

	private:
		size_t BoxBrushes(const vec3r_t mins, const vec3r_t maxs, collisionBrush_t** list, size_t listsize);

		void StoreLeafs(leafList_t* ll, int nodenum);
		void StoreBrushes(leafList_t* ll, int nodenum);

		void BoxLeafnums_r(leafList_t* ll, int nodenum);

		collisionModel_t* ClipHandleToModel(clipHandle_t handle);
		bool BoundsIntersect(const vec3r_t mins, const vec3r_t maxs, const vec3r_t mins2, const vec3r_t maxs2);
		bool BoundsIntersectPoint(const vec3r_t mins, const vec3r_t maxs, const vec3r_t point);

	// Trace
	private:
		void ProjectPointOntoVector(vec3_t point, vec3_t vStart, vec3_t vDir, vec3_t vProj);
		float DistanceFromLineSquared(vec3_t p, vec3_t lp1, vec3_t lp2, vec3_t dir);
		float VectorDistanceSquared(vec3_t p1, vec3_t p2);
		void TestBoxInBrush(traceWork_t* tw, collisionBrush_t* brush);
		void TestInLeaf(traceWork_t* tw, collisionLeaf_t* leaf);
		void TestCapsuleInCapsule(traceWork_t* tw, clipHandle_t model);
		void TestBoundingBoxInCapsule(traceWork_t* tw, clipHandle_t model);
		void TraceThroughSphere(traceWork_t* tw, vec3_t origin, float radius, vec3_t start, vec3_t end);
		void PositionTest(traceWork_t* tw);
		void TraceThroughPatch(traceWork_t* tw, const collisionPatch_t* patch);
		void TraceThroughTerrain(traceWork_t* tw, const collisionTerrain_t* terrain);
		void TraceThroughBrush(traceWork_t* tw, const collisionBrush_t* brush);
		void TraceThroughVerticalCylinder(traceWork_t* tw, vec3_t origin, float radius, float halfheight, vec3_t start, vec3_t end);
		void TraceCapsuleThroughCapsule(traceWork_t* tw, clipHandle_t model);
		void TraceBoundingBoxThroughCapsule(traceWork_t* tw, clipHandle_t model);
		void TraceToLeaf(traceWork_t* tw, collisionLeaf_t* leaf);
		void TraceThroughTree(traceWork_t* tw, int num, float p1f, float p2f, vec3_t p1, vec3_t p2);
		bool TraceThroughFence(const traceWork_t* tw, const collisionBrush_t* brush, const collisionBrushSide_t* side, float fTraceFraction);
		bool SightTraceThroughPatch(traceWork_t* tw, collisionPatch_t* patch);
		bool SightTraceThroughTerrain(traceWork_t* tw, collisionTerrain_t* terrain);
		bool SightTraceThroughBrush(traceWork_t* tw, collisionBrush_t* brush);
		bool SightTraceToLeaf(traceWork_t* tw, collisionLeaf_t* leaf);
		bool SightTraceThroughTree(traceWork_t* tw, int num, float p1f, float p2f, vec3_t p1, vec3_t p2);

	private:
		int PointLeafnum_r(const vec3r_t p, int num);
		int PointLeafnum(const vec3r_t p);
		size_t	BoxLeafnums(const vec3r_t mins, const vec3r_t maxs, int* list, size_t listsize, int* lastLeaf);
		collisionShader_t* ShaderPointer(int iShaderNum);
		uint8_t* ClusterPVS(int cluster);
		void FloodArea_r(int areaNum, int floodnum);
		void FloodAreaConnections(void);
		void AdjustAreaPortalState(int area1, int area2, bool open);
		int AreaForPoint(vec3_t vPos);
		bool AreasConnected(int area1, int area2);
		int WriteAreaBits(uint8_t* buffer, int area);
		bool inPVS(vec3_t p1, vec3_t p2);
		bool LeafInPVS(int leaf1, int leaf2);

	// Terrain
	private:
		void TraceThroughTerrainCollide(traceWork_t* tw, const terrainCollide_t* tc);
		bool PositionTestInTerrainCollide(traceWork_t* tw, const terrainCollide_t* tc);
		bool SightTracePointThroughTerrainCollide(void);
		bool SightTraceThroughTerrainCollide(traceWork_t* tw, const terrainCollide_t* tc);
		float CheckTerrainPlane(const vec4_t plane);
		float CheckTerrainTriSpherePoint(const vec3_t v);
		float CheckTerrainTriSphereCorner(const vec4_t plane, float x0, float y0, int i, int j);
		float CheckTerrainTriSphereEdge(const float* plane, float x0, float y0, int i0, int j0, int i1, int j1);
		float CheckTerrainTriSphere(float x0, float y0, int iPlane);
		bool ValidateTerrainCollidePointSquare(float frac);
		bool ValidateTerrainCollidePointTri(int eMode, float frac);
		bool TestTerrainCollideSquare(void);
		bool CheckStartInsideTerrain(int i, int j, float fx, float fy);
		bool PositionTestPointInTerrainCollide(void);
		void TracePointThroughTerrainCollide(void);
		void TraceCylinderThroughTerrainCollide(traceWork_t* tw, const terrainCollide_t* tc);

	// Patch
	private:
		bool PlaneFromPoints(vec4_t plane, vec3r_t a, vec3r_t b, vec3r_t c);
		void TraceThroughPatchCollide(traceWork_t* tw, const patchCollide_t* pc);
		bool PositionTestInPatchCollide(traceWork_t* tw, const patchCollide_t* pc);
		void ClearLevelPatches(void);
		void TracePointThroughPatchCollide(traceWork_t* tw, const patchCollide_t* pc);
		int CheckFacetPlane(float* plane, vec3_t start, vec3_t end, float* enterFrac, float* leaveFrac, uint32_t* hit);

	private:
		std::vector<collisionFencemask_t> fencemasks;
		std::vector<collisionShader_t> shaders;
		std::vector<collisionSideEq_t> sideequations;
		std::vector<collisionBrushSide_t> brushsides;
		std::vector<collisionPlane_t> planes;
		std::vector<collisionNode_t> nodes;
		std::vector<collisionLeaf_t> leafs;
		std::vector<intptr_t> leafbrushes;
		std::vector<intptr_t> leafsurfaces;
		std::vector<collisionTerrain_t*> leafterrains;
		std::vector<collisionModel_t> cmodels;
		std::vector<collisionBrush_t> brushes;
		std::vector<collisionPatch_t*> surfaces;
		std::vector<collisionTerrain_t> terrain;
		std::vector<collisionPatch_t> patchList;

		// incremented on each trace
		size_t checkcount;

		sphere_t sphere;
		pointtrace_t g_trace;
		size_t c_traces;
		size_t c_patch_traces;
		size_t c_brush_traces;
		size_t c_pointcontents;
	};

	using CollisionWorldPtr = SharedPtr<CollisionWorld>;

	/*
	template<typename Archive>
	Archive& operator<<(Archive& ar, const collisionShader_t& fenceMask)
	{
		return ar;
	}

	template<typename Archive>
	Archive& operator<<(Archive& ar, const collisionFencemask_t& fenceMask)
	{
		return ar;
	}

	template<typename Archive>
	Archive& operator<<(Archive& ar, const collisionLeaf_t& fenceMask)
	{
		return ar;
	}

	template<typename Archive>
	Archive& operator<<(Archive& ar, const collisionPlane_t& fenceMask)
	{
		return ar;
	}

	template<typename Archive>
	Archive& operator<<(Archive& ar, const collisionNode_t& fenceMask)
	{
		return ar;
	}

	template<typename Archive>
	Archive& operator<<(Archive& ar, const collisionTerrain_t& fenceMask)
	{
		return ar;
	}

	template<typename Archive>
	Archive& operator<<(Archive& ar, const collisionArea_t& fenceMask)
	{
		return ar;
	}

	template<typename Archive>
	Archive& operator<<(Archive& ar, const collisionModel_t& fenceMask)
	{
		return ar;
	}

	template<typename Archive>
	Archive& operator<<(Archive& ar, const collisionSideEq_t& fenceMask)
	{
		return ar;
	}

	template<typename Archive>
	Archive& operator<<(Archive& ar, const collisionBrushSide_t& fenceMask)
	{
		return ar;
	}

	template<typename Archive>
	Archive& operator<<(Archive& ar, const collisionBrush_t& fenceMask)
	{
		return ar;
	}

	template<typename Archive>
	Archive& operator<<(Archive& ar, const collisionPatch_t& fenceMask)
	{
		return ar;
	}
	*/
}
