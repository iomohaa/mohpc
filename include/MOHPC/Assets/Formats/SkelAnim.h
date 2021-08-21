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

	using skelFlag_t = uint32_t;

	/**
	 * Skeleton animation flags.
	 */
	namespace SKF
	{
		 /** The animation should loop. */
		MOHPC_ASSETS_EXPORTS extern const skelFlag_t LOOP;
		/** Contains delta data. */
		MOHPC_ASSETS_EXPORTS extern const skelFlag_t HASDELTA;
		/** Contains morph data. */
		MOHPC_ASSETS_EXPORTS extern const skelFlag_t HASMORPH;
	}

	struct SkanAnimFrame
	{
		size_t nFrameNum;
		vec3_t pos;
		vec3_t rot;
	};

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

	class SkeletonAnimation : public Asset
	{
		MOHPC_ASSET_OBJECT_DECLARATION(SkeletonAnimation);

	public:
		MOHPC_ASSETS_EXPORTS SkeletonAnimation(const fs::path& fileName);

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
		SkeletonChannelList& getChannelList();

		/** Returns whether or not transforms change over time. */
		MOHPC_ASSETS_EXPORTS bool IsDynamic() const;

		std::vector<AnimFrame>& getFrames();
		std::vector<SkanChannelHdr>& getAryChannels();
		void setFlags(int32_t newFlags);
		void setDelta(bool hasDeltaValue);
		void setMorph(bool hasMorphValue);
		void setUpper(bool hasUpperValue);
		void setTotalDelta(const_vec3r_t newDelta);
		void setTotalAngleDelta(float newAngle);
		void setFrameTime(float newFrametime);
		void setBounds(const_vec3r_t mins, const_vec3r_t maxs);

	private:
		int32_t flags;
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
	};

	class SkeletonAnimationReader : public AssetReader
	{
		MOHPC_ASSET_OBJECT_DECLARATION(SkeletonAnimationReader);

	public:
		using AssetType = SkeletonAnimation;

	public:
		MOHPC_ASSETS_EXPORTS SkeletonAnimationReader();
		MOHPC_ASSETS_EXPORTS ~SkeletonAnimationReader();

		MOHPC_ASSETS_EXPORTS AssetPtr read(const IFilePtr& file) override;
		MOHPC_ASSETS_EXPORTS static SkeletonAnimationPtr readNewAnim(const AssetManagerPtr& assetManager, const fs::path& path);

	private:
		void ConvertSkelFileToGame(const File_AnimDataHeader* pHeader, size_t iBuffLength, SkeletonAnimation& skelAnim);
		void ReadEncodedFrames(MSG& msg, std::vector<AnimFrame>& m_frame, std::vector<SkanChannelHdr>& ary_channels);
		void ReadEncodedFramesEx(MSG& msg, const SkeletonChannelList& channelList, std::vector<AnimFrame>& m_frame, std::vector<SkanChannelHdr>& ary_channels);
		SkeletonAnimationPtr LoadProcessedAnim(const fs::path& path, void* buffer, size_t len);
		SkeletonAnimationPtr LoadProcessedAnimEx(const fs::path& path, void* buffer, size_t len);
	};

	class SkeletonAnimationWriter : public AssetWriter
	{
		MOHPC_ASSET_OBJECT_DECLARATION(SkeletonAnimationWriter);

	private:
		void WriteEncodedFrames(MSG& msg);
		void WriteEncodedFramesEx(MSG& msg);
		void SaveProcessedAnim(const fs::path& path, File_AnimDataHeader* pHeader);
	};

	namespace SkelAnimError
	{
		class Base : public std::exception {};

		/**
		 * The skeleton animation is too small.
		 */
		class BadSize : public Base
		{
		public:
			BadSize(size_t size);

			MOHPC_ASSETS_EXPORTS size_t getSize() const;

		public:
			const char* what() const noexcept override;

		private:
			size_t foundSize;
		};

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
