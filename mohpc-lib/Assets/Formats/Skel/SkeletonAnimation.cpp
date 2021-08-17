#include <Shared.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Files/FileHelpers.h>
#include <MOHPC/Utility/Misc/Endian.h>
#include <MOHPC/Utility/Misc/EndianCoordHelpers.h>
#include "SkelPrivate.h"
#include "../TIKI/TIKI_Private.h"

using namespace MOHPC;

static SkelVec3 LittleSkelVec(const SkelVec3& inVec)
{
	SkelVec3 result;
	result[0] = Endian.LittleFloat(inVec[0]);
	result[1] = Endian.LittleFloat(inVec[1]);
	result[2] = Endian.LittleFloat(inVec[2]);
	return result;
}

AnimFrame::AnimFrame()
{
	pChannels = NULL;
}

AnimFrame::~AnimFrame()
{
	if (pChannels)
	{
		delete[] pChannels;
		pChannels = NULL;
	}
}

MOHPC_OBJECT_DEFINITION(SkeletonAnimation);
SkeletonAnimation::SkeletonAnimation(const fs::path& fileName)
	: Asset2(fileName)
{
}

bool Compress(AnimFrame *current, AnimFrame *last, size_t channelIndex, const SkeletonChannelList *channelList, const SkeletonChannelNameTable *channelNames)
{
	// high-end PCs don't need to compress...
	return false;

	/*
	float tolerance;
	float difference;

	tolerance = current->pChannels[ channelIndex ][ 0 ];
	if( tolerance > -0.000001 && tolerance < 0.000001 )
	{
		current->pChannels[ channelIndex ][ 0 ] = 0.0f;
	}

	tolerance = current->pChannels[ channelIndex ][ 1 ];
	if( tolerance > -0.000001 && tolerance < 0.000001 )
	{
		current->pChannels[ channelIndex ][ 1 ] = 0.0f;
	}

	tolerance = current->pChannels[ channelIndex ][ 2 ];
	if( tolerance > -0.000001 && tolerance < 0.000001 )
	{
		current->pChannels[ channelIndex ][ 2 ] = 0.0f;
	}

	if( !last )
	{
		return false;
	}

	difference = last->pChannels[ channelIndex ][ 0 ] - current->pChannels[ channelIndex ][ 0 ];
	if( difference < -0.001f || difference >= 0.001f )
	{
		return false;
	}

	difference = last->pChannels[ channelIndex ][ 1 ] - current->pChannels[ channelIndex ][ 1 ];
	if( difference < -0.001f || difference >= 0.001f )
	{
		return false;
	}

	difference = last->pChannels[ channelIndex ][ 2 ] - current->pChannels[ channelIndex ][ 2 ];
	if( difference < -0.001f || difference >= 0.001f )
	{
		return false;
	}
	
	return true;
	*/
}

void EncodeFrames(const SkeletonChannelList *channelList, const SkeletonChannelNameTable *channelNames, std::vector<AnimFrame>& m_frame, std::vector<SkanChannelHdr>& ary_channels)
{
	int frameCnt;
	AnimFrame *pCurrFrame;
	AnimFrame *pLastFrame;
	SkanGameFrame *pFrame;
	int indexLastFrameAdded;

	const size_t numFrames = m_frame.size();
	const size_t nTotalChannels = ary_channels.size();

	SkanChannelHdr* pChannel = &ary_channels[0];
	const size_t endFrameCap = numFrames - 2;

	for (size_t i = 0; i < nTotalChannels; i++, pChannel++)
	{
		pLastFrame = NULL;
		pCurrFrame = &m_frame[0];

		frameCnt = 0;
		for (size_t j = 0; j < numFrames; j++)
		{
			if (!Compress(pCurrFrame, pLastFrame, i, channelList, channelNames) || j >= endFrameCap)
			{
				frameCnt++;
				pLastFrame = pCurrFrame;
			}

			pCurrFrame++;
		}

		pChannel->ary_frames.resize(frameCnt);

		pLastFrame = NULL;
		indexLastFrameAdded = 0;

		pCurrFrame = &m_frame[0];
		pFrame = &pChannel->ary_frames[0];

		for (size_t j = 0; j < numFrames; j++)
		{
			if (!Compress(pCurrFrame, pLastFrame, i, channelList, channelNames) || j >= endFrameCap)
			{
				pFrame->nFrameNum = j;
				pFrame->nPrevFrameIndex = indexLastFrameAdded;

				if (j > 0)
					indexLastFrameAdded++;

				pFrame->pChannelData[0] = pCurrFrame->pChannels[i][0];
				pFrame->pChannelData[1] = pCurrFrame->pChannels[i][1];
				pFrame->pChannelData[2] = pCurrFrame->pChannels[i][2];
				pFrame->pChannelData[3] = pCurrFrame->pChannels[i][3];
				pFrame++;

				pLastFrame = pCurrFrame;
			}

			pCurrFrame++;
		}
	}
}

void SkeletonAnimationReader::ConvertSkelFileToGame(const File_AnimDataHeader *pHeader, size_t iBuffLength, SkeletonAnimation& skelAnim)
{
	const uint32_t numFrames = Endian.LittleInteger(pHeader->numFrames);
	if( numFrames <= 0 )
	{
		// no frame to process
		return;
	}

	std::vector<AnimFrame>& m_frame = skelAnim.getFrames();
	m_frame.resize(numFrames);

	const uint32_t numChannels = Endian.LittleInteger(pHeader->numChannels);
	AnimFrame* pGameFrame = &m_frame[0];
	const File_AnimFrame* pFileFrame = pHeader->frame;
	AnimFrame* newFrame = pGameFrame;

	for(uint32_t i = 0; i < numFrames; i++)
	{
		newFrame->bounds[0] = LittleSkelVec(pFileFrame->bounds[0]);
		newFrame->bounds[1] = LittleSkelVec(pFileFrame->bounds[1]);
		newFrame->delta = LittleSkelVec(pFileFrame->delta);
		newFrame->angleDelta = Endian.LittleFloat(pFileFrame->angleDelta);
		newFrame->pChannels = new vec4_t[numChannels];

		for (uint32_t j = 0; j < numChannels; j++)
		{
			const vec4_t* const pChannels = (vec4_t*)((uint8_t*)pHeader + Endian.LittleInteger(pFileFrame->iOfsChannels));
			newFrame->pChannels[j][0] = Endian.LittleFloat(pChannels[j][0]);
			newFrame->pChannels[j][1] = Endian.LittleFloat(pChannels[j][1]);
			newFrame->pChannels[j][2] = Endian.LittleFloat(pChannels[j][2]);
			newFrame->pChannels[j][3] = Endian.LittleFloat(pChannels[j][3]);
		}

		pFileFrame++;
		newFrame++;
	}

	skelAnim.setFlags(Endian.LittleInteger(pHeader->flags));
	skelAnim.setFrameTime(Endian.LittleFloat(pHeader->frameTime));
	vec3_t totalDelta;
	EndianHelpers::LittleVector(Endian, (float*)pHeader->totalDelta, totalDelta);
	skelAnim.setTotalDelta(totalDelta);
	skelAnim.setTotalAngleDelta(Endian.LittleFloat(pHeader->totalAngleDelta));

	std::vector<SkanChannelHdr>& ary_channels = skelAnim.getAryChannels();
	ary_channels.resize(numChannels);

	SkeletonChannelList& channelList = skelAnim.getChannelList();
	channelList.ZeroChannels();

	SkeletonChannelNameTable* channelNameTable = GetAssetManager()->getManager<SkeletorManager>()->GetChannelNamesTable();
	const skelChannelName_t* pChannelNames = (const skelChannelName_t*)((char*)pHeader + Endian.LittleInteger(pHeader->ofsChannelNames));

	for (uint32_t i = 0; i < numChannels; i++)
	{
		const size_t channelIndex = channelNameTable->RegisterChannel(*pChannelNames);
		channelList.AddChannel(channelIndex);
		pChannelNames++;
	}

	channelList.PackChannels();
	EncodeFrames(&channelList, channelNameTable, m_frame, ary_channels);

	if (channelList.HasChannel(channelNameTable, "Bip01 pos") &&
		channelList.HasChannel(channelNameTable, "Bip01 R Foot pos") &&
		channelList.HasChannel(channelNameTable, "Bip01 L Foot pos"))
	{
		skelAnim.setDelta(true);
	}
	else
	{
		skelAnim.setDelta(false);
	}

	if (channelList.HasChannel(channelNameTable, "Bip01 Spine rot") &&
		channelList.HasChannel(channelNameTable, "Bip01 Spine1 rot"))
	{
		skelAnim.setUpper(true);
	}
	else
	{
		skelAnim.setUpper(false);
	}

	if (channelList.HasChannel(channelNameTable, "VISEME_Bump")
		|| channelList.HasChannel(channelNameTable, "VISME_Cage_")
		|| channelList.HasChannel(channelNameTable, "VISME_Earth")
		|| channelList.HasChannel(channelNameTable, "VISME_Fave")
		|| channelList.HasChannel(channelNameTable, "VISME_If")
		|| channelList.HasChannel(channelNameTable, "VISME_New")
		|| channelList.HasChannel(channelNameTable, "VISME_Ox")
		|| channelList.HasChannel(channelNameTable, "VISME_Roar")
		|| channelList.HasChannel(channelNameTable, "VISME_Size")
		|| channelList.HasChannel(channelNameTable, "VISME_Though")
		|| channelList.HasChannel(channelNameTable, "VISME_Told")
		|| channelList.HasChannel(channelNameTable, "VISME_Wet")
		|| channelList.HasChannel(channelNameTable, "BROW_frown")
		|| channelList.HasChannel(channelNameTable, "BROW_R_lift")
		|| channelList.HasChannel(channelNameTable, "BROW_lift")
		|| channelList.HasChannel(channelNameTable, "BROW_worry")
		|| channelList.HasChannel(channelNameTable, "EYE_blink")
		|| channelList.HasChannel(channelNameTable, "EYES_Excited__")
		|| channelList.HasChannel(channelNameTable, "EYES_L_squint")
		|| channelList.HasChannel(channelNameTable, "EYES_narrow__")
		|| channelList.HasChannel(channelNameTable, "EYES_down")
		|| channelList.HasChannel(channelNameTable, "EYES_left")
		|| channelList.HasChannel(channelNameTable, "EYES_right")
		|| channelList.HasChannel(channelNameTable, "EYES_smile")
		|| channelList.HasChannel(channelNameTable, "EYES_up__")
		|| channelList.HasChannel(channelNameTable, "JAW_open-closed")
		|| channelList.HasChannel(channelNameTable, "JAW_open-open")
		|| channelList.HasChannel(channelNameTable, "LIPS_compressed")
		|| channelList.HasChannel(channelNameTable, "MOUTH_L_smile_closed")
		|| channelList.HasChannel(channelNameTable, "MOUTH_L_smile_open")
		|| channelList.HasChannel(channelNameTable, "MOUTH_L_snarl_closed_")
		|| channelList.HasChannel(channelNameTable, "MOUTH_L_snarl_open")
		|| channelList.HasChannel(channelNameTable, "MOUTH_grimace")
		|| channelList.HasChannel(channelNameTable, "MOUTH_smile_closed")
		|| channelList.HasChannel(channelNameTable, "MOUTH_smile_open")
		|| channelList.HasChannel(channelNameTable, "MOUTH_Snarl_closed")
		|| channelList.HasChannel(channelNameTable, "MOUTH_Snarl_open"))
	{
		skelAnim.setMorph(true);
	}
	else
	{
		skelAnim.setMorph(false);
	}
}

void SkeletonAnimationReader::ReadEncodedFrames(MSG& msg, std::vector<AnimFrame>& m_frame, std::vector<SkanChannelHdr>& ary_channels)
{
	const uint32_t numFrames = msg.ReadUInteger();
	const uint16_t nTotalChannels = msg.ReadUShort();

	m_frame.resize(numFrames);
	ary_channels.resize(nTotalChannels);

	for (uint16_t i = 0; i < nTotalChannels; i++)
	{
		SkanChannelHdr *pChannel = &ary_channels[ i ];
		const uint16_t frameCnt = msg.ReadUShort();

		pChannel->ary_frames.resize(frameCnt);

		for (uint32_t j = 0; j < frameCnt; j++)
		{
			SkanGameFrame* pFrame = &pChannel->ary_frames[j];
			const uint16_t frameNum = msg.ReadUShort();
			const uint16_t prevFrameIndex = msg.ReadUShort();

			pFrame->nFrameNum = frameNum;
			pFrame->nPrevFrameIndex = prevFrameIndex;
			msg.ReadData(pFrame->pChannelData, sizeof(vec4_t));
		}
	}

	const uint32_t nBytesUsed = msg.ReadUInteger();
}

void SkeletonAnimationReader::ReadEncodedFramesEx(MSG& msg, const SkeletonChannelList& channelList, std::vector<AnimFrame>& m_frame, std::vector<SkanChannelHdr>& ary_channels)
{
	size_t nTotalChannels = ary_channels.size();
	SkeletorManagerPtr skeletorManager = getManager<SkeletorManager>();
	for (size_t i = 0; i < nTotalChannels; i++)
	{
		SkanChannelHdr *pChannel = &ary_channels[i];

		const char* name = channelList.ChannelName(skeletorManager->GetChannelNamesTable(), i);
		int32_t type = SkeletonChannelNameTable::GetChannelTypeFromName(name);
		const uint16_t frameCnt = msg.ReadUShort();

		pChannel->ary_frames.resize(frameCnt);
		SkanGameFrame* ary_frames = pChannel->ary_frames.data();

		if (type)
		{
			if (type == 1)
			{
				for (uint16_t j = 0; j < frameCnt; j++)
				{
					SkanGameFrame *pFrame = &ary_frames[j];
					const uint16_t nFrameNum = msg.ReadUShort();
					const uint16_t nPrevFrameIndex = msg.ReadUShort();

					pFrame->nFrameNum = nFrameNum;
					pFrame->nPrevFrameIndex = nPrevFrameIndex;
					pFrame->pChannelData[0] = msg.ReadFloat();
					pFrame->pChannelData[1] = msg.ReadFloat();
					pFrame->pChannelData[2] = msg.ReadFloat();
					pFrame->pChannelData[3] = 0;
				}
			}
			else if (type == 3)
			{
				for (uint16_t j = 0; j < frameCnt; j++)
				{
					SkanGameFrame* pFrame = &ary_frames[j];
					const uint16_t nFrameNum = msg.ReadUShort();
					const uint16_t nPrevFrameIndex = msg.ReadUShort();

					pFrame->nFrameNum = nFrameNum;
					pFrame->nPrevFrameIndex = nPrevFrameIndex;
					pFrame->pChannelData[0] = msg.ReadFloat();
					pFrame->pChannelData[1] = 0;
					pFrame->pChannelData[2] = 0;
					pFrame->pChannelData[3] = 0;
				}
			}
		}
		else
		{
			for (uint16_t j = 0; j < frameCnt; j++)
			{
				SkanGameFrame* pFrame = &ary_frames[j];
				const uint16_t nFrameNum = msg.ReadUShort();
				const uint16_t nPrevFrameIndex = msg.ReadUShort();

				pFrame->nFrameNum = nFrameNum;
				pFrame->nPrevFrameIndex = nPrevFrameIndex;
				pFrame->pChannelData[0] = msg.ReadFloat();
				pFrame->pChannelData[1] = msg.ReadFloat();
				pFrame->pChannelData[2] = msg.ReadFloat();
				pFrame->pChannelData[3] = msg.ReadFloat();
			}
		}
	}
}

SkeletonAnimationPtr SkeletonAnimationReader::LoadProcessedAnim(const fs::path& path, void *buffer, size_t len)
{
	FixedDataMessageStream stream(buffer, len);

	MSG msg(stream, MOHPC::msgMode_e::Reading);

	// number of channels
	// but useless as it will be used later
	msg.ReadUInteger();

	SkeletonAnimationPtr skelAnim(new SkeletonAnimation(path));

	skelAnim->setFlags(msg.ReadInteger());
	skelAnim->setFrameTime(msg.ReadFloat());

	vec3_t totalDelta;
	totalDelta[0] = msg.ReadFloat();
	totalDelta[1] = msg.ReadFloat();
	totalDelta[2] = msg.ReadFloat();
	skelAnim->setTotalDelta(totalDelta);
	skelAnim->setTotalAngleDelta(msg.ReadFloat());

	const uint32_t numFrames = msg.ReadUInteger();
	const uint16_t nTotalChannels = msg.ReadUShort();
	skelAnim->setDelta(msg.ReadByteBool());
	skelAnim->setUpper(msg.ReadByteBool());
	skelAnim->setMorph(msg.ReadByteBool());

	std::vector<AnimFrame>& m_frame = skelAnim->getFrames();
	std::vector<SkanChannelHdr>& ary_channels = skelAnim->getAryChannels();
	m_frame.resize(numFrames);
	ary_channels.resize(nTotalChannels);

	AnimFrame* newFrame = &m_frame[0];

	for(uint32_t i = 0; i < numFrames; i++)
	{
		newFrame->bounds[0][0] = msg.ReadFloat();
		newFrame->bounds[0][1] = msg.ReadFloat();
		newFrame->bounds[0][2] = msg.ReadFloat();
		newFrame->bounds[1][0] = msg.ReadFloat();
		newFrame->bounds[1][2] = msg.ReadFloat();
		newFrame->bounds[1][2] = msg.ReadFloat();
		newFrame->radius = msg.ReadFloat();
		newFrame->delta[0] = msg.ReadFloat();
		newFrame->delta[1] = msg.ReadFloat();
		newFrame->delta[2] = msg.ReadFloat();
		newFrame->angleDelta = msg.ReadFloat();
		newFrame->pChannels = NULL;
		newFrame++;
	}

	vec3_t bounds[2];
	bounds[0][0] = msg.ReadFloat();
	bounds[0][1] = msg.ReadFloat();
	bounds[0][2] = msg.ReadFloat();
	bounds[1][0] = msg.ReadFloat();
	bounds[1][2] = msg.ReadFloat();
	bounds[1][2] = msg.ReadFloat();
	skelAnim->setBounds(bounds[0], bounds[1]);
	ReadEncodedFrames(msg, m_frame, ary_channels);

	const uint32_t numChannels = msg.ReadUInteger();

	SkeletonChannelList& channelList = skelAnim->getChannelList();
	channelList.ZeroChannels();

	SkeletonChannelNameTable* const channelNamesTable = GetAssetManager()->getManager<SkeletorManager>()->GetChannelNamesTable();

	for (uint32_t i = 0; i < numChannels; i++)
	{
		const MOHPC::StringMessage channelName = msg.ReadString();
		channelList.AddChannel(channelNamesTable->RegisterChannel(channelName));
	}

	channelList.PackChannels();

	return skelAnim;
}

SkeletonAnimationPtr SkeletonAnimationReader::LoadProcessedAnimEx(const fs::path& path, void *buffer, size_t len)
{
	FixedDataMessageStream stream(buffer, len);

	MSG msg(stream, MOHPC::msgMode_e::Reading);

	const uint16_t numChannels = msg.ReadUShort();

	SkeletonAnimationPtr skelAnim(new SkeletonAnimation(path));

	SkeletonChannelList& channelList = skelAnim->getChannelList();
	channelList.ZeroChannels();

	skelAnim->setFlags(msg.ReadInteger());
	skelAnim->setFrameTime(msg.ReadFloat());

	vec3_t totalDelta;
	totalDelta[0] = msg.ReadFloat();
	totalDelta[1] = msg.ReadFloat();
	totalDelta[2] = msg.ReadFloat();
	skelAnim->setTotalDelta(totalDelta);
	skelAnim->setTotalAngleDelta(msg.ReadFloat());

	const uint32_t numFrames = msg.ReadUInteger();
	skelAnim->setDelta(msg.ReadByteBool());
	skelAnim->setUpper(msg.ReadByteBool());
	skelAnim->setMorph(msg.ReadByteBool());

	SkeletonChannelNameTable* const channelNamesTable = GetAssetManager()->getManager<SkeletorManager>()->GetChannelNamesTable();

	std::vector<SkanChannelHdr>& ary_channels = skelAnim->getAryChannels();
	ary_channels.resize(numChannels);
	for (int32_t i = 0; i < numChannels; i++)
	{
		const MOHPC::StringMessage channelName = msg.ReadString();
		channelList.AddChannel(channelNamesTable->RegisterChannel(channelName));
	}

	channelList.PackChannels();

	std::vector<AnimFrame>& m_frame = skelAnim->getFrames();
	m_frame.resize(numFrames);
	AnimFrame* newFrame = &m_frame[0];

	for (uint32_t i = 0; i < numFrames; i++)
	{
		newFrame->bounds[0][0] = msg.ReadFloat();
		newFrame->bounds[0][1] = msg.ReadFloat();
		newFrame->bounds[0][2] = msg.ReadFloat();
		newFrame->bounds[1][0] = msg.ReadFloat();
		newFrame->bounds[1][2] = msg.ReadFloat();
		newFrame->bounds[1][2] = msg.ReadFloat();
		newFrame->radius = msg.ReadFloat();
		newFrame->delta[0] = msg.ReadFloat();
		newFrame->delta[1] = msg.ReadFloat();
		newFrame->delta[2] = msg.ReadFloat();
		newFrame->angleDelta = msg.ReadFloat();
		newFrame->pChannels = NULL;
		newFrame++;
	}

	vec3_t bounds[2];
	bounds[0][0] = msg.ReadFloat();
	bounds[0][1] = msg.ReadFloat();
	bounds[0][2] = msg.ReadFloat();
	bounds[1][0] = msg.ReadFloat();
	bounds[1][2] = msg.ReadFloat();
	bounds[1][2] = msg.ReadFloat();
	skelAnim->setBounds(bounds[0], bounds[1]);
	ReadEncodedFramesEx(msg, channelList, m_frame, ary_channels);

	return skelAnim;
}


size_t SkeletonAnimation::GetNumFrames() const
{
	return m_frame.size();
}

const AnimFrame *SkeletonAnimation::GetFrame(size_t index) const
{
	if (index >= 0 && index < m_frame.size())
	{
		return &m_frame[index];
	}
	else
	{
		return nullptr;
	}
}

size_t SkeletonAnimation::GetNumAryChannels() const
{
	return ary_channels.size();
}

const SkanChannelHdr *SkeletonAnimation::GetAryChannel(size_t index) const
{
	if (index >= 0 && index < ary_channels.size())
	{
		return &ary_channels[index];
	}
	else
	{
		return nullptr;
	}
}

void SkeletonAnimation::setFlags(int32_t newFlags)
{
	flags = newFlags;
}

void SkeletonAnimation::setDelta(bool hasDeltaValue)
{
	bHasDelta = hasDeltaValue;
}

void SkeletonAnimation::setMorph(bool hasMorphValue)
{
	bHasMorph = hasMorphValue;
}

void SkeletonAnimation::setUpper(bool hasUpperValue)
{
	bHasUpper = hasUpperValue;
}

void SkeletonAnimation::setTotalDelta(const_vec3r_t newDelta)
{
	totalDelta[0] = newDelta[0];
	totalDelta[1] = newDelta[1];
	totalDelta[2] = newDelta[2];
}

void SkeletonAnimation::setTotalAngleDelta(float newAngle)
{
	totalAngleDelta = newAngle;
}

void SkeletonAnimation::setFrameTime(float newFrametime)
{
	frameTime = newFrametime;
}

void SkeletonAnimation::setBounds(const_vec3r_t mins, const_vec3r_t maxs)
{
	bounds[0] = mins;
	bounds[1] = maxs;
}

float SkeletonAnimation::GetTime() const
{
	return flags & TAF_DELTADRIVEN
		? frameTime * m_frame.size()
		: frameTime * (m_frame.size() - 1);
}

float SkeletonAnimation::GetFrameTime() const
{
	return frameTime;
}

const_vec3p_t SkeletonAnimation::GetDelta() const
{
	return totalDelta;
}

bool SkeletonAnimation::HasDelta() const
{
	return bHasDelta;
}

int32_t SkeletonAnimation::GetFlags() const
{
	return flags;
}

int32_t SkeletonAnimation::GetFlagsSkel() const
{
	int32_t flagsSkel = flags;

	if (bHasDelta)
	{
		flagsSkel |= TAF_HASDELTA;
	}

	if (bHasMorph)
	{
		flagsSkel |= TAF_HASMORPH;
	}

	return flagsSkel;
}

void SkeletonAnimation::GetBounds(vec3r_t OutMins, vec3r_t OutMaxs) const
{
	OutMins[0] = bounds[0][0];
	OutMins[1] = bounds[0][1];
	OutMins[2] = bounds[0][2];
	OutMaxs[0] = bounds[1][0];
	OutMaxs[1] = bounds[1][1];
	OutMaxs[2] = bounds[1][2];
}

void SkeletonAnimation::GetBounds(SkelVec3& OutMins, SkelVec3& OutMaxs) const
{
	OutMins = bounds[0];
	OutMaxs = bounds[1];
}

const SkeletonChannelList *SkeletonAnimation::GetChannelList() const
{
	return &channelList;
}

SkeletonChannelList& SkeletonAnimation::getChannelList()
{
	return channelList;
}

bool SkeletonAnimation::IsDynamic() const
{
	// Check for any dynamic channels
	for (size_t i = 0; i < GetNumAryChannels(); ++i)
	{
		const SkanChannelHdr& channel = *GetAryChannel(i);

		// Check if any frame is dynamic
		for (size_t j = 1; j < channel.ary_frames.size(); ++j)
		{
			const SkanGameFrame& frame = channel.ary_frames[j];
			if (!Vec4Compare(channel.ary_frames[0].pChannelData, frame.pChannelData, 0.05f))
			{
				return true;
			}
		}
	}

	return false;
}

MOHPC_OBJECT_DEFINITION(SkeletonAnimationReader);
SkeletonAnimationReader::SkeletonAnimationReader()
{
}

SkeletonAnimationReader::~SkeletonAnimationReader()
{
}

Asset2Ptr SkeletonAnimationReader::read(const IFilePtr& file)
{
	const fs::path& Fname = file->getName();
	void* buf;
	uint64_t length = 0;

	// MOH:AA new anim
	if (!strHelpers::icmp(Fname.generic_string().c_str(), "/newanim/"))
	{
		length = file->ReadBuffer(&buf);
		if (length > 0)
		{
			// MOH:AA Animation file
			return LoadProcessedAnim(Fname, buf, (size_t)length);
		}
	}
	else
	{
		const File_AnimDataHeader* pHeader;
		length = file->ReadBuffer((void**)&buf);
		if (length > sizeof(File_AnimDataHeader::ident))
		{
			pHeader = (const File_AnimDataHeader*)buf;

			// check for header and version
			if (memcmp(pHeader->ident, TIKI_SKC_HEADER_IDENT, sizeof(pHeader->ident))) {
				throw SkelAnimError::BadHeader(pHeader->ident);
			}

			const uint32_t version = Endian.LittleInteger(pHeader->version);
			if (version != TIKI_SKC_HEADER_OLD_VERSION && version != TIKI_SKC_HEADER_VERSION) {
				throw SkelAnimError::WrongVersion(version);
			}

			if (version == TIKI_SKC_HEADER_OLD_VERSION)
			{
				SkeletonAnimationPtr skelAnim(new SkeletonAnimation(file->getName()));
				ConvertSkelFileToGame(pHeader, (size_t)length, *skelAnim);

				return skelAnim;
			}
			else
			{
				// points the buffer to the animation data
				void* buffer = (char*)pHeader + sizeof(int) + sizeof(int);
				length -= sizeof(int) + sizeof(int);

				// loads the processed animation
				// MOH:SH and MOH:BT animations
				return LoadProcessedAnimEx(Fname, buffer, (size_t)length);
			}
		}
		else
		{
			throw SkelAnimError::BadSize(length);
		}
	}

	return nullptr;
}

Asset2Ptr SkeletonAnimationReader::readNewAnim(const AssetManagerPtr& assetManager, const fs::path& path)
{
	Asset2Ptr asset;
	try
	{
		asset = assetManager->readAsset<SkeletonAnimationReader>(path);
	}
	catch(std::exception& baseException)
	{
		try
		{
			fs::path nwPath = fs::path("/newanim/");
			nwPath.append(FileHelpers::removeRootDir(path.c_str()));
			asset = assetManager->readAsset<SkeletonAnimationReader>(nwPath);
		}
		catch(...)
		{
			throw baseException;
		}
	}

	return asset;
}

#if 0
// FIXME: not implemented
void MOHPC::SkeletonAnimationWriter::WriteEncodedFrames(MSG& msg)
{
	const uint32_t numFrames = (uint32_t)m_frame.size();
	const uint16_t numChannels = (uint16_t)ary_channels.size();

	msg.WriteUInteger(numFrames);
	msg.WriteUShort(numChannels);

	for (uint16_t i = 0; i < numChannels; i++)
	{
		const SkanChannelHdr* const pChannel = &ary_channels[i];
		const uint16_t nFramesInChannel = (uint16_t)pChannel->ary_frames.size();
		msg.WriteUShort(nFramesInChannel);

		for (uint16_t j = 0; j < nFramesInChannel; j++)
		{
			const SkanGameFrame* const pFrame = &pChannel->ary_frames[i];

			msg.WriteUShort((uint16_t)pFrame->nFrameNum);
			msg.WriteUShort((uint16_t)pFrame->nPrevFrameIndex);
			msg.WriteData(pFrame->pChannelData, sizeof(vec4_t));
		}
	}

	const uint32_t nBytesUsed = sizeof(int32_t) + sizeof(int32_t)
		+ sizeof(bool) + sizeof(bool) + sizeof(bool)
		+ sizeof(SkelVec3)
		+ sizeof(float) + sizeof(float)
		+ sizeof(SkeletonChannelList)
		+ sizeof(SkelVec3) * 2
		+ sizeof(AnimFrame) * (uint32_t)m_frame.size()
		+ sizeof(SkanChannelHdr) * (uint32_t)ary_channels.size();

	msg.WriteUInteger(nBytesUsed);
}

void MOHPC::SkeletonAnimationWriter::WriteEncodedFramesEx(MSG& msg)
{
	size_t nTotalChannels = ary_channels.size();
	const SkeletorManagerPtr skeletorManager = getManager<SkeletorManager>();
	for (size_t i = 0; i < nTotalChannels; i++)
	{
		const SkanChannelHdr* const pChannel = &ary_channels[i];

		const char* name = channelList.ChannelName(skeletorManager->GetChannelNamesTable(), i);
		const int32_t type = SkeletonChannelNameTable::GetChannelTypeFromName(name);
		const uint16_t frameCnt = (uint16_t)pChannel->ary_frames.size();
		msg.WriteUShort(frameCnt);

		const SkanGameFrame* const ary_frames = pChannel->ary_frames.data();

		if (type)
		{
			if (type == 1)
			{
				for (uint16_t j = 0; j < frameCnt; j++)
				{
					const SkanGameFrame* pFrame = &ary_frames[j];
					msg.WriteUShort((uint16_t)pFrame->nFrameNum);
					msg.WriteUShort((uint16_t)pFrame->nPrevFrameIndex);

					msg.WriteFloat(pFrame->pChannelData[0]);
					msg.WriteFloat(pFrame->pChannelData[1]);
					msg.WriteFloat(pFrame->pChannelData[2]);
				}
			}
			else if (type == 3)
			{
				for (uint16_t j = 0; j < frameCnt; j++)
				{
					const SkanGameFrame* pFrame = &ary_frames[j];
					msg.WriteUShort((uint16_t)pFrame->nFrameNum);
					msg.WriteUShort((uint16_t)pFrame->nPrevFrameIndex);

					msg.WriteFloat(pFrame->pChannelData[0]);
				}
			}
		}
		else
		{
			for (uint16_t j = 0; j < frameCnt; j++)
			{
				const SkanGameFrame* pFrame = &ary_frames[j];
				msg.WriteUShort((uint16_t)pFrame->nFrameNum);
				msg.WriteUShort((uint16_t)pFrame->nPrevFrameIndex);

				msg.WriteFloat(pFrame->pChannelData[0]);
				msg.WriteFloat(pFrame->pChannelData[1]);
				msg.WriteFloat(pFrame->pChannelData[2]);
				msg.WriteFloat(pFrame->pChannelData[3]);
			}
		}
	}
}

void SkeletonAnimationWriter::SaveProcessedAnim(const fs::path& path, File_AnimDataHeader* pHeader)
{
	/*
		int i;
		skelChannelName_t *pChannelNames;
		msg.t msg;
		skelAnimGameFrame_t *newFrame;
		char npath[ 128 ];
		unsigned char buf[ 2000000 ];
	*/

	// FIXME:
	// Write animation
}
#endif

SkelAnimError::BadSize::BadSize(size_t size)
	: foundSize(size)
{
}

size_t SkelAnimError::BadSize::getSize() const
{
	return foundSize;
}

const char* SkelAnimError::BadSize::what() const noexcept
{
	return "Skeleton animation size too small";
}

SkelAnimError::BadHeader::BadHeader(const uint8_t inHeader[4])
	: foundHeader{ inHeader[0], inHeader[1], inHeader[2], inHeader[3] }
{
}

const uint8_t* SkelAnimError::BadHeader::getHeader() const
{
	return foundHeader;
}

const char* SkelAnimError::BadHeader::what() const noexcept
{
	return "Bad skeleton animation header";
}

SkelAnimError::WrongVersion::WrongVersion(const uint32_t inVersion)
	: foundVersion(inVersion)
{
}

uint32_t SkelAnimError::WrongVersion::getVersion() const
{
	return foundVersion;
}

const char* SkelAnimError::WrongVersion::what() const noexcept
{
	return "Wrong skeleton animation version";
}
