#include <Shared.h>
#include "SkelPrivate.h"

#include <MOHPC/Utility/Misc/Endian.h>

#include <cassert>

using namespace MOHPC;

void BoneFileData::getBaseData(float* outData, size_t num, uintptr_t start) const
{
	const float* baseData = (const float*)((const char*)this + Endian.LittleInteger(ofsBaseData));
	for (uintptr_t i = 0; i < num; ++i)
	{
		memcpy(&outData[i], baseData + start + i, sizeof(float));
		outData[i] = Endian.LittleFloat(outData[i]);
	}
}

void ConvertToRotationName(const char* boneName, str& rotChannelName)
{
	rotChannelName = str(boneName) + " rot";
}

void ConvertToPositionName(const char* boneName, str& posChannelName)
{
	posChannelName = str(boneName) + " rot";
}

void ConvertToFKRotationName(const char* boneName, str& rotChannelName)
{
	rotChannelName = str(boneName) + " rosFK";
}

void ConvertToFKPositionName(const char* boneName, str& posChannelName)
{
	posChannelName = str(boneName) + " posFK";
}

uint32_t SkeletonReader::CreateRotationBoneFileData(const char* newBoneName, const char* newBoneParentName, SkelVec3 basePos, BoneFileData* fileData)
{
	char* saveChannelName;
	str rotChannelName;
	BoneData* boneData;

	strncpy(fileData->name, newBoneName, sizeof(fileData->name));
	strncpy(fileData->parent, newBoneParentName, sizeof(fileData->parent));
	fileData->boneType = BoneType::SKELBONE_ROTATION;
	fileData->ofsBaseData = sizeof(BoneFileData);
	boneData = (BoneData*)((char*)fileData + fileData->ofsBaseData);
	boneData->offset[0] = basePos[0];
	boneData->offset[1] = basePos[1];
	boneData->offset[2] = basePos[2];
	boneData->length = 1.0f;
	boneData->weight = 1.0f;
	boneData->bendRatio = 1.0f;
	fileData->ofsChannelNames = fileData->ofsBaseData + 24;
	saveChannelName = (char*)((char*)fileData + fileData->ofsChannelNames);

	ConvertToRotationName(newBoneName, rotChannelName);
	strcpy(saveChannelName, rotChannelName.c_str());
	fileData->ofsBoneNames = fileData->ofsChannelNames + (int)rotChannelName.length() + 1;
	fileData->ofsEnd = fileData->ofsBoneNames;
	return fileData->ofsEnd;
}

uint32_t SkeletonReader::CreatePosRotBoneFileData(char* newBoneName, char* newBoneParentName, BoneFileData* fileData)
{
	int channelNamesLength;
	char* saveChannelName;
	str rotChannelName;
	str posChannelName;
	BoneData* boneData;

	strncpy(fileData->name, newBoneName, sizeof(fileData->name));
	strncpy(fileData->parent, newBoneParentName, sizeof(fileData->parent));
	fileData->boneType = BoneType::SKELBONE_POSROT;
	fileData->ofsBaseData = sizeof(BoneFileData);
	boneData = (BoneData*)((char*)fileData + fileData->ofsBaseData);
	boneData->offset[0] = 1.0f;
	boneData->offset[1] = 1.0f;
	boneData->offset[2] = 1.0f;
	fileData->ofsChannelNames = fileData->ofsBaseData + 12;
	saveChannelName = (char*)((char*)fileData + fileData->ofsChannelNames);

	ConvertToRotationName(newBoneName, rotChannelName);
	strcpy(saveChannelName, rotChannelName.c_str());

	channelNamesLength = (int)strlen(saveChannelName);
	saveChannelName = (char*)((char*)fileData + fileData->ofsChannelNames + channelNamesLength);
	ConvertToPositionName(newBoneName, posChannelName);
	strcpy(saveChannelName, posChannelName.c_str());

	channelNamesLength += (int)strlen(saveChannelName);

	fileData->ofsBoneNames = fileData->ofsChannelNames + channelNamesLength + 2;
	fileData->ofsEnd = fileData->ofsBoneNames;
	return fileData->ofsEnd;
}

void SkeletonReader::CreatePosRotBoneData(const char* newBoneName, const char* newBoneParentName, BoneData* boneData)
{
	str rotChannelName;
	str posChannelName;

	SkeletonChannelNameTable* const boneNamesTable = GetAssetManager()->getManager<SkeletorManager>()->GetBoneNamesTable();
	SkeletonChannelNameTable* const channelNamesTable = GetAssetManager()->getManager<SkeletorManager>()->GetChannelNamesTable();

	boneData->channel = boneNamesTable->RegisterChannel(newBoneName);

	if (!strHelpers::cmp(newBoneParentName, "worldbone"))
	{
		boneData->parent = -1;
	}
	else
	{
		boneData->parent = boneNamesTable->RegisterChannel(newBoneParentName);
		assert(boneData->parent >= 0);
	}

	boneData->boneType = BoneType::SKELBONE_POSROT;

	ConvertToRotationName(newBoneName, rotChannelName);
	boneData->channelIndex[0] = channelNamesTable->RegisterChannel(rotChannelName.c_str());
	if (boneData->channelIndex[0] < 0)
	{
		//SKEL_Warning( "Channel named %s not added. (Bone will not work without it)\n", rotChannelName );
		boneData->boneType = BoneType::SKELBONE_ZERO;
	}

	ConvertToPositionName(newBoneName, posChannelName);
	boneData->channelIndex[1] = channelNamesTable->RegisterChannel(posChannelName.c_str());
	if (boneData->channelIndex[1] < 0)
	{
		//SKEL_Warning( "Channel named %s not added. (Bone will not work without it)\n", posChannelName );
		boneData->boneType = BoneType::SKELBONE_ZERO;
	}

	boneData->numChannels = 2;
	boneData->numRefs = 0;
}

uint32_t SkeletonReader::CreateIKShoulderBoneFileData(const char* newBoneName, const char* newBoneParentName, SkelQuat baseOrient, SkelVec3 basePos, BoneData* boneData)
{
	// FIXME: stub
	return 0;
}

uint32_t SkeletonReader::CreateIKElbowBoneFileData(const char* newBoneName, const char* newBoneParentName, SkelVec3 basePos, BoneData* boneData)
{
	// FIXME: stub
	return 0;
}

uint32_t SkeletonReader::CreateIKWristBoneFileData(const char* newBoneName, const char* newBoneParentName, const char* shoulderBoneName, SkelVec3 basePos, BoneFileData* fileData)
{
	// FIXME: stub
	return 0;
}

uint32_t SkeletonReader::CreateHoseRotBoneFileData(char* newBoneName, char* newBoneParentName, char* targetBoneName, float bendRatio, float bendMax, float spinRatio,
	HoseRotType hoseRotType, SkelVec3 basePos, BoneFileData* fileData)
{
	// FIXME: stub
	return 0;
}

uint32_t SkeletonReader::CreateAvRotBoneFileData(char* newBoneName, char* newBoneParentName, char* baseBoneName, char* targetBoneName, float rotRatio,
	SkelVec3 basePos, BoneFileData* fileData)
{
	// FIXME: stub
	return 0;
}
