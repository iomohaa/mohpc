#pragma once

static const char *SKEL_BONENAME_WORLD = "worldbone";

struct Skeleton::BoneFileData {
	char name[ 32 ];
	char parent[ 32 ];
	Skeleton::BoneType boneType;
	int ofsBaseData;
	int ofsChannelNames;
	int ofsBoneNames;
	int ofsEnd;
};

struct Skeleton::File_SkelHeader
{
	int32_t ident;
	int32_t version;
	char name[ 64 ];
	uint32_t numSurfaces;
	uint32_t numBones;
	int32_t ofsBones;
	int32_t ofsSurfaces;
	int32_t ofsEnd;
	int32_t lodIndex[ 10 ];
	uint32_t numBoxes;
	int32_t ofsBoxes;
	uint32_t numMorphTargets;
	int32_t ofsMorphTargets;
};

struct Skeleton::File_SkelBoneName
{
	int16_t parent;
	int16_t boxIndex;
	int32_t flags;
	char name[64];
};

struct Skeleton::File_Weight
{
	int32_t boneIndex;
	float boneWeight;
	vec3_t offset;
};

struct Skeleton::File_Morph
{
	int32_t morphIndex;
	vec3_t offset;
};

struct Skeleton::File_SKB_Vertex
{
	vec3_t normal;
	vec2_t texCoords;
	int32_t numWeights;
	Skeleton::File_Weight weights[1];
};


struct Skeleton::File_SKD_Vertex
{
	vec3_t normal;
	vec2_t texCoords;
	int32_t numWeights;
	int32_t numMorphs;
};

struct Skeleton::File_Surface
{
	int ident;
	char name[64];
	int numTriangles;
	int numVerts;
	int staticSurfProcessed;
	int ofsTriangles;
	int ofsVerts;
	int ofsCollapse;
	int ofsEnd;
	int ofsCollapseIndex;
};

