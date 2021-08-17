#include <Shared.h>
#include <MOHPC/Assets/Formats/Skel.h>
#include <MOHPC/Utility/SharedPtr.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Files/FileHelpers.h>
#include <MOHPC/Common/Log.h>
#include <MOHPC/Utility/Misc/Endian.h>
#include <MOHPC/Utility/Misc/EndianCoordHelpers.h>
#include "SkelPrivate.h"
#include "../TIKI/TIKI_Private.h"

#define MOHPC_LOG_NAMESPACE "skeleton"

void MOHPC::AddToBounds(SkelVec3 *bounds, const SkelVec3 *newBounds)
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

MOHPC_OBJECT_DEFINITION(Skeleton);

Skeleton::Skeleton(
	const fs::path& fileName,
	std::vector<BoneData>&& bonesData,
	std::vector<Surface>&& surfacesData,
	uint32_t* lodIndexPtr,
	std::vector<int32_t>&& boxesData,
	std::vector<str>&& morphsData
)
	: Asset2(fileName)
	, bones(bonesData)
	, surfaces(surfacesData)
	, boxes(boxesData)
	, morphTargets(morphsData)
{
	memcpy(lodIndex, lodIndexPtr, sizeof(lodIndex));
}

size_t Skeleton::GetNumSurfaces() const
{
	return surfaces.size();
}

const Surface *Skeleton::GetSurface(size_t index) const
{
	if (index >= 0 || index < surfaces.size())
	{
		return &surfaces[index];
	}
	else
	{
		return nullptr;
	}
}

size_t Skeleton::GetNumBones() const
{
	return bones.size();
}

const BoneData *Skeleton::GetBone(size_t index) const
{
	return &bones[index];
}

const std::vector<BoneData>& Skeleton::getBones() const
{
	return bones;
}

size_t Skeleton::GetNumMorphTargets() const
{
	return morphTargets.size();
}

const char* Skeleton::GetMorphTarget(size_t index) const
{
	return morphTargets.at(index).c_str();
}

std::vector<MOHPC::AnimFrame>& SkeletonAnimation::getFrames()
{
	return m_frame;
}

std::vector<MOHPC::SkanChannelHdr>& SkeletonAnimation::getAryChannels()
{
	return ary_channels;
}

MOHPC_OBJECT_DEFINITION(SkeletonReader);
SkeletonReader::SkeletonReader()
{
}

SkeletonReader::~SkeletonReader()
{
}

Asset2Ptr SkeletonReader::read(const IFilePtr& file)
{
	const fs::path& Fname = file->getName();

	const std::string ext = FileHelpers::getExtension(Fname).generic_string();
	if (ext.length())
	{
		if (!strHelpers::icmp(ext.c_str(), "skb") || !strHelpers::icmp(ext.c_str(), "skd")) {
			return LoadModel(file);
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

	return nullptr;
}

void MOHPC::LoadBoneFromBuffer2(const SkeletorManagerPtr& skeletorManager, const BoneFileData* fileData, BoneData* boneData)
{
	SkeletonChannelNameTable* const boneNamesTable = skeletorManager->GetBoneNamesTable();
	SkeletonChannelNameTable* const channelNamesTable = skeletorManager->GetChannelNamesTable();

	boneData->channel = boneNamesTable->RegisterChannel(fileData->name);
	boneData->boneType = (BoneType)fileData->boneType;

	if (boneData->boneType == BoneType::SKELBONE_HOSEROT)
	{
		uint32_t parentId = Endian.LittleInteger(*(const unsigned int*)&fileData->parent[Endian.LittleInteger(fileData->ofsBaseData) + 4]);

		if (parentId == 1)
		{
			boneData->boneType = BoneType::SKELBONE_HOSEROTPARENT;
		}
		else if (parentId == 2)
		{
			boneData->boneType = BoneType::SKELBONE_HOSEROTBOTH;
		}
	}

	const char* newChannelName = (const char*)fileData + Endian.LittleInteger(fileData->ofsChannelNames);
	boneData->numChannels = skelBone_Base::GetNumChannels(boneData->boneType);

	for (uint16_t i = 0; i < boneData->numChannels; i++)
	{
		boneData->channelIndex[i] = channelNamesTable->RegisterChannel(newChannelName);
		if (boneData->channelIndex[i] < 0)
		{
			MOHPC_LOG(Warn, "Channel named %s not added. (Bone will not work without it)", newChannelName);
			boneData->boneType = BoneType::SKELBONE_ZERO;
		}

		newChannelName += strlen(newChannelName) + 1;
	}

	const char* newBoneRefName = (const char*)fileData + Endian.LittleInteger(fileData->ofsBoneNames);
	boneData->numRefs = skelBone_Base::GetNumBoneRefs(boneData->boneType);

	for (uint16_t i = 0; i < boneData->numRefs; i++)
	{
		boneData->refIndex[i] = boneNamesTable->RegisterChannel(newBoneRefName);
		newBoneRefName += strlen(newBoneRefName) + 1;
	}

	if (!strHelpers::cmp(fileData->parent, SKEL_BONENAME_WORLD))
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
	case BoneType::SKELBONE_ROTATION:
	{
		fileData->getBaseData(baseData, 3);
		boneData->offset[0] = baseData[0];
		boneData->offset[1] = baseData[1];
		boneData->offset[2] = baseData[2];
		break;
	}
	case BoneType::SKELBONE_IKSHOULDER:
	{
		fileData->getBaseData(baseData, 3, 4);
		boneData->offset[0] = baseData[0];
		boneData->offset[1] = baseData[1];
		boneData->offset[2] = baseData[2];
		break;
	}
	case BoneType::SKELBONE_IKELBOW:
	case BoneType::SKELBONE_IKWRIST:
	{
		fileData->getBaseData(baseData, 3);
		boneData->length = VectorLength(baseData);
		break;
	}
	case BoneType::SKELBONE_AVROT:
	{
		fileData->getBaseData(baseData, 4);
		boneData->length = baseData[0];
		boneData->offset[0] = baseData[1];
		boneData->offset[1] = baseData[2];
		boneData->offset[2] = baseData[3];
		break;
	}
	case BoneType::SKELBONE_HOSEROT:
	case BoneType::SKELBONE_HOSEROTBOTH:
	case BoneType::SKELBONE_HOSEROTPARENT:
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

SkeletonPtr SkeletonReader::LoadModel(const IFilePtr& file)
{
	const File_SkelHeader* pHeader;
	// Read the header
	uint64_t length = file->ReadBuffer((void**)&pHeader);

	// Check if the header matches
	if (memcmp(pHeader->ident, TIKI_SKB_HEADER_IDENT, sizeof(pHeader->ident)) && memcmp(pHeader->ident, TIKI_SKD_HEADER_IDENT, sizeof(pHeader->ident)))
	{
		MOHPC_LOG(Error, "Tried to load '%s' as a skeletal base frame (File has invalid header)", file->getName().generic_string().c_str());
		return nullptr;
	}

	// Check if the version matches
	const uint32_t version = Endian.LittleInteger(pHeader->version);
	if (version < TIKI_SKB_HEADER_VER_3 || version > TIKI_SKD_HEADER_VERSION)
	{
		MOHPC_LOG(Error, "%s has wrong version (%i should be %i or %i)\n", file->getName().generic_string().c_str(), version, TIKI_SKB_HEADER_VER_3, TIKI_SKB_HEADER_VERSION);
		return nullptr;
	}

	uint32_t lodIndex[10];

	// Import LODs
	constexpr size_t numLodIndexes = sizeof(pHeader->lodIndex) / sizeof(pHeader->lodIndex[0]);
	for (uintptr_t i = 0; i < numLodIndexes; i++) {
		lodIndex[i] = Endian.LittleInteger(pHeader->lodIndex[i]);
	}

	// Import surfaces
	std::vector<Surface> surfaces;
	if (version <= TIKI_SKB_HEADER_VERSION) {
		LoadSKBSurfaces(pHeader, length, surfaces);
	}
	else {
		LoadSKDSurfaces(pHeader, length, surfaces);
	}

	// Import collapses
	if (version > TIKI_SKB_HEADER_VER_3) {
		LoadCollapses(pHeader, length, surfaces);
	}

	// Create bones
	std::vector<BoneData> bones;
	if (version > TIKI_SKB_HEADER_VERSION) {
		LoadSKDBones(pHeader, length, bones);
	}
	else {
		LoadSKBBones(pHeader, length, bones);
	}

	std::vector<int32_t> boxes;
	std::vector<str> morphs;
	if (version > TIKI_SKB_HEADER_VER_3)
	{
		// Import box data for SKB version 4
		LoadBoxes(file, pHeader, length, boxes);

		if (version > TIKI_SKB_HEADER_VERSION)
		{
			// Import morph data for SKD
			LoadMorphs(file, pHeader, length, morphs);
		}
	}

	return SkeletonPtr(new Skeleton(
		file->getName(),
		std::move(bones),
		std::move(surfaces),
		lodIndex,
		std::move(boxes),
		std::move(morphs)
	));
}

void SkeletonReader::LoadSKBSurfaces(const File_SkelHeader* pHeader, size_t length, std::vector<Surface>& surfaces)
{
	const uint32_t numSurfaces = Endian.LittleInteger(pHeader->numSurfaces);
	const File_Surface* oldSurf = (const File_Surface*)((const uint8_t*)pHeader + Endian.LittleInteger(pHeader->ofsSurfaces));

	// Setup surfaces
	surfaces.reserve(numSurfaces);

	for (uint32_t i = 0; i < numSurfaces; i++)
	{
		Surface& surf = surfaces.emplace_back();
		const uint32_t numTriangles = Endian.LittleInteger(oldSurf->numTriangles);
		const uint32_t numVerts = Endian.LittleInteger(oldSurf->numVerts);

		surf.name = oldSurf->name;
		surf.Triangles.resize(numTriangles * 3);
		surf.Vertices.reserve(numVerts);

		const File_SKB_Vertex* oldVerts = (const File_SKB_Vertex*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsVerts));

		for (uint32_t j = 0; j < numVerts; j++)
		{
			SkeletorVertex& newVert = surf.Vertices.emplace_back();
			const uint32_t numWeights = Endian.LittleInteger(oldVerts->numWeights);

			EndianHelpers::LittleVector(Endian, oldVerts->normal, newVert.normal);
			newVert.textureCoords[0] = Endian.LittleFloat(oldVerts->texCoords[0]);
			newVert.textureCoords[1] = Endian.LittleFloat(oldVerts->texCoords[1]);
			newVert.Weights.reserve(numWeights);

			for (uint32_t k = 0; k < numWeights; k++)
			{
				SkeletorWeight& newWeight = newVert.Weights.emplace_back();
				newWeight.boneIndex = Endian.LittleInteger(oldVerts->weights[k].boneIndex);
				newWeight.boneWeight = Endian.LittleFloat(oldVerts->weights[k].boneWeight);
				EndianHelpers::LittleVector(Endian, oldVerts->weights[k].offset, newWeight.offset);
			}

			oldVerts = (const File_SKB_Vertex*)((const uint8_t*)oldVerts + sizeof(File_Weight) * numWeights + (sizeof(File_SKB_Vertex) - sizeof(File_Weight)));
		}

		const uint32_t* oldTriangles = (const uint32_t*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsTriangles));

		for (uint32_t j = 0; j < numTriangles * 3; j++)
		{
			surf.Triangles[j] = *oldTriangles;
			oldTriangles++;
		}

		oldSurf = (const File_Surface*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsEnd));
	}
}

void SkeletonReader::LoadSKDSurfaces(const File_SkelHeader* pHeader, size_t length, std::vector<Surface>& surfaces)
{
	const uint32_t numSurfaces = Endian.LittleInteger(pHeader->numSurfaces);
	const File_Surface* oldSurf = (const File_Surface*)((const uint8_t*)pHeader + Endian.LittleInteger(pHeader->ofsSurfaces));

	// Setup surfaces
	surfaces.reserve(numSurfaces);

	for (uint32_t i = 0; i < numSurfaces; i++)
	{
		Surface& surf = surfaces.emplace_back();
		const uint32_t numTriangles = Endian.LittleInteger(oldSurf->numTriangles);
		const uint32_t numVerts = Endian.LittleInteger(oldSurf->numVerts);

		surf.name = oldSurf->name;
		surf.Triangles.resize(numTriangles * 3);
		surf.Vertices.reserve(numVerts);

		const File_SKD_Vertex* oldVerts = (const File_SKD_Vertex*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsVerts));

		for (uint32_t j = 0; j < numVerts; j++)
		{
			SkeletorVertex& newVert = surf.Vertices.emplace_back();
			const uint32_t numWeights = Endian.LittleInteger(oldVerts->numWeights);
			const uint32_t numMorphs = Endian.LittleInteger(oldVerts->numMorphs);

			EndianHelpers::LittleVector(Endian, oldVerts->normal, newVert.normal);
			newVert.textureCoords[0] = Endian.LittleFloat(oldVerts->texCoords[0]);
			newVert.textureCoords[1] = Endian.LittleFloat(oldVerts->texCoords[1]);
			newVert.Weights.reserve(numWeights);
			newVert.Morphs.reserve(numMorphs);

			const File_Morph* morph = (const File_Morph*)((const uint8_t*)oldVerts + sizeof(File_SKD_Vertex));

			for (uint32_t k = 0; k < numMorphs; k++)
			{
				SkeletorMorph& newMorph = newVert.Morphs.emplace_back();
				newMorph.morphIndex = Endian.LittleInteger(morph->morphIndex);
				EndianHelpers::LittleVector(Endian, morph->offset, newMorph.offset);
				++morph;
			}

			const File_Weight* weight = (const File_Weight*)((const uint8_t*)oldVerts + sizeof(File_SKD_Vertex) + sizeof(File_Morph) * numMorphs);

			for (uint32_t k = 0; k < numWeights; k++)
			{
				SkeletorWeight& newWeight = newVert.Weights.emplace_back();
				newWeight.boneIndex = Endian.LittleInteger(weight->boneIndex);
				newWeight.boneWeight = Endian.LittleFloat(weight->boneWeight);
				EndianHelpers::LittleVector(Endian, weight->offset, newWeight.offset);
				++weight;
			}

			oldVerts = (const File_SKD_Vertex*)((const uint8_t*)oldVerts + sizeof(File_SKD_Vertex) + sizeof(File_Morph) * numMorphs + sizeof(File_Weight) * numWeights);
		}

		const uint32_t* oldTriangles = (const uint32_t*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsTriangles));

		for (uint32_t j = 0; j < numTriangles * 3; j++)
		{
			surf.Triangles[j] = Endian.LittleInteger(*oldTriangles);
			oldTriangles++;
		}

		oldSurf = (const File_Surface*)((const uint8_t*)oldSurf + Endian.LittleInteger(oldSurf->ofsEnd));
	}
}

void SkeletonReader::LoadCollapses(const File_SkelHeader* pHeader, size_t length, std::vector<Surface>& surfaces)
{
	const uint32_t numSurfaces = Endian.LittleInteger(pHeader->numSurfaces);
	const File_Surface* oldSurf = (const File_Surface*)((const uint8_t*)pHeader + Endian.LittleInteger(pHeader->ofsSurfaces));

	for (uint32_t i = 0; i < numSurfaces; i++)
	{
		Surface& surf = surfaces[i];
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

void SkeletonReader::LoadSKBBones(const File_SkelHeader* pHeader, size_t length, std::vector<BoneData>& bones)
{
	const uint32_t numBones = Endian.LittleInteger(pHeader->numBones);
	bones.resize(numBones);

	MOHPC_LOG(Debug, "Old skeleton (SKB), creating bone data");

	const File_SkelBoneName* TIKI_bones = (const File_SkelBoneName*)((const uint8_t*)pHeader + Endian.LittleInteger(pHeader->ofsBones));
	for (uint32_t i = 0; i < numBones; i++)
	{
		const char* boneName;

		const uint16_t parent = Endian.LittleShort(TIKI_bones->parent);
		if (parent == uint16_t(-1)) {
			boneName = SKEL_BONENAME_WORLD;
		}
		else {
			boneName = TIKI_bones[parent].name;
		}

		CreatePosRotBoneData(TIKI_bones->name, boneName, &bones[i]);
		TIKI_bones++;
	}
}

void SkeletonReader::LoadSKDBones(const File_SkelHeader* pHeader, size_t length, std::vector<BoneData>& bones)
{
	const uint32_t numBones = Endian.LittleInteger(pHeader->numBones);
	bones.resize(numBones);

	const BoneFileData* boneBuffer = (const BoneFileData*)((const uint8_t*)pHeader + Endian.LittleInteger(pHeader->ofsBones));
	for (uint32_t i = 0; i < numBones; i++)
	{
		LoadBoneFromBuffer2(GetAssetManager()->getManager<SkeletorManager>(), boneBuffer, &bones[i]);
		boneBuffer = (const BoneFileData*)((const uint8_t*)boneBuffer + Endian.LittleInteger(boneBuffer->ofsEnd));
	}
}

void SkeletonReader::LoadBoxes(const IFilePtr& file, const File_SkelHeader* pHeader, size_t length, std::vector<int32_t>& boxes)
{
	const uint32_t numBoxes = Endian.LittleInteger(pHeader->numBoxes);
	if (numBoxes)
	{
		const uint32_t ofsBoxes = Endian.LittleInteger(pHeader->ofsBoxes);
		if (ofsBoxes >= 0 && (uint32_t)(ofsBoxes + numBoxes * sizeof(uint32_t)) < length)
		{
			boxes.resize(numBoxes);

			const int32_t* pBoxes = (const int32_t*)((const uint8_t*)pHeader + ofsBoxes);
			for (uint32_t i = 0; i < numBoxes; i++)
			{
				boxes[i] = Endian.LittleInteger(pBoxes[i]);
			}
		}
		else {
			MOHPC_LOG(Warn, "Box data is corrupted for '%s'", file->getName().generic_string().c_str());
		}
	}
}

void SkeletonReader::LoadMorphs(const IFilePtr& file, const File_SkelHeader* pHeader, size_t length, std::vector<str>& morphs)
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
				size_t nLen = strHelpers::len(pMorphTargets) + 1;
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
			morphs.resize(numMorphTargets);

			const char* pMorphTargets = (const char*)pHeader + ofsMorphTargets;

			for (uint32_t i = 0; i < numMorphTargets; i++)
			{
				size_t nLen = strHelpers::len(pMorphTargets) + 1;
				morphs[i] = pMorphTargets;
				pMorphTargets += nLen;
			}
		}
		else {
			MOHPC_LOG(Warn, "Morph targets data is corrupted for '%s'", file->getName().generic_string().c_str());
		}
	}
}

SkelError::BadExtension::BadExtension(const fs::path& inExtension)
	: extension(inExtension)
{
}

const fs::path& SkelError::BadExtension::getExtension() const
{
	return extension;
}

const char* SkelError::BadExtension::what() const noexcept
{
	return "Bad or unsupported skeleton extension";
}

const char* SkelError::NoExtension::what() const noexcept
{
	return "No extension was specified for loading the skeleton file";
}
