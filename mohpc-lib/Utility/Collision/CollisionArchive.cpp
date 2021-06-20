#include <MOHPC/Utility/Collision/CollisionArchive.h>

using namespace MOHPC;

void CollisionWorldSerializer::save(IArchiveWriter& ar)
{
	const size_t numFenceMasks = cm.getNumFenceMasks();
	const size_t numShaders = cm.getNumShaders();
	const size_t numSideEquations = cm.getNumSideEquations();
	const size_t numBrushSides = cm.getNumBrushSides();
	const size_t numPlanes = cm.getNumPlanes();
	const size_t numNodes = cm.getNumNodes();
	const size_t numLeafs = cm.getNumLeafs();
	const size_t numLeafBrushes = cm.getNumLeafBrushes();
	const size_t numLeafSurfaces = cm.getNumLeafSurfaces();
	const size_t numLeafTerrains = cm.getNumLeafTerrains();
	const size_t numModels = cm.getNumModels();
	const size_t numBrushes = cm.getNumBrushes();
	const size_t numTerrains = cm.getNumTerrains();
	const size_t numPatches = cm.getNumPatches();
	const size_t numSurfaces = cm.getNumSurfaces();

	ar << (uint32_t)numFenceMasks;
	ar << (uint32_t)numShaders;
	ar << (uint32_t)numSideEquations;
	ar << (uint32_t)numBrushSides;
	ar << (uint32_t)numPlanes;
	ar << (uint32_t)numNodes;
	ar << (uint32_t)numLeafs;
	ar << (uint32_t)numLeafBrushes;
	ar << (uint32_t)numLeafSurfaces;
	ar << (uint32_t)numLeafTerrains;
	ar << (uint32_t)numModels;
	ar << (uint32_t)numBrushes;
	ar << (uint32_t)numTerrains;
	ar << (uint32_t)numPatches;
	ar << (uint32_t)numSurfaces;

	for (size_t i = 0; i < numFenceMasks; ++i)
	{
		CollisionMaskSerializer ser(*cm.getFenceMask(i));
		ser.save(ar);
	}

	for (size_t i = 0; i < numShaders; ++i)
	{
		CollisionShaderSerializer ser(*cm.getShader(i));
		ser.save(ar, cm.getFenceMasks());
	}

	for (size_t i = 0; i < numSideEquations; ++i)
	{
		CollisionSideEqSerializer ser(*cm.getSideEquation(i));
		ser.serialize(ar);
	}

	for (size_t i = 0; i < numPlanes; ++i)
	{
		CollisionPlaneSerializer ser(*cm.getPlane(i));
		ser.serialize(ar);
	}

	for (size_t i = 0; i < numBrushSides; ++i)
	{
		CollisionBrushSideSerializer ser(*cm.getBrushSide(i));
		ser.save(ar, cm.getPlanes(), cm.getSideEquations());
	}

	for (size_t i = 0; i < numModels; ++i)
	{
		CollisionModelSerializer ser(*cm.getModel(i));
		ser.serialize(ar);
	}

	for (size_t i = 0; i < numBrushes; ++i)
	{
		CollisionBrushSerializer ser(*cm.getBrush(i));
		ser.save(ar, cm.getBrushSides());
	}

	for (size_t i = 0; i < numTerrains; ++i)
	{
		CollisionTerrainSerializer ser(*cm.getTerrain(i));
		ser.serialize(ar);
	}

	for (size_t i = 0; i < numPatches; ++i)
	{
		CollisionPatchSerializer ser(*cm.getPatch(i));
		ser.save(ar);
	}

	for (size_t i = 0; i < numSurfaces; ++i)
	{
		collisionPatch_t* patch = cm.getSurface(i);

		const uint32_t patchNum = patch ? (uint32_t)(patch - cm.getPatches()) : -1;
		ar << patchNum;
	}

	for (size_t i = 0; i < numNodes; ++i)
	{
		CollisionNodeSerializer ser(*cm.getNode(i));
		ser.save(ar, cm.getPlanes());
	}

	for (size_t i = 0; i < numLeafs; ++i)
	{
		CollisionLeafSerializer ser(*cm.getLeaf(i));
		ser.serialize(ar);
	}

	for (size_t i = 0; i < numLeafBrushes; ++i)
	{
		uint32_t num = (uint32_t)cm.getLeafBrush(i);
		ar << num;
	}

	for (size_t i = 0; i < numLeafSurfaces; ++i)
	{
		uint32_t num = (uint32_t)cm.getLeafSurface(i);
		ar << num;
	}

	for (size_t i = 0; i < numLeafTerrains; ++i)
	{
		collisionTerrain_t* ter = cm.getLeafTerrain(i);

		const uint32_t terrainNum = ter ? (uint32_t)(ter - cm.getTerrains()) : -1;
		ar << terrainNum;
	}
}

void CollisionWorldSerializer::load(IArchiveReader& ar)
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
	uint32_t numPatches = 0;
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
	ar >> numPatches;
	ar >> numSurfaces;

	cm.clearAll();
	cm.reserve(
		numFenceMasks,
		numShaders,
		numSideEquations,
		numBrushSides,
		numPlanes,
		numNodes,
		numLeafs,
		numLeafBrushes,
		numLeafSurfaces,
		numLeafTerrains,
		numCModels,
		numBrushes,
		numSurfaces,
		numTerrains,
		numPatches
	);

	for (size_t i = 0; i < numFenceMasks; ++i)
	{
		CollisionMaskSerializer ser(*cm.createFenceMask());
		ser.load(ar);
	}

	for (size_t i = 0; i < numShaders; ++i)
	{
		CollisionShaderSerializer ser(*cm.createShader());
		ser.load(ar, cm.getFenceMasks());
	}

	for (size_t i = 0; i < numSideEquations; ++i)
	{
		CollisionSideEqSerializer ser(*cm.createSideEquation());
		ser.serialize(ar);
	}

	for (size_t i = 0; i < numPlanes; ++i)
	{
		CollisionPlaneSerializer ser(*cm.createPlane());
		ser.serialize(ar);
	}

	for (size_t i = 0; i < numBrushSides; ++i)
	{
		CollisionBrushSideSerializer ser(*cm.createBrushSide());
		ser.load(ar, cm.getPlanes(), cm.getSideEquations());
	}

	for (size_t i = 0; i < numCModels; ++i)
	{
		CollisionModelSerializer ser(*cm.createModel());
		ser.serialize(ar);
	}

	for (size_t i = 0; i < numBrushes; ++i)
	{
		CollisionBrushSerializer ser(*cm.createBrush());
		ser.load(ar, cm.getBrushSides());
	}

	for (size_t i = 0; i < numTerrains; ++i)
	{
		CollisionTerrainSerializer ser(*cm.createTerrain());
		ser.serialize(ar);
	}

	for (size_t i = 0; i < numPatches; ++i)
	{
		CollisionPatchSerializer ser(*cm.createPatch());
		ser.load(ar);
	}

	for (size_t i = 0; i < numSurfaces; ++i)
	{
		uint32_t patchNum = 0;
		ar >> patchNum;

		if (patchNum != -1) {
			cm.createSurface(cm.getPatch(patchNum));
		}
		else {
			cm.createSurface(nullptr);
		}
	}

	for (size_t i = 0; i < numNodes; ++i)
	{
		CollisionNodeSerializer ser(*cm.createNode());
		ser.load(ar, cm.getPlanes());
	}

	for (size_t i = 0; i < numLeafs; ++i)
	{
		CollisionLeafSerializer ser(*cm.createLeaf());
		ser.serialize(ar);
	}

	for (size_t i = 0; i < numLeafBrushes; ++i)
	{
		uint32_t num = 0;
		ar >> num;
		cm.createLeafBrush(num);
	}

	for (size_t i = 0; i < numLeafSurfaces; ++i)
	{
		uint32_t num = 0;
		ar >> num;
		cm.createLeafSurface(num);
	}

	for (size_t i = 0; i < numLeafTerrains; ++i)
	{
		uint32_t terrainNum = 0;
		ar >> terrainNum;

		if (terrainNum != -1) {
			cm.createLeafTerrain(cm.getTerrain(terrainNum));
		}
		else {
			cm.createLeafTerrain(nullptr);
		}
	}
}

void CollisionPlaneSerializer::serialize(IArchive& ar)
{
	ar(obj.normal[0]);
	ar(obj.normal[1]);
	ar(obj.normal[2]);
	ar(obj.dist);
	ar(obj.typedata);
}

void CollisionNodeSerializer::save(IArchiveWriter& ar, const collisionPlane_t* planeList)
{
	uint32_t planeNum = (uint32_t)(obj.plane - planeList);
	ar << planeNum;
	ar << obj.children[0];
	ar << obj.children[1];
}

void CollisionNodeSerializer::load(IArchiveReader& ar, const collisionPlane_t* planeList)
{
	uint32_t planeNum = 0;
	ar >> planeNum;
	obj.plane = &planeList[planeNum];

	ar >> obj.children[0];
	ar >> obj.children[1];
}

void CollisionLeafSerializer::serialize(IArchive& ar)
{
	ar(obj.cluster);
	ar(obj.area);
	ar(obj.firstLeafBrush);
	ar(obj.numLeafBrushes);
	ar(obj.firstLeafSurface);
	ar(obj.numLeafSurfaces);
	ar(obj.firstLeafTerrain);
	ar(obj.numLeafTerrains);
}

void CollisionMaskSerializer::save(IArchiveWriter& ar)
{
	ar << obj.name;
	ar << obj.iWidth;
	ar << obj.iHeight;
	// No need to save data
}

void CollisionMaskSerializer::load(IArchiveReader& ar)
{
	ar >> obj.name;
	ar >> obj.iWidth;
	ar >> obj.iHeight;
}

void CollisionShaderSerializer::save(IArchiveWriter& ar, const collisionFencemask_t* fencemaskList)
{
	ar << obj.shader;
	ar << obj.surfaceFlags;
	ar << obj.contentFlags;

	if (obj.mask)
	{
		const uint32_t fenceNum = (uint32_t)(obj.mask - fencemaskList);
		ar << fenceNum;
	}
	else {
		const uint32_t fenceNum = -1;
		ar << fenceNum;
	}
}

void CollisionShaderSerializer::load(IArchiveReader& ar, collisionFencemask_t* fencemaskList)
{
	ar >> obj.shader;
	ar >> obj.surfaceFlags;
	ar >> obj.contentFlags;

	uint32_t fenceNum = 0;
	ar >> fenceNum;

	if (fenceNum != -1) {
		obj.mask = &fencemaskList[fenceNum];
	}
}

void TerrainCollideSquareSerializer::serialize(IArchive& ar)
{
	ar(obj.plane[0][0]);
	ar(obj.plane[0][1]);
	ar(obj.plane[0][2]);
	ar(obj.plane[0][3]);
	ar(obj.plane[1][0]);
	ar(obj.plane[1][1]);
	ar(obj.plane[1][2]);
	ar(obj.plane[1][3]);
	ar(obj.eMode);
}

void TerrainCollideSerializer::serialize(IArchive& ar)
{
	ar(obj.vBounds[0][0]);
	ar(obj.vBounds[0][1]);
	ar(obj.vBounds[0][2]);
	ar(obj.vBounds[1][0]);
	ar(obj.vBounds[1][1]);
	ar(obj.vBounds[1][2]);

	for (size_t i = 0; i < 8; ++i)
	{
		for (size_t j = 0; j < 8; ++j)
		{
			TerrainCollideSquareSerializer tcSer(obj.squares[i][j]);
			tcSer.serialize(ar);
		}
	}
}

void CollisionTerrainSerializer::serialize(IArchive& ar)
{
	ar(obj.surfaceFlags);
	ar(obj.contents);
	ar(obj.shaderNum);
	TerrainCollideSerializer tcSer(obj.tc);
	tcSer.serialize(ar);
}

void CollisionAreaSerializer::serialize(IArchive& ar)
{
	ar(obj.floodnum);
	ar(obj.floodvalid);
}

void CollisionModelSerializer::serialize(IArchive& ar)
{
	ar(obj.mins[0]);
	ar(obj.mins[1]);
	ar(obj.mins[2]);
	ar(obj.maxs[0]);
	ar(obj.maxs[1]);
	ar(obj.maxs[2]);
	CollisionLeafSerializer leafSer(obj.leaf);
	leafSer.serialize(ar);
}

void CollisionSideEqSerializer::serialize(IArchive& ar)
{
	ar(obj.fSeq[0]);
	ar(obj.fSeq[1]);
	ar(obj.fSeq[2]);
	ar(obj.fTeq[0]);
	ar(obj.fTeq[1]);
	ar(obj.fTeq[2]);
}

void CollisionBrushSideSerializer::save(IArchiveWriter& ar, collisionPlane_t* planeList, collisionSideEq_t* sideEqList)
{
	ar << obj.surfaceFlags;
	ar << obj.shaderNum;

	const uint32_t planeNum = obj.plane ? (uint32_t)(obj.plane - planeList) : -1;
	ar << planeNum;

	const uint32_t eqNum = obj.pEq ? (uint32_t)(obj.pEq - sideEqList) : -1;
	ar << eqNum;
}

void CollisionBrushSideSerializer::load(IArchiveReader& ar, collisionPlane_t* planeList, collisionSideEq_t* sideEqList)
{
	ar >> obj.surfaceFlags;
	ar >> obj.shaderNum;

	uint32_t planeNum = 0;
	ar >> planeNum;
	if (planeNum != -1) obj.plane = &planeList[planeNum];

	uint32_t eqNum = 0;
	ar >> eqNum;
	if (eqNum != -1) obj.pEq = &sideEqList[eqNum];
}

void CollisionBrushSerializer::serialize(IArchive& ar)
{
	ar(obj.shaderNum);
	ar(obj.contents);
	ar(obj.bounds[0][0]);
	ar(obj.bounds[0][1]);
	ar(obj.bounds[0][2]);
	ar(obj.bounds[1][0]);
	ar(obj.bounds[1][1]);
	ar(obj.bounds[1][2]);
	ar(obj.numsides);
}

void CollisionBrushSerializer::save(IArchiveWriter& ar, collisionBrushSide_t* brushSidesList)
{
	serialize(ar);

	const uint32_t sideNum = obj.sides ? (uint32_t)(obj.sides - brushSidesList) : -1;
	ar << sideNum;
}

void CollisionBrushSerializer::load(IArchiveReader& ar, collisionBrushSide_t* brushSidesList)
{
	serialize(ar);

	uint32_t sideNum = 0;
	ar >> sideNum;

	if (sideNum != -1) obj.sides = &brushSidesList[sideNum];
}

void CollisionPatchPlaneSerializer::serialize(IArchive& ar)
{
	ar(obj.plane[0]);
	ar(obj.plane[1]);
	ar(obj.plane[2]);
	ar(obj.plane[3]);
	ar(obj.signbits);
}

void CollisionFacetSerializer::serialize(IArchive& ar)
{
	ar(obj.surfacePlane);
	ar(obj.numBorders);

	for (uint32_t i = 0; i < obj.numBorders; ++i) {
		ar(obj.borderPlanes[i]);
	}

	for (uint32_t i = 0; i < obj.numBorders; ++i) {
		ar(obj.borderInward[i]);
	}

	for (uint32_t i = 0; i < obj.numBorders; ++i) {
		ar(obj.borderNoAdjust[i]);
	}
}

void CollisionPatchCollideSerializer::serialize(IArchive& ar)
{
	ar(obj.bounds[0][0]);
	ar(obj.bounds[0][1]);
	ar(obj.bounds[0][2]);
	ar(obj.bounds[1][0]);
	ar(obj.bounds[1][1]);
	ar(obj.bounds[1][2]);
	ar(obj.numPlanes);
	ar(obj.numFacets);
}

void CollisionPatchCollideSerializer::save(IArchiveWriter& ar)
{
	serialize(ar);

	for (size_t i = 0; i < obj.numPlanes; ++i)
	{
		CollisionPatchPlaneSerializer planeSer(obj.planes[i]);
		planeSer.serialize(ar);
	}

	for (size_t i = 0; i < obj.numFacets; ++i)
	{
		CollisionFacetSerializer facetSer(obj.facets[i]);
		facetSer.serialize(ar);
	}
}

void CollisionPatchCollideSerializer::load(IArchiveReader& ar)
{
	serialize(ar);

	if (obj.numPlanes)
	{
		obj.planes = new patchPlane_t[obj.numPlanes];

		for (size_t i = 0; i < obj.numPlanes; ++i)
		{
			CollisionPatchPlaneSerializer planeSer(obj.planes[i]);
			planeSer.serialize(ar);
		}
	}

	if (obj.numFacets)
	{
		obj.facets = new facet_t[obj.numFacets];

		for (size_t i = 0; i < obj.numFacets; ++i)
		{
			CollisionFacetSerializer facetSer(obj.facets[i]);
			facetSer.serialize(ar);
		}
	}
}

void CollisionPatchSerializer::save(IArchiveWriter& ar)
{
	ar << obj.surfaceFlags;
	ar << obj.contents;
	ar << obj.shaderNum;
	ar << obj.subdivisions;

	CollisionPatchCollideSerializer patchColSer(obj.pc);
	patchColSer.save(ar);
}

void CollisionPatchSerializer::load(IArchiveReader& ar)
{
	ar >> obj.surfaceFlags;
	ar >> obj.contents;
	ar >> obj.shaderNum;
	ar >> obj.subdivisions;

	CollisionPatchCollideSerializer patchColSer(obj.pc);
	patchColSer.load(ar);
}
