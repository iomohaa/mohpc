#include <Shared.h>
#include "SkelPrivate.h"

#include <MOHPC/Misc/Endian.h>

using namespace MOHPC;

void Skeleton::BoneFileData::getBaseData(float* outData, size_t num, uintptr_t start) const
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

uint32_t Skeleton::CreateRotationBoneFileData(const char* newBoneName, const char* newBoneParentName, SkelVec3 basePos, Skeleton::BoneFileData* fileData)
{
	char* saveChannelName;
	str rotChannelName;
	Skeleton::BoneData* boneData;

	strncpy(fileData->name, newBoneName, sizeof(fileData->name));
	strncpy(fileData->parent, newBoneParentName, sizeof(fileData->parent));
	fileData->boneType = Skeleton::SKELBONE_ROTATION;
	fileData->ofsBaseData = sizeof(Skeleton::BoneFileData);
	boneData = (Skeleton::BoneData*)((char*)fileData + fileData->ofsBaseData);
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

uint32_t Skeleton::CreatePosRotBoneFileData(char* newBoneName, char* newBoneParentName, Skeleton::BoneFileData* fileData)
{
	int channelNamesLength;
	char* saveChannelName;
	str rotChannelName;
	str posChannelName;
	Skeleton::BoneData* boneData;

	strncpy(fileData->name, newBoneName, sizeof(fileData->name));
	strncpy(fileData->parent, newBoneParentName, sizeof(fileData->parent));
	fileData->boneType = Skeleton::SKELBONE_POSROT;
	fileData->ofsBaseData = sizeof(Skeleton::BoneFileData);
	boneData = (Skeleton::BoneData*)((char*)fileData + fileData->ofsBaseData);
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

void Skeleton::CreatePosRotBoneData(const char* newBoneName, const char* newBoneParentName, Skeleton::BoneData* boneData)
{
	str rotChannelName;
	str posChannelName;

	boneData->channel = GetAssetManager()->GetManager<SkeletorManager>()->GetBoneNamesTable()->RegisterChannel(newBoneName);

	if (!strcmp(newBoneParentName, "worldbone"))
	{
		boneData->parent = -1;
	}
	else
	{
		boneData->parent = GetAssetManager()->GetManager<SkeletorManager>()->GetBoneNamesTable()->RegisterChannel(newBoneParentName);
		assert(boneData->parent >= 0);
	}

	boneData->boneType = Skeleton::SKELBONE_POSROT;

	ConvertToRotationName(newBoneName, rotChannelName);
	boneData->channelIndex[0] = GetAssetManager()->GetManager<SkeletorManager>()->GetChannelNamesTable()->RegisterChannel(rotChannelName.c_str());
	if (boneData->channelIndex[0] < 0)
	{
		//SKEL_Warning( "Channel named %s not added. (Bone will not work without it)\n", rotChannelName );
		boneData->boneType = Skeleton::SKELBONE_ZERO;
	}

	ConvertToPositionName(newBoneName, posChannelName);
	boneData->channelIndex[1] = GetAssetManager()->GetManager<SkeletorManager>()->GetChannelNamesTable()->RegisterChannel(posChannelName.c_str());
	if (boneData->channelIndex[1] < 0)
	{
		//SKEL_Warning( "Channel named %s not added. (Bone will not work without it)\n", posChannelName );
		boneData->boneType = Skeleton::SKELBONE_ZERO;
	}

	boneData->numChannels = 2;
	boneData->numRefs = 0;
}

uint32_t Skeleton::CreateIKShoulderBoneFileData(const char* newBoneName, const char* newBoneParentName, SkelQuat baseOrient, SkelVec3 basePos, Skeleton::BoneData* boneData)
{
	// FIXME: stub
	return 0;
}

uint32_t Skeleton::CreateIKElbowBoneFileData(const char* newBoneName, const char* newBoneParentName, SkelVec3 basePos, Skeleton::BoneData* boneData)
{
	// FIXME: stub
	return 0;
}

uint32_t Skeleton::CreateIKWristBoneFileData(const char* newBoneName, const char* newBoneParentName, const char* shoulderBoneName, SkelVec3 basePos, Skeleton::BoneFileData* fileData)
{
	// FIXME: stub
	return 0;
}

uint32_t Skeleton::CreateHoseRotBoneFileData(char* newBoneName, char* newBoneParentName, char* targetBoneName, float bendRatio, float bendMax, float spinRatio,
	Skeleton::HoseRotType hoseRotType, SkelVec3 basePos, Skeleton::BoneFileData* fileData)
{
	// FIXME: stub
	return 0;
}

uint32_t Skeleton::CreateAvRotBoneFileData(char* newBoneName, char* newBoneParentName, char* baseBoneName, char* targetBoneName, float rotRatio,
	SkelVec3 basePos, Skeleton::BoneFileData* fileData)
{
	// FIXME: stub
	return 0;
}

void Skeletor::LoadMorphTargetNames(Skeleton* skelmodel)
{
	size_t numTargets = skelmodel->GetNumMorphTargets();

	for (size_t i = 0; i < numTargets; i++)
	{
		const char* newTargetName = skelmodel->GetMorphTarget(i);

		size_t newChannel = GetAssetManager()->GetManager<SkeletorManager>()->GetChannelNamesTable()->RegisterChannel(newTargetName);
		size_t morphTargetIndex = m_morphTargetList.AddChannel(newChannel);

		if (!strncmp(newTargetName, "EYES_left", 9))
		{
			m_targetLookLeft = morphTargetIndex;
		}
		else if (!strncmp(newTargetName, "EYES_right", 10))
		{
			m_targetLookRight = morphTargetIndex;
		}
		else if (!strncmp(newTargetName, "EYES_up", 70))
		{
			m_targetLookUp = morphTargetIndex;
		}
		else if (!strncmp(newTargetName, "EYES_down", 9))
		{
			m_targetLookDown = morphTargetIndex;
		}
		else if (!strncmp(newTargetName, "EYES_blink", 9))
		{
			m_targetBlink = morphTargetIndex;
		}
	}
}
