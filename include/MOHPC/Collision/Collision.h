#pragma once

#include <stdint.h>
#include "../Math.h"
#include "../Vector.h"
#include "../Utilities/SharedPtr.h"
#include "../Script/Container.h"
#include "../Script/str.h"

namespace MOHPC
{
	class CollisionWorld;

	using clipHandle_t = unsigned int;

	struct MOHPC_EXPORTS collisionPlane_t
	{
		Vector normal;
		float dist;
		union {
			struct {
				uint8_t type : 2;
				uint8_t signbits : 3;
			};
			uint8_t typedata;
		};

	public:
		collisionPlane_t();

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(normal[0]);
			ar(normal[1]);
			ar(normal[2]);
			ar(dist);
			ar(typedata);
		}
	};

	struct collisionNode_t
	{
		const collisionPlane_t* plane;
		// negative numbers are leafs
		int32_t children[2];

	public:
		collisionNode_t();

		template<typename Archive>
		void save(Archive& ar, const collisionPlane_t* planeList)
		{
			uint32_t planeNum = (uint32_t)(plane - planeList);
			ar << planeNum;
			ar << children[0];
			ar << children[1];
		}

		template<typename Archive>
		void load(Archive& ar, const collisionPlane_t* planeList)
		{
			uint32_t planeNum = 0;
			ar >> planeNum;
			plane = &planeList[planeNum];

			ar >> children[0];
			ar >> children[1];
		}
	};

	struct collisionLeaf_t
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

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(cluster);
			ar(area);
			ar(firstLeafBrush);
			ar(numLeafBrushes);
			ar(firstLeafSurface);
			ar(numLeafSurfaces);
			ar(firstLeafTerrain);
			ar(numLeafTerrains);
		}
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

		template<typename Archive>
		void save(Archive& ar)
		{
			ar << name;
			ar << iWidth;
			ar << iHeight;
			// No need to save data
		}

		template<typename Archive>
		void load(Archive& ar)
		{
			ar >> name;
			ar >> iWidth;
			ar >> iHeight;
		}
	};

	struct collisionShader_t
	{
		str shader;
		uint32_t surfaceFlags;
		uint32_t contentFlags;
		const collisionFencemask_t* mask;

	public:
		collisionShader_t();

		template<typename Archive>
		void save(Archive& ar, const collisionFencemask_t* fencemaskList)
		{
			ar << shader;
			ar << surfaceFlags;
			ar << contentFlags;
			
			if (mask)
			{
				const uint32_t fenceNum = (uint32_t)(mask - fencemaskList);
				ar << fenceNum;
			}
			else {
				const uint32_t fenceNum = -1;
				ar << fenceNum;
			}
		}

		template<typename Archive>
		void load(Archive& ar, collisionFencemask_t* fencemaskList)
		{
			ar >> shader;
			ar >> surfaceFlags;
			ar >> contentFlags;

			uint32_t fenceNum = 0;
			ar >> fenceNum;

			if (fenceNum != -1) {
				mask = &fencemaskList[fenceNum];
			}
		}
	};

	struct terrainCollideSquare_t
	{
		vec4_t plane[2];
		int32_t eMode;

	public:
		terrainCollideSquare_t();

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(plane[0][0]);
			ar(plane[0][1]);
			ar(plane[0][2]);
			ar(plane[0][3]);
			ar(plane[1][0]);
			ar(plane[1][1]);
			ar(plane[1][2]);
			ar(plane[1][3]);
			ar(eMode);
		}
	};

	struct terrainCollide_t
	{
		Vector vBounds[2];
		terrainCollideSquare_t squares[8][8];

	public:
		terrainCollide_t();

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(vBounds[0][0]);
			ar(vBounds[0][1]);
			ar(vBounds[0][2]);
			ar(vBounds[1][0]);
			ar(vBounds[1][1]);
			ar(vBounds[1][2]);

			for (size_t i = 0; i < 8; ++i)
			{
				for (size_t j = 0; j < 8; ++j) {
					squares[i][j].serialize(ar);
				}
			}
		}
	};

	struct collisionTerrain_t
	{
		size_t checkcount;
		int32_t surfaceFlags;
		int32_t contents;
		uintptr_t shaderNum;
		terrainCollide_t tc;

	public:
		collisionTerrain_t();

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(surfaceFlags);
			ar(contents);
			ar(shaderNum);
			tc.serialize(ar);
		}
	};

	struct collisionArea_t
	{
		int32_t floodnum;
		int32_t floodvalid;

	public:
		collisionArea_t();

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(floodnum);
			ar(floodvalid);
		}
	};

	struct collisionModel_t
	{
		Vector mins, maxs;
		// submodels don't reference the main tree
		collisionLeaf_t leaf;

	public:
		collisionModel_t();

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(mins[0]);
			ar(mins[1]);
			ar(mins[2]);
			ar(maxs[0]);
			ar(maxs[1]);
			ar(maxs[2]);
			leaf.serialize(ar);
		}
	};

	struct collisionSideEq_t
	{
		float fSeq[4];
		float fTeq[4];

	public:
		collisionSideEq_t();

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(fSeq[0]);
			ar(fSeq[1]);
			ar(fSeq[2]);
			ar(fTeq[0]);
			ar(fTeq[1]);
			ar(fTeq[2]);
		}
	};

	struct collisionBrushSide_t
	{
		collisionPlane_t* plane;
		int32_t surfaceFlags;
		uintptr_t shaderNum;
		collisionSideEq_t* pEq;

	public:
		collisionBrushSide_t();

		template<typename Archive>
		void save(Archive& ar, collisionPlane_t* planeList, collisionSideEq_t* sideEqList)
		{
			ar << surfaceFlags;
			ar << shaderNum;

			const uint32_t planeNum = plane ? (uint32_t)(plane - planeList) : -1;
			ar << planeNum;

			const uint32_t eqNum = pEq ? (uint32_t)(pEq - sideEqList) : -1;
			ar << eqNum;
		}

		template<typename Archive>
		void load(Archive& ar, collisionPlane_t* planeList, collisionSideEq_t* sideEqList)
		{
			ar >> surfaceFlags;
			ar >> shaderNum;

			uint32_t planeNum = 0;
			ar >> planeNum;
			if(planeNum != -1) plane = &planeList[planeNum];

			uint32_t eqNum = 0;
			ar >> eqNum;
			if (eqNum != -1) pEq = &sideEqList[eqNum];
		}
	};

	struct collisionBrush_t
	{
		// the shader that determined the contents
		uintptr_t shaderNum;
		int32_t contents;
		Vector bounds[2];
		size_t numsides;
		collisionBrushSide_t* sides;
		// to avoid repeated testings
		size_t checkcount;

	public:
		collisionBrush_t();

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(shaderNum);
			ar(contents);
			ar(bounds[0][0]);
			ar(bounds[0][1]);
			ar(bounds[0][2]);
			ar(bounds[1][0]);
			ar(bounds[1][1]);
			ar(bounds[1][2]);
			ar(numsides);
		}

		template<typename Archive>
		void save(Archive& ar, collisionBrushSide_t* brushSidesList)
		{
			serialize(ar);

			const uint32_t sideNum = sides ? (uint32_t)(sides - brushSidesList) : -1;
			ar << sideNum;
		}

		template<typename Archive>
		void load(Archive& ar, collisionBrushSide_t* brushSidesList)
		{
			serialize(ar);

			uint32_t sideNum = 0;
			ar >> sideNum;

			if(sideNum != -1) sides = &brushSidesList[sideNum];
		}
	};

	struct patchPlane_t
	{
		vec4_t plane;
		// signx + (signy<<1) + (signz<<2), used as lookup during collision
		uint8_t signbits;

	public:
		patchPlane_t();

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(plane[0]);
			ar(plane[1]);
			ar(plane[2]);
			ar(plane[3]);
			ar(signbits);
		}
	};

	struct facet_t
	{
		int32_t surfacePlane;
		// 3 or four + 6 axial bevels + 4 or 3 * 4 edge bevels
		int32_t numBorders;
		int32_t borderPlanes[4 + 6 + 16];
		bool borderInward[4 + 6 + 16];
		bool borderNoAdjust[4 + 6 + 16];

	public:
		facet_t();

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(surfacePlane);
			ar(numBorders);

			for (size_t i = 0; i < numBorders; ++i) {
				ar(borderPlanes[i]);
			}

			for (size_t i = 0; i < numBorders; ++i) {
				ar(borderInward[i]);
			}

			for (size_t i = 0; i < numBorders; ++i) {
				ar(borderNoAdjust[i]);
			}
		}
	};

	struct patchCollide_t
	{
		Vector bounds[2];
		// surface planes plus edge planes
		int32_t numPlanes;
		patchPlane_t* planes;
		int32_t numFacets;
		facet_t* facets;

	public:
		patchCollide_t();

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(bounds[0][0]);
			ar(bounds[0][1]);
			ar(bounds[0][2]);
			ar(bounds[1][0]);
			ar(bounds[1][1]);
			ar(bounds[1][2]);
			ar(numPlanes);
			ar(numFacets);
		}

		template<typename Archive>
		void save(Archive& ar)
		{
			serialize(ar);

			for (size_t i = 0; i < numPlanes; ++i)
			{
				planes[i].serialize(ar);
			}

			for (size_t i = 0; i < numFacets; ++i)
			{
				facets[i].serialize(ar);
			}
		}

		template<typename Archive>
		void load(Archive& ar)
		{
			serialize(ar);

			if (numPlanes)
			{
				planes = new patchPlane_t[numPlanes];

				for (size_t i = 0; i < numPlanes; ++i)
				{
					planes[i].serialize(ar);
				}
			}

			if (numFacets)
			{
				facets = new facet_t[numFacets];

				for (size_t i = 0; i < numFacets; ++i)
				{
					facets[i].serialize(ar);
				}
			}
		}
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

		template<typename Archive>
		void save(Archive& ar)
		{
			ar << surfaceFlags;
			ar << contents;
			ar << shaderNum;
			ar << subdivisions;
			pc.save(ar);
		}

		template<typename Archive>
		void load(Archive& ar)
		{
			ar >> surfaceFlags;
			ar >> contents;
			ar >> shaderNum;
			ar >> subdivisions;
			pc.load(ar);
		}
	};

	// a trace is returned when a box is swept through the world
	struct MOHPC_EXPORTS trace_t
	{
		// if true, plane is not valid
		bool allsolid;
		// if true, the initial point was in a solid area
		bool startsolid;
		// time completed, 1.0 = didn't hit anything
		float fraction;
		// final position
		Vector endpos;
		// surface normal at impact, transformed to world space
		collisionPlane_t plane;
		// surface hit
		uint32_t surfaceFlags;
		uintptr_t shaderNum;
		// contents on other side of surface hit
		uint32_t contents;
		// entity the contacted surface is a part of
		uint16_t entityNum;
		// Bone location when using traceDeep
		uint8_t location;

	public:
		trace_t();
	};

	// Used for oriented capsule collision detection
	struct sphere_t
	{
		bool use;
		float radius;
		Vector offset;

	public:
		sphere_t();
	};

	struct traceWork_t
	{
		Vector start;
		Vector end;
		// size of the box being swept through the model
		Vector size[2];
		// [signbits][x] = either size[0][x] or size[1][x]
		Vector offsets[8];
		// longest corner length from origin
		float maxOffset;
		// greatest of abs(size[0]) and abs(size[1])
		Vector extents;
		// enclosing box of start and end surrounding by size
		Vector bounds[2];
		float height;
		float radius;
		// ored contents of the model tracing through
		uint32_t contents;
		// optimized case
		bool isPoint;
		// returned from trace call
		trace_t trace;

	public:
		traceWork_t();
	};

	struct leafList_t
	{
		size_t count;
		size_t maxcount;
		bool overflowed;
		int* list;
		Vector bounds[2];
		int lastLeaf;		// for overflows where each leaf can't be stored individually
		void (CollisionWorld::*storeLeafs)(leafList_t* ll, int nodenum);

	public:
		leafList_t();
	};

	struct pointtrace_t
	{
		traceWork_t* tw;
		terrainCollide_t* tc;
		Vector vStart;
		Vector vEnd;
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
	private:
		Container<collisionFencemask_t> fencemasks;
		Container<collisionShader_t> shaders;
		Container<collisionSideEq_t> sideequations;
		Container<collisionBrushSide_t> brushsides;
		Container<collisionPlane_t> planes;
		Container<collisionNode_t> nodes;
		Container<collisionLeaf_t> leafs;
		Container<intptr_t> leafbrushes;
		Container<intptr_t> leafsurfaces;
		Container<collisionTerrain_t*> leafterrains;
		Container<collisionModel_t> cmodels;
		Container<collisionBrush_t> brushes;
		Container<collisionPatch_t*> surfaces;
		Container<collisionTerrain_t> terrain;
		Container<collisionPatch_t> patchList;

		// incremented on each trace
		size_t checkcount;

		sphere_t sphere;
		pointtrace_t g_trace;
		size_t c_traces;
		size_t c_patch_traces;
		size_t c_brush_traces;
		size_t c_pointcontents;

	public:
		MOHPC_EXPORTS CollisionWorld();
		MOHPC_EXPORTS ~CollisionWorld();

		MOHPC_EXPORTS void clearAll();
		void reserve(
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
		);

		MOHPC_EXPORTS collisionFencemask_t* createFenceMask();
		MOHPC_EXPORTS collisionFencemask_t* getFenceMask(uintptr_t num) const;
		MOHPC_EXPORTS size_t getNumFenceMasks() const;
		MOHPC_EXPORTS collisionShader_t* createShader();
		MOHPC_EXPORTS collisionShader_t* getShader(uintptr_t num) const;
		MOHPC_EXPORTS size_t getNumShaders() const;
		MOHPC_EXPORTS collisionSideEq_t* createSideEquation();
		MOHPC_EXPORTS collisionSideEq_t* getSideEquation(uintptr_t num) const;
		MOHPC_EXPORTS size_t getNumSideEquations() const;
		MOHPC_EXPORTS collisionBrushSide_t* createBrushSide();
		MOHPC_EXPORTS collisionBrushSide_t* getBrushSide(uintptr_t num) const;
		MOHPC_EXPORTS size_t getNumBrushSides() const;
		MOHPC_EXPORTS collisionPlane_t* createPlane();
		MOHPC_EXPORTS collisionPlane_t* getPlane(uintptr_t num) const;
		MOHPC_EXPORTS size_t getNumPlanes() const;
		MOHPC_EXPORTS collisionNode_t* createNode();
		MOHPC_EXPORTS collisionNode_t* getNode(uintptr_t num) const;
		MOHPC_EXPORTS size_t getNumNodes() const;
		MOHPC_EXPORTS collisionLeaf_t* createLeaf();
		MOHPC_EXPORTS collisionLeaf_t* getLeaf(uintptr_t num) const;
		MOHPC_EXPORTS size_t getNumLeafs() const;
		MOHPC_EXPORTS collisionModel_t* createModel();
		MOHPC_EXPORTS collisionModel_t* getModel(uintptr_t num) const;
		MOHPC_EXPORTS size_t getNumModels() const;
		MOHPC_EXPORTS collisionBrush_t* createBrush();
		MOHPC_EXPORTS collisionBrush_t* getBrush(uintptr_t num) const;
		MOHPC_EXPORTS size_t getNumBrushes() const;
		MOHPC_EXPORTS collisionPatch_t* createPatch();
		MOHPC_EXPORTS collisionPatch_t* getPatch(uintptr_t num) const;
		MOHPC_EXPORTS size_t getNumPatches() const;
		MOHPC_EXPORTS collisionTerrain_t* createTerrain();
		MOHPC_EXPORTS collisionTerrain_t* getTerrain(uintptr_t num) const;
		MOHPC_EXPORTS size_t getNumTerrains() const;
		MOHPC_EXPORTS void createLeafTerrain(collisionTerrain_t* terrain);
		MOHPC_EXPORTS size_t getNumLeafTerrains() const;
		MOHPC_EXPORTS collisionTerrain_t* getLeafTerrain(uintptr_t num) const;
		MOHPC_EXPORTS void createLeafBrush(uintptr_t num);
		MOHPC_EXPORTS size_t getNumLeafBrushes() const;
		MOHPC_EXPORTS collisionBrush_t* getLeafBrush(uintptr_t num) const;
		MOHPC_EXPORTS void createLeafSurface(uintptr_t num);
		MOHPC_EXPORTS size_t getNumLeafSurfaces() const;
		MOHPC_EXPORTS collisionPatch_t* getLeafSurface(uintptr_t num) const;
		MOHPC_EXPORTS void createSurface(collisionPatch_t* patch);

		template<typename Archive>
		void save(Archive& ar)
		{
			ar << (uint32_t)fencemasks.NumObjects();
			ar << (uint32_t)shaders.NumObjects();
			ar << (uint32_t)sideequations.NumObjects();
			ar << (uint32_t)brushsides.NumObjects();
			ar << (uint32_t)planes.NumObjects();
			ar << (uint32_t)nodes.NumObjects();
			ar << (uint32_t)leafs.NumObjects();
			ar << (uint32_t)leafbrushes.NumObjects();
			ar << (uint32_t)leafsurfaces.NumObjects();
			ar << (uint32_t)leafterrains.NumObjects();
			ar << (uint32_t)cmodels.NumObjects();
			ar << (uint32_t)brushes.NumObjects();
			ar << (uint32_t)terrain.NumObjects();
			ar << (uint32_t)patchList.NumObjects();
			ar << (uint32_t)surfaces.NumObjects();

			for (size_t i = 0; i < fencemasks.NumObjects(); ++i) fencemasks[i].save(ar);
			for (size_t i = 0; i < shaders.NumObjects(); ++i) shaders[i].save(ar, fencemasks.Data());
			for (size_t i = 0; i < sideequations.NumObjects(); ++i) sideequations[i].serialize(ar);
			for (size_t i = 0; i < brushsides.NumObjects(); ++i) brushsides[i].save(ar, planes.Data(), sideequations.Data());
			for (size_t i = 0; i < planes.NumObjects(); ++i) planes[i].serialize(ar);
			for (size_t i = 0; i < nodes.NumObjects(); ++i) nodes[i].save(ar, planes.Data());
			for (size_t i = 0; i < leafs.NumObjects(); ++i) leafs[i].serialize(ar);
			for (size_t i = 0; i < leafbrushes.NumObjects(); ++i) { uint32_t num = (uint32_t)leafbrushes[i]; ar << num; }
			for (size_t i = 0; i < leafsurfaces.NumObjects(); ++i) { uint32_t num = (uint32_t)leafsurfaces[i]; ar << num; }

			for (size_t i = 0; i < leafterrains.NumObjects(); ++i)
			{
				collisionTerrain_t* ter = leafterrains[i];

				const uint32_t terrainNum = ter ? (uint32_t)(ter - terrain.Data()) : -1;
				ar << terrainNum;
			}

			for (size_t i = 0; i < cmodels.NumObjects(); ++i) cmodels[i].serialize(ar);
			for (size_t i = 0; i < brushes.NumObjects(); ++i) brushes[i].save(ar, brushsides.Data());
			for (size_t i = 0; i < terrain.NumObjects(); ++i) terrain[i].serialize(ar);
			for (size_t i = 0; i < patchList.NumObjects(); ++i) patchList[i].save(ar);
			for (size_t i = 0; i < surfaces.NumObjects(); ++i)
			{
				collisionPatch_t* patch = surfaces[i];

				const uint32_t patchNum = patch ? (uint32_t)(patch - patchList.Data()) : -1;
				ar << patchNum;
			}
		}

		template<typename Archive>
		void load(Archive& ar)
		{
			uint32_t numFenceMasks = 0;
			uint32_t numShaders = 0;
			uint32_t numSideEquations = 0;
			uint32_t numBrushSides = 0;
			uint32_t numPlanes = 0;
			uint32_t numNodes = 0;
			uint32_t numLeafs = 0;
			uint32_t numLeafBrushes = 0;
			uint32_t numLeafSurfaces = 0;
			uint32_t numLeafTerrains = 0;
			uint32_t numCModels = 0;
			uint32_t numBrushes = 0;
			uint32_t numTerrains = 0;
			uint32_t numPatchList = 0;
			uint32_t numSurfaces = 0;

			ar >> numFenceMasks;
			ar >> numShaders;
			ar >> numSideEquations;
			ar >> numBrushSides;
			ar >> numPlanes;
			ar >> numNodes;
			ar >> numLeafs;
			ar >> numLeafBrushes;
			ar >> numLeafSurfaces;
			ar >> numLeafTerrains;
			ar >> numCModels;
			ar >> numBrushes;
			ar >> numTerrains;
			ar >> numPatchList;
			ar >> numSurfaces;

			clearAll();
			fencemasks.SetNumObjects(numFenceMasks);
			shaders.SetNumObjects(numShaders);
			sideequations.SetNumObjects(numSideEquations);
			brushsides.SetNumObjects(numBrushSides);
			planes.SetNumObjects(numPlanes);
			nodes.SetNumObjects(numNodes);
			leafs.SetNumObjects(numLeafs);
			leafbrushes.SetNumObjects(numLeafBrushes);
			leafsurfaces.SetNumObjects(numLeafSurfaces);
			leafterrains.SetNumObjects(numLeafTerrains);
			cmodels.SetNumObjects(numCModels);
			brushes.SetNumObjects(numBrushes);
			terrain.SetNumObjects(numTerrains);
			patchList.SetNumObjects(numPatchList);
			surfaces.SetNumObjects(numSurfaces);

			for (size_t i = 0; i < numFenceMasks; ++i) fencemasks[i].load(ar);
			for (size_t i = 0; i < numShaders; ++i) shaders[i].load(ar, fencemasks.Data());
			for (size_t i = 0; i < numSideEquations; ++i) sideequations[i].serialize(ar);
			for (size_t i = 0; i < numBrushSides; ++i) brushsides[i].load(ar, planes.Data(), sideequations.Data());
			for (size_t i = 0; i < numPlanes; ++i) planes[i].serialize(ar);
			for (size_t i = 0; i < numNodes; ++i) nodes[i].load(ar, planes.Data());
			for (size_t i = 0; i < numLeafs; ++i) leafs[i].serialize(ar);
			for (size_t i = 0; i < numLeafBrushes; ++i) { uint32_t num = 0; ar >> num; leafbrushes[i] = num; }
			for (size_t i = 0; i < numLeafSurfaces; ++i) { uint32_t num = 0; ar >> num; leafsurfaces[i] = num; }

			for (size_t i = 0; i < numLeafTerrains; ++i)
			{
				uint32_t terrainNum = 0;
				ar >> terrainNum;

				if (terrainNum != -1) {
					leafterrains[i] = &terrain[terrainNum];
				}
				else {
					leafterrains[i] = nullptr;
				}
			}

			for (size_t i = 0; i < numCModels; ++i) cmodels[i].serialize(ar);
			for (size_t i = 0; i < numBrushes; ++i) brushes[i].load(ar, brushsides.Data());
			for (size_t i = 0; i < numTerrains; ++i) terrain[i].serialize(ar);
			for (size_t i = 0; i < numPatchList; ++i) patchList[i].load(ar);
			for (size_t i = 0; i < numSurfaces; ++i)
			{
				uint32_t patchNum = 0;
				ar >> patchNum;

				if (patchNum != -1) {
					surfaces[i] = &patchList[patchNum];
				}
				else {
					surfaces[i] = nullptr;
				}
			}
		}

	public:
		MOHPC_EXPORTS void CM_ModelBounds(clipHandle_t model, Vector& mins, Vector& maxs);
		MOHPC_EXPORTS void CM_InitBoxHull();
		MOHPC_EXPORTS int CM_PointContents(const vec3_t p, clipHandle_t model);
		MOHPC_EXPORTS clipHandle_t inlineModel(uint32_t index);
		MOHPC_EXPORTS clipHandle_t CM_TempBoxModel(const vec3_t mins, const vec3_t maxs, int contents);
		MOHPC_EXPORTS uintptr_t CM_PointBrushNum(const Vector& p, clipHandle_t model);
		MOHPC_EXPORTS int CM_TransformedPointContents(const Vector& p, clipHandle_t model, const Vector& origin, const Vector& angles);

		MOHPC_EXPORTS bool CM_BoxSightTrace(const Vector& start, const Vector& end,
			const Vector& mins, const Vector& maxs,
			clipHandle_t model, uint32_t brushmask, bool cylinder);

		MOHPC_EXPORTS bool CM_TransformedBoxSightTrace(const Vector& start, const Vector& end,
			const Vector& mins, const Vector& maxs,
			clipHandle_t model, uint32_t brushmask, const Vector& origin, const Vector& angles, bool cylinder);

		MOHPC_EXPORTS void CM_BoxTrace(trace_t* results, const Vector& start, const Vector& end,
			const Vector& mins, const Vector& maxs,
			clipHandle_t model, uint32_t brushmask, bool cylinder);

		MOHPC_EXPORTS void CM_TransformedBoxTrace(trace_t* results, const Vector& start, const Vector& end,
			const Vector& mins, const Vector& maxs,
			clipHandle_t model, uint32_t brushmask,
			const Vector& origin, const Vector& angles, bool cylinder);

	private:
		size_t CM_BoxBrushes(const Vector& mins, const Vector& maxs, collisionBrush_t** list, size_t listsize);

		void CM_StoreLeafs(leafList_t* ll, int nodenum);
		void CM_StoreBrushes(leafList_t* ll, int nodenum);

		void CM_BoxLeafnums_r(leafList_t* ll, int nodenum);

		collisionModel_t* CM_ClipHandleToModel(clipHandle_t handle);
		bool CM_BoundsIntersect(const Vector& mins, const Vector& maxs, const Vector& mins2, const Vector& maxs2);
		bool CM_BoundsIntersectPoint(const Vector& mins, const Vector& maxs, const Vector& point);

	// Trace
	private:
		void CM_ProjectPointOntoVector(vec3_t point, vec3_t vStart, vec3_t vDir, vec3_t vProj);
		float CM_DistanceFromLineSquared(vec3_t p, vec3_t lp1, vec3_t lp2, vec3_t dir);
		float CM_VectorDistanceSquared(vec3_t p1, vec3_t p2);
		void CM_TestBoxInBrush(traceWork_t* tw, collisionBrush_t* brush);
		void CM_TestInLeaf(traceWork_t* tw, collisionLeaf_t* leaf);
		void CM_TestCapsuleInCapsule(traceWork_t* tw, clipHandle_t model);
		void CM_TestBoundingBoxInCapsule(traceWork_t* tw, clipHandle_t model);
		void CM_TraceThroughSphere(traceWork_t* tw, vec3_t origin, float radius, vec3_t start, vec3_t end);
		void CM_PositionTest(traceWork_t* tw);
		void CM_TraceThroughPatch(traceWork_t* tw, collisionPatch_t* patch);
		void CM_TraceThroughTerrain(traceWork_t* tw, collisionTerrain_t* terrain);
		void CM_TraceThroughBrush(traceWork_t* tw, collisionBrush_t* brush);
		void CM_TraceThroughVerticalCylinder(traceWork_t* tw, vec3_t origin, float radius, float halfheight, vec3_t start, vec3_t end);
		void CM_TraceCapsuleThroughCapsule(traceWork_t* tw, clipHandle_t model);
		void CM_TraceBoundingBoxThroughCapsule(traceWork_t* tw, clipHandle_t model);
		void CM_TraceToLeaf(traceWork_t* tw, collisionLeaf_t* leaf);
		void CM_TraceThroughTree(traceWork_t* tw, int num, float p1f, float p2f, vec3_t p1, vec3_t p2);
		bool CM_TraceThroughFence(traceWork_t* tw, const collisionBrush_t* brush, const collisionBrushSide_t* side, float fTraceFraction);
		bool CM_SightTraceThroughPatch(traceWork_t* tw, collisionPatch_t* patch);
		bool CM_SightTraceThroughTerrain(traceWork_t* tw, collisionTerrain_t* terrain);
		bool CM_SightTraceThroughBrush(traceWork_t* tw, collisionBrush_t* brush);
		bool CM_SightTraceToLeaf(traceWork_t* tw, collisionLeaf_t* leaf);
		bool CM_SightTraceThroughTree(traceWork_t* tw, int num, float p1f, float p2f, vec3_t p1, vec3_t p2);

	private:
		int CM_PointLeafnum_r(const Vector& p, int num);
		int CM_PointLeafnum(const Vector& p);
		size_t	CM_BoxLeafnums(const Vector& mins, const Vector& maxs, int* list, size_t listsize, int* lastLeaf);
		collisionShader_t* CM_ShaderPointer(int iShaderNum);
		uintptr_t CM_PointBrushNum(const vec3_t p, clipHandle_t model);
		int	CM_TransformedPointContents(const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles);
		uint8_t* CM_ClusterPVS(int cluster);
		void CM_FloodArea_r(int areaNum, int floodnum);
		void CM_FloodAreaConnections(void);
		void CM_AdjustAreaPortalState(int area1, int area2, bool open);
		int CM_AreaForPoint(vec3_t vPos);
		bool CM_AreasConnected(int area1, int area2);
		int CM_WriteAreaBits(uint8_t* buffer, int area);
		bool CM_inPVS(vec3_t p1, vec3_t p2);
		bool CM_LeafInPVS(int leaf1, int leaf2);

	// Terrain
	private:
		void CM_TraceThroughTerrainCollide(traceWork_t* tw, terrainCollide_t* tc);
		bool CM_PositionTestInTerrainCollide(traceWork_t* tw, terrainCollide_t* tc);
		bool CM_SightTracePointThroughTerrainCollide(void);
		bool CM_SightTraceThroughTerrainCollide(traceWork_t* tw, terrainCollide_t* tc);
		float CM_CheckTerrainPlane(vec4_t plane);
		float CM_CheckTerrainTriSpherePoint(vec3_t v);
		float CM_CheckTerrainTriSphereCorner(vec4_t plane, float x0, float y0, int i, int j);
		float CM_CheckTerrainTriSphereEdge(float* plane, float x0, float y0, int i0, int j0, int i1, int j1);
		float CM_CheckTerrainTriSphere(float x0, float y0, int iPlane);
		bool CM_ValidateTerrainCollidePointSquare(float frac);
		bool CM_ValidateTerrainCollidePointTri(int eMode, float frac);
		bool CM_TestTerrainCollideSquare(void);
		bool CM_CheckStartInsideTerrain(int i, int j, float fx, float fy);
		bool CM_PositionTestPointInTerrainCollide(void);
		void CM_TracePointThroughTerrainCollide(void);
		void CM_TraceCylinderThroughTerrainCollide(traceWork_t* tw, const terrainCollide_t* tc);

	// Patch
	private:
		bool CM_PlaneFromPoints(vec4_t plane, Vector& a, Vector& b, Vector& c);
		void CM_TraceThroughPatchCollide(traceWork_t* tw, const patchCollide_t* pc);
		bool CM_PositionTestInPatchCollide(traceWork_t* tw, const patchCollide_t* pc);
		void CM_ClearLevelPatches(void);
		void CM_TracePointThroughPatchCollide(traceWork_t* tw, const patchCollide_t* pc);
		int CM_CheckFacetPlane(float* plane, vec3_t start, vec3_t end, float* enterFrac, float* leaveFrac, int* hit);
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

	inline MOHPC::sphere_t::sphere_t()
		: use(false)
		, radius(0.f)
	{
	}

	inline MOHPC::trace_t::trace_t()
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

	inline MOHPC::pointtrace_t::pointtrace_t()
		: tw(nullptr)
		, tc(nullptr)
		, i(0)
		, j(0)
		, fSurfaceClipEpsilon(0.f)
	{
	}

	inline MOHPC::collisionPlane_t::collisionPlane_t()
		: dist(0.f)
		, type(0)
		, signbits(0)
	{
	}

	inline MOHPC::collisionNode_t::collisionNode_t()
		: plane(nullptr)
		, children{ 0 }
	{
	}

	inline MOHPC::collisionLeaf_t::collisionLeaf_t()
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

	inline MOHPC::collisionFencemask_t::collisionFencemask_t()
		: iWidth(0)
		, iHeight(0)
		, pData(nullptr)
		, pNext(nullptr)
	{
	}

	inline MOHPC::collisionShader_t::collisionShader_t()
		: surfaceFlags(0)
		, contentFlags(0)
		, mask(nullptr)
	{
	}

	inline MOHPC::terrainCollideSquare_t::terrainCollideSquare_t()
		: eMode(0)
	{
	}

	inline MOHPC::terrainCollide_t::terrainCollide_t()
	{
	}

	inline MOHPC::collisionTerrain_t::collisionTerrain_t()
		: checkcount(0)
		, surfaceFlags(0)
		, contents(0)
		, shaderNum(0)
	{
	}

	inline MOHPC::collisionArea_t::collisionArea_t()
		: floodnum(0)
		, floodvalid(0)
	{
	}

	inline MOHPC::collisionModel_t::collisionModel_t()
	{
	}

	inline MOHPC::collisionSideEq_t::collisionSideEq_t()
		: fSeq{ 0 }
		, fTeq{ 0 }
	{
	}

	inline MOHPC::collisionBrushSide_t::collisionBrushSide_t()
		: plane(nullptr)
		, surfaceFlags(0)
		, shaderNum(0)
		, pEq(nullptr)
	{
	}

	inline MOHPC::collisionBrush_t::collisionBrush_t()
		: shaderNum(0)
		, contents(0)
		, numsides(0)
		, sides(nullptr)
		, checkcount(0)
	{
	}

	inline MOHPC::patchPlane_t::patchPlane_t()
		: plane{ 0 }
		, signbits(0)
	{
	}

	inline MOHPC::facet_t::facet_t()
		: surfacePlane(0)
		, numBorders(0)
		, borderPlanes{ 0 }
		, borderInward{ false }
		, borderNoAdjust{ false }
	{
	}

	inline MOHPC::patchCollide_t::patchCollide_t()
		: numPlanes(0)
		, planes(nullptr)
		, numFacets(0)
		, facets(nullptr)
	{
	}

	inline MOHPC::collisionPatch_t::collisionPatch_t()
		: checkcount(0)
		, surfaceFlags(0)
		, contents(0)
		, shaderNum(0)
		, subdivisions(0)
	{
	}

	inline MOHPC::traceWork_t::traceWork_t()
		: maxOffset(0.f)
		, height(0.f)
		, radius(0.f)
		, contents(0)
		, isPoint(false)
	{
	}

	inline MOHPC::leafList_t::leafList_t()
		: count(0)
		, maxcount(0)
		, overflowed(false)
		, list(nullptr)
		, lastLeaf(0)
		, storeLeafs(nullptr)
	{
	}
}
