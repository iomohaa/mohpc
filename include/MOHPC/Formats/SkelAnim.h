#pragma once

#include "../Global.h"
#include <stdint.h>
#include <vector>
#include <string>
#include <map>

namespace MOHPC
{
	typedef std::shared_ptr<class SkeletonAnimation> SkeletonAnimationPtr;
	typedef std::shared_ptr<const class SkeletonAnimation> ConstSkeletonAnimationPtr;

#define MAX_ANIM_POSES 64
#define MAX_ANIM_MOVEMENTS_POSES (MAX_ANIM_POSES >> 1)
#define MAX_ANIM_ACTIONS_POSES (MAX_ANIM_POSES >> 1)

	struct SkanAnimFrame
	{
		size_t nFrameNum;
		Vector pos;
		Vector rot;
	};

	class SkeletonAnimation : public Asset
	{
		CLASS_BODY(SkeletonAnimation);

	public:
		struct File_AnimFrame;
		struct File_AnimDataHeader;

		struct AnimFrame
		{
			SkelVec3 bounds[2];
			float radius;
			SkelVec3 delta;
			float angleDelta;
			vec4_t* pChannels;

			AnimFrame();
			~AnimFrame();
		};

		struct SkanGameFrame
		{
			size_t nFrameNum;
			size_t nPrevFrameIndex;
			float pChannelData[4];
		};

		struct SkanChannelHdr
		{
			std::vector<SkanGameFrame> ary_frames;
		};

	private:
		int32_t flags;
		int32_t nBytesUsed;
		bool bHasDelta;
		bool bHasMorph;
		bool bHasUpper;
		SkelVec3 totalDelta;
		float totalAngleDelta;
		float frameTime;
		SkeletonChannelList channelList;
		SkelVec3 bounds[2];
		std::vector<AnimFrame> m_frame;
		std::vector<SkanChannelHdr> ary_channels;

	public:
		virtual bool Load() override;

	public:
		MOHPC_EXPORTS SkeletonAnimation();

		/** Returns the number of frames to use with the given time. */
		MOHPC_EXPORTS size_t GetFrameNums(float timeSeconds, float timeTolerance, size_t *beforeFrame, size_t *afterFrame, float *beforeWeight, float *afterWeight) const;

		/** Returns the number of frames. */
		MOHPC_EXPORTS size_t GetNumFrames() const;

		/** Return the frame at the specified index. */
		MOHPC_EXPORTS const AnimFrame *GetFrame(size_t index) const;

		/** Retuns the number of channels. */
		MOHPC_EXPORTS size_t GetNumAryChannels() const;

		/** Returns the channel at the specified index. */
		MOHPC_EXPORTS const SkanChannelHdr *GetAryChannel(size_t index) const;

		/** Returns the total animation time. */
		MOHPC_EXPORTS float GetTime() const;

		/** Returns the time it takes for a frame to complete. */
		MOHPC_EXPORTS float GetFrameTime() const;

		/** Returns the total delta of the animation. */
		MOHPC_EXPORTS Vector GetDelta() const;

		/** Returns true if the animation contains a delta data. */
		MOHPC_EXPORTS bool HasDelta() const;

		/** Returns the delta between two times in seconds. */
		MOHPC_EXPORTS Vector GetDeltaOverTime(float Time1, float Time2);

		/** Returns the animation flags. */
		MOHPC_EXPORTS int32_t GetFlags() const;

		/** Returns the animation's skeleton flags. */
		MOHPC_EXPORTS int32_t GetFlagsSkel() const;

		/** Returns the bounds an animation takes. */
		MOHPC_EXPORTS void GetBounds(Vector& OutMins, Vector& OutMaxs) const;
		MOHPC_EXPORTS void GetBounds(SkelVec3& OutMins, SkelVec3& OutMaxs) const;

		/** Returns the channel list of the animation. */
		MOHPC_EXPORTS const SkeletonChannelList *GetChannelList() const;

		/** Returns whether or not transforms change over time. */
		MOHPC_EXPORTS bool IsDynamic() const;

	private:
		void EncodeFrames(SkeletonChannelList *channelList, SkeletonChannelNameTable *channelNames);
		void ConvertSkelFileToGame(File_AnimDataHeader *pHeader, size_t iBuffLength, const char *path);
		void WriteEncodedFrames(struct msg_s *msg);
		void SaveProcessedAnim(const char *path, File_AnimDataHeader *pHeader);
		void ReadEncodedFrames(struct msg_s *msg);
		void ReadEncodedFramesEx(struct msg_s *msg);
		void LoadProcessedAnim(const char *path, void *buffer, size_t len, const char *name);
		void LoadProcessedAnimEx(const char *path, void *buffer, size_t len, const char *name);

		static std::map<std::string, std::weak_ptr<SkeletonAnimation>> g_skelAnimCache;
	};
};
