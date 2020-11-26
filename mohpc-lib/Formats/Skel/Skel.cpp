#include <Shared.h>
#include <MOHPC/Formats/Skel.h>
#include <MOHPC/Utilities/SharedPtr.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Log.h>
#include "SkelPrivate.h"
#include "../TIKI/TIKI_Private.h"
#include <MOHPC/Misc/Endian.h>
#include <MOHPC/Misc/EndianHelpers.h>

#define MOHPC_LOG_NAMESPACE "skeleton"

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
	SkeletonChannelNameTable* const boneNamesTable = GetAssetManager()->GetManager<SkeletorManager>()->GetBoneNamesTable();

	boneData->channel = boneNamesTable->RegisterChannel(fileData->name);
	boneData->boneType = (BoneType)fileData->boneType;

	if (boneData->boneType == Skeleton::SKELBONE_HOSEROT)
	{
		uint32_t parentId = Endian.LittleInteger(*(const unsigned int*)&fileData->parent[Endian.LittleInteger(fileData->ofsBaseData) + 4]);

		if (parentId == 1)
		{
			boneData->boneType = Skeleton::SKELBONE_HOSEROTPARENT;
		}
		else if (parentId == 2)
		{
			boneData->boneType = Skeleton::SKELBONE_HOSEROTBOTH;
		}
	}

	const char* newChannelName = (const char *)fileData + Endian.LittleInteger(fileData->ofsChannelNames);
	boneData->numChannels = skelBone_Base::GetNumChannels(boneData->boneType);

	for (uint16_t i = 0; i < boneData->numChannels; i++)
	{
		boneData->channelIndex[i] = boneNamesTable->RegisterChannel(newChannelName);
		if (boneData->channelIndex[i] < 0)
		{
			MOHPC_LOG(Warn, "Channel named %s not added. (Bone will not work without it)", newChannelName);
			boneData->boneType = Skeleton::SKELBONE_ZERO;
		}

		newChannelName += strlen(newChannelName) + 1;
	}

	const char* newBoneRefName = (const char *)fileData + Endian.LittleInteger(fileData->ofsBoneNames);
	boneData->numRefs = skelBone_Base::GetNumBoneRefs(boneData->boneType);

	for (uint16_t i = 0; i < boneData->numRefs; i++)
	{
		boneData->refIndex[i] = boneNamesTable->RegisterChannel(newBoneRefName);
		newBoneRefName += strlen(newBoneRefName) + 1;
	}

	if (!strcmp(fileData->parent, SKEL_BONENAME_WORLD))
	{
		// no parent if it's the worldBone in the file
		boneData->parent = -1;
	}
	else
	{
		boneData->parent = boneNamesTable->RegisterChannel(fileData->parent);
	}

	float baseData[7];

	switch (boneData->boneType)
	{
	case Skeleton::SKELBONE_ROTATION:
	{
		fileData->getBaseData(baseData, 3);
		boneData->offset[0] = baseData[0];
		boneData->offset[1] = baseData[1];
		boneData->offset[2] = baseData[2];
		break;
	}
	case Skeleton::SKELBONE_IKSHOULDER:
	{
		fileData->getBaseData(baseData, 3, 4);
		boneData->offset[0] = baseData[4];
		boneData->offset[1] = baseData[5];
		boneData->offset[2] = baseData[6];
		break;
	}
	case Skeleton::SKELBONE_IKELBOW:
	case Skeleton::SKELBONE_IKWRIST:
	{
		fileData->getBaseData(baseData, 3);
		boneData->length = VectorLength(baseData);
		break;
	}
	case Skeleton::SKELBONE_AVROT:
	{
		fileData->getBaseData(baseData, 4);
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
		fileData->getBaseData(baseData, 6);
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
	// Load file
	FilePtr file = GetFileManager()->OpenFile(path);
	if (!file)
	{
		MOHPC_LOG(Error, "Couldn't load '%s'", path);
		return false;
	}

	const File_SkelHeader* pHeader;
	// Read the header
	uint64_t length = file->ReadBuffer((void**)&pHeader);

	// Check if the header matches
	if (memcmp(pHeader->ident, TIKI_SKB_HEADER_IDENT, sizeof(pHeader->ident)) && memcmp(pHeader->ident, TIKI_SKD_HEADER_IDENT, sizeof(pHeader->ident)))
	{
		MOHPC_LOG(Error, "Tried to load '%s' as a skeletal base frame (File has invalid header)", path);
		return false;
	}

	// Check if the version matches
	const uint32_t version = Endian.LittleInteger(pHeader->version);
	if (version < TIKI_SKB_HEADER_VER_3 || version > TIKI_SKD_HEADER_VERSION)
	{
		MOHPC_LOG(Error, "%s has wrong version (%i should be %i or %i)\n", path, version, TIKI_SKB_HEADER_VER_3, TIKI_SKB_HEADER_VERSION);
		return false;
	}

	name = pHeader->name;
	pLOD = nullptr;

	// Import LODs
	constexpr size_t numLodIndexes = sizeof(pHeader->lodIndex) / sizeof(pHeader->lodIndex[0]);
	for (uintptr_t i = 0; i < numLodIndexes; i++) {
		lodIndex[i] = Endian.LittleInteger(pHeader->lodIndex[i]);
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
	if (version > TIKI_SKB_HEADER_VERSION) {
		LoadSKDBones(pHeader, length);
	}
	else {
		LoadSKBBones(pHeader, length);
	}

	if (version > TIKI_SKB_HEADER_VER_3)
	{
		// Import box data for SKB version 4
		LoadBoxes(pHeader, length);

		if (version > TIKI_SKB_HEADER_VERSION)
		{
			// Import morph data for SKD
			LoadMorphs(pHeader, length);
		}
	}

	HashUpdate((const uint8_t*)pHeader, (size_t)length);

	return true;
}

void Skeleton::Load()
{
	const char *Fname = GetFilename().c_str();
	size_t length = GetFilename().length();

	const char* ext = length > 4 ? &Fname[length - 4] : nullptr;
	if (ext)
	{
		if (!str::icmp(ext, ".skb") || !str::icmp(ext, ".skd")) {
			LoadModel(Fname);
		}
		else
		{
			MOHPC_LOG(Error, "Invalid extension '%s'", ext);
			throw SkelError::BadExtension(ext);
		}
	}
	else {
		throw SkelError::NoExtension();
	}
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

const char* Skeleton::GetMorphTarget(size_t index) const
{
	return MorphTargets.at(index).c_str();
}

const char* MOHPC::Skeleton::GetName() const
{
	return name.c_str();
}

void MOHPC::Skeleton::LoadSKBSurfaces(const File_SkelHeader* pHeader, size_t length)
{
	const uint32_t numSurfaces = Endian.LittleInteger(pHeader->numSurfaces);
	const File_Surface* oldSurf = (const File_Surface*)((const uint8_t*)pHeader + Endian.LittleInteger(pHeader->ofsSurfaces));

	// Setup surfaces
	Surfaces.reserve(numSurfaces);

	for (uint32_t i = 0; i < numSurfaces; i++)
	{
		Surface* surf = new(Surfaces) Surface();
		const uint32_t numTriangles = Endian.LittleInteger(oldSurf->numTriangles);
		const uint32_t numVerts = Endian.LittleInteger(oldSurf->numVerts);

		surf->name = oldSurf->name;
		surf->Triangles.resize(numTriangles * 3);
		surf->Vertices.reserve(numVerts);

		const File_SKB_Vertex* oldVerts = (const File_SKB_Vertex*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsVerts));

		for (uint32_t j = 0; j < numVerts; j++)
		{
			SkeletorVertex* newVert = new(surf->Vertices) SkeletorVertex();
			const uint32_t numWeights = Endian.LittleInteger(oldVerts->numWeights);

			newVert->normal = EndianHelpers::LittleVector(Endian, oldVerts->normal);
			newVert->textureCoords[0] = Endian.LittleFloat(oldVerts->texCoords[0]);
			newVert->textureCoords[1] = Endian.LittleFloat(oldVerts->texCoords[1]);
			newVert->Weights.reserve(numWeights);

			for (uint32_t k = 0; k < numWeights; k++)
			{
				SkeletorWeight* newWeight = new (newVert->Weights) SkeletorWeight();
				newWeight->boneIndex = Endian.LittleInteger(oldVerts->weights[k].boneIndex);
				newWeight->boneWeight = Endian.LittleFloat(oldVerts->weights[k].boneWeight);
				newWeight->offset = EndianHelpers::LittleVector(Endian, oldVerts->weights[k].offset);
			}

			oldVerts = (const File_SKB_Vertex*)((const uint8_t*)oldVerts + sizeof(File_Weight) * numWeights + (sizeof(File_SKB_Vertex) - sizeof(File_Weight)));
		}

		const uint32_t* oldTriangles = (const uint32_t*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsTriangles));

		for (uint32_t j = 0; j < numTriangles * 3; j++)
		{
			surf->Triangles[j] = *oldTriangles;
			oldTriangles++;
		}

		oldSurf = (const File_Surface*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsEnd));
	}
}

void MOHPC::Skeleton::LoadSKDSurfaces(const File_SkelHeader* pHeader, size_t length)
{
	const uint32_t numSurfaces = Endian.LittleInteger(pHeader->numSurfaces);
	const File_Surface* oldSurf = (const File_Surface*)((const uint8_t*)pHeader + Endian.LittleInteger(pHeader->ofsSurfaces));

	// Setup surfaces
	Surfaces.reserve(numSurfaces);

	for (uint32_t i = 0; i < numSurfaces; i++)
	{
		Surface* surf = new(Surfaces) Surface();
		const uint32_t numTriangles = Endian.LittleInteger(oldSurf->numTriangles);
		const uint32_t numVerts = Endian.LittleInteger(oldSurf->numVerts);

		surf->name = oldSurf->name;
		surf->Triangles.resize(numTriangles * 3);
		surf->Vertices.reserve(numVerts);

		const File_SKD_Vertex* oldVerts = (const File_SKD_Vertex*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsVerts));

		for (uint32_t j = 0; j < numVerts; j++)
		{
			SkeletorVertex* newVert = new(surf->Vertices) SkeletorVertex();
			const uint32_t numWeights = Endian.LittleInteger(oldVerts->numWeights);
			const uint32_t numMorphs = Endian.LittleInteger(oldVerts->numMorphs);

			newVert->normal = EndianHelpers::LittleVector(Endian, oldVerts->normal);
			newVert->textureCoords[0] = Endian.LittleFloat(oldVerts->texCoords[0]);
			newVert->textureCoords[1] = Endian.LittleFloat(oldVerts->texCoords[1]);
			newVert->Weights.reserve(numWeights);
			newVert->Morphs.reserve(numMorphs);

			const File_Morph* morph = (const File_Morph*)((const uint8_t*)oldVerts + sizeof(File_SKD_Vertex));

			for (uint32_t k = 0; k < numMorphs; k++)
			{
				SkeletorMorph* newMorph = new(newVert->Morphs) SkeletorMorph();
				newMorph->morphIndex = Endian.LittleInteger(morph->morphIndex);
				newMorph->offset = EndianHelpers::LittleVector(Endian, morph->offset);
				++morph;
			}

			const File_Weight* weight = (const File_Weight*)((const uint8_t*)oldVerts + sizeof(File_SKD_Vertex) + sizeof(File_Morph) * numMorphs);

			for (uint32_t k = 0; k < numWeights; k++)
			{
				SkeletorWeight* newWeight = new(newVert->Weights) SkeletorWeight();
				newWeight->boneIndex = Endian.LittleInteger(weight->boneIndex);
				newWeight->boneWeight = Endian.LittleFloat(weight->boneWeight);
				newWeight->offset = EndianHelpers::LittleVector(Endian, weight->offset);
				++weight;
			}

			oldVerts = (const File_SKD_Vertex*)((const uint8_t*)oldVerts + sizeof(File_SKD_Vertex) + sizeof(File_Morph) * numMorphs + sizeof(File_Weight) * numWeights);
		}

		const uint32_t* oldTriangles = (const uint32_t*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsTriangles));

		for (uint32_t j = 0; j < numTriangles * 3; j++)
		{
			surf->Triangles[j] = Endian.LittleInteger(*oldTriangles);
			oldTriangles++;
		}

		oldSurf = (const File_Surface*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsEnd));
	}
}

void MOHPC::Skeleton::LoadCollapses(const File_SkelHeader* pHeader, size_t length)
{
	const uint32_t numSurfaces = Endian.LittleInteger(pHeader->numSurfaces);
	const File_Surface* oldSurf = (const File_Surface*)((const uint8_t*)pHeader + Endian.LittleInteger(pHeader->ofsSurfaces));

	for (uint32_t i = 0; i < numSurfaces; i++)
	{
		Surface& surf = Surfaces[i];
		const uint32_t numVerts = Endian.LittleInteger(oldSurf->numVerts);

		surf.Collapse.resize(numVerts);
		surf.CollapseIndex.resize(numVerts);

		const uint32_t* oldCollapse = (const uint32_t*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsCollapse));
		for (uint32_t j = 0; j < numVerts; j++)
		{
			surf.Collapse[j] = *oldCollapse;
			oldCollapse++;
		}

		const uint32_t* oldCollapseIndex = (const uint32_t*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsCollapseIndex));
		for (uint32_t j = 0; j < numVerts; j++)
		{
			surf.CollapseIndex[j] = *oldCollapseIndex;
			oldCollapseIndex++;
		}

		oldSurf = (const File_Surface*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsEnd));
	}
}

void MOHPC::Skeleton::LoadSKBBones(const File_SkelHeader* pHeader, size_t length)
{
	const uint32_t numBones = Endian.LittleInteger(pHeader->numBones);
	Bones.resize(numBones);

	MOHPC_LOG(Debug, "Old skeleton (SKB), creating bone data");

	const File_SkelBoneName* TIKI_bones = (const File_SkelBoneName*)((const uint8_t*)pHeader + Endian.LittleInteger(pHeader->ofsBones));
	for (uint32_t i = 0; i < numBones; i++)
	{
		const char* boneName;

		const uint32_t parent = Endian.LittleInteger(TIKI_bones->parent);
		if (parent == -1) {
			boneName = SKEL_BONENAME_WORLD;
		}
		else {
			boneName = TIKI_bones[parent].name;
		}

		CreatePosRotBoneData(TIKI_bones->name, boneName, &Bones[i]);
		TIKI_bones++;
	}
}

void MOHPC::Skeleton::LoadSKDBones(const File_SkelHeader* pHeader, size_t length)
{
	const uint32_t numBones = Endian.LittleInteger(pHeader->numBones);
	Bones.resize(numBones);

	const BoneFileData* boneBuffer = (const BoneFileData*)((const uint8_t*)pHeader + Endian.LittleInteger(pHeader->ofsBones));
	for (uint32_t i = 0; i < numBones; i++)
	{
		LoadBoneFromBuffer2(boneBuffer, &Bones[i]);
		boneBuffer = (const BoneFileData*)((const uint8_t*)boneBuffer + Endian.LittleInteger(boneBuffer->ofsEnd));
	}
}

void MOHPC::Skeleton::LoadBoxes(const File_SkelHeader* pHeader, size_t length)
{
	const uint32_t numBoxes = Endian.LittleInteger(pHeader->numBoxes);
	if (numBoxes)
	{
		const uint32_t ofsBoxes = Endian.LittleInteger(pHeader->ofsBoxes);
		if (ofsBoxes >= 0 && (uint32_t)(ofsBoxes + numBoxes * sizeof(uint32_t)) < length)
		{
			Boxes.SetNumObjectsUninitialized(numBoxes);

			const int32_t* pBoxes = (const int32_t*)((const uint8_t*)pHeader + ofsBoxes);
			for (uint32_t i = 0; i < numBoxes; i++)
			{
				Boxes[i] = Endian.LittleInteger(pBoxes[i]);
			}
		}
		else {
			MOHPC_LOG(Warn, "Box data is corrupted for '%s'", GetFilename().c_str());
		}
	}
}

void MOHPC::Skeleton::LoadMorphs(const File_SkelHeader* pHeader, size_t length)
{
	const uint32_t numMorphTargets = Endian.LittleInteger(pHeader->numMorphTargets);
	if (numMorphTargets)
	{
		size_t nMorphBytes = 0;

		const uint32_t ofsMorphTargets = Endian.LittleInteger(pHeader->ofsMorphTargets);
		if (ofsMorphTargets > 0 || (ofsMorphTargets + numMorphTargets) < length)
		{
			const char* pMorphTargets = (const char*)pHeader + ofsMorphTargets;

			for (uint32_t i = 0; i < numMorphTargets; i++)
			{
				size_t nLen = strlen(pMorphTargets) + 1;
				nMorphBytes += nLen;
				pMorphTargets += nLen;
			}
		}
		else
		{
			nMorphBytes = numMorphTargets;
		}

		if (ofsMorphTargets >= 0 && (uint32_t)(ofsMorphTargets + nMorphBytes) <= length)
		{
			MorphTargets.SetNumObjects(numMorphTargets);

			const char* pMorphTargets = (const char*)pHeader + ofsMorphTargets;

			for (uint32_t i = 0; i < numMorphTargets; i++)
			{
				size_t nLen = strlen(pMorphTargets) + 1;
				MorphTargets[i] = pMorphTargets;
				pMorphTargets += nLen;
			}
		}
		else {
			MOHPC_LOG(Warn, "Morph targets data is corrupted for '%s'", GetFilename().c_str());
		}
	}
}

MOHPC::SkelError::BadExtension::BadExtension(const str& inExtension)
	: extension(inExtension)
{
}

const char* MOHPC::SkelError::BadExtension::getExtension() const
{
	return extension.c_str();
}

const char* MOHPC::SkelError::BadExtension::what() const
{
	return "Bad or unsupported skeleton extension";
}

const char* MOHPC::SkelError::NoExtension::what() const
{
	return "No extension was specified for loading the skeleton file";
}
