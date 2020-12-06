#include <Shared.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Misc/Endian.h>
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

SkeletonAnimation::AnimFrame::AnimFrame()
{
	pChannels = NULL;
}

SkeletonAnimation::AnimFrame::~AnimFrame()
{
	if (pChannels)
	{
		delete[] pChannels;
		pChannels = NULL;
	}
}

CLASS_DEFINITION(SkeletonAnimation);
SkeletonAnimation::SkeletonAnimation()
{
}

void SkeletonAnimation::Load()
{
	const char *Fname = GetFilename().c_str();
	if (*Fname == '/' || *Fname == '\\') Fname++;
	const str nwPath = str("/newanim/") + Fname;
	void* buf;
	uint64_t length = 0;

	FilePtr file = GetFileManager()->OpenFile(nwPath.c_str());
	if (file)
	{
		length = file->ReadBuffer(&buf);
		if (length > 0)
		{
			// MOH:AA Animation file
			LoadProcessedAnim(Fname, buf, (size_t)length, Fname);
		}
	}
	else
	{
		file = GetFileManager()->OpenFile(Fname);
		if (!file)
		{
			// not a valid file
			throw AssetError::AssetNotFound(Fname);
		}

		const File_AnimDataHeader* pHeader;
		length = file->ReadBuffer((void**)&buf);
		if (length > 0)
		{
			pHeader = (const File_AnimDataHeader*)buf;

			// check for header and version
			if (memcmp(pHeader->ident, TIKI_SKC_HEADER_IDENT, sizeof(pHeader->ident))) {
				throw SkelAnimError::BadHeader(pHeader->ident);
			}

			const uint32_t version = Endian.LittleInteger(pHeader->version);
			if(version != TIKI_SKC_HEADER_OLD_VERSION && version != TIKI_SKC_HEADER_VERSION) {
				throw SkelAnimError::WrongVersion(version);
			}

			if (version == TIKI_SKC_HEADER_OLD_VERSION)
			{
				//Com_Printf("WARNING- DOWNGRADING TO OLD ANIMATION FORMAT FOR FILE: %s\n", path);
				ConvertSkelFileToGame(pHeader, (size_t)length, Fname);
				/*
				if (convertAnims && convertAnims->integer)
				{
				Skeletor::SaveProcessedAnim(finishedHeader, path, pHeader);
				}
				*/
			}
			else
			{
				// points the buffer to the animation data
				void* buffer = (char *)pHeader + sizeof(int) + sizeof(int);
				length -= sizeof(int) + sizeof(int);

				// loads the processed animation
				// MOH:SH and MOH:BT animations
				LoadProcessedAnimEx(Fname, buffer, (size_t)length, Fname);
			}
		}
	}

	HashUpdate((uint8_t*)buf, length);
}

bool Compress( SkeletonAnimation::AnimFrame *current, SkeletonAnimation::AnimFrame *last, size_t channelIndex, const SkeletonChannelList *channelList, const SkeletonChannelNameTable *channelNames )
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

void SkeletonAnimation::EncodeFrames(const SkeletonChannelList *channelList, const SkeletonChannelNameTable *channelNames )
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

void SkeletonAnimation::ConvertSkelFileToGame(const File_AnimDataHeader *pHeader, size_t iBuffLength, const char *path)
{
	const uint32_t numFrames = Endian.LittleInteger(pHeader->numFrames);
	if( numFrames <= 0 )
	{
		// no frame to process
		return;
	}

	m_frame.SetNumObjects(numFrames);

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

	flags = Endian.LittleInteger(pHeader->flags);
	frameTime = Endian.LittleFloat(pHeader->frameTime);
	totalDelta = LittleSkelVec(pHeader->totalDelta);
	totalAngleDelta = Endian.LittleFloat(pHeader->totalAngleDelta);
	ary_channels.resize(numChannels);

	channelList.ZeroChannels();

	SkeletonChannelNameTable* channelNameTable = GetAssetManager()->GetManager<SkeletorManager>()->GetChannelNamesTable();
	const skelChannelName_t* pChannelNames = (const skelChannelName_t*)((char*)pHeader + Endian.LittleInteger(pHeader->ofsChannelNames));

	for (uint32_t i = 0; i < numChannels; i++)
	{
		const size_t channelIndex = channelNameTable->RegisterChannel(*pChannelNames);
		channelList.AddChannel(channelIndex);
		pChannelNames++;
	}

	channelList.PackChannels();
	EncodeFrames(&channelList, channelNameTable);

	if (channelList.HasChannel(channelNameTable, "Bip01 pos") &&
		channelList.HasChannel(channelNameTable, "Bip01 R Foot pos") &&
		channelList.HasChannel(channelNameTable, "Bip01 L Foot pos"))
	{
		bHasDelta = true;
	}
	else
	{
		bHasDelta = false;
	}

	if (channelList.HasChannel(channelNameTable, "Bip01 Spine rot") &&
		channelList.HasChannel(channelNameTable, "Bip01 Spine1 rot"))
	{
		bHasUpper = true;
	}
	else
	{
		bHasUpper = false;
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
		bHasMorph = true;
	}
	else
	{
		bHasMorph = false;
	}
}

void MOHPC::SkeletonAnimation::WriteEncodedFrames(MSG& msg)
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

void MOHPC::SkeletonAnimation::WriteEncodedFramesEx(MSG& msg)
{
	size_t nTotalChannels = ary_channels.size();
	const SkeletorManagerPtr skeletorManager = GetManager<SkeletorManager>();
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

void SkeletonAnimation::ReadEncodedFrames(MSG& msg)
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

void SkeletonAnimation::ReadEncodedFramesEx(MSG& msg)
{
	size_t nTotalChannels = ary_channels.size();
	SkeletorManagerPtr skeletorManager = GetManager<SkeletorManager>();
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

void SkeletonAnimation::SaveProcessedAnim(const char* path, File_AnimDataHeader* pHeader)
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

void SkeletonAnimation::LoadProcessedAnim(const char *path, void *buffer, size_t len, const char *name)
{
	FixedDataMessageStream stream(buffer, len);

	MSG msg(stream, MOHPC::msgMode_e::Reading);

	// number of channels
	// but useless as it will be used later
	msg.ReadUInteger();

	flags = msg.ReadInteger();
	frameTime = msg.ReadFloat();
	totalDelta[0] = msg.ReadFloat();
	totalDelta[1] = msg.ReadFloat();
	totalDelta[2] = msg.ReadFloat();
	totalAngleDelta = msg.ReadFloat();
	const uint32_t numFrames = msg.ReadUInteger();
	const uint16_t nTotalChannels = msg.ReadUShort();
	bHasDelta = msg.ReadByteBool();
	bHasUpper = msg.ReadByteBool();
	bHasMorph = msg.ReadByteBool();

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

	bounds[0][0] = msg.ReadFloat();
	bounds[0][1] = msg.ReadFloat();
	bounds[0][2] = msg.ReadFloat();
	bounds[1][0] = msg.ReadFloat();
	bounds[1][2] = msg.ReadFloat();
	bounds[1][2] = msg.ReadFloat();
	ReadEncodedFrames(msg);

	const uint32_t numChannels = msg.ReadUInteger();
	channelList.ZeroChannels();

	SkeletonChannelNameTable* const channelNamesTable = GetAssetManager()->GetManager<SkeletorManager>()->GetChannelNamesTable();

	for (uint32_t i = 0; i < numChannels; i++)
	{
		const MOHPC::StringMessage channelName = msg.ReadString();
		channelList.AddChannel(channelNamesTable->RegisterChannel(channelName));
	}

	channelList.PackChannels();
}

void SkeletonAnimation::LoadProcessedAnimEx(const char *path, void *buffer, size_t len, const char *name)
{
	FixedDataMessageStream stream(buffer, len);

	MSG msg(stream, MOHPC::msgMode_e::Reading);

	const uint16_t numChannels = msg.ReadUShort();

	channelList.ZeroChannels();
	flags = msg.ReadInteger();
	frameTime = msg.ReadFloat();
	totalDelta[0] = msg.ReadFloat();
	totalDelta[1] = msg.ReadFloat();
	totalDelta[2] = msg.ReadFloat();
	totalAngleDelta = msg.ReadFloat();
	const uint32_t numFrames = msg.ReadUInteger();
	bHasDelta = msg.ReadByteBool();
	bHasUpper = msg.ReadByteBool();
	bHasMorph = msg.ReadByteBool();

	SkeletonChannelNameTable* const channelNamesTable = GetAssetManager()->GetManager<SkeletorManager>()->GetChannelNamesTable();

	ary_channels.resize(numChannels);
	for (int32_t i = 0; i < numChannels; i++)
	{
		const MOHPC::StringMessage channelName = msg.ReadString();
		channelList.AddChannel(channelNamesTable->RegisterChannel(channelName));
	}

	channelList.PackChannels();

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

	bounds[0][0] = msg.ReadFloat();
	bounds[0][1] = msg.ReadFloat();
	bounds[0][2] = msg.ReadFloat();
	bounds[1][0] = msg.ReadFloat();
	bounds[1][2] = msg.ReadFloat();
	bounds[1][2] = msg.ReadFloat();
	ReadEncodedFramesEx(msg);
}


size_t SkeletonAnimation::GetNumFrames() const
{
	return m_frame.size();
}

const SkeletonAnimation::AnimFrame *SkeletonAnimation::GetFrame(size_t index) const
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

const SkeletonAnimation::SkanChannelHdr *SkeletonAnimation::GetAryChannel(size_t index) const
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

Vector SkeletonAnimation::GetDelta() const
{
	return Vector(totalDelta[0], totalDelta[1], totalDelta[2]);
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

void SkeletonAnimation::GetBounds(Vector& OutMins, Vector& OutMaxs) const
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

bool SkeletonAnimation::IsDynamic() const
{
	// Check for any dynamic channels
	for (size_t i = 0; i < GetNumAryChannels(); ++i)
	{
		const SkeletonAnimation::SkanChannelHdr& channel = *GetAryChannel(i);

		// Check if any frame is dynamic
		for (size_t j = 1; j < channel.ary_frames.size(); ++j)
		{
			const SkeletonAnimation::SkanGameFrame& frame = channel.ary_frames[j];
			if (!Vec4Compare(channel.ary_frames[0].pChannelData, frame.pChannelData, 0.05f))
			{
				return true;
			}
		}
	}

	return false;
}

SkelAnimError::BadHeader::BadHeader(const uint8_t inHeader[4])
	: foundHeader{ inHeader[0], inHeader[1], inHeader[2], inHeader[3] }
{
}

const uint8_t* SkelAnimError::BadHeader::getHeader() const
{
	return foundHeader;
}

const char* SkelAnimError::BadHeader::what() const
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

const char* SkelAnimError::WrongVersion::what() const
{
	return "Wrong skeleton animation version";
}
