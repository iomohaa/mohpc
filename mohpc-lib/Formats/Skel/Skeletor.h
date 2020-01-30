#pragma once

#include <MOHPC/Asset.h>
#include <MOHPC/Managers/Manager.h>

#define MAX_FRAMEINFOS 32

namespace MOHPC
{
	class TIKI;
	class Skeleton;

	struct skelAnimFrame
	{
		float radius;
		SkelVec3 bounds[2];
		std::vector<SkelMat4> bones;
	};

	struct skelAnimTime
	{
		float seconds;
	};

	struct skelAnimBlendFrame_t
	{
		float weight;
		const SkeletonAnimation *pAnimationData;
		SkeletonAnimation::AnimFrame *frame;
	};

	struct SkanBlendInfo
	{
		float weight;
		SafePtr<const SkeletonAnimation> pAnimationData;
		size_t frame;
	};

	class skelAnimStoreFrameList_c
	{
	public:
		short int numMovementFrames;
		short int numActionFrames;
		float actionWeight;
		SkanBlendInfo m_blendInfo[MAX_ANIM_POSES];

	public:
		SkelQuat GetSlerpValue(size_t globalChannelNum) const;
		void GetLerpValue3(size_t globalChannelNum, SkelVec3 *outVec) const;
	};

	class Skeletor : public Asset
	{
	public:
		struct FrameInfo
		{
			SafePtr<const SkeletonAnimation> anim;
			float time;
			float weight;
		};

	private:
		SkelVec3 m_frameBounds[2];
		float m_frameRadius;
		skelAnimStoreFrameList_c m_frameList;
		size_t m_targetLookLeft;
		size_t m_targetLookRight;
		size_t m_targetLookUp;
		size_t m_targetLookDown;
		size_t m_targetLookCrossed;
		int64_t m_targetBlink;
		int64_t m_timeNextBlink;
		size_t m_headBoneIndex;
		Vector m_eyeTargetPos;
		Vector m_eyePrevTargetPos;
		class skelBone_Base *m_leftFoot;
		class skelBone_Base *m_rightFoot;
		SkeletonChannelList m_morphTargetList;
		size_t m_numBones;
		class skelBone_Base **m_bone;

	public:
		Skeletor(TIKI *tiki);
		Skeletor(Skeleton *skelmodel);
		~Skeletor();

		void PrintBoneCacheList();
		void PrintBoneList();
		void LoadMorphTargetNames( Skeleton *modelHeader );
		void GetFrame(skelAnimFrame *newFrame);
		size_t GetMorphWeightFrame(int64_t *data);
		SkelMat4 GetBoneFrame(size_t boneIndex);
		void GetFrameBounds( SkelVec3 *, SkelVec3 * );
		float GetModelLODRadius();
		bool IsBoneOnGround( int boneIndex, float threshold );
		size_t GetMorphWeightFrame(size_t index, float time, int64_t *data);
		float GetRadius();
		float GetCentroidRadius(float *centroid);
		void SetPose(const FrameInfo *frameInfo, const int32_t *contIndices, const vec4_t *contValues, float actionWeight);
		void SetEyeTargetPos( const float *pEyeTargetPos );
		size_t GetBoneParent(size_t boneIndex);

	private:
		Skeletor();

		void Init();
		static class ChannelNameTable *ChannelNames();
		SkelMat4 *BoneTransformation(int, int *, float(*)[4]);
	};

	/*
		//
		// skeletor.cpp
		//

		void ConvertToRotationName(const char *boneName, char *rotChannelName);
		void ConvertToPositionName(const char *boneName, char *posChannelName);
		void ConvertToFKRotationName(const char *boneName, char *rotChannelName);
		void ConvertToFKPositionName(const char *boneName, char *rotChannelName);
		void AddToBounds(SkelVec3 *bounds, SkelVec3 *newBounds);
		void BoneGetFrames(skelHeaderGame_t *skelmodel, skelAnimDataGameHeader_t *animData, skelChannelList_c *boneList, int boneNum, Container< skanAnimFrame >& outFrames);
		void TIKI_GetSkelAnimFrameInternal2(dtiki_t *tiki, skelBoneCache_t *bones, skelAnimStoreFrameList_c *frameList, float *radius, vec3_t *mins, vec3_t *maxes);
		void SkeletorGetAnimFrame2(skelHeaderGame_t *skelmodel, skelChannelList_c *boneList, skelBoneCache_t *bones, skelAnimStoreFrameList_c *frameList, float *radius, vec3_t *mins, vec3_t *maxes);
		void SkeletorGetAnimFrame(skelHeaderGame_t *skelmodel, skelAnimDataGameHeader_t *animData, skelChannelList_c *boneList, skelBoneCache_t *bones, int frame, float *radius, vec3_t *mins, vec3_t *maxes);
		void TIKI_GetSkelAnimFrame(dtiki_t *tiki, skelBoneCache_t *bones, float *radius, vec3_t *mins, vec3_t *maxes);
		void TIKI_GetSkelAnimFrame2(dtiki_t *tiki, skelBoneCache_t *bones, int anim, int frame, float *radius, vec3_t *mins, vec3_t *maxes);
		void TIKI_GetSkelAnimFrameInternal(dtiki_t *tiki, skelBoneCache_t *bones, skelAnimDataGameHeader_t *animData, int frame, float *radius, vec3_t *mins, vec3_t *maxes);

		//
		// skeletorbones.cpp
		//

		void SkeletorLoadBoneFromBuffer(skelChannelList_c *boneList, boneData_t *boneData, skelBone_Base **bone);
		void SkeletorLoadBonesFromBuffer(skelChannelList_c *boneList, skelHeaderGame_t *buffer, skelBone_Base **bone);
	*/
};
