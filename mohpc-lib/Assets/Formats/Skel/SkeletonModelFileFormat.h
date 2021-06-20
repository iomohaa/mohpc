#pragma once

#include <MOHPC/Assets/Formats/Skel/SkeletonNameLists.h>

namespace MOHPC
{
	static constexpr char SKEL_BONENAME_WORLD[] = "worldbone";
	static constexpr size_t MAX_NAME_LENGTH = 64;

	struct Skeleton::BoneFileData
	{
		char name[MAX_CHANNEL_NAME];
		char parent[MAX_CHANNEL_NAME];
		uint8_t boneType;
		uint32_t ofsBaseData;
		uint32_t ofsChannelNames;
		uint32_t ofsBoneNames;
		uint32_t ofsEnd;

	public:
		void getBaseData(float* outData, size_t num, uintptr_t start = 0) const;
	};

	struct Skeleton::File_SkelHeader
	{
		char ident[4];
		uint32_t version;
		char name[MAX_NAME_LENGTH];
		uint32_t numSurfaces;
		uint32_t numBones;
		uint32_t ofsBones;
		uint32_t ofsSurfaces;
		uint32_t ofsEnd;
		uint32_t lodIndex[10];
		uint32_t numBoxes;
		uint32_t ofsBoxes;
		uint32_t numMorphTargets;
		uint32_t ofsMorphTargets;
	};

	struct Skeleton::File_SkelBoneName
	{
		uint16_t parent;
		uint16_t boxIndex;
		uint32_t flags;
		char name[MAX_NAME_LENGTH];
	};

	struct Skeleton::File_Weight
	{
		uint32_t boneIndex;
		float boneWeight;
		vec3_t offset;
	};

	struct Skeleton::File_Morph
	{
		uint32_t morphIndex;
		vec3_t offset;
	};

	struct Skeleton::File_SKB_Vertex
	{
		vec3_t normal;
		vec2_t texCoords;
		uint32_t numWeights;
		Skeleton::File_Weight weights[1];
	};


	struct Skeleton::File_SKD_Vertex
	{
		vec3_t normal;
		vec2_t texCoords;
		uint32_t numWeights;
		uint32_t numMorphs;
	};

	struct Skeleton::File_Surface
	{
		int32_t ident;
		char name[MAX_NAME_LENGTH];
		uint32_t numTriangles;
		uint32_t numVerts;
		uint32_t staticSurfProcessed;
		uint32_t ofsTriangles;
		uint32_t ofsVerts;
		uint32_t ofsCollapse;
		uint32_t ofsEnd;
		uint32_t ofsCollapseIndex;
	};

}
