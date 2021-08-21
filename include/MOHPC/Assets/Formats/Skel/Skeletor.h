#pragma once

#include "../../Asset.h"
#include "../../Managers/Manager.h"

namespace MOHPC
{
	static constexpr unsigned int MAX_FRAMEINFOS = 32;

	static constexpr unsigned int MAX_ANIM_POSES = 64;
	static constexpr unsigned int MAX_ANIM_MOVEMENTS_POSES = (MAX_ANIM_POSES >> 1);
	static constexpr unsigned int MAX_ANIM_ACTIONS_POSES = (MAX_ANIM_POSES >> 1);

	class TIKI;
	class Skeleton;

	struct SkanBlendInfo
	{
		float weight;
		ConstSkeletonAnimationPtr pAnimationData;
		uintptr_t frame;
	};

	class skelAnimStoreFrameList_c
	{
	public:
		SkelQuat GetSlerpValue(size_t globalChannelNum) const;
		void GetLerpValue3(size_t globalChannelNum, SkelVec3* outVec) const;

	public:
		short int numMovementFrames;
		short int numActionFrames;
		float actionWeight;
		SkanBlendInfo m_blendInfo[MAX_ANIM_POSES];
	};
};
