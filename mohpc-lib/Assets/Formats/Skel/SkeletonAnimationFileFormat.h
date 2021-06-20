#pragma once

#include <cstdint>

using skelAnimChannel_t = float[4];

struct SkeletonAnimation::File_AnimFrame
{
	SkelVec3 bounds[ 2 ];
	float radius;
	SkelVec3 delta;
	float angleDelta;
	uint32_t iOfsChannels;
};

struct SkeletonAnimation::File_AnimDataHeader
{
	uint8_t ident[4];
	uint32_t version;
	uint32_t flags;
	uint32_t nBytesUsed;
	float frameTime;
	SkelVec3 totalDelta;
	float totalAngleDelta;
	uint32_t numChannels;
	uint32_t ofsChannelNames;
	uint32_t numFrames;
	File_AnimFrame frame[1];
};
