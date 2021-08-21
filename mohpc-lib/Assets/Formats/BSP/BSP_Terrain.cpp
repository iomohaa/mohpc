#include <Shared.h>
#include <MOHPC/Assets/Formats/BSP.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>
#include <MOHPC/Common/Math.h>
#include "../../../Common/VectorPrivate.h"

using namespace MOHPC;
using namespace BSPData;

#define MAX_TERRAIN_LOD 6

BSPData::TerrainVert::TerrainVert()
{
	VectorClear(xyz);
	texCoords[0][0] = 0;
	texCoords[0][1] = 0;
	texCoords[1][0] = 0;
	texCoords[1][1] = 0;
	fVariance = 0.f;
	fHgtAvg = 0.f;
	fHgtAdd = 0.f;
	uiDistRecalc = 0;
	nRef = 0;
	iVertArray = 0;
	pHgt = nullptr;
	iNext = 0;
	iPrev = 0;
}

BSPData::TerrainTri::TerrainTri()
{
	iPt[0] = iPt[1] = iPt[2] = 0;
	nSplit = 0;
	uiDistRecalc = 0;
	patch = nullptr;
	info = nullptr;
	varnode = nullptr;
	index = 0;
	lod = 0;
	byConstChecks = 0;
	iLeft = 0;
	iRight = 0;
	iBase = 0;
	iLeftChild = 0;
	iRightChild = 0;
	iParent = 0;
	iPrev = 0;
	iNext = 0;
}

BSPData::PoolInfo::PoolInfo()
	: iFreeHead(0)
	, iCur(0)
	, nFree(0)
{
}

void BSPReader::GenerateTerrainPatch(const TerrainPatch* Patch, Surface* Out)
{
	int32_t x, y;
	size_t ndx;
	int16_t verts, quads;
	float f, distU, distV;

	verts = (short)ceilf(9.f);
	quads = 8;

	Out->shader = Patch->shader;
	Out->indexes.resize(quads * quads * 6);
	Out->vertices.resize(verts * verts);

	distU = Patch->texCoord[1][0][0] - Patch->texCoord[0][0][0];
	distV = Patch->texCoord[0][1][0] - Patch->texCoord[1][1][0];

	if (distU == 0.0f)
	{
		distU = 1.0f;
	}

	if (distV == 0.0f)
	{
		distV = -1.0f;
	}

	// vertexes
	for (ndx = 0, y = 0; y < 9; y++)
	{
		for (x = 0; x < 9; x++, ndx++)
		{
			float v[3];

			v[0] = Patch->x0 + (x << 6);
			v[1] = Patch->y0 + (y << 6);
			v[2] = Patch->z0 + (Patch->heightmap[ndx] * 2);
			VectorCopy(v, Out->vertices[ndx].xyz);

			f = x / 8.f;
			Out->vertices[ndx].st[0] = Patch->texCoord[0][0][0] + f * distU;
			Out->vertices[ndx].lightmap[0] = Patch->texCoord[0][0][1] + f * lightmapTerrainLength;
			f = y / 8.f;
			Out->vertices[ndx].st[1] = Patch->texCoord[1][1][0] + f * distV;
			Out->vertices[ndx].lightmap[1] = Patch->texCoord[0][1][1] + f * lightmapTerrainLength;
		}
	}

	//  triangles
	for (ndx = 0, y = 0; y < quads; y++)
	{
		for (x = 0; x < quads; x++, ndx += 6)
		{
			if ((x % 2) ^ (y % 2))
			{
				const Varnode* vn = &Patch->varTree[0][y * quads + x];

				Out->indexes[ndx + 0] = y * verts + x;
				Out->indexes[ndx + 1] = (y + 1) * verts + x;
				Out->indexes[ndx + 2] = y * verts + x + 1;

				Out->indexes[ndx + 3] = Out->indexes[ndx + 2];
				Out->indexes[ndx + 4] = Out->indexes[ndx + 1];
				Out->indexes[ndx + 5] = Out->indexes[ndx + 1] + 1;
			}
			else
			{
				const Varnode* vn = &Patch->varTree[1][y * quads + x];

				Out->indexes[ndx + 0] = y * verts + x;
				Out->indexes[ndx + 1] = (y + 1) * verts + x + 1;
				Out->indexes[ndx + 2] = y * verts + x + 1;

				Out->indexes[ndx + 3] = Out->indexes[ndx + 0];
				Out->indexes[ndx + 4] = Out->indexes[ndx + 1] - 1;
				Out->indexes[ndx + 5] = Out->indexes[ndx + 1];
			}
		}
	}

	// vertexes normal
	for (ndx = 0; ndx < Out->indexes.size(); ndx += 3)
	{
		Vertice* v1 = &Out->vertices[Out->indexes[ndx]];
		Vertice* v2 = &Out->vertices[Out->indexes[ndx + 1]];
		Vertice* v3 = &Out->vertices[Out->indexes[ndx + 2]];

		const Vector3 Delta1 = castVector(v1->xyz) - castVector(v2->xyz);
		const Vector3 Delta2 = castVector(v3->xyz) - castVector(v2->xyz);

		Vector3 normal = Delta1.cross(Delta2);
		normal.normalize();

		castVector(v1->normal) = castVector(v2->normal) = castVector(v3->normal) = normal;
	}

	Out->CalculateCentroid();
}

void BSPReader::GenerateTerrainPatch2(const TerrainPatch* Patch, TerrainPatchDrawInfo& info, Surface* Out)
{
	Out->shader = Patch->shader;

	MOHPC::ShaderManagerPtr ShaderManager = GetAssetManager()->getManager<MOHPC::ShaderManager>();
	ShaderPtr Shader = ShaderManager->GetShader(Out->shader->shaderName.c_str());

	if (!info.drawInfo.vertHead || !info.drawInfo.triHead)
	{
		return;
	}

	size_t numIndexes = 0;
	for (terraInt triNum = info.drawInfo.triHead; triNum; triNum = trTris[triNum].iNext)
	{
		if (trTris[triNum].byConstChecks & 4)
		{
			numIndexes += 3;
		}
	}

	size_t numVerts = 0;
	for (terraInt vertNum = info.drawInfo.vertHead; vertNum; vertNum = trVerts[vertNum].iNext)
	{
		numVerts++;
	}


	Out->indexes.resize(numIndexes);
	Out->vertices.resize(numVerts);

	size_t ndx = 0;
	for (terraInt vertNum = info.drawInfo.vertHead; vertNum; vertNum = trVerts[vertNum].iNext)
	{
		Vertice* vert = &Out->vertices[ndx];
		TerrainVert* terrainVert = &trVerts[vertNum];

		VecCopy(terrainVert->xyz, vert->xyz);
		vert->st[0] = terrainVert->texCoords[0][0];
		vert->st[1] = terrainVert->texCoords[0][1];
		vert->lightmap[0] = terrainVert->texCoords[1][0];
		vert->lightmap[1] = terrainVert->texCoords[1][1];
		vert->color[0] = vert->color[1] = vert->color[2] = -1;

		terrainVert->iVertArray = (terraInt)ndx++;
	}

	ndx = 0;
	for (terraInt triNum = info.drawInfo.triHead; triNum; triNum = trTris[triNum].iNext)
	{
		TerrainTri* terrainTri = &trTris[triNum];

		if (terrainTri->byConstChecks & 4)
		{
			Out->indexes[ndx] = trVerts[terrainTri->iPt[0]].iVertArray;
			Out->indexes[ndx + 1] = trVerts[terrainTri->iPt[1]].iVertArray;
			Out->indexes[ndx + 2] = trVerts[terrainTri->iPt[2]].iVertArray;
			ndx += 3;
		}
	}

	// calculate vertices normal
	for (ndx = 0; ndx < numIndexes; ndx += 3)
	{
		Vertice* v1 = &Out->vertices[Out->indexes[ndx]];
		Vertice* v2 = &Out->vertices[Out->indexes[ndx + 1]];
		Vertice* v3 = &Out->vertices[Out->indexes[ndx + 2]];

		const Vector3 Delta1 = castVector(v1->xyz) - castVector(v2->xyz);
		const Vector3 Delta2 = castVector(v3->xyz) - castVector(v2->xyz);
		
		Vector3 normal = Delta1.cross(Delta2);
		normal.normalize();

		castVector(v1->normal) = castVector(v2->normal) = castVector(v3->normal) = normal;
	}

	Out->CalculateCentroid();
}

void BSPReader::CreateTerrainSurfaces(const std::vector<BSPData::TerrainPatch>& terrainPatches, std::vector<BSPData::TerrainPatchDrawInfo>& infoList, std::vector<BSPData::Surface>& terrainSurfaces)
{
	size_t numTerrainPatches = terrainPatches.size();
	if (numTerrainPatches)
	{
		// Allocate and prepare vertices & indexes
		TR_PreTessellateTerrain(terrainPatches, infoList);

		// Split vertices
		TR_DoTriSplitting(terrainPatches, infoList);

		// Morph geometry according to the view
		// It is useless anyway so don't call it
		TR_DoGeomorphs();

		// Merge all tris
		//TR_DoTriMerging();

		// Remove unused verts/tris
		//TR_ShrinkData();

		terrainSurfaces.resize(numTerrainPatches);
		for (size_t i = 0; i < numTerrainPatches; i++)
		{
			const TerrainPatch *terrain = &terrainPatches[i];
			Surface *tsurf = &terrainSurfaces[i];

			GenerateTerrainPatch2(terrain, infoList[i], tsurf);
		}

		trTris.clear();
		trTris.shrink_to_fit();
		trVerts.clear();
		trVerts.shrink_to_fit();
	}
}

/////////////////////////////////////////
///////////   Disassembly   /////////////
/////////////////////////////////////////

terraInt BSPReader::TR_AllocateVert(const TerrainPatch *patch, TerrainPatchDrawInfo& info)
{
	terraInt iVert = trpiVert.iFreeHead;

	trpiVert.iFreeHead = trVerts[trpiVert.iFreeHead].iNext;
	trVerts[trpiVert.iFreeHead].iPrev = 0;

	trVerts[iVert].iPrev = 0;
	trVerts[iVert].iNext = info.drawInfo.vertHead;
	trVerts[info.drawInfo.vertHead].iPrev = iVert;

	info.drawInfo.vertHead = iVert;
	info.drawInfo.numVerts++;
	trpiVert.nFree--;

	trVerts[iVert].nRef = 0;
	trVerts[iVert].uiDistRecalc = 0;

	return iVert;
}

void BSPReader::TR_InterpolateVert(TerrainTri *pTri, TerrainVert *pVert)
{
	const TerrainVert *pVert0 = &trVerts[pTri->iPt[0]];
	const TerrainVert *pVert1 = &trVerts[pTri->iPt[1]];
	const TerrainPatch *pPatch = pTri->patch;

	// Interpolate texture coordinates
	pVert->texCoords[0][0] = (pVert0->texCoords[0][0] + pVert1->texCoords[0][0]) * 0.5f;
	pVert->texCoords[0][1] = (pVert0->texCoords[0][1] + pVert1->texCoords[0][1]) * 0.5f;
	pVert->texCoords[1][0] = (pVert0->texCoords[1][0] + pVert1->texCoords[1][0]) * 0.5f;
	pVert->texCoords[1][1] = (pVert0->texCoords[1][1] + pVert1->texCoords[1][1]) * 0.5f;

	pVert->pHgt = (uint8_t *)(((size_t)pVert0->pHgt + (size_t)pVert1->pHgt) >> 1);
	pVert->fHgtAvg = (float)(*pVert0->pHgt + *pVert1->pHgt);
	pVert->fHgtAdd = (float)(*pVert->pHgt * 2) - pVert->fHgtAvg;
	pVert->fHgtAvg += pPatch->z0;
	pVert->xyz[0] = (pVert0->xyz[0] + pVert1->xyz[0]) * 0.5f;
	pVert->xyz[1] = (pVert0->xyz[1] + pVert1->xyz[1]) * 0.5f;
	pVert->xyz[2] = pVert->fHgtAvg;
}

void BSPReader::TR_ReleaseVert(const TerrainPatch *patch, TerrainPatchDrawInfo& info, terraInt iVert)
{
	TerrainVert *pVert = &trVerts[iVert];

	terraInt iPrev = pVert->iPrev;
	terraInt iNext = pVert->iNext;
	trVerts[iPrev].iNext = iNext;
	trVerts[iNext].iPrev = iPrev;

	assert(info.drawInfo.numVerts > 0);
	info.drawInfo.numVerts--;

	if (info.drawInfo.vertHead == iVert)
	{
		info.drawInfo.vertHead = iNext;
	}

	pVert->iPrev = 0;
	pVert->iNext = trpiVert.iFreeHead;
	trVerts[trpiVert.iFreeHead].iPrev = iVert;
	trpiVert.iFreeHead = iVert;
	trpiVert.nFree++;
}

terraInt BSPReader::TR_AllocateTri(const TerrainPatch* patch, TerrainPatchDrawInfo& info, uint8_t byConstChecks)
{
	terraInt iTri = trpiTri.iFreeHead;

	trpiTri.iFreeHead = trTris[iTri].iNext;
	trTris[trpiTri.iFreeHead].iPrev = 0;

	trTris[iTri].iPrev = info.drawInfo.triTail;
	trTris[iTri].iNext = 0;
	trTris[info.drawInfo.triTail].iNext = iTri;
	info.drawInfo.triTail = iTri;

	if (!info.drawInfo.triHead)
	{
		info.drawInfo.triHead = iTri;
	}

	info.drawInfo.numTris++;
	trpiTri.nFree--;

	trTris[iTri].byConstChecks = byConstChecks;
	trTris[iTri].uiDistRecalc = 0;
	info.distRecalc = 0;

	return iTri;
}

void BSPReader::TR_FixTriHeight(TerrainTri* pTri)
{
	for (terraInt i = 0; i < 3; i++)
	{
		TerrainVert* pVert = &trVerts[pTri->iPt[i]];
		if (pVert->pHgt < pTri->patch->heightmap || pVert->pHgt > &pTri->patch->heightmap[80])
		{
			pVert->pHgt = pTri->patch->heightmap;
		}
	}
}

void BSPReader::TR_SetTriConstChecks(TerrainTri* pTri)
{
	Varnode vn = *pTri->varnode;
	vn.s.flags &= 0xF0u;
	if (vn.variance != 0.0)
	{
		if (pTri->varnode->s.flags & 8)
		{
			pTri->byConstChecks |= 3;
		}
		else if (!(pTri->byConstChecks & 4) || (pTri->varnode->s.flags & 4) || pTri->lod >= MAX_TERRAIN_LOD)
		{
			pTri->byConstChecks |= 2;
		}
	}
	else if (pTri->varnode->s.flags & 8)
	{
		pTri->byConstChecks |= 3;
	}
	else
	{
		pTri->byConstChecks |= 2;
	}
}

void BSPReader::TR_ReleaseTri(const TerrainPatch *patch, TerrainPatchDrawInfo& info, terraInt iTri)
{
	TerrainTri *pTri = &trTris[iTri];

	terraInt iPrev = pTri->iPrev;
	terraInt iNext = pTri->iNext;
	trTris[iPrev].iNext = iNext;
	trTris[iNext].iPrev = iPrev;

	if (trpiTri.iCur == iTri)
	{
		trpiTri.iCur = iNext;
	}

	info.distRecalc = 0;

	assert(info.drawInfo.numTris > 0);
	info.drawInfo.numTris--;

	if (info.drawInfo.triHead == iTri)
	{
		info.drawInfo.triHead = iNext;
	}

	if (info.drawInfo.triTail == iTri)
	{
		info.drawInfo.triTail = iPrev;
	}

	pTri->iPrev = 0;
	pTri->iNext = trpiTri.iFreeHead;
	trTris[trpiTri.iFreeHead].iPrev = iTri;
	trpiTri.iFreeHead = iTri;
	trpiTri.nFree++;

	for (int i = 0; i < 3; i++)
	{
		terraInt ptNum = pTri->iPt[i];

		trVerts[ptNum].nRef--;
		if (trVerts[ptNum].nRef-- == 1)
		{
			TR_ReleaseVert(patch, info, ptNum);
		}
	}
}

void BSPReader::TR_DemoteInAncestry(const TerrainPatch* patch, TerrainPatchDrawInfo& info, terraInt iTri)
{
	terraInt iPrev = trTris[iTri].iPrev;
	terraInt iNext = trTris[iTri].iNext;

	trTris[iPrev].iNext = iNext;
	trTris[iNext].iPrev = iPrev;

	if (trpiTri.iCur == iTri)
	{
		trpiTri.iCur = iNext;
	}

	assert(info.drawInfo.numTris > 0);
	info.drawInfo.numTris--;

	if (info.drawInfo.triHead == iTri)
	{
		info.drawInfo.triHead = iNext;
	}

	if (info.drawInfo.triTail == iTri)
	{
		info.drawInfo.triTail = iPrev;
	}

	trTris[iTri].iPrev = 0;
	trTris[iTri].iNext = info.drawInfo.mergeHead;
	trTris[info.drawInfo.mergeHead].iPrev = iTri;
	info.drawInfo.mergeHead = iTri;
}

void BSPReader::TR_TerrainHeapInit()
{
	trpiTri.iFreeHead = 1;
	trpiTri.nFree = trTris.size() - 1;
	trpiVert.iFreeHead = 1;
	trpiVert.nFree = trVerts.size() - 1;

	for (size_t i = 0; i < trTris.size(); i++)
	{
		trTris[i].iPrev = (terraInt)i - 1;
		trTris[i].iNext = (terraInt)i + 1;
	}

	trTris[0].iPrev = 0;
	trTris[trTris.size() - 1].iNext = 0;

	for (size_t i = 0; i < trVerts.size(); i++)
	{
		trVerts[i].iPrev = (terraInt)i - 1;
		trVerts[i].iNext = (terraInt)i + 1;
	}

	trVerts[0].iPrev = 0;
	trVerts[trVerts.size() - 1].iNext = 0;
}

void BSPReader::TR_SplitTri(terraInt iSplit, terraInt iNewPt, terraInt iLeft, terraInt iRight, terraInt iRightOfLeft, terraInt iLeftOfRight)
{
	TerrainTri *pSplit = &trTris[iSplit];

	TerrainTri *pLeft;
	if (iRight)
	{
		pLeft = &trTris[iLeft];
	}
	else
	{
		pLeft = nullptr;
	}

	TerrainTri *pRight;
	if (iRight)
	{
		pRight = &trTris[iRight];
	}
	else
	{
		pRight = nullptr;
	}

	int iNextLod = pSplit->lod + 1;
	int index = pSplit->index;
	const Varnode *varnode = pSplit->varnode + index;

	if (pLeft)
	{
		pLeft->patch = pSplit->patch;
		pLeft->info = pSplit->info;
		pLeft->index = index * 2;
		pLeft->varnode = varnode;
		pLeft->lod = iNextLod;
		pLeft->iLeft = iRight;
		pLeft->iRight = iRightOfLeft;
		pLeft->iBase = pSplit->iLeft;
		pLeft->iPt[0] = pSplit->iPt[1];
		pLeft->iPt[1] = pSplit->iPt[2];
		pLeft->iPt[2] = iNewPt;

		TR_FixTriHeight(pLeft);

		if (pLeft->lod != MAX_TERRAIN_LOD)
		{
			TR_SetTriConstChecks(pLeft);
		}
		else
		{
			pLeft->byConstChecks |= 2;
		}

		trVerts[pLeft->iPt[0]].nRef++;
		trVerts[pLeft->iPt[1]].nRef++;
		trVerts[pLeft->iPt[2]].nRef++;
		trTris[pSplit->iParent].nSplit++;
		pLeft->nSplit = 0;
	}

	if (pSplit->iLeft)
	{
		if (trTris[pSplit->iLeft].lod == iNextLod)
		{
			trTris[pSplit->iLeft].iBase = iLeft;
		}
		else
		{
			trTris[pSplit->iLeft].iRight = iLeft;
		}
	}

	if (pRight)
	{
		pRight->patch = pSplit->patch;
		pRight->info = pSplit->info;
		pRight->index = index * 2 + 1;
		pRight->varnode = varnode + 1;
		pRight->lod = iNextLod;
		pRight->iLeft = iLeftOfRight;
		pRight->iRight = iLeft;
		pRight->iBase = pSplit->iRight;
		pRight->iPt[0] = pSplit->iPt[2];
		pRight->iPt[1] = pSplit->iPt[0];
		pRight->iPt[2] = iNewPt;

		TR_FixTriHeight(pRight);

		if (pRight->lod != MAX_TERRAIN_LOD)
		{
			TR_SetTriConstChecks(pRight);
		}
		else
		{
			pRight->byConstChecks |= 2;
		}
		
		trVerts[pRight->iPt[0]].nRef++;
		trVerts[pRight->iPt[1]].nRef++;
		trVerts[pRight->iPt[2]].nRef++;
		trTris[pSplit->iParent].nSplit++;
		pRight->nSplit = 0;
	}

	if (pSplit->iRight)
	{
		if (trTris[pSplit->iRight].lod == iNextLod)
		{
			trTris[pSplit->iRight].iBase = iRight;
		}
		else
		{
			trTris[pSplit->iRight].iLeft = iRight;
		}
	}

	pSplit->iLeftChild = iLeft;
	pSplit->iRightChild = iRight;
	trTris[iLeft].iParent = iSplit;
	trTris[iRight].iParent = iSplit;

	TR_DemoteInAncestry(pSplit->patch, *pSplit->info, iSplit);
}

void BSPReader::TR_ForceSplit(terraInt iTri)
{
	TerrainTri *pTri = &trTris[iTri];

	terraInt iBase = pTri->iBase;
	TerrainTri *pBase = &trTris[iBase];
	if (iBase && pBase->lod != pTri->lod)
	{
		TR_ForceSplit(iBase);
		iBase = pTri->iBase;
		pBase = &trTris[iBase];
	}

	uint8_t flags = pTri->varnode->s.flags;

	terraInt iTriLeft = TR_AllocateTri(pTri->patch, *pTri->info, (flags & 2) ? 0 : 4);
	terraInt iTriRight = TR_AllocateTri(pTri->patch, *pTri->info, (flags & 1) ? 0 : 4);

	terraInt iNewPt = TR_AllocateVert(pTri->patch, *pTri->info);
	TR_InterpolateVert(pTri, &trVerts[iNewPt]);

	trVerts[iNewPt].fVariance = pTri->varnode->variance;

	terraInt iBaseLeft = 0;
	terraInt iBaseRight = 0;

	if (iBase)
	{
		uint8_t flags2 = pBase->varnode->s.flags;
		flags |= flags2;

		iBaseLeft = TR_AllocateTri(pBase->patch, *pBase->info, (flags2 & 2) ? 0 : 4);
		iBaseRight = TR_AllocateTri(pBase->patch, *pBase->info, (flags2 & 1) ? 0 : 4);

		terraInt iNewBasePt = iNewPt;
		if (pBase->patch != pTri->patch)
		{
			iNewBasePt = TR_AllocateVert(pBase->patch, *pBase->info);
			TerrainVert *pVert = &trVerts[iNewBasePt];
			TR_InterpolateVert(pBase, pVert);

			pVert->fVariance = trVerts[iNewPt].fVariance;
			if (flags & 8)
			{
				pVert->fHgtAvg += pVert->fHgtAdd;
				pVert->fHgtAdd = 0.0;
				pVert->fVariance = 0.0;
				pVert->xyz[2] = pVert->fHgtAvg;
			}
		}

		TR_SplitTri(iBase, iNewBasePt, iBaseLeft, iBaseRight, iTriRight, iTriLeft);
	}

	if (flags & 8)
	{
		TerrainVert *pVert = &trVerts[iNewPt];
		pVert->fHgtAvg += pVert->fHgtAdd;
		pVert->fHgtAdd = 0.0;
		pVert->fVariance = 0.0;
		pVert->xyz[2] = pVert->fHgtAvg;
	}

	TR_SplitTri(iTri, iNewPt, iTriLeft, iTriRight, iBaseRight, iBaseLeft);
}

void BSPReader::TR_ForceMerge(terraInt iTri)
{
	TerrainTri *pTri = &trTris[iTri];
	const TerrainPatch *patch = pTri->patch;
	TerrainPatchDrawInfo& info = *pTri->info;
	terraInt iPrev = pTri->iPrev;
	terraInt iNext = pTri->iNext;

	//g_nMerge++;

	if (pTri->iLeftChild)
	{
		terraInt iLeft = trTris[pTri->iLeftChild].iBase;

		trTris[iTri].iLeft = iLeft;
		if (iLeft)
		{
			if (trTris[iLeft].lod == pTri->lod)
			{
				trTris[iLeft].iRight = iTri;
			}
			else
			{
				trTris[iLeft].iBase = iTri;
			}
		}

		TR_ReleaseTri(pTri->patch, *pTri->info, pTri->iLeftChild);

		pTri->iLeftChild = 0;
		trTris[pTri->iParent].nSplit--;
	}

	if (pTri->iRightChild)
	{
		terraInt iRight = trTris[pTri->iRightChild].iBase;
		trTris[iTri].iRight = iRight;
		if (iRight)
		{
			if (trTris[iRight].lod == pTri->lod)
			{
				trTris[iRight].iLeft = iTri;
			}
			else
			{
				trTris[iRight].iBase = iTri;
			}
		}

		TR_ReleaseTri(pTri->patch, *pTri->info, pTri->iRightChild);

		pTri->iRightChild = 0;
		trTris[pTri->iParent].nSplit--;
	}

	trTris[iPrev].iNext = iNext;
	trTris[iNext].iPrev = iPrev;

	if (trpiTri.iCur == iTri)
	{
		trpiTri.iCur = iNext;
	}

	info.drawInfo.numTris++;
	if (info.drawInfo.mergeHead == iTri)
	{
		info.drawInfo.mergeHead = iNext;
	}

	trTris[iTri].iPrev = info.drawInfo.triTail;
	trTris[iTri].iNext = 0;

	trTris[info.drawInfo.triTail].iNext = iTri;

	info.drawInfo.triTail = iTri;
	if (!info.drawInfo.triHead)
	{
		info.drawInfo.triHead = iTri;
	}
}

int BSPReader::TR_TerraTriNeighbor(const TerrainPatch *terraPatches, int iPatch, int dir)
{
	if (iPatch >= 0)
	{
		int iNeighbor = 2 * iPatch + 1;

		if (dir == 1)
		{
			if (terraPatches[iPatch].flags & 0x80)
			{
				return iNeighbor;
			}
			else
			{
				return iNeighbor + 1;
			}
		}
		else if (dir > 1)
		{
			if (dir == 2)
			{
				return 2 * iPatch + 2;
			}
			if (dir == 3)
			{
				if (terraPatches[iPatch].flags & 0x80)
				{
					return iNeighbor + 1;
				}
				else
				{
					return iNeighbor;
				}
			}
		}
		else if (!dir)
		{
			return 2 * iPatch + 1;
		}
	}

	return 0;
}

void BSPReader::TR_PreTessellateTerrain(const std::vector<BSPData::TerrainPatch>& terrainPatches, std::vector<BSPData::TerrainPatchDrawInfo>& infoList)
{
	const size_t numTerrainPatches = terrainPatches.size();
	if (!numTerrainPatches)
	{
		return;
	}

	// each terrain contains 8 * 8 * 6 indices and 9 * 9 vertices
	const size_t numTris = numTerrainPatches * 8 * 8 * 6 * MAX_TERRAIN_LOD + 1;
	const size_t numVerts = numTris; // numTerrainPatches * 9 * 9 * MAX_TERRAIN_LOD + 1;

	trVerts.resize(numVerts);
	trTris.resize(numTris);

	// Init triangles & vertices
	TR_TerrainHeapInit();

	for (size_t i = 0; i < numTerrainPatches; i++)
	{
		const TerrainPatch *patch = &terrainPatches[i];
		TerrainPatchDrawInfo& info = infoList[i];

		info.drawInfo.numTris = 0;
		info.drawInfo.numVerts = 0;
		info.drawInfo.triHead = 0;
		info.drawInfo.triTail = 0;
		info.drawInfo.vertHead = 0;

		float sMin, tMin;

		if (patch->texCoord[0][0][0] < patch->texCoord[0][1][0])
		{
			sMin = patch->texCoord[0][0][0];
		}
		else
		{
			sMin = patch->texCoord[0][1][0];
		}

		float sMin2;
		if (patch->texCoord[1][0][0] < patch->texCoord[1][1][0])
		{
			sMin2 = patch->texCoord[1][0][0];
		}
		else
		{
			sMin2 = patch->texCoord[1][1][0];
		}

		if (sMin >= sMin2)
		{
			if (patch->texCoord[1][0][0] >= patch->texCoord[1][1][0])
			{
				sMin = floor(patch->texCoord[1][1][0]);
			}
			else
			{
				sMin = floor(patch->texCoord[1][0][0]);
			}
		}
		else if (patch->texCoord[0][0][0] >= patch->texCoord[0][1][0])
		{
			sMin = floor(patch->texCoord[0][1][0]);
		}
		else
		{
			sMin = floor(patch->texCoord[0][0][0]);
		}

		if (patch->texCoord[0][0][1] < patch->texCoord[0][1][1])
		{
			tMin = patch->texCoord[0][0][1];
		}
		else
		{
			tMin = patch->texCoord[0][1][1];
		}

		float tMin2;
		if (patch->texCoord[1][0][1] < patch->texCoord[1][1][1])
		{
			tMin2 = patch->texCoord[1][0][1];
		}
		else
		{
			tMin2 = patch->texCoord[1][1][1];
		}

		if (tMin >= tMin2)
		{
			if (patch->texCoord[1][0][1] >= patch->texCoord[1][1][1])
			{
				tMin = floor(patch->texCoord[1][1][1]);
			}
			else
			{
				tMin = floor(patch->texCoord[1][0][1]);
			}
		}
		else if (patch->texCoord[0][0][1] >= patch->texCoord[0][1][1])
		{
			tMin = floor(patch->texCoord[0][1][1]);
		}
		else
		{
			tMin = floor(patch->texCoord[0][0][1]);
		}

		const float s00 = patch->texCoord[0][0][0] - sMin;
		const float s01 = patch->texCoord[0][1][0] - sMin;
		const float s10 = patch->texCoord[1][0][0] - sMin;
		const float s11 = patch->texCoord[1][1][0] - sMin;

		const float t00 = patch->texCoord[0][0][1] - tMin;
		const float t01 = patch->texCoord[0][1][1] - tMin;
		const float t10 = patch->texCoord[1][0][1] - tMin;
		const float t11 = patch->texCoord[1][1][1] - tMin;

		const float lmapSize = (float)(info.drawInfo.lmapSize - 1) * lightmapSize;
		const float ls = patch->s * lmapSize;
		const float lt = patch->t * lmapSize;

		terraInt iTri0 = TR_AllocateTri(patch, info);
		terraInt iTri1 = TR_AllocateTri(patch, info);
		terraInt i00 = TR_AllocateVert(patch, info);
		terraInt i01 = TR_AllocateVert(patch, info);
		terraInt i10 = TR_AllocateVert(patch, info);
		terraInt i11 = TR_AllocateVert(patch, info);

		TerrainVert *pVert;
		pVert = &trVerts[i00];
		pVert->xyz[0] = patch->x0;
		pVert->xyz[1] = patch->y0;
		pVert->xyz[2] = (float)(patch->heightmap[0] * 2) + patch->z0;
		pVert->pHgt = &patch->heightmap[0];
		pVert->fHgtAvg = pVert->xyz[2];
		pVert->texCoords[0][0] = s00;
		pVert->texCoords[0][1] = t00;
		pVert->texCoords[1][0] = patch->s;
		pVert->texCoords[1][1] = patch->t;
		pVert->fVariance = 0.0f;
		pVert->nRef = 4;

		pVert = &trVerts[i01];
		pVert->xyz[0] = patch->x0;
		pVert->xyz[1] = patch->y0 + 512.0f;
		pVert->xyz[2] = (float)(patch->heightmap[72] * 2) + patch->z0;
		pVert->pHgt = &patch->heightmap[72];
		pVert->fHgtAvg = pVert->xyz[2];
		pVert->texCoords[0][0] = s01;
		pVert->texCoords[0][1] = t01;
		pVert->texCoords[1][0] = patch->s;
		pVert->texCoords[1][1] = lt;
		pVert->fVariance = 0.0f;
		pVert->nRef = 4;

		pVert = &trVerts[i10];
		pVert->xyz[0] = patch->x0 + 512.0f;
		pVert->xyz[1] = patch->y0;
		pVert->xyz[2] = (float)(patch->heightmap[8] * 2) + patch->z0;
		pVert->pHgt = &patch->heightmap[8];
		pVert->fHgtAvg = pVert->xyz[2];
		pVert->texCoords[0][0] = s10;
		pVert->texCoords[0][1] = t10;
		pVert->texCoords[1][0] = ls;
		pVert->texCoords[1][1] = patch->t;
		pVert->fVariance = 0.0f;
		pVert->nRef = 4;

		pVert = &trVerts[i11];
		pVert->xyz[0] = patch->x0 + 512.0f;
		pVert->xyz[1] = patch->y0 + 512.0f;
		pVert->xyz[2] = (float)(patch->heightmap[80] * 2) + patch->z0;
		pVert->pHgt = &patch->heightmap[80];
		pVert->fHgtAvg = pVert->xyz[2];
		pVert->texCoords[0][0] = s11;
		pVert->texCoords[0][1] = t11;
		pVert->texCoords[1][0] = ls;
		pVert->texCoords[1][1] = lt;
		pVert->fVariance = 0.0f;
		pVert->nRef = 4;

		TerrainTri *pTri = &trTris[iTri0];
		pTri->patch = patch;
		pTri->info = &info;
		pTri->varnode = &patch->varTree[0][0];
		pTri->index = 1;
		pTri->lod = 0;

		TR_SetTriConstChecks(pTri);

		pTri->iBase = iTri1;
		if ((patch->flags & 0x80u) == 0)
		{
			pTri->iLeft = TR_TerraTriNeighbor(terrainPatches.data(), patch->west, 1);
			pTri->iRight = TR_TerraTriNeighbor(terrainPatches.data(), patch->north, 2);
			if (patch->flags & 0x40)
			{
				pTri->iPt[0] = i00;
				pTri->iPt[1] = i11;
			}
			else
			{
				pTri->iPt[0] = i11;
				pTri->iPt[1] = i00;
			}
			pTri->iPt[2] = i01;
		}
		else
		{
			pTri->iLeft = TR_TerraTriNeighbor(terrainPatches.data(), patch->north, 2);
			pTri->iRight = TR_TerraTriNeighbor(terrainPatches.data(), patch->east, 3);
			if (patch->flags & 0x40)
			{
				pTri->iPt[0] = i01;
				pTri->iPt[1] = i10;
			}
			else
			{
				pTri->iPt[0] = i10;
				pTri->iPt[1] = i01;
			}
			pTri->iPt[2] = i11;
		}

		TR_FixTriHeight(pTri);

		pTri = &trTris[iTri1];
		pTri->patch = patch;
		pTri->info = &info;
		pTri->varnode = &patch->varTree[1][0];
		pTri->index = 1;
		pTri->lod = 0;

		TR_SetTriConstChecks(pTri);

		pTri->iBase = iTri0;
		if ((patch->flags & 0x80u) == 0)
		{
			pTri->iLeft = TR_TerraTriNeighbor(terrainPatches.data(), patch->east, 3);
			pTri->iRight = TR_TerraTriNeighbor(terrainPatches.data(), patch->south, 0);
			if (patch->flags & 0x40)
			{
				pTri->iPt[0] = i11;
				pTri->iPt[1] = i00;
			}
			else
			{
				pTri->iPt[0] = i00;
				pTri->iPt[1] = i11;
			}
			pTri->iPt[2] = i10;
		}
		else
		{
			pTri->iLeft = TR_TerraTriNeighbor(terrainPatches.data(), patch->south, 0);
			pTri->iRight = TR_TerraTriNeighbor(terrainPatches.data(), patch->west, 1);
			if (patch->flags & 0x40)
			{
				pTri->iPt[0] = i10;
				pTri->iPt[1] = i01;
			}
			else
			{
				pTri->iPt[0] = i01;
				pTri->iPt[1] = i10;
			}
			pTri->iPt[2] = i00;
		}

		TR_FixTriHeight(pTri);
	}
}

bool BSPReader::TR_NeedSplitTri(TerrainTri *pTri)
{
	uint8_t byConstChecks = pTri->byConstChecks;
	if (byConstChecks & 2)
	{
		return byConstChecks & 1;
	}

	/*
	if (g_uiTerDist < pTri->uiDistRecalc)
	{
		return false;
	}

	float fRatio = ((trVerts[pTri->iPt[0]].xyz[0] + trVerts[pTri->iPt[1]].xyz[0]) * g_vViewVector[0]
		+ (trVerts[pTri->iPt[0]].xyz[1] + trVerts[pTri->iPt[1]].xyz[1]) * g_vViewVector[1])
		* 0.5
		+ g_vViewVector[2]
		- *(float *)pTri->varnode * g_fCheck;

	if (fRatio <= 0)
	{
		return true;
	}
	pTri->uiDistRecalc = floor(fRatio) + g_uiTerDist;
	*/

	return true;
}

void BSPReader::TR_DoTriSplitting(const std::vector<BSPData::TerrainPatch>& terrainPatches, std::vector<BSPData::TerrainPatchDrawInfo>& infoList)
{
	for (size_t i = 0; i < terrainPatches.size(); i++)
	{
		const TerrainPatch *patch = &terrainPatches[i];
		TerrainPatchDrawInfo& info = infoList[i];

		info.distRecalc = -1;
		trpiTri.iCur = info.drawInfo.triHead;
		while(trpiTri.iCur != 0)
		{
			TerrainTri *pTri = &trTris[trpiTri.iCur];

			if (TR_NeedSplitTri(pTri))
			{
				assert(trpiTri.nFree > 13 && trpiVert.nFree > 13);

				info.distRecalc = 0;
				TR_ForceSplit(trpiTri.iCur);

				if (&trTris[trpiTri.iCur] == pTri)
				{
					trpiTri.iCur = trTris[trpiTri.iCur].iNext;
				}
			}
			else
			{
				if ((pTri->byConstChecks & 3) != 2)
				{
					if (info.distRecalc > pTri->uiDistRecalc)
					{
						info.distRecalc = pTri->uiDistRecalc;
					}
				}

				trpiTri.iCur = trTris[trpiTri.iCur].iNext;
			}
		}
	}
}

void BSPReader::TR_DoGeomorphs()
{
	// Geomorph not needed

#if 0
	std::vector<BSPData::TerrainPatch>& terrainPatches = bsp->getTerrainPatches();
	for (size_t n = 0; n < terrainPatches.size(); n++)
	{
		TerrainPatch* patch = &terrainPatches[n];

		trpiVert.iCur = info.drawInfo.vertHead;
		if (!patch->bByDirty)
		{
			patch->bByDirty = true;
			while (trpiVert.iCur)
			{
				TerrainVert* pVert = &trVerts[trpiVert.iCur];
				pVert->xyz[2] = pVert->fHgtAvg + pVert->fHgtAdd;
				trpiVert.iCur = pVert->iNext;
			}
		}
	}
#endif
}

bool BSPReader::TR_MergeInternalAggressive()
{
	TerrainTri *pTri = &trTris[trpiTri.iCur];
	if (!pTri->nSplit
		&& trVerts[trTris[pTri->iLeftChild].iPt[2]].xyz[2] == trVerts[trTris[pTri->iLeftChild].iPt[2]].fHgtAvg)
	{
		if (!pTri->iBase)
		{
			TR_ForceMerge(trpiTri.iCur);
			return true;
		}

		if (!trTris[pTri->iBase].nSplit)
		{
			TR_ForceMerge(pTri->iBase);
			TR_ForceMerge(trpiTri.iCur);
			return true;
		}
	}

	return false;
}

bool BSPReader::TR_MergeInternalCautious()
{
	TerrainTri *pTri = pTri = &trTris[trpiTri.iCur];
	if (!pTri->nSplit
		&& !(pTri->varnode->s.flags & 8)
		&& trVerts[trTris[pTri->iLeftChild].iPt[2]].xyz[2] == trVerts[trTris[pTri->iLeftChild].iPt[2]].fHgtAvg)
	{
		if (!pTri->iBase)
		{
			TR_ForceMerge(trpiTri.iCur);
			return true;
		}

		TerrainTri *pBase = &trTris[pTri->iBase];
		if (!pBase->nSplit && !(pBase->varnode->s.flags & 8))
		{
			TR_ForceMerge(pTri->iBase);
			TR_ForceMerge(trpiTri.iCur);
			return true;
		}
	}

	return false;
}

void BSPReader::TR_DoTriMerging(const std::vector<BSPData::TerrainPatch>& terrainPatches, std::vector<BSPData::TerrainPatchDrawInfo>& infoList)
{
	for (size_t n = 0; n < terrainPatches.size(); n++)
	{
		const BSPData::TerrainPatch *patch = &terrainPatches[n];
		BSPData::TerrainPatchDrawInfo& info = infoList[n];

		trpiTri.iCur = info.drawInfo.mergeHead;
		while (trpiTri.iCur)
		{
			if (!TR_MergeInternalCautious())
			{
				trpiTri.iCur = trTris[trpiTri.iCur].iNext;
			}
		}

		/*
		if (patch->visCountDraw >= iCautiousFrame)
		{
			while (trpiTri.iCur)
			{
				if (!TR_MergeInternalCautious())
				{
					trpiTri.iCur = trTris[trpiTri.iCur].iNext;
				}
			}
		}
		else if(patch->visCountDraw > iCautiousFrame - 10)
		{
			while (trpiTri.iCur)
			{
				if (!TR_MergeInternalAggressive())
				{
					trpiTri.iCur = trTris[trpiTri.iCur].iNext;
				}
			}
		}
		*/
	}
}

void BSPReader::TR_ShrinkData()
{
	for (auto pTri = trTris.begin(); pTri != trTris.end(); ++pTri)
	{
		if (!pTri->patch)
		{
			pTri = trTris.erase(pTri);
		}
	}

	for (auto pVert = trVerts.begin(); pVert != trVerts.end(); ++pVert)
	{
		if (!pVert->nRef)
		{
			pVert = trVerts.erase(pVert);
		}
	}
}
