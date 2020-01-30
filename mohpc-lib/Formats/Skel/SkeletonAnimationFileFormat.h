#pragma once

typedef float skelAnimChannel_t[ 4 ];

struct SkeletonAnimation::File_AnimFrame
{
	SkelVec3 bounds[ 2 ];
	float radius;
	SkelVec3 delta;
	float angleDelta;
	int iOfsChannels; 
};

struct SkeletonAnimation::File_AnimDataHeader
{
	int ident;
	int version;
	int flags;
	int nBytesUsed;
	float frameTime;
	SkelVec3 totalDelta;
	float totalAngleDelta;
	int numChannels;
	int ofsChannelNames;
	int numFrames;
	File_AnimFrame frame[ 1 ];
};
