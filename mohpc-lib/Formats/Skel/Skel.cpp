#include <Shared.h>
#include <MOHPC/Formats/Skel.h>
#include "SkelPrivate.h"
#include "../TIKI/TIKI_Private.h"
#include <MOHPC/Managers/FileManager.h>
#include <map>
#include <memory>

using namespace std;

map<string, weak_ptr<Skeleton>> g_skelCache;

MOHPC_EXPORTS void MOHPC::AddToBounds(SkelVec3 *bounds, const SkelVec3 *newBounds)
{
	int i;

	for (i = 0; i < 3; i++)
	{
		if (bounds[0][i] > newBounds[0][i])
		{
			bounds[0][i] = newBounds[0][i];
		}

		if (bounds[1][i] > newBounds[1][i])
		{
			bounds[1][i] = newBounds[1][i];
		}
	}
}

CLASS_DEFINITION(Skeleton);
Skeleton::Skeleton()
{
}

void Skeleton::LoadBoneFromBuffer2(const BoneFileData *fileData, BoneData *boneData) const
{
	const char *newChannelName;
	const char *newBoneRefName;
	int i;

	boneData->channel = GetAssetManager()->GetManager<SkeletorManager>()->GetBoneNamesTable()->RegisterChannel(fileData->name);
	boneData->boneType = fileData->boneType;

	if (boneData->boneType == Skeleton::SKELBONE_HOSEROT)
	{
		i = *(int *)&fileData->parent[fileData->ofsBaseData + 4];

		if (i == 1)
		{
			boneData->boneType = Skeleton::SKELBONE_HOSEROTPARENT;
		}
		else if (i == 2)
		{
			boneData->boneType = Skeleton::SKELBONE_HOSEROTBOTH;
		}
	}

	newChannelName = (char *)fileData + fileData->ofsChannelNames;
	boneData->numChannels = skelBone_Base::GetNumChannels(boneData->boneType);

	for (i = 0; i < boneData->numChannels; i++)
	{
		boneData->channelIndex[i] = GetAssetManager()->GetManager<SkeletorManager>()->GetChannelNamesTable()->RegisterChannel(newChannelName);
		if (boneData->channelIndex[i] < 0)
		{
			//SKEL_Warning("Channel named %s not added. (Bone will not work without it)\n", newChannelName);
			boneData->boneType = Skeleton::SKELBONE_ZERO;
		}

		newChannelName += strlen(newChannelName) + 1;
	}

	newBoneRefName = (char *)fileData + fileData->ofsBoneNames;
	boneData->numRefs = skelBone_Base::GetNumBoneRefs(boneData->boneType);

	for (i = 0; i < boneData->numRefs; i++)
	{
		boneData->refIndex[i] = GetAssetManager()->GetManager<SkeletorManager>()->GetBoneNamesTable()->RegisterChannel(newBoneRefName);
		newBoneRefName += strlen(newBoneRefName) + 1;
	}

	if (!strcmp(fileData->parent, SKEL_BONENAME_WORLD))
	{
		boneData->parent = -1;
	}
	else
	{
		boneData->parent = GetAssetManager()->GetManager<SkeletorManager>()->GetBoneNamesTable()->RegisterChannel(fileData->parent);
	}

	switch (boneData->boneType)
	{
	case Skeleton::SKELBONE_ROTATION:
	{
		float *baseData = (float *)((char *)fileData + fileData->ofsBaseData);
		boneData->offset[0] = baseData[0];
		boneData->offset[1] = baseData[1];
		boneData->offset[2] = baseData[2];
		break;
	}
	case Skeleton::SKELBONE_IKSHOULDER:
	{
		float *baseData = (float *)((char *)fileData + fileData->ofsBaseData);
		boneData->offset[0] = baseData[4];
		boneData->offset[1] = baseData[5];
		boneData->offset[2] = baseData[6];
		break;
	}
	case Skeleton::SKELBONE_IKELBOW:
	case Skeleton::SKELBONE_IKWRIST:
	{
		float *baseData = (float *)((char *)fileData + fileData->ofsBaseData);
		boneData->length = VectorLength(baseData);
		break;
	}
	case Skeleton::SKELBONE_AVROT:
	{
		float *baseData = (float *)((char *)fileData + fileData->ofsBaseData);
		boneData->length = baseData[0];
		boneData->offset[0] = baseData[1];
		boneData->offset[1] = baseData[2];
		boneData->offset[2] = baseData[3];
		break;
	}
	case Skeleton::SKELBONE_HOSEROT:
	case Skeleton::SKELBONE_HOSEROTBOTH:
	case Skeleton::SKELBONE_HOSEROTPARENT:
	{
		float *baseData = (float *)((char *)fileData + fileData->ofsBaseData);
		boneData->offset[0] = baseData[3];
		boneData->offset[1] = baseData[4];
		boneData->offset[2] = baseData[5];
		boneData->bendRatio = baseData[0];
		boneData->bendMax = baseData[1];
		boneData->spinRatio = baseData[2];
		break;
	}
	default:
		break;
	}
}

bool Skeleton::LoadModel(const char *path)
{
	File_SkelHeader *pHeader;

	FilePtr file = GetFileManager()->OpenFile(path);
	if (!file)
	{
		//TIKI_DPrintf("Tiki:LoadAnim Couldn't load %s\n", path);
		return false;
	}

	std::streamoff length = file->ReadBuffer((void**)&pHeader);

	int32_t header = pHeader->ident;
	if (header != TIKI_SKB_HEADER_IDENT && header != TIKI_SKD_HEADER_IDENT)
	{
		//TIKI_Error("TIKI_LoadSKB: Tried to load '%s' as a skeletal base frame (File has invalid header)\n", path);
		return false;
	}

	int32_t version = pHeader->version;
	if (version < TIKI_SKB_HEADER_VER_3 || version > TIKI_SKD_HEADER_VERSION)
	{
		//TIKI_Error("TIKI_LoadSKB: %s has wrong version (%i should be %i or %i)\n", path, version, TIKI_SKB_HEADER_VER_3, TIKI_SKB_HEADER_VERSION);
		return false;
	}

	name = pHeader->name;
	pLOD = NULL;

	// Import LODs
	for (int32_t i = 0; i < sizeof(pHeader->lodIndex) / sizeof(pHeader->lodIndex[0]); i++) {
		lodIndex[i] = pHeader->lodIndex[i];
	}

	// Import surfaces
	if (version <= TIKI_SKB_HEADER_VERSION) {
		LoadSKBSurfaces(pHeader, length);
	}
	else {
		LoadSKDSurfaces(pHeader, length);
	}

	// Import collapses
	if (version > TIKI_SKB_HEADER_VER_3) {
		LoadCollapses(pHeader, length);
	}

	// Create bones
	if (pHeader->version <= TIKI_SKB_HEADER_VERSION) {
		LoadSKBBones(pHeader, length);
	}
	else {
		LoadSKDBones(pHeader, length);
	}

	if (pHeader->version > TIKI_SKB_HEADER_VER_3)
	{
		LoadBoxes(pHeader, length);

		if (pHeader->version > TIKI_SKB_HEADER_VERSION) {
			LoadMorphs(pHeader, length);
		}
	}

	HashUpdate((uint8_t*)pHeader, (size_t)length);

	return true;
}

bool Skeleton::Load()
{
	const char *Fname = GetFilename().c_str();
	size_t length = GetFilename().length();

	const char* ext = length > 4 ? &Fname[length - 4] : NULL;
	if (ext)
	{
		if (!stricmp(ext, ".skb") || !stricmp(ext, ".skd")) {
			return LoadModel(Fname);
		}
		else {
			return false;
		}
	}

	return true;
}

size_t Skeleton::GetNumSurfaces() const
{
	return Surfaces.size();
}

const Skeleton::Surface *Skeleton::GetSurface(size_t index) const
{
	if (index >= 0 || index < Surfaces.size())
	{
		return &Surfaces[index];
	}
	else
	{
		return nullptr;
	}
}

size_t Skeleton::GetNumBones() const
{
	return Bones.size();
}

const Skeleton::BoneData *Skeleton::GetBone(size_t index) const
{
	return &Bones[index];
}

size_t Skeleton::GetNumMorphTargets() const
{
	return MorphTargets.size();
}

const string& Skeleton::GetMorphTarget(size_t index) const
{
	return MorphTargets.at(index);
}

const char* MOHPC::Skeleton::GetName() const
{
	return name.c_str();
}

void MOHPC::Skeleton::LoadSKBSurfaces(File_SkelHeader* pHeader, size_t length)
{
	File_Surface* oldSurf = (File_Surface*)((uint8_t*)pHeader + pHeader->ofsSurfaces);

	// Setup surfaces
	Surfaces.resize(pHeader->numSurfaces);

	for (uint32_t i = 0; i < pHeader->numSurfaces; i++)
	{
		Surface surf;

		surf.name = oldSurf->name;
		surf.Triangles.resize(oldSurf->numTriangles * 3);
		surf.Vertices.resize(oldSurf->numVerts);

		File_SKB_Vertex* oldVerts = (File_SKB_Vertex*)((uint8_t*)oldSurf + oldSurf->ofsVerts);

		for (int32_t j = 0; j < oldSurf->numVerts; j++)
		{
			SkeletorVertex newVert;
			newVert.normal = oldVerts->normal;
			newVert.textureCoords[0] = oldVerts->texCoords[0];
			newVert.textureCoords[1] = oldVerts->texCoords[1];
			newVert.Weights.resize(oldVerts->numWeights);

			for (int32_t k = 0; k < oldVerts->numWeights; k++)
			{
				SkeletorWeight& newWeight = newVert.Weights[k];
				newWeight.boneIndex = oldVerts->weights[k].boneIndex;
				newWeight.boneWeight = oldVerts->weights[k].boneWeight;
				newWeight.offset = oldVerts->weights[k].offset;
			}

			surf.Vertices[j] = newVert;

			oldVerts = (File_SKB_Vertex*)((uint8_t*)oldVerts + sizeof(File_Weight) * oldVerts->numWeights + (sizeof(File_SKB_Vertex) - sizeof(File_Weight)));
		}

		int32_t* oldTriangles = (int32_t*)((uint8_t*)oldSurf + oldSurf->ofsTriangles);

		for (int32_t j = 0; j < oldSurf->numTriangles * 3; j++)
		{
			surf.Triangles[j] = *oldTriangles;
			oldTriangles++;
		}

		Surfaces[i] = surf;

		oldSurf = (File_Surface*)((uint8_t*)oldSurf + oldSurf->ofsEnd);
	}
}

void MOHPC::Skeleton::LoadSKDSurfaces(File_SkelHeader* pHeader, size_t length)
{
	File_Surface* oldSurf = (File_Surface*)((uint8_t*)pHeader + pHeader->ofsSurfaces);

	// Setup surfaces
	Surfaces.resize(pHeader->numSurfaces);

	for (uint32_t i = 0; i < pHeader->numSurfaces; i++)
	{
		Surface surf;

		surf.name = oldSurf->name;
		surf.Triangles.resize(oldSurf->numTriangles * 3);
		surf.Vertices.resize(oldSurf->numVerts);

		File_SKD_Vertex* oldVerts = (File_SKD_Vertex*)((uint8_t*)oldSurf + oldSurf->ofsVerts);

		for (int32_t j = 0; j < oldSurf->numVerts; j++)
		{
			SkeletorVertex newVert;
			newVert.normal = oldVerts->normal;
			newVert.textureCoords[0] = oldVerts->texCoords[0];
			newVert.textureCoords[1] = oldVerts->texCoords[1];
			newVert.Morphs.reserve(oldVerts->numMorphs);
			newVert.Weights.reserve(oldVerts->numWeights);

			File_Morph* morph = (File_Morph*)((uint8_t*)oldVerts + sizeof(File_SKD_Vertex));

			for (int32_t k = 0; k < oldVerts->numMorphs; k++)
			{
				SkeletorMorph newMorph;
				newMorph.morphIndex = morph->morphIndex;
				newMorph.offset = morph->offset;
				newVert.Morphs.push_back(newMorph);
				morph++;
			}

			File_Weight* weight = (File_Weight*)((uint8_t*)oldVerts + sizeof(File_SKD_Vertex) + sizeof(File_Morph) * oldVerts->numMorphs);

			for (int32_t k = 0; k < oldVerts->numWeights; k++)
			{
				SkeletorWeight newWeight;
				newWeight.boneIndex = weight->boneIndex;
				newWeight.boneWeight = weight->boneWeight;
				newWeight.offset = weight->offset;
				newVert.Weights.push_back(newWeight);
				weight++;
			}

			surf.Vertices[j] = newVert;

			oldVerts = (File_SKD_Vertex*)((uint8_t*)oldVerts + sizeof(File_SKD_Vertex) + sizeof(File_Morph) * oldVerts->numMorphs + sizeof(File_Weight) * oldVerts->numWeights);
		}

		int32_t* oldTriangles = (int32_t*)((uint8_t*)oldSurf + oldSurf->ofsTriangles);

		for (int32_t j = 0; j < oldSurf->numTriangles * 3; j++)
		{
			surf.Triangles[j] = *oldTriangles;
			oldTriangles++;
		}

		Surfaces[i] = surf;

		oldSurf = (File_Surface*)((uint8_t*)oldSurf + oldSurf->ofsEnd);
	}
}

void MOHPC::Skeleton::LoadCollapses(File_SkelHeader* pHeader, size_t length)
{
	File_Surface* oldSurf = (File_Surface*)((uint8_t*)pHeader + pHeader->ofsSurfaces);

	for (uint32_t i = 0; i < pHeader->numSurfaces; i++)
	{
		Surface& surf = Surfaces[i];

		surf.Collapse.resize(oldSurf->numVerts);
		surf.CollapseIndex.resize(oldSurf->numVerts);

		int32_t* oldCollapse = (int32_t*)((uint8_t*)oldSurf + oldSurf->ofsCollapse);
		for (int32_t j = 0; j < oldSurf->numVerts; j++)
		{
			surf.Collapse[j] = *oldCollapse;
			oldCollapse++;
		}

		int32_t* oldCollapseIndex = (int32_t*)((uint8_t*)oldSurf + oldSurf->ofsCollapseIndex);
		for (int32_t j = 0; j < oldSurf->numVerts; j++)
		{
			surf.CollapseIndex[j] = *oldCollapseIndex;
			oldCollapseIndex++;
		}

		oldSurf = (File_Surface*)((uint8_t*)oldSurf + oldSurf->ofsEnd);
	}
}

void MOHPC::Skeleton::LoadSKBBones(File_SkelHeader* pHeader, size_t length)
{
	Bones.resize(pHeader->numBones);

	File_SkelBoneName* TIKI_bones = (File_SkelBoneName*)((uint8_t*)pHeader + pHeader->ofsBones);
	for (uint32_t i = 0; i < pHeader->numBones; i++)
	{
		const char* boneName;

		if (TIKI_bones->parent == -1)
		{
			boneName = SKEL_BONENAME_WORLD;
		}
		else
		{
			boneName = TIKI_bones[TIKI_bones->parent].name;
		}

		CreatePosRotBoneData(TIKI_bones->name, boneName, &Bones[i]);
		TIKI_bones++;
	}
}

void MOHPC::Skeleton::LoadSKDBones(File_SkelHeader* pHeader, size_t length)
{
	Bones.resize(pHeader->numBones);

	BoneFileData* boneBuffer = (BoneFileData*)((uint8_t*)pHeader + pHeader->ofsBones);
	for (uint32_t i = 0; i < pHeader->numBones; i++)
	{
		LoadBoneFromBuffer2(boneBuffer, &Bones[i]);
		boneBuffer = (BoneFileData*)((uint8_t*)boneBuffer + boneBuffer->ofsEnd);
	}
}

void MOHPC::Skeleton::LoadBoxes(File_SkelHeader* pHeader, size_t length)
{
	if (pHeader->numBoxes)
	{
		if (pHeader->ofsBoxes >= 0 && (uint32_t)(pHeader->ofsBoxes + pHeader->numBoxes * sizeof(uint32_t)) < length)
		{
			Boxes.resize(pHeader->numBoxes);

			int32_t* pBoxes = (int32_t*)((uint8_t*)pHeader + pHeader->ofsBoxes);

			for (uint32_t i = 0; i < pHeader->numBoxes; i++)
			{
				Boxes[i] = pBoxes[i];
			}
		}
	}
}

void MOHPC::Skeleton::LoadMorphs(File_SkelHeader* pHeader, size_t length)
{
	if (pHeader->numMorphTargets)
	{
		size_t nMorphBytes = 0;

		if (pHeader->ofsMorphTargets > 0 || (pHeader->ofsMorphTargets + pHeader->numMorphTargets) < length)
		{
			const char* pMorphTargets = (const char*)pHeader + pHeader->ofsMorphTargets;

			for (uint32_t i = 0; i < pHeader->numMorphTargets; i++)
			{
				size_t nLen = strlen(pMorphTargets) + 1;
				nMorphBytes += nLen;
				pMorphTargets += nLen;
			}
		}
		else
		{
			nMorphBytes = pHeader->numMorphTargets;
		}

		if (pHeader->ofsMorphTargets >= 0 && (uint32_t)(pHeader->ofsMorphTargets + nMorphBytes) <= length)
		{
			MorphTargets.resize(pHeader->numMorphTargets);

			const char* pMorphTargets = (const char*)pHeader + pHeader->ofsMorphTargets;

			for (uint32_t i = 0; i < pHeader->numMorphTargets; i++)
			{
				size_t nLen = strlen(pMorphTargets) + 1;
				MorphTargets[i] = pMorphTargets;
				pMorphTargets += nLen;
			}
		}
	}
}
