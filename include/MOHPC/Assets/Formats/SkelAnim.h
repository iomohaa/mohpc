#pragma once

#include "../Asset.h"
#include "../../Utility/SharedPtr.h"
#include "../../Common/SimpleVector.h"

#include <cstdint>

namespace MOHPC
{
	class MSG;

	using SkeletonAnimationPtr = SharedPtr<class SkeletonAnimation>;
	using ConstSkeletonAnimationPtr =  SharedPtr<const class SkeletonAnimation>;

	static constexpr unsigned int MAX_ANIM_POSES = 64;
	static constexpr unsigned int MAX_ANIM_MOVEMENTS_POSES = (MAX_ANIM_POSES >> 1);
	static constexpr unsigned int MAX_ANIM_ACTIONS_POSES = (MAX_ANIM_POSES >> 1);

	struct SkanAnimFrame
	{
		size_t nFrameNum;
		vec3_t pos;
		vec3_t rot;
	};

	class SkeletonAnimation : public Asset
	{
		MOHPC_ASSET_OBJECT_DECLARATION(SkeletonAnimation);

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
		vec3_t totalDelta;
		float totalAngleDelta;
		float frameTime;
		SkeletonChannelList channelList;
		SkelVec3 bounds[2];
		std::vector<AnimFrame> m_frame;
		std::vector<SkanChannelHdr> ary_channels;

	public:
		void Load() override;

	public:
		MOHPC_ASSETS_EXPORTS SkeletonAnimation();

		/** Returns the number of frames to use with the given time. */
		MOHPC_ASSETS_EXPORTS size_t GetFrameNums(float timeSeconds, float timeTolerance, size_t *beforeFrame, size_t *afterFrame, float *beforeWeight, float *afterWeight) const;

		/** Returns the number of frames. */
		MOHPC_ASSETS_EXPORTS size_t GetNumFrames() const;

		/** Return the frame at the specified index. */
		MOHPC_ASSETS_EXPORTS const AnimFrame *GetFrame(size_t index) const;

		/** Retuns the number of channels. */
		MOHPC_ASSETS_EXPORTS size_t GetNumAryChannels() const;

		/** Returns the channel at the specified index. */
		MOHPC_ASSETS_EXPORTS const SkanChannelHdr *GetAryChannel(size_t index) const;

		/** Returns the total animation time. */
		MOHPC_ASSETS_EXPORTS float GetTime() const;

		/** Returns the time it takes for a frame to complete. */
		MOHPC_ASSETS_EXPORTS float GetFrameTime() const;

		/** Returns the total delta of the animation. */
		MOHPC_ASSETS_EXPORTS const_vec3p_t GetDelta() const;

		/** Returns true if the animation contains a delta data. */
		MOHPC_ASSETS_EXPORTS bool HasDelta() const;

		/** Returns the delta between two times in seconds. */
		MOHPC_ASSETS_EXPORTS void GetDeltaOverTime(float Time1, float Time2, vec3r_t delta);

		/** Returns the animation flags. */
		MOHPC_ASSETS_EXPORTS int32_t GetFlags() const;

		/** Returns the animation's skeleton flags. */
		MOHPC_ASSETS_EXPORTS int32_t GetFlagsSkel() const;

		/** Returns the bounds an animation takes. */
		MOHPC_ASSETS_EXPORTS void GetBounds(vec3r_t OutMins, vec3r_t OutMaxs) const;
		MOHPC_ASSETS_EXPORTS void GetBounds(SkelVec3& OutMins, SkelVec3& OutMaxs) const;

		/** Returns the channel list of the animation. */
		MOHPC_ASSETS_EXPORTS const SkeletonChannelList *GetChannelList() const;

		/** Returns whether or not transforms change over time. */
		MOHPC_ASSETS_EXPORTS bool IsDynamic() const;

	private:
		void EncodeFrames(const SkeletonChannelList *channelList, const SkeletonChannelNameTable *channelNames);
		void ConvertSkelFileToGame(const File_AnimDataHeader *pHeader, size_t iBuffLength, const char *path);
		void WriteEncodedFrames(MSG& msg);
		void WriteEncodedFramesEx(MSG& msg);
		void SaveProcessedAnim(const char *path, File_AnimDataHeader *pHeader);
		void ReadEncodedFrames(MSG& msg);
		void ReadEncodedFramesEx(MSG& msg);
		void LoadProcessedAnim(const char *path, void *buffer, size_t len, const char *name);
		void LoadProcessedAnimEx(const char *path, void *buffer, size_t len, const char *name);
	};

	namespace SkelAnimError
	{
		class Base : public std::exception {};

		/**
		 * The skeleton animation has wrong header.
		 */
		class BadHeader : public Base
		{
		public:
			BadHeader(const uint8_t foundHeader[4]);

			MOHPC_ASSETS_EXPORTS const uint8_t* getHeader() const;

		public:
			const char* what() const noexcept override;

		private:
			uint8_t foundHeader[4];
		};

		/**
		 * The skeleton animation has wrong version.
		 */
		class WrongVersion : public Base
		{
		public:
			WrongVersion(const uint32_t version);

			MOHPC_ASSETS_EXPORTS uint32_t getVersion() const;

		public:
			const char* what() const noexcept override;

		private:
			uint32_t foundVersion;
		};
	}
};
