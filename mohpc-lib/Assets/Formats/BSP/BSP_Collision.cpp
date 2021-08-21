#include <MOHPC/Assets/Formats/BSP_Collision.h>
#include <MOHPC/Assets/Formats/BSP.h>
#include <MOHPC/Utility/Collision/Collision.h>
#include "../../../Common/VectorPrivate.h"

using namespace MOHPC;

constexpr uint8_t modeTable[] =
{
	2,
	2,
	5,
	6,
	4,
	3,
	0,
	0
};

MOHPC_OBJECT_DEFINITION(BSPCollision);
BSPCollision::BSPCollision(const SharedPtr<BSP>& bspAsset)
	: bsp(bspAsset)
{
	TR_PrepareGerrainCollide();
}

BSPCollision::~BSPCollision()
{
}

void BSPCollision::FillCollisionWorld(CollisionWorld& cm)
{
	const size_t numShaders = bsp->GetNumShaders();
	const size_t numSideEquations = bsp->GetNumSideEquations();
	const size_t numBrushSides = bsp->GetNumBrushSides();
	const size_t numPlanes = bsp->GetNumPlanes();
	const size_t numNodes = bsp->GetNumNodes();
	const size_t numLeafs = bsp->GetNumLeafs();
	const size_t numLeafBrushes = bsp->GetNumLeafBrushes();
	const size_t numLeafSurfaces = bsp->GetNumLeafSurfaces();
	const size_t numLeafTerrains = bsp->GetNumLeafTerrains();
	const size_t numSubmodels = bsp->GetNumSubmodels();
	const size_t numBrushes = bsp->GetNumBrushes();
	const size_t numSurfaces = bsp->GetNumSurfaces();
	const size_t numTerrainPatches = bsp->GetNumTerrainPatches();

	// Pre-allocate data
	cm.reserve(
		0,
		numShaders,
		numSideEquations,
		numBrushSides,
		numPlanes,
		numNodes,
		numLeafs,
		numLeafBrushes,
		numLeafSurfaces,
		numLeafTerrains,
		numSubmodels,
		numBrushes,
		numSurfaces,
		numTerrainPatches,
		numSurfaces
	);

	// Put shaders
	for (size_t i = 0; i < numShaders; ++i)
	{
		const BSPData::Shader* shader = bsp->GetShader(i);

		collisionShader_t* colShader = cm.createShader();
		colShader->shader = shader->shaderName;
		colShader->contentFlags = shader->contentFlags;
		colShader->surfaceFlags = shader->surfaceFlags;
	}

	// Put side equations
	for (size_t i = 0; i < numSideEquations; ++i)
	{
		const BSPData::SideEquation* sideEq = bsp->GetSideEquation(i);

		collisionSideEq_t* colSideEq = cm.createSideEquation();
		memcpy(colSideEq->fSeq, sideEq->sEq, sizeof(colSideEq->fSeq));
		memcpy(colSideEq->fTeq, sideEq->tEq, sizeof(colSideEq->fTeq));
	}

	// Put planes
	for (size_t i = 0; i < numPlanes; ++i)
	{
		const BSPData::Plane* plane = bsp->GetPlane(i);

		collisionPlane_t* colPlane = cm.createPlane();
		VecCopy(plane->normal, colPlane->normal);
		colPlane->dist = plane->distance;
		colPlane->signbits = plane->signBits;
		colPlane->type = plane->type;
	}

	// Put brushsides
	for (size_t i = 0; i < numBrushSides; ++i)
	{
		const BSPData::BrushSide* bside = bsp->GetBrushSide(i);

		collisionBrushSide_t* colBside = cm.createBrushSide();
		colBside->pEq = bside->Eq ? cm.getSideEquation(bside->Eq - bsp->getSideEquations().data()) : nullptr;
		colBside->plane = cm.getPlane(bside->plane - bsp->getPlanes().data());
		colBside->shaderNum = bside->shader - bsp->getShaders().data();
		colBside->surfaceFlags = bside->surfaceFlags;
	}

	// Put nodes
	for (size_t i = 0; i < numNodes; ++i)
	{
		const BSPData::Node* node = bsp->GetNode(i);

		collisionNode_t* colNode = cm.createNode();
		colNode->plane = cm.getPlane(node->plane - bsp->getPlanes().data());
		colNode->children[0] = node->children[0];
		colNode->children[1] = node->children[1];
	}

	// Put brushes
	for (size_t i = 0; i < numBrushes; ++i)
	{
		const BSPData::Brush* brush = bsp->GetBrush(i);

		collisionBrush_t* colBrush = cm.createBrush();
		colBrush->shaderNum = brush->shader - bsp->getShaders().data();
		colBrush->numsides = brush->numsides;
		colBrush->sides = cm.getBrushSide(brush->sides - bsp->getBrushSides().data());
		VectorCopy(brush->bounds[0], colBrush->bounds[0]);
		VectorCopy(brush->bounds[1], colBrush->bounds[1]);
		colBrush->contents = brush->contents;
	}

	// Put patches
	for (size_t i = 0; i < numSurfaces; ++i)
	{
		const BSPData::Surface* patch = bsp->GetSurface(i);

		if (patch->IsPatch())
		{
			const BSPData::Shader* shader = patch->GetShader();

			collisionPatch_t* colPatch = cm.createPatch();
			colPatch->shaderNum = shader - bsp->getShaders().data();
			colPatch->surfaceFlags = shader->surfaceFlags;
			colPatch->contents = shader->contentFlags;
			colPatch->subdivisions = shader->subdivisions;

			const BSPData::PatchCollide* pc = patch->GetPatchCollide();

			VectorCopy(pc->bounds[0], colPatch->pc.bounds[0]);
			VectorCopy(pc->bounds[1], colPatch->pc.bounds[1]);
			colPatch->pc.numFacets = pc->numFacets;
			colPatch->pc.facets = new facet_t[pc->numFacets];
			colPatch->pc.numPlanes = pc->numPlanes;
			colPatch->pc.planes = new patchPlane_t[pc->numPlanes];

			// Fill facets
			for (size_t j = 0; j < colPatch->pc.numFacets; ++j)
			{
				const BSPData::Facet& facet = pc->facets[j];
				facet_t& colFacet = colPatch->pc.facets[j];

				const size_t maxBorders = sizeof(colFacet.borderInward) / sizeof(colFacet.borderInward[0]);
				for (size_t k = 0; k < maxBorders; k++)
				{
					colFacet.borderInward[k] = facet.borderInward[k];
					colFacet.borderNoAdjust[k] = facet.borderNoAdjust[k];
					colFacet.borderPlanes[k] = facet.borderPlanes[k];
				}

				colFacet.surfacePlane = facet.surfacePlane;
				colFacet.numBorders = facet.numBorders;
			}

			// Fill planes
			for (size_t j = 0; j < colPatch->pc.numPlanes; ++j)
			{
				const BSPData::PatchPlane& plane = pc->planes[j];
				patchPlane_t& colPlane = colPatch->pc.planes[j];

				for (size_t k = 0; k < 4; ++k) colPlane.plane[k] = plane.plane[k];
				colPlane.signbits = plane.signbits;
			}

			cm.createSurface(colPatch);
		}
		else {
			cm.createSurface(nullptr);
		}
	}

	// Put terrains
	for (size_t i = 0; i < numTerrainPatches; ++i)
	{
		const BSPData::TerrainPatch* terrain = bsp->GetTerrainPatch(i);
		const BSPData::Shader* shader = terrain->shader;

		BSPData::TerrainCollide collision;
		GenerateTerrainCollide(terrain, collision);

		collisionTerrain_t* colTerrain = cm.createTerrain();
		VectorCopy(collision.vBounds[0], colTerrain->tc.vBounds[0]);
		VectorCopy(collision.vBounds[1], colTerrain->tc.vBounds[1]);
		memcpy(colTerrain->tc.squares, collision.squares, sizeof(colTerrain->tc.squares));
		colTerrain->contents = shader->contentFlags;
		colTerrain->shaderNum = shader - bsp->getShaders().data();
		colTerrain->surfaceFlags = shader->surfaceFlags;
	}

	// Put leafs
	for (size_t i = 0; i < numLeafs; ++i)
	{
		const BSPData::Leaf* leaf = bsp->GetLeaf(i);

		collisionLeaf_t* colLeaf = cm.createLeaf();
		colLeaf->area = leaf->area;
		colLeaf->cluster = leaf->cluster;
		colLeaf->firstLeafBrush = (uint32_t)leaf->firstLeafBrush;
		colLeaf->firstLeafSurface = (uint32_t)leaf->firstLeafSurface;
		colLeaf->firstLeafTerrain = (uint32_t)leaf->firstLeafTerrain;
		colLeaf->numLeafBrushes = (uint32_t)leaf->numLeafBrushes;
		colLeaf->numLeafSurfaces = (uint32_t)leaf->numLeafSurfaces;
		colLeaf->numLeafTerrains = (uint32_t)leaf->numLeafTerrains;
	}

	// Put leaf brushes
	for (size_t i = 0; i < numLeafBrushes; ++i)
	{
		const uintptr_t leafNum = bsp->GetLeafBrush(i);

		cm.createLeafBrush(leafNum);
	}

	// Put leaf surfaces
	for (size_t i = 0; i < numLeafSurfaces; ++i)
	{
		const uintptr_t leafNum = bsp->GetLeafSurface(i);

		cm.createLeafSurface(leafNum);
	}

	// Put leaf terrains
	for (size_t i = 0; i < numLeafTerrains; ++i)
	{
		const BSPData::TerrainPatch* leafTerrain = bsp->GetLeafTerrain(i);

		cm.createLeafTerrain(cm.getTerrain(leafTerrain - bsp->getTerrainPatches().data()));
	}

	// Put brushmodels
	for (size_t i = 0; i < numSubmodels; ++i)
	{
		const BSPData::Model* bmodel = bsp->GetSubmodel(i);

		collisionModel_t* colModel = cm.createModel();
		VectorCopy(bmodel->bounds[0], colModel->mins);
		VectorCopy(bmodel->bounds[1], colModel->maxs);
		colModel->leaf.area = bmodel->leaf.area;
		colModel->leaf.cluster = bmodel->leaf.cluster;
		colModel->leaf.firstLeafBrush = (uint32_t)bmodel->leaf.firstLeafBrush;
		colModel->leaf.firstLeafSurface = (uint32_t)bmodel->leaf.firstLeafSurface;
		colModel->leaf.firstLeafTerrain = (uint32_t)bmodel->leaf.firstLeafTerrain;
		colModel->leaf.numLeafBrushes = (uint32_t)bmodel->leaf.numLeafBrushes;
		colModel->leaf.numLeafSurfaces = (uint32_t)bmodel->leaf.numLeafSurfaces;
		colModel->leaf.numLeafTerrains = (uint32_t)bmodel->leaf.numLeafTerrains;
	}
}

void BSPCollision::TR_CalculateTerrainIndices(struct worknode_t* worknode, int iDiagonal, int iTree)
{
	int i;
	int i2;
	int j2;
	BSPData::varnodeIndex* vni;

	for (i = 0; i <= 30; i++)
	{
		i2 = worknode[i + 1].i0 + worknode[i + 1].i1;
		j2 = worknode[i + 1].j0 + worknode[i + 1].j1;
		worknode[i * 2 + 2].i0 = worknode[i + 1].i1;
		worknode[i * 2 + 2].j0 = worknode[i + 1].j1;
		worknode[i * 2 + 2].i1 = worknode[i + 1].i2;
		worknode[i * 2 + 2].j1 = worknode[i + 1].j2;
		worknode[i * 2 + 2].i2 = i2 >> 1;
		worknode[i * 2 + 2].j2 = j2 >> 1;
		worknode[i * 2 + 2 + 1].i0 = worknode[i + 1].i2;
		worknode[i * 2 + 2 + 1].j0 = worknode[i + 1].j2;
		worknode[i * 2 + 2 + 1].i1 = worknode[i + 1].i0;
		worknode[i * 2 + 2 + 1].j1 = worknode[i + 1].j0;
		worknode[i * 2 + 2 + 1].i2 = i2 >> 1;
		worknode[i * 2 + 2 + 1].j2 = j2 >> 1;
	}

	for (i = 32; i < 64; i++)
	{
		i2 = (worknode[i].i0 + worknode[i].i1) >> 1;
		j2 = (worknode[i].j0 + worknode[i].j1) >> 1;

		if (worknode[i].i0 == worknode[i].i1)
		{
			if (worknode[i].j0 <= worknode[i].j1)
			{
				vni = &varnodeIndexes[iDiagonal][i2][j2][1];
				vni->iNode = i - 1;
				vni->iTreeAndMask = iTree | 0x2000;

				vni = &varnodeIndexes[iDiagonal][i2][j2 - 1][0];
				vni->iNode = i - 1;
				vni->iTreeAndMask = iTree | 0x1000;
			}
			else
			{
				vni = &varnodeIndexes[iDiagonal][i2 - 1][j2][1];
				vni->iNode = i - 1;
				vni->iTreeAndMask = iTree | 0x1000;

				vni = &varnodeIndexes[iDiagonal][i2 - 1][j2 - 1][0];
				vni->iNode = i - 1;
				vni->iTreeAndMask = iTree | 0x2000;
			}
		}
		else
		{
			if (worknode[i].i0 <= worknode[i].i1)
			{
				vni = &varnodeIndexes[iDiagonal][i2][j2 - 1][0];
				vni->iNode = i - 1;
				vni->iTreeAndMask = iTree | 0x2000;

				vni = &varnodeIndexes[iDiagonal][i2 - 1][j2 - 1][0];
				vni->iNode = i - 1;
				vni->iTreeAndMask = iTree | 0x1000;
			}
			else
			{
				vni = &varnodeIndexes[iDiagonal][i2][j2][1];
				vni->iNode = i - 1;
				vni->iTreeAndMask = iTree | 0x1000;

				vni = &varnodeIndexes[iDiagonal][i2 - 1][j2][1];
				vni->iNode = i - 1;
				vni->iTreeAndMask = iTree | 0x2000;
			}
		}

	}
}

void BSPCollision::TR_PrepareGerrainCollide()
{
	worknode_t worknode[64];

	memset(&varnodeIndexes, 0, sizeof(varnodeIndexes));

	worknode[1].i0 = 8;
	worknode[1].j0 = 8;
	worknode[1].i1 = 0;
	worknode[1].j1 = 0;
	worknode[1].i2 = 0;
	worknode[1].j2 = 8;

	TR_CalculateTerrainIndices(worknode, 0, 0);

	worknode[1].i0 = 0;
	worknode[1].j0 = 0;
	worknode[1].i1 = 8;
	worknode[1].j1 = 8;
	worknode[1].i2 = 8;
	worknode[1].j2 = 0;

	TR_CalculateTerrainIndices(worknode, 0, 1);

	worknode[1].i0 = 8;
	worknode[1].j0 = 0;
	worknode[1].i1 = 0;
	worknode[1].j1 = 8;
	worknode[1].i2 = 8;
	worknode[1].j2 = 8;

	TR_CalculateTerrainIndices(worknode, 1, 0);

	worknode[1].i0 = 0;
	worknode[1].j0 = 8;
	worknode[1].i1 = 8;
	worknode[1].j1 = 0;
	worknode[1].i2 = 0;
	worknode[1].j2 = 0;

	TR_CalculateTerrainIndices(worknode, 1, 1);
}

void BSPCollision::GenerateTerrainCollide(const BSPData::TerrainPatch* patch, BSPData::TerrainCollide& collision)
{
	int i;
	int j;
	int x0, y0, z0;
	float fMaxHeight;
	float heightmap[9][9];
	BSPData::TerrainCollideSquare* square;
	vec3_t v1;
	vec3_t v2;
	vec3_t v3;
	vec3_t v4;

	BSPData::TerrainCollide* tc = &collision;
	x0 = (int)patch->x0;
	y0 = (int)patch->y0;
	z0 = (int)patch->z0;

	fMaxHeight = (float)z0;

	for (j = 0; j < 9; j++)
	{
		for (i = 0; i < 9; i++)
		{
			heightmap[i][j] = (float)(z0 + 2 * patch->heightmap[j * 9 + i]);
		}
	}

	for (j = 0; j < 8; j++)
	{
		for (i = 0; i < 8; i++)
		{
			v1[0] = (float)((i << 6) + x0);
			v1[1] = (float)((j << 6) + y0);
			v1[2] = (float)heightmap[i][j];

			v2[0] = (float)((i << 6) + x0) + 64;
			v2[1] = (float)((j << 6) + y0);
			v2[2] = (float)heightmap[i + 1][j];

			v3[0] = (float)((i << 6) + x0) + 64;
			v3[1] = (float)((j << 6) + y0) + 64;
			v3[2] = (float)heightmap[i + 1][j + 1];

			v4[0] = (float)((i << 6) + x0);
			v4[1] = (float)((j << 6) + y0) + 64;
			v4[2] = (float)heightmap[i][j + 1];

			if (fMaxHeight < v1[2]) {
				fMaxHeight = v1[2];
			}

			if (fMaxHeight < v2[2]) {
				fMaxHeight = v2[2];
			}

			if (fMaxHeight < v3[2]) {
				fMaxHeight = v3[2];
			}

			if (fMaxHeight < v4[2]) {
				fMaxHeight = v4[2];
			}

			square = &tc->squares[i][j];

			if ((i + j) & 1)
			{
				if (patch->flags & 0x40)
				{
					PlaneFromPoints(square->plane[0], v4, v2, v3);
					PlaneFromPoints(square->plane[1], v2, v4, v1);
				}
				else
				{
					PlaneFromPoints(square->plane[0], v2, v4, v3);
					PlaneFromPoints(square->plane[1], v4, v2, v1);
				}
				TR_PickTerrainSquareMode(square, v1, i, j, patch);
			}
			else
			{
				if (patch->flags & 0x40)
				{
					PlaneFromPoints(square->plane[0], v1, v3, v4);
					PlaneFromPoints(square->plane[1], v3, v1, v2);
				}
				else
				{
					PlaneFromPoints(square->plane[0], v3, v1, v4);
					PlaneFromPoints(square->plane[1], v1, v3, v2);
				}
				TR_PickTerrainSquareMode(square, v2, i, j, patch);
			}
		}
	}

	tc->vBounds[0][0] = (float)x0;
	tc->vBounds[0][1] = (float)y0;
	tc->vBounds[0][2] = (float)z0;
	tc->vBounds[1][0] = (float)(x0 + 512);
	tc->vBounds[1][1] = (float)(y0 + 512);
	tc->vBounds[1][2] = fMaxHeight;
}

void BSPCollision::TR_PickTerrainSquareMode(BSPData::TerrainCollideSquare* square, const vec3r_t vTest, terraInt i, terraInt j, const BSPData::TerrainPatch* patch)
{
	int flags0, flags1;
	BSPData::varnodeIndex* vni;

	vni = &varnodeIndexes[(patch->flags & 0x80) ? 1 : 0][i][j][0];

	flags0 = ((unsigned short)(patch->varTree[vni->iTreeAndMask & 1][vni->iNode].s.flags & 0xFFFE & vni->iTreeAndMask) != 0);
	flags1 = ((unsigned short)(patch->varTree[vni[1].iTreeAndMask & 1][vni[1].iNode].s.flags & 0xFFFE & vni[1].iTreeAndMask) != 0);

	square->eMode = modeTable[((j + i) & 1) | (2 * flags0) | (4 * flags1)];

	if (square->eMode == 2)
	{
		if (castVector(vTest).dot(castVector(square->plane[0])) < square->plane[0][3]) {
			square->eMode = 1;
		}
	}
	else if (square->eMode == 5 || square->eMode == 6)
	{
		Vec4Copy(square->plane[1], square->plane[0]);
	}
}
