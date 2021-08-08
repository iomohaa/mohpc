#include <Shared.h>
#include "SkelPrivate.h"
#include "../TIKI/TIKI_Private.h"
#include "../../../Common/VectorPrivate.h"

#include <chrono>

using namespace MOHPC;

constexpr float EPSILON = 0.000000000001f;

MOHPC_OBJECT_DEFINITION(SkeletorManager);
SkeletorManager::SkeletorManager()
{
	m_boneNames = new SkeletonChannelNameTable;
	m_channelNames = new SkeletonChannelNameTable;
	m_worldBone = new skelBone_World;
}

SkeletorManager::~SkeletorManager()
{
	delete m_worldBone;
	delete m_channelNames;
	delete m_boneNames;
}

SkeletonChannelNameTable *SkeletorManager::GetBoneNamesTable() const
{
	return m_boneNames;
}

SkeletonChannelNameTable *SkeletorManager::GetChannelNamesTable() const
{
	return m_channelNames;
}

skelBone_World *SkeletorManager::GetWorldBone() const
{
	return m_worldBone;
}

size_t SkeletonAnimation::GetFrameNums(float timeSeconds, float timeTolerance, size_t *beforeFrame, size_t *afterFrame, float *beforeWeight, float *afterWeight) const
{
	size_t frameNum1;
	size_t frameNum2;

	size_t numFrames = m_frame.size();

	frameNum1 = (int)(timeSeconds / frameTime);
	frameNum2 = frameNum1 + 1;

	*afterWeight = (timeSeconds / frameTime) - (float)frameNum1;

	if ((1.0 - *afterWeight) * frameTime < timeTolerance ||
		*afterWeight * frameTime < timeTolerance)
	{
		if (*afterWeight > 0.5)
			frameNum1++;

		if (frameNum1 >= numFrames)
		{
			if ((flags & TAF_DELTADRIVEN)) {
				frameNum1 %= numFrames;
			}
			else {
				frameNum1 = numFrames - 1;
			}
		}

		*beforeFrame = frameNum1;
		*beforeWeight = 1.0;
		*afterFrame = 0;
		*afterWeight = 0.0;

		return 1;
	}
	else if (frameNum2 >= numFrames)
	{
		if ((flags & TAF_DELTADRIVEN)) {
			frameNum2 %= numFrames;
		}
		else {
			frameNum2 = numFrames - 1;
		}

		if (frameNum1 >= numFrames)
		{
			if ((flags & TAF_DELTADRIVEN))
			{
				frameNum1 %= numFrames;
			}
			else
			{
				*beforeFrame = numFrames - 1;
				*beforeWeight = 1.0;
				return 1;
			}
		}
	}

	*beforeFrame = frameNum1;
	*afterFrame = frameNum2;
	*beforeWeight = 1.0f - *afterWeight;

	return 2;
}

void SkeletonAnimation::GetDeltaOverTime(float time1, float time2, vec3r_t delta)
{
	float deltaWeight1;
	int frameNum1;
	float deltaWeight2;
	int frameNum2;
	int currFrame;
	float s, d;

	deltaWeight1 = time1 / frameTime;
	deltaWeight2 = time2 / frameTime;
	frameNum1 = (int)(deltaWeight1 + 1.0);
	frameNum2 = (int)(deltaWeight2 + 1.0);

	size_t numFrames = m_frame.size();

	d = frameNum1 - (time1 / frameTime);
	s = 1.0f - (frameNum2 - (time2 / frameTime));

	if (frameNum1 < frameNum2)
	{
		delta[0] = m_frame[frameNum1 % numFrames].delta[0];
		delta[1] = m_frame[frameNum1 % numFrames].delta[1];
		delta[2] = m_frame[frameNum1 % numFrames].delta[2];

		for (currFrame = frameNum1 + 1; currFrame < frameNum2; currFrame++)
		{
			delta[0] += m_frame[currFrame % numFrames].delta[0];
			delta[1] += m_frame[currFrame % numFrames].delta[1];
			delta[2] += m_frame[currFrame % numFrames].delta[2];
		}
	}
	else
	{
		s = s - (1.0f - d);
	}

	delta[0] += m_frame[frameNum2 % numFrames].delta[0] * s;
	delta[1] += m_frame[frameNum2 % numFrames].delta[1] * s;
	delta[2] += m_frame[frameNum2 % numFrames].delta[2] * s;

	if (delta[0] > -0.001f && delta[0] < 0.001f) {
		delta[0] = 0;
	}
	if (delta[1] > -0.001f && delta[1] < 0.001f) {
		delta[1] = 0;
	}
	if (delta[2] > -0.001f && delta[2] < 0.001f) {
		delta[2] = 0;
	}
}

Skeletor::Skeletor(TIKI *tiki)
	: Skeletor()
{
	m_numBones = tiki->GetBoneList()->NumChannels();
	m_bone = new skelBone_Base*[m_numBones];

	size_t numMeshes = tiki->GetNumMeshes();
	for (size_t mesh = 0; mesh < numMeshes; mesh++)
	{
		const SkeletonPtr skelmodel = tiki->GetMesh(mesh);
		skelmodel->LoadBonesFromBuffer(tiki->GetBoneList(), m_bone);
		LoadMorphTargetNames(skelmodel.get());
	}

	m_morphTargetList.PackChannels();
	m_headBoneIndex = tiki->GetBoneNumFromName("Bip01 Head");
}

Skeletor::Skeletor(Skeleton* skelmodel)
{
	m_numBones = skelmodel->GetNumBones();

	SkeletonChannelList boneList;

	for (size_t i = 0; i < m_numBones; i++)
	{
		const Skeleton::BoneData* boneData = skelmodel->GetBone(i);
		boneList.AddChannel(boneData->channel);
	}

	boneList.PackChannels();

	skelmodel->LoadBonesFromBuffer(&boneList, m_bone);
	LoadMorphTargetNames(skelmodel);

	m_morphTargetList.PackChannels();

	intptr_t iGlobalChannel = GetAssetManager()->GetManager<SkeletorManager>()->GetBoneNamesTable()->FindNameLookup("Bip01 Head");
	if (iGlobalChannel < 0)
	{
		m_headBoneIndex = -1;
	}

	m_headBoneIndex = boneList.GetLocalFromGlobal(iGlobalChannel);
}

Skeletor::Skeletor()
{
	m_morphTargetList.InitChannels();
	m_morphTargetList.ZeroChannels();

	m_frameBounds[0].set(-32, -32, 0);
	m_frameBounds[1].set(32, 32, 28);
	m_frameRadius = 64;
	m_frameList.numActionFrames = 0;
	m_frameList.numMovementFrames = 0;
	m_targetLookLeft = 0;
	m_targetLookRight = 0;
	m_targetLookUp = 0;
	m_targetLookDown = 0;
	m_targetLookCrossed = 0;
	m_targetBlink = 0;

	VectorClear(m_eyeTargetPos);
	VectorClear(m_eyePrevTargetPos);

	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	m_timeNextBlink = ms.count();
}

Skeletor::~Skeletor()
{
	for (size_t i = 0; i < m_numBones; i++)
	{
		delete m_bone[i];
	}

	m_morphTargetList.CleanUpChannels();
	delete[] m_bone;
	m_bone = NULL;
}

void ConvertToRotationName(const char *boneName, char *rotChannelName)
{
	strcpy(rotChannelName, boneName);
	strcat(rotChannelName, " rot");
}

void ConvertToPositionName(const char *boneName, char *posChannelName)
{
	strcpy(posChannelName, boneName);
	strcat(posChannelName, " rot");
}

void ConvertToFKRotationName(const char *boneName, char *rotChannelName)
{
	strcpy(rotChannelName, boneName);
	strcat(rotChannelName, " rot");
	strcat(rotChannelName, "FK");
}

void ConvertToFKPositionName(const char *boneName, char *rotChannelName)
{
	strcpy(rotChannelName, boneName);
	strcat(rotChannelName, " pos");
	strcat(rotChannelName, "FK");
}

int GetHighestFloat(float *selection)
{
	float currentHighest;
	int currentHighestIndex;
	int i;

	currentHighest = selection[0];
	currentHighestIndex = 0;

	for (i = 0; i < 8; i++)
	{
		if (selection[i] > currentHighest)
		{
			currentHighest = selection[i];
			currentHighestIndex = i;
		}
	}

	return currentHighestIndex;
}

void Skeletor::SetPose(const FrameInfo *frameInfo, const int32_t *contIndices, const vec4_t *contValues, float actionWeight)
{
	//int boneNum;
	size_t blendNum;
	size_t blendFrame;
	size_t movementBlendFrame;
	size_t actionBlendFrame;
	//int realAnimIndex;
	size_t beforeFrame;
	size_t afterFrame;
	float beforeWeight;
	float afterWeight;
	size_t numFramesAdded;
	float cutoff_weight;
	int contNum;
	float animWeight;
	SkanBlendInfo *frame1, *frame2;

	for (size_t i = 0; i < m_numBones; i++)
	{
		m_bone[i]->m_controller = NULL;
		m_bone[i]->m_isDirty = true;
	}

	if (contIndices && contValues)
	{
		for (size_t i = 0; i < 5; i++)
		{
			contNum = contIndices[i];
			if (contNum != -1)
			{
				cutoff_weight = (contValues[i][3] - 1.0f) * (contValues[i][3] - 1.0f);
				if (cutoff_weight >= EPSILON)
				{
					m_bone[contNum]->m_controller = (float *)contValues[i];
				}
			}
		}
	}

	for (int i = 0; i < 3; i++)
	{
		m_frameBounds[0][i] = -2.0f;
		m_frameBounds[1][i] = 2.0f;
	}

	m_frameRadius = 2.0f;
	animWeight = 0.0f;
	movementBlendFrame = 0;
	actionBlendFrame = 32;

	for (size_t i = 0; i < MAX_FRAMEINFOS; i++)
	{
		if (animWeight < frameInfo[i].weight)
		{
			animWeight = frameInfo[i].weight;
		}
	}

	cutoff_weight = animWeight * 0.01f;

	for (size_t i = 0; i < MAX_FRAMEINFOS; i++)
	{
		if (frameInfo[i].weight > cutoff_weight)
		{
			const ConstSkeletonAnimationPtr animData = frameInfo[i].anim;
			if (animData->HasDelta())
			{
				blendFrame = movementBlendFrame;
			}
			else
			{
				blendFrame = actionBlendFrame;
			}

			beforeWeight = 0.0f;
			afterWeight = 0.0f;
			beforeFrame = 0;
			afterFrame = 0;

			numFramesAdded = animData->GetFrameNums(frameInfo[i].time, 0.01f, &beforeFrame, &afterFrame, &beforeWeight, &afterWeight);

			frame1 = &m_frameList.m_blendInfo[blendFrame];
			frame1->frame = beforeFrame;
			frame1->pAnimationData = animData;
			frame1->weight = beforeWeight * frameInfo[i].weight;

			SkelVec3 bounds[2];
			animData->GetBounds(bounds[0], bounds[1]);
			AddToBounds(m_frameBounds, bounds);

			if (frame1->pAnimationData->GetFrame(frame1->frame)->radius > m_frameRadius)
				m_frameRadius = frame1->pAnimationData->GetFrame(frame1->frame)->radius;

			if (numFramesAdded == 2)
			{
				frame2 = &m_frameList.m_blendInfo[blendFrame + 1];
				frame2->frame = afterFrame;
				frame2->pAnimationData = animData;
				frame2->weight = afterWeight * frameInfo[i].weight;

				animData->GetBounds(bounds[0], bounds[1]);
				AddToBounds(m_frameBounds, bounds);

				if (frame2->pAnimationData->GetFrame(frame2->frame)->radius > m_frameRadius)
				{
					m_frameRadius = frame2->pAnimationData->GetFrame(frame2->frame)->radius;
				}

			}

			blendNum = blendFrame + numFramesAdded;

			if (animData->HasDelta())
			{
				movementBlendFrame = blendNum;
			}
			else
			{
				actionBlendFrame = blendNum;
			}
		}
	}

	for (int i = 0; i < 3; i++)
	{
		m_frameBounds[0][i] += -7.0f;
		m_frameBounds[1][i] += 7.0f;
	}

	m_frameList.numMovementFrames = (int16_t)movementBlendFrame;
	m_frameList.numActionFrames = (int16_t)actionBlendFrame - 32;
	m_frameList.actionWeight = actionWeight;
}

static SkelMat4 GetGlobalDefaultPosition(skelBone_Base *bone)
{
	SkelMat4 lLocalPosition;
	SkelMat4 lGlobalPosition;
	SkelMat4 lParentGlobalPosition;

	lLocalPosition = bone->GetTransform(NULL);

	if (bone->Parent())
	{
		//lParentGlobalPosition = GetGlobalDefaultPosition( bone->Parent() );
		//lGlobalPosition.Multiply( lParentGlobalPosition, lLocalPosition );
		lGlobalPosition = lLocalPosition;
	}
	else
	{
		lGlobalPosition = lLocalPosition;
	}

	return lGlobalPosition;
}

SkelMat4 GlobalToLocal(skelBone_Base *bone, SkelMat4 pGlobalPosition)
{
	SkelMat4 lLocalPosition;
	SkelMat4 lParentGlobalPosition;

	if (bone->Parent())
	{
		lParentGlobalPosition = GetGlobalDefaultPosition(bone->Parent());
		lParentGlobalPosition.Inverse();
		lLocalPosition.Multiply(lParentGlobalPosition, pGlobalPosition);
	}
	else
	{
		lLocalPosition = pGlobalPosition;
	}

	return lLocalPosition;
}

void BoneGetFrames(Skeleton *skelmodel, SkeletonAnimation *animData, SkeletonChannelList *boneList, int boneNum, std::vector<SkanAnimFrame>& outFrames)
{
	skelBone_Base **bone;
	SkanAnimFrame frame;

	outFrames.clear();

	size_t numBones = boneList->NumChannels();

	bone = new skelBone_Base*[numBones];
	memset(bone, 0, sizeof(skelBone_Base *) * numBones);

	skelmodel->LoadBonesFromBuffer(boneList, bone);

	for (size_t i = 0; i < numBones; i++)
	{
		bone[i]->m_controller = NULL;
		bone[i]->m_isDirty = true;
	}

	// process the rot channel
	size_t localChannelNum = animData->GetChannelList()->GetLocalFromGlobal(bone[boneNum]->GetChannelIndex(1));
	if (localChannelNum >= 0)
	{
		const SkeletonAnimation::SkanChannelHdr *channel = animData->GetAryChannel(localChannelNum);

		for (size_t i = 0; i < channel->ary_frames.size(); i++)
		{
			const SkeletonAnimation::SkanGameFrame *pFrame = &channel->ary_frames[i];

			frame.nFrameNum = pFrame->nFrameNum;
			VectorClear(frame.pos);
			QuatToAngles(pFrame->pChannelData, frame.rot);

			outFrames.push_back(frame);
		}
	}

	if (bone[0]->GetChannelIndex(0) != bone[0]->GetChannelIndex(1))
	{
		// process the pos channel
		localChannelNum = animData->GetChannelList()->GetLocalFromGlobal(bone[boneNum]->GetChannelIndex(0));
		if (localChannelNum >= 0)
		{
			const SkeletonAnimation::SkanChannelHdr *channel = animData->GetAryChannel(localChannelNum);

			for (size_t i = 0; i < channel->ary_frames.size(); i++)
			{
				const SkeletonAnimation::SkanGameFrame *pFrame = &channel->ary_frames[i];
				SkanAnimFrame *pOutFrame = &outFrames[i];

				VecCopy(pFrame->pChannelData, pOutFrame->pos);
			}
		}
	}
}

#if 0
void SkeletorGetAnimFrame2(skelHeaderGame_t *skelmodel, skelChannelList_c *boneList, skelBoneCache_t *bones, skelAnimStoreFrameList_c *frameList, float *radius, vec3_t *mins, vec3_t *maxes)
{
	int numBones;
	skelBone_Base **bone;
	int i;
	skelAnimFrame_t *newFrame;

	numBones = skelmodel->numBones;

	bone = (skelBone_Base **)Skel_Alloc(sizeof(skelBone_Base *) * numBones);
	memset(bone, 0, sizeof(skelBone_Base *) * numBones);

	SkeletorLoadBonesFromBuffer(boneList, skelmodel, bone);

	for (i = 0; i < numBones; i++)
	{
		bone[i]->m_controller = NULL;
		bone[i]->m_isDirty = true;
	}

	newFrame = (skelAnimFrame_t *)Skel_Alloc(sizeof(skelAnimFrame_t) + sizeof(SkelMat4) * numBones);
	newFrame->radius = 0;
	newFrame->bounds[0] = SkelVec3();
	newFrame->bounds[1] = SkelVec3();

	for (i = 0; i < numBones; i++)
	{
		//skelBone_Base *Parent = bone[ i ]->Parent();
		//bone[ i ]->SetParent( &Skeletor::m_worldBone );
		newFrame->bones[i] = bone[i]->GetTransform(frameList);
		//bone[ i ]->SetParent( Parent );
	}

	for (i = 0; i < numBones; i++)
	{
		VecCopy(newFrame->bones[i][3], bones[i].offset);
		bones[i].matrix[0][0] = newFrame->bones[i][0][0];
		bones[i].matrix[0][1] = newFrame->bones[i][0][1];
		bones[i].matrix[0][2] = newFrame->bones[i][0][2];
		bones[i].matrix[0][3] = 0;
		bones[i].matrix[1][0] = newFrame->bones[i][1][0];
		bones[i].matrix[1][1] = newFrame->bones[i][1][1];
		bones[i].matrix[1][2] = newFrame->bones[i][1][2];
		bones[i].matrix[1][3] = 0;
		bones[i].matrix[2][0] = newFrame->bones[i][2][0];
		bones[i].matrix[2][1] = newFrame->bones[i][2][1];
		bones[i].matrix[2][2] = newFrame->bones[i][2][2];
		bones[i].matrix[2][3] = 0;
	}

	for (i = 0; i < numBones; i++)
	{
		delete bone[i];
	}

	Skel_Free(bone);

	if (radius) {
		*radius = newFrame->radius;
	}

	if (mins || maxes)
	{
		for (i = 0; i < 3; i++)
		{
			if (mins) {
				(*mins)[i] = newFrame->bounds[0][i];
			}
			if (maxes) {
				(*maxes)[i] = newFrame->bounds[1][i];
			}
		}
	}

	Skel_Free(newFrame);
}

void SkeletorGetAnimFrame(skelHeaderGame_t *skelmodel, skelAnimDataGameHeader_t *animData, skelChannelList_c *boneList, skelBoneCache_t *bones, int frame, float *radius, vec3_t *mins, vec3_t *maxes)
{
	int numBones;
	skelBone_Base **bone;
	int i;
	skelAnimStoreFrameList_c frameList;
	skelAnimFrame_t *newFrame;

	frameList.actionWeight = animData ? 1.0 : 0;

	if (animData)
	{
		if (!animData->bHasDelta)
		{
			frameList.numMovementFrames = 0;
			frameList.numActionFrames = 1;
			frameList.m_blendInfo[32].weight = 1.0;
			frameList.m_blendInfo[32].pAnimationData = animData;
			frameList.m_blendInfo[32].frame = frame;
		}
		else
		{
			frameList.numMovementFrames = 1;
			frameList.numActionFrames = 0;
			frameList.m_blendInfo[0].weight = 1.0;
			frameList.m_blendInfo[0].pAnimationData = animData;
			frameList.m_blendInfo[0].frame = frame;
		}
	}
	numBones = skelmodel->numBones;

	bone = (skelBone_Base **)Skel_Alloc(sizeof(skelBone_Base *) * numBones);
	memset(bone, 0, sizeof(skelBone_Base *) * numBones);

	SkeletorLoadBonesFromBuffer(boneList, skelmodel, bone);

	for (i = 0; i < numBones; i++)
	{
		bone[i]->m_controller = NULL;
		bone[i]->m_isDirty = true;
	}

	newFrame = (skelAnimFrame_t *)Skel_Alloc(sizeof(skelAnimFrame_t) + sizeof(SkelMat4) * numBones);

	if (animData)
	{
		if (animData->m_frame)
		{
			newFrame->radius = animData->m_frame->radius;
		}
		else
		{
			newFrame->radius = 0;
		}

		newFrame->bounds[0] = animData->bounds[0];
		newFrame->bounds[1] = animData->bounds[1];
	}
	else
	{
		newFrame->radius = 0;
		newFrame->bounds[0] = SkelVec3();
		newFrame->bounds[1] = SkelVec3();
	}

	if (animData)
	{
		for (i = 0; i < numBones; i++)
		{
			//skelBone_Base *Parent = bone[ i ]->Parent();
			//bone[ i ]->SetParent( &Skeletor::m_worldBone );
			newFrame->bones[i] = bone[i]->GetTransform(&frameList);
			//bone[ i ]->SetParent( Parent );
		}
	}
	else
	{
		for (i = 0; i < numBones; i++)
		{
			newFrame->bones[i] = SkelMat4();
		}
	}

	for (i = 0; i < numBones; i++)
	{
		VecCopy(newFrame->bones[i][3], bones[i].offset);
		bones[i].matrix[0][0] = newFrame->bones[i][0][0];
		bones[i].matrix[0][1] = newFrame->bones[i][0][1];
		bones[i].matrix[0][2] = newFrame->bones[i][0][2];
		bones[i].matrix[0][3] = 0;
		bones[i].matrix[1][0] = newFrame->bones[i][1][0];
		bones[i].matrix[1][1] = newFrame->bones[i][1][1];
		bones[i].matrix[1][2] = newFrame->bones[i][1][2];
		bones[i].matrix[1][3] = 0;
		bones[i].matrix[2][0] = newFrame->bones[i][2][0];
		bones[i].matrix[2][1] = newFrame->bones[i][2][1];
		bones[i].matrix[2][2] = newFrame->bones[i][2][2];
		bones[i].matrix[2][3] = 0;
	}

	for (i = 0; i < numBones; i++)
	{
		delete bone[i];
	}

	Skel_Free(bone);

	if (radius) {
		*radius = newFrame->radius;
	}

	if (mins || maxes)
	{
		for (i = 0; i < 3; i++)
		{
			if (mins) {
				(*mins)[i] = newFrame->bounds[0][i];
			}
			if (maxes) {
				(*maxes)[i] = newFrame->bounds[1][i];
			}
		}
	}

	Skel_Free(newFrame);
}

void TIKI_GetSkelAnimFrameInternal2(TIKI *tiki, skelBoneCache_t *bones, skelAnimStoreFrameList_c* frameList, float *radius, vec3_t *mins, vec3_t *maxes)
{
	//int boneNum;
	int numBones;
	skelBone_Base **bone;
	int i;
	skelAnimFrame_t *newFrame;
	//int realAnimIndex;
	//skanBlendInfo *frame;
	skelHeaderGame_t *skelmodel;

	numBones = tiki->boneList.NumChannels();

	bone = (skelBone_Base **)Skel_Alloc(sizeof(skelBone_Base *) * numBones);
	memset(bone, 0, sizeof(skelBone_Base *) * numBones);

	for (i = 0; i < tiki->numMeshes; i++)
	{
		skelmodel = TIKI_GetSkel(tiki->mesh[i]);
		SkeletorLoadBonesFromBuffer(&tiki->boneList, skelmodel, bone);
	}

	for (i = 0; i < numBones; i++)
	{
		bone[i]->m_controller = NULL;
		bone[i]->m_isDirty = true;
	}

	newFrame = (skelAnimFrame_t *)Skel_Alloc(sizeof(skelAnimFrame_t) + sizeof(SkelMat4) * numBones);

	/*
	if (animData)
	{
	if (animData->m_frame)
	{
	newFrame->radius = animData->m_frame->radius;
	}
	else
	{
	newFrame->radius = 0;
	}

	newFrame->bounds[0] = animData->bounds[0];
	newFrame->bounds[1] = animData->bounds[1];
	}
	else
	*/
	{
		newFrame->radius = 0;
		newFrame->bounds[0] = SkelVec3();
		newFrame->bounds[1] = SkelVec3();
	}

	for (i = 0; i < numBones; i++)
	{
		newFrame->bones[i] = bone[i]->GetTransform(frameList);
	}

	for (i = 0; i < numBones; i++)
	{
		delete bone[i];
	}

	Skel_Free(bone);

	for (i = 0; i < numBones; i++)
	{
		VecCopy(newFrame->bones[i][3], bones[i].offset);
		bones[i].matrix[0][0] = newFrame->bones[i][0][0];
		bones[i].matrix[0][1] = newFrame->bones[i][0][1];
		bones[i].matrix[0][2] = newFrame->bones[i][0][2];
		bones[i].matrix[0][3] = 0;
		bones[i].matrix[1][0] = newFrame->bones[i][1][0];
		bones[i].matrix[1][1] = newFrame->bones[i][1][1];
		bones[i].matrix[1][2] = newFrame->bones[i][1][2];
		bones[i].matrix[1][3] = 0;
		bones[i].matrix[2][0] = newFrame->bones[i][2][0];
		bones[i].matrix[2][1] = newFrame->bones[i][2][1];
		bones[i].matrix[2][2] = newFrame->bones[i][2][2];
		bones[i].matrix[2][3] = 0;
	}

	if (radius) {
		*radius = newFrame->radius;
	}

	if (mins && maxes)
	{
		for (i = 0; i < 3; i++)
		{
			(*mins)[i] = newFrame->bounds[0][i];
			(*maxes)[i] = newFrame->bounds[1][i];
		}
	}

	Skel_Free(newFrame);
}

void TIKI_GetSkelAnimFrameInternal(TIKI *tiki, skelBoneCache_t *bones, skelAnimDataGameHeader_t *animData, int frame, float *radius, vec3_t *mins, vec3_t *maxes)
{
	//int boneNum;
	int numBones;
	skelBone_Base **bone;
	int i;
	skelAnimStoreFrameList_c frameList;
	skelAnimFrame_t *newFrame;
	//int realAnimIndex;
	//skanBlendInfo *frame;
	skelHeaderGame_t *skelmodel;

	frameList.actionWeight = animData ? 1.0 : 0;
	if (!animData || !animData->bHasDelta)
	{
		frameList.numMovementFrames = 0;
		frameList.numActionFrames = 1;
		frameList.m_blendInfo[32].weight = 1.0;
		frameList.m_blendInfo[32].pAnimationData = animData;
		frameList.m_blendInfo[32].frame = frame;
	}
	else
	{
		frameList.numMovementFrames = 1;
		frameList.numActionFrames = 0;
		frameList.m_blendInfo[0].weight = 1.0;
		frameList.m_blendInfo[0].pAnimationData = animData;
		frameList.m_blendInfo[0].frame = frame;
	}
	numBones = tiki->boneList.NumChannels();

	bone = (skelBone_Base **)Skel_Alloc(sizeof(skelBone_Base *) * numBones);
	memset(bone, 0, sizeof(skelBone_Base *) * numBones);

	for (i = 0; i < tiki->numMeshes; i++)
	{
		skelmodel = TIKI_GetSkel(tiki->mesh[i]);
		SkeletorLoadBonesFromBuffer(&tiki->boneList, skelmodel, bone);
	}

	for (i = 0; i < numBones; i++)
	{
		bone[i]->m_controller = NULL;
		bone[i]->m_isDirty = true;
	}

	newFrame = (skelAnimFrame_t *)Skel_Alloc(sizeof(skelAnimFrame_t) + sizeof(SkelMat4) * numBones);

	if (animData)
	{
		if (animData->m_frame)
		{
			newFrame->radius = animData->m_frame->radius;
		}
		else
		{
			newFrame->radius = 0;
		}

		newFrame->bounds[0] = animData->bounds[0];
		newFrame->bounds[1] = animData->bounds[1];
	}
	else
	{
		newFrame->radius = 0;
		newFrame->bounds[0] = SkelVec3();
		newFrame->bounds[1] = SkelVec3();
	}

	for (i = 0; i < numBones; i++)
	{
		newFrame->bones[i] = bone[i]->GetTransform(&frameList);
	}

	for (i = 0; i < numBones; i++)
	{
		delete bone[i];
	}

	Skel_Free(bone);

	for (i = 0; i < numBones; i++)
	{
		VecCopy(newFrame->bones[i][3], bones[i].offset);
		bones[i].matrix[0][0] = newFrame->bones[i][0][0];
		bones[i].matrix[0][1] = newFrame->bones[i][0][1];
		bones[i].matrix[0][2] = newFrame->bones[i][0][2];
		bones[i].matrix[0][3] = 0;
		bones[i].matrix[1][0] = newFrame->bones[i][1][0];
		bones[i].matrix[1][1] = newFrame->bones[i][1][1];
		bones[i].matrix[1][2] = newFrame->bones[i][1][2];
		bones[i].matrix[1][3] = 0;
		bones[i].matrix[2][0] = newFrame->bones[i][2][0];
		bones[i].matrix[2][1] = newFrame->bones[i][2][1];
		bones[i].matrix[2][2] = newFrame->bones[i][2][2];
		bones[i].matrix[2][3] = 0;
	}

	if (radius) {
		*radius = newFrame->radius;
	}

	if (mins && maxes)
	{
		for (i = 0; i < 3; i++)
		{
			(*mins)[i] = newFrame->bounds[0][i];
			(*maxes)[i] = newFrame->bounds[1][i];
		}
	}

	Skel_Free(newFrame);
}

void TIKI_GetSkelAnimFrame2(TIKI *tiki, skelBoneCache_t *bones, int anim, int frame, float *radius, vec3_t *mins, vec3_t *maxes)
{
	short *aliases;
	skelAnimDataGameHeader_t *animData;

	aliases = tiki->a->m_aliases;
	if (*aliases == -1)
	{
		SKEL_Warning("TIKI_GetSkelAnimFrame: Bad anim in static model %s, couldn't generate pose properly.\n", tiki->name);
		return;
	}

	animData = SkeletorCacheGetData(aliases[anim]);

	TIKI_GetSkelAnimFrameInternal(tiki, bones, animData, frame, NULL, NULL, NULL);
}

void TIKI_GetSkelAnimFrame(TIKI *tiki, skelBoneCache_t *bones, float *radius, vec3_t *mins, vec3_t *maxes)
{
	TIKI_GetSkelAnimFrame2(tiki, bones, 0, 0, radius, mins, maxes);
}

#endif

void Skeletor::GetFrame(skelAnimFrame *newFrame)
{
	for (size_t boneNum = 0; boneNum < m_numBones; boneNum++)
	{
		newFrame->bones[boneNum] = GetBoneFrame(boneNum);
	}

	newFrame->bounds[0] = m_frameBounds[0];
	newFrame->bounds[1] = m_frameBounds[1];
	newFrame->radius = m_frameRadius;
}


SkelMat4 Skeletor::GetBoneFrame(size_t boneIndex)
{
	return m_bone[boneIndex]->GetTransform(&m_frameList);
}

bool Skeletor::IsBoneOnGround(int boneIndex, float threshold)
{
	return GetBoneFrame(boneIndex).val[3][2] < threshold;
}


float Skeletor::GetRadius()
{
	return m_frameRadius;
}


float Skeletor::GetCentroidRadius(float *centroid)
{
	centroid[0] = (m_frameBounds[0][0] + m_frameBounds[1][0]) * 0.5f;
	centroid[1] = (m_frameBounds[0][1] + m_frameBounds[1][1]) * 0.5f;
	centroid[2] = (m_frameBounds[0][2] + m_frameBounds[1][2]) * 0.5f;
	return m_frameRadius;
}


size_t Skeletor::GetMorphWeightFrame(size_t index, float time, int64_t *data)
{
	return GetMorphWeightFrame(data);
}

const vec4_t *DecodeFrameValue(const SkeletonAnimation::SkanChannelHdr *channelFrames, size_t desiredFrameNum)
{
	const SkeletonAnimation::SkanGameFrame *foundFrame = &channelFrames->ary_frames[0];

	for (size_t i = 0; i < channelFrames->ary_frames.size(); i++)
	{
		if (channelFrames->ary_frames[i].nFrameNum >= desiredFrameNum)
		{
			foundFrame = &channelFrames->ary_frames[i];
			break;
		}
	}

	return &foundFrame->pChannelData;
}


size_t Skeletor::GetMorphWeightFrame(int64_t *data)
{
	size_t animChannelNum;
	int blendNum;
	float weight;
	int modelChannelNum;
	const vec4_t *channelData;

	size_t numTargets = m_morphTargetList.NumChannels();

	if (!numTargets) {
		return 0;
	}

	memset(data, 0, sizeof(*data) * numTargets);

	for (blendNum = 0; blendNum < m_frameList.numMovementFrames; blendNum++)
	{
		weight = m_frameList.m_blendInfo[blendNum].weight;

		if (weight > 0.001)
		{
			for (modelChannelNum = 0; modelChannelNum < m_morphTargetList.NumChannels(); modelChannelNum++)
			{
				animChannelNum = m_morphTargetList.GlobalChannel(modelChannelNum);
				animChannelNum = m_morphTargetList.GetLocalFromGlobal(animChannelNum);

				if (animChannelNum >= 0)
				{
					channelData = DecodeFrameValue(m_frameList.m_blendInfo[blendNum].pAnimationData->GetAryChannel(animChannelNum), m_frameList.m_blendInfo[blendNum].frame);
					data[modelChannelNum] += (int)((*channelData)[0] * weight);
				}
			}
		}
	}

	for (blendNum = 32; blendNum < m_frameList.numActionFrames + 32; blendNum++)
	{
		weight = m_frameList.m_blendInfo[blendNum].weight;

		if (weight > 0.001)
		{
			for (modelChannelNum = 0; modelChannelNum < m_morphTargetList.NumChannels(); modelChannelNum++)
			{
				animChannelNum = m_morphTargetList.GlobalChannel(modelChannelNum);
				animChannelNum = m_morphTargetList.GetLocalFromGlobal(animChannelNum);

				if (animChannelNum >= 0)
				{
					channelData = DecodeFrameValue(m_frameList.m_blendInfo[blendNum].pAnimationData->GetAryChannel(animChannelNum), m_frameList.m_blendInfo[blendNum].frame);
					data[modelChannelNum] += (int)((*channelData)[0] * weight);
				}
			}
		}
	}

	if (m_headBoneIndex >= 0 && !castVector(m_eyeTargetPos).isMuchSmallerThan(castVector(vec3_zero), EPSILON))
	{
		SkelVec3 lookPos;
		SkelVec3 temp;
		SkelMat4 headOrient;
		SkelMat4 invHeadOrient;
		float lookUpAmount;
		float lookLeftAmount;
		float l;
		float s;

		lookPos = m_eyeTargetPos;
		headOrient = GetBoneFrame(m_headBoneIndex);
		invHeadOrient.TransposeRotOf(headOrient);

		temp = lookPos;
		lookPos[0] = temp[0] * invHeadOrient[0][0] * temp[1] * invHeadOrient[1][0] + temp[2] * invHeadOrient[2][0];
		lookPos[1] = temp[0] * invHeadOrient[0][1] * temp[1] * invHeadOrient[1][1] + temp[2] * invHeadOrient[2][1];
		lookPos[2] = temp[0] * invHeadOrient[0][2] * temp[1] * invHeadOrient[1][2] + temp[2] * invHeadOrient[2][2];

		lookLeftAmount = lookPos[2] * 100 + data[m_targetLookLeft] - data[m_targetLookRight];
		lookUpAmount = lookPos[0] * 100 + data[m_targetLookUp] - data[m_targetLookDown];

		s = VectorLengthSquared(lookPos);

		if (s == 0.0f) {
			lookPos[0] = 1.0f;
		}
		else if (s != 1.0f) {
			l = 1.0f / sqrtf(s);
			VectorScale(lookPos, l, lookPos);
		}

		if (m_targetLookLeft >= 0 && m_targetLookRight >= 0
			&& m_targetLookUp >= 0 && m_targetLookDown >= 0)
		{
			if (lookLeftAmount > 0.0)
			{
				if (lookLeftAmount > 100.0) {
					lookLeftAmount = 100.0;
				}

				data[m_targetLookLeft] = (int32_t)lookLeftAmount;
				data[m_targetLookRight] = 0;
			}
			else
			{
				if (lookLeftAmount < -100.0) {
					lookLeftAmount = -100.0;
				}

				data[m_targetLookLeft] = 0;
				data[m_targetLookRight] = (int32_t)-lookLeftAmount;
			}

			if (m_targetLookUp > 0.0)
			{
				if (lookUpAmount > 100.0) {
					lookUpAmount = 100.0;
				}

				data[m_targetLookUp] = (int32_t)lookUpAmount;
				data[m_targetLookDown] = 0;
			}
			else
			{
				if (lookUpAmount < -133.0) {
					lookUpAmount = -133.0;
				}

				data[m_targetLookUp] = 0;
				data[m_targetLookDown] = ((int32_t)(-lookUpAmount * 0.75f));
			}
		}
	}

	// check for blink
	if (m_targetBlink >= 0)
	{
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

		int64_t sysMilliseconds = ms.count();

		if (sysMilliseconds > m_timeNextBlink)
		{
			if (sysMilliseconds <= m_timeNextBlink + 250)
			{
				int64_t blinkAmount = sysMilliseconds - m_timeNextBlink;

				if (blinkAmount > 100)
				{
					blinkAmount = 250 - blinkAmount;

					if (blinkAmount > 100) {
						blinkAmount = 100;
					}
				}

				if (data[m_targetBlink] < blinkAmount) {
					data[m_targetBlink] = (int32_t)blinkAmount;
				}
			}
			else
			{
				m_timeNextBlink = rand() / 5 + sysMilliseconds - 1000;
			}
		}
	}

	return numTargets;
}

void Skeletor::SetEyeTargetPos(const float *pEyeTargetPos)
{
	VecCopy(pEyeTargetPos, m_eyeTargetPos);
}


size_t Skeletor::GetBoneParent(size_t boneIndex)
{
	skelBone_Base *pBoneParent = m_bone[boneIndex]->Parent();

	for (size_t iBoneNum = 0; iBoneNum < m_numBones; iBoneNum++)
	{
		if (m_bone[iBoneNum] == pBoneParent)
		{
			return iBoneNum;
		}
	}

	return -1;
	//return m_bone[ boneIndex ] - m_bone[ boneIndex ]->Parent();
}

const SkeletonChannelList *TIKI::GetBoneList() const
{
	return &boneList;
}

const char *TIKI::GetBoneNameFromNum(int num) const
{
	return boneList.ChannelName(GetAssetManager()->GetManager<SkeletorManager>()->GetBoneNamesTable(), num);
}


intptr_t TIKI::GetBoneNumFromName(const char *name) const
{
	intptr_t iGlobalChannel = GetAssetManager()->GetManager<SkeletorManager>()->GetBoneNamesTable()->FindNameLookup(name);

	if (iGlobalChannel < 0)
	{
		return -1;
	}

	return boneList.GetLocalFromGlobal(iGlobalChannel);
}
