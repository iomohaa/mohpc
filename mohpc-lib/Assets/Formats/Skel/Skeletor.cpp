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
			if ((flags & TAF::DELTADRIVEN)) {
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
		if ((flags & TAF::DELTADRIVEN)) {
			frameNum2 %= numFrames;
		}
		else {
			frameNum2 = numFrames - 1;
		}

		if (frameNum1 >= numFrames)
		{
			if ((flags & TAF::DELTADRIVEN))
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

const SkeletonChannelList *TIKI::GetBoneList() const
{
	return &boneList;
}

SkeletonChannelList& MOHPC::TIKI::getBoneList()
{
	return boneList;
}

const char *TIKI::GetBoneNameFromNum(int num) const
{
	return boneList.ChannelName(skeletorManager->GetBoneNamesTable(), num);
}

intptr_t TIKI::GetBoneNumFromName(const char *name) const
{
	intptr_t iGlobalChannel = skeletorManager->GetBoneNamesTable()->FindNameLookup(name);

	if (iGlobalChannel < 0)
	{
		return -1;
	}

	return boneList.GetLocalFromGlobal(iGlobalChannel);
}
