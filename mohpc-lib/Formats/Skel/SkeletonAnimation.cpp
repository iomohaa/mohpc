#include <Shared.h>
#include "SkelPrivate.h"
#include "../TIKI/TIKI_Private.h"
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Managers/FileManager.h>

using namespace MOHPC;

con_set<str, WeakPtr<SkeletonAnimation>> SkeletonAnimation::g_skelAnimCache;

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

bool SkeletonAnimation::Load()
{
	const char *Fname = GetFilename().c_str();
	if (*Fname == '/' || *Fname == '\\') Fname++;
	const str nwPath = str("/newanim/") + Fname;
	void* buf;
	std::streamsize length = 0;

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
			return false;
		}

		File_AnimDataHeader* pHeader;
		length = file->ReadBuffer((void**)&buf);
		if (length > 0)
		{
			pHeader = (File_AnimDataHeader*)buf;

			if (pHeader->ident != TIKI_SKC_HEADER_IDENT || (pHeader->version != TIKI_SKC_HEADER_OLD_VERSION && pHeader->version != TIKI_SKC_HEADER_VERSION))
			{
				return false;
			}

			if (pHeader->version == TIKI_SKC_HEADER_OLD_VERSION)
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

	return true;
}

bool Compress( SkeletonAnimation::AnimFrame *current, SkeletonAnimation::AnimFrame *last, size_t channelIndex, SkeletonChannelList *channelList, SkeletonChannelNameTable *channelNames )
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

void SkeletonAnimation::EncodeFrames(SkeletonChannelList *channelList, SkeletonChannelNameTable *channelNames )
{
	int frameCnt;
	AnimFrame *pCurrFrame;
	AnimFrame *pLastFrame;
	SkanGameFrame *pFrame;
	int indexLastFrameAdded;

	size_t numFrames = m_frame.size();
	size_t nTotalChannels = ary_channels.size();

	SkanChannelHdr* pChannel = &ary_channels[0];
	size_t endFrameCap = numFrames - 2;

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

void SkeletonAnimation::ConvertSkelFileToGame(File_AnimDataHeader *pHeader, size_t iBuffLength, const char *path)
{
	if( pHeader->numFrames <= 0 )
	{
		return;
	}

	m_frame.resize(pHeader->numFrames);

	AnimFrame* pGameFrame = &m_frame[0];
	File_AnimFrame* pFileFrame = pHeader->frame;
	AnimFrame* newFrame = pGameFrame;

	for(int32_t i = 0; i < pHeader->numFrames; i++)
	{
		newFrame->bounds[ 0 ] = pFileFrame->bounds[ 0 ];
		newFrame->bounds[ 1 ] = pFileFrame->bounds[ 1 ];
		newFrame->delta = pFileFrame->delta;
		newFrame->angleDelta = pFileFrame->angleDelta;
		newFrame->pChannels = new vec4_t[pHeader->numChannels];

		for (int j = 0; j < pHeader->numChannels; j++)
		{
			vec4_t* pChannels = (vec4_t*)((uint8_t*)pHeader + pFileFrame->iOfsChannels);
			Vec4Copy(pChannels[j], newFrame->pChannels[j]);
		}

		AddToBounds( newFrame->bounds, pFileFrame->bounds );

		pFileFrame++;
		newFrame++;
	}

	flags = pHeader->flags;
	frameTime = pHeader->frameTime;
	totalDelta = pHeader->totalDelta;
	totalAngleDelta = pHeader->totalAngleDelta;
	ary_channels.resize(pHeader->numChannels);

	channelList.ZeroChannels();

	skelChannelName_t *pChannelNames = (skelChannelName_t *)((char *)pHeader + pHeader->ofsChannelNames);

	for(int32_t i = 0; i < pHeader->numChannels; i++)
	{
		size_t channelIndex = GetAssetManager()->GetManager<SkeletorManager>()->GetChannelNamesTable()->RegisterChannel( *pChannelNames );
		channelList.AddChannel( channelIndex );
		pChannelNames++;
	}

	channelList.PackChannels();
	EncodeFrames(&channelList, GetManager<SkeletorManager>()->GetChannelNamesTable());

	if (channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "Bip01 pos") &&
		channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "Bip01 R Foot pos") &&
		channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "Bip01 L Foot pos"))
	{
		bHasDelta = true;
	}
	else
	{
		bHasDelta = false;
	}

	if (channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "Bip01 Spine rot") &&
		channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "Bip01 Spine1 rot"))
	{
		bHasUpper = true;
	}
	else
	{
		bHasUpper = false;
	}

	if (channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "VISEME_Bump")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "VISME_Cage_")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "VISME_Earth")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "VISME_Fave")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "VISME_If")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "VISME_New")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "VISME_Ox")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "VISME_Roar")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "VISME_Size")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "VISME_Though")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "VISME_Told")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "VISME_Wet")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "BROW_frown")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "BROW_R_lift")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "BROW_lift")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "BROW_worry")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "EYE_blink")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "EYES_Excited__")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "EYES_L_squint")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "EYES_narrow__")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "EYES_down")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "EYES_left")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "EYES_right")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "EYES_smile")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "EYES_up__")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "JAW_open-closed")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "JAW_open-open")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "LIPS_compressed")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "MOUTH_L_smile_closed")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "MOUTH_L_smile_open")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "MOUTH_L_snarl_closed_")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "MOUTH_L_snarl_open")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "MOUTH_grimace")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "MOUTH_smile_closed")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "MOUTH_smile_open")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "MOUTH_Snarl_closed")
		|| channelList.HasChannel(GetManager<SkeletorManager>()->GetChannelNamesTable(), "MOUTH_Snarl_open"))
	{
		bHasMorph = true;
	}
	else
	{
		bHasMorph = false;
	}
}

void MOHPC::SkeletonAnimation::WriteEncodedFrames(class MSG* msg)
{
	SkanChannelHdr* pChannel;
	SkanGameFrame* pFrame;

	uint32_t numFrames = (uint32_t)m_frame.size();
	uint16_t numChannels = (uint16_t)ary_channels.size();

	msg->SerializeUInteger(numFrames);
	msg->SerializeUShort(numChannels);

	size_t nTotalChannels = ary_channels.size();

	for (size_t i = 0; i < nTotalChannels; i++)
	{
		pChannel = &ary_channels[i];
		uint16_t nFramesInChannel = (uint16_t)pChannel->ary_frames.size();
		msg->SerializeUShort(nFramesInChannel);

		for (size_t j = 0; j < nFramesInChannel; j++)
		{
			pFrame = &pChannel->ary_frames[i];

			uint16_t frameNum = (uint16_t)pFrame->nFrameNum;
			uint16_t prevFrameIndex = (uint16_t)pFrame->nPrevFrameIndex;
			msg->SerializeUShort(frameNum);
			msg->SerializeUShort(prevFrameIndex);
			msg->Serialize(pFrame->pChannelData, sizeof(vec4_t));
		}
	}

	uint32_t nBytesUsed = sizeof(int32_t) + sizeof(int32_t);
	nBytesUsed += sizeof(bool) + sizeof(bool) + sizeof(bool);
	nBytesUsed += sizeof(SkelVec3);
	nBytesUsed += sizeof(float) + sizeof(float);
	nBytesUsed += sizeof(SkeletonChannelList);
	nBytesUsed += sizeof(SkelVec3) * 2;
	nBytesUsed += sizeof(AnimFrame) * (uint32_t)m_frame.size();
	nBytesUsed += sizeof(SkanChannelHdr) * (uint32_t)ary_channels.size();

	msg->SerializeUInteger(nBytesUsed);
}

void MOHPC::SkeletonAnimation::WriteEncodedFramesEx(class MSG* msg)
{
	size_t nTotalChannels = ary_channels.size();
	SkeletorManager* skeletorManager = GetManager<SkeletorManager>();
	for (size_t i = 0; i < nTotalChannels; i++)
	{
		SkanChannelHdr* pChannel = &ary_channels[i];

		const char* name = channelList.ChannelName(skeletorManager->GetChannelNamesTable(), i);
		int32_t type = SkeletonChannelNameTable::GetChannelTypeFromName(name);
		uint16_t frameCnt = (uint16_t)pChannel->ary_frames.size();
		msg->SerializeUShort(frameCnt);

		SkanGameFrame* ary_frames = pChannel->ary_frames.data();

		if (type)
		{
			if (type == 1)
			{
				for (uint16_t j = 0; j < frameCnt; j++)
				{
					const SkanGameFrame* pFrame = &ary_frames[j];

					uint16_t nFrameNum = (uint16_t)pFrame->nFrameNum;
					uint16_t nPrevFrameIndex = (uint16_t)pFrame->nPrevFrameIndex;
					msg->SerializeUShort(nFrameNum);
					msg->SerializeUShort(nPrevFrameIndex);

					float channelData[] = { pFrame->pChannelData[0],  pFrame->pChannelData[1], pFrame->pChannelData[2] };
					msg->SerializeFloat(channelData[0]);
					msg->SerializeFloat(channelData[1]);
					msg->SerializeFloat(channelData[2]);
				}
			}
			else if (type == 3)
			{
				for (uint16_t j = 0; j < frameCnt; j++)
				{
					const SkanGameFrame* pFrame = &ary_frames[j];

					uint16_t nFrameNum = (uint16_t)pFrame->nFrameNum;
					uint16_t nPrevFrameIndex = (uint16_t)pFrame->nPrevFrameIndex;
					msg->SerializeUShort(nFrameNum);
					msg->SerializeUShort(nPrevFrameIndex);

					float channelData[] = { pFrame->pChannelData[0] };
					msg->SerializeFloat(channelData[0]);
				}
			}
		}
		else
		{
			for (uint16_t j = 0; j < frameCnt; j++)
			{
				const SkanGameFrame* pFrame = &ary_frames[j];

				uint16_t nFrameNum = (uint16_t)pFrame->nFrameNum;
				uint16_t nPrevFrameIndex = (uint16_t)pFrame->nPrevFrameIndex;
				msg->SerializeUShort(nFrameNum);
				msg->SerializeUShort(nPrevFrameIndex);

				float channelData[] = { pFrame->pChannelData[0],  pFrame->pChannelData[1], pFrame->pChannelData[2], pFrame->pChannelData[3] };
				msg->SerializeFloat(channelData[0]);
				msg->SerializeFloat(channelData[1]);
				msg->SerializeFloat(channelData[2]);
				msg->SerializeFloat(channelData[3]);
			}
		}
	}
}

void SkeletonAnimation::ReadEncodedFrames(class MSG* msg)
{
	uint32_t numFrames;
	uint16_t nTotalChannels;
	msg->SerializeUInteger(numFrames);
	msg->SerializeUShort(nTotalChannels);

	m_frame.resize(numFrames);
	ary_channels.resize(nTotalChannels);

	for (int32_t i = 0; i < nTotalChannels; i++)
	{
		SkanChannelHdr *pChannel = &ary_channels[ i ];
		uint16_t frameCnt;
		msg->SerializeUShort(frameCnt);

		pChannel->ary_frames.resize(frameCnt);

		for (int32_t j = 0; j < frameCnt; j++)
		{
			SkanGameFrame* pFrame = &pChannel->ary_frames[j];

			uint16_t frameNum;
			uint16_t prevFrameIndex;

			msg->SerializeUShort(frameNum);
			msg->SerializeUShort(prevFrameIndex);

			pFrame->nFrameNum = frameNum;
			pFrame->nPrevFrameIndex = prevFrameIndex;
			msg->Serialize(pFrame->pChannelData, sizeof(vec4_t));
		}
	}

	uint32_t nBytesUsed;
	msg->SerializeUInteger(nBytesUsed);
}

void SkeletonAnimation::ReadEncodedFramesEx(class MSG* msg)
{
	size_t nTotalChannels = ary_channels.size();
	SkeletorManager* skeletorManager = GetManager<SkeletorManager>();
	for (size_t i = 0; i < nTotalChannels; i++)
	{
		SkanChannelHdr *pChannel = &ary_channels[i];

		const char* name = channelList.ChannelName(skeletorManager->GetChannelNamesTable(), i);
		int32_t type = SkeletonChannelNameTable::GetChannelTypeFromName(name);
		uint16_t frameCnt;
		msg->SerializeUShort(frameCnt);

		pChannel->ary_frames.resize(frameCnt);
		SkanGameFrame* ary_frames = pChannel->ary_frames.data();

		if (type)
		{
			if (type == 1)
			{
				for (uint16_t j = 0; j < frameCnt; j++)
				{
					SkanGameFrame *pFrame = &ary_frames[j];

					uint16_t nFrameNum, nPrevFrameIndex;
					msg->SerializeUShort(nFrameNum);
					msg->SerializeUShort(nPrevFrameIndex);

					pFrame->nFrameNum = nFrameNum;
					pFrame->nPrevFrameIndex = nPrevFrameIndex;
					msg->SerializeFloat(pFrame->pChannelData[0]);
					msg->SerializeFloat(pFrame->pChannelData[1]);
					msg->SerializeFloat(pFrame->pChannelData[2]);
					pFrame->pChannelData[3] = 0;
				}
			}
			else if (type == 3)
			{
				for (uint16_t j = 0; j < frameCnt; j++)
				{
					SkanGameFrame *pFrame = &ary_frames[j];

					uint16_t nFrameNum, nPrevFrameIndex;
					msg->SerializeUShort(nFrameNum);
					msg->SerializeUShort(nPrevFrameIndex);

					pFrame->nFrameNum = nFrameNum;
					pFrame->nPrevFrameIndex = nPrevFrameIndex;
					msg->SerializeFloat(pFrame->pChannelData[0]);
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
				uint16_t nFrameNum, nPrevFrameIndex;
				msg->SerializeUShort(nFrameNum);
				msg->SerializeUShort(nPrevFrameIndex);

				pFrame->nFrameNum = nFrameNum;
				pFrame->nPrevFrameIndex = nPrevFrameIndex;
				msg->SerializeFloat(pFrame->pChannelData[0]);
				msg->SerializeFloat(pFrame->pChannelData[1]);
				msg->SerializeFloat(pFrame->pChannelData[2]);
				msg->SerializeFloat(pFrame->pChannelData[3]);
			}
		}
	}
}

void SkeletonAnimation::SaveProcessedAnim(const char* path, File_AnimDataHeader* pHeader)
{
	/*
		int i;
		skelChannelName_t *pChannelNames;
		msg->t msg;
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

	uint32_t numChannels;
	msg.SerializeUInteger(numChannels);

	msg.SerializeInteger(flags);
	msg.SerializeFloat(frameTime);
	msg.SerializeFloat(totalDelta[0]);
	msg.SerializeFloat(totalDelta[1]);
	msg.SerializeFloat(totalDelta[2]);
	msg.SerializeFloat(totalAngleDelta);
	uint32_t numFrames;
	uint16_t nTotalChannels;
	msg.SerializeUInteger(numFrames);
	msg.SerializeUShort(nTotalChannels);
	msg.SerializeByteBool(bHasDelta);
	msg.SerializeByteBool(bHasUpper);
	msg.SerializeByteBool(bHasMorph);

	m_frame.resize(numFrames);
	ary_channels.resize(nTotalChannels);

	AnimFrame* newFrame = &m_frame[0];

	for(uint32_t i = 0; i < numFrames; i++)
	{
		msg.SerializeFloat(newFrame->bounds[0][0]);
		msg.SerializeFloat(newFrame->bounds[0][1]);
		msg.SerializeFloat(newFrame->bounds[0][2]);
		msg.SerializeFloat(newFrame->bounds[1][0]);
		msg.SerializeFloat(newFrame->bounds[1][2]);
		msg.SerializeFloat(newFrame->bounds[1][2]);
		msg.SerializeFloat(newFrame->radius);
		msg.SerializeFloat(newFrame->delta[0]);
		msg.SerializeFloat(newFrame->delta[1]);
		msg.SerializeFloat(newFrame->delta[2]);
		msg.SerializeFloat(newFrame->angleDelta);
		newFrame->pChannels = NULL;
		newFrame++;
	}

	msg.SerializeFloat(bounds[0][0]);
	msg.SerializeFloat(bounds[0][1]);
	msg.SerializeFloat(bounds[0][2]);
	msg.SerializeFloat(bounds[1][0]);
	msg.SerializeFloat(bounds[1][2]);
	msg.SerializeFloat(bounds[1][2]);
	ReadEncodedFrames(&msg);

	msg.SerializeUInteger(numChannels);
	channelList.ZeroChannels();

	for (uint32_t i = 0; i < numChannels; i++)
	{
		MOHPC::StringMessage channelName;
		msg.SerializeString(channelName);

		channelList.AddChannel(GetAssetManager()->GetManager<SkeletorManager>()->GetChannelNamesTable()->RegisterChannel(channelName));
	}

	channelList.PackChannels();
}

void SkeletonAnimation::LoadProcessedAnimEx(const char *path, void *buffer, size_t len, const char *name)
{
	FixedDataMessageStream stream(buffer, len);

	MSG msg(stream, MOHPC::msgMode_e::Reading);

	uint16_t numChannels;
	msg.SerializeUShort(numChannels);

	channelList.ZeroChannels();
	msg.SerializeInteger(flags);
	msg.SerializeFloat(frameTime);
	msg.SerializeFloat(totalDelta[0]);
	msg.SerializeFloat(totalDelta[1]);
	msg.SerializeFloat(totalDelta[2]);
	msg.SerializeFloat(totalAngleDelta);

	uint32_t numFrames;
	msg.SerializeUInteger(numFrames);
	
	msg.SerializeByteBool(bHasDelta);
	msg.SerializeByteBool(bHasUpper);
	msg.SerializeByteBool(bHasMorph);

	ary_channels.resize(numChannels);
	int32_t nTotalChannels = numChannels;
	for (int32_t i = 0; i < nTotalChannels; i++)
	{
		MOHPC::StringMessage channelName;
		msg.SerializeString(channelName);

		channelList.AddChannel(GetAssetManager()->GetManager<SkeletorManager>()->GetChannelNamesTable()->RegisterChannel(channelName));
	}

	channelList.PackChannels();

	m_frame.resize(numFrames);
	AnimFrame* newFrame = &m_frame[0];

	for (uint32_t i = 0; i < numFrames; i++)
	{
		msg.SerializeFloat(newFrame->bounds[0][0]);
		msg.SerializeFloat(newFrame->bounds[0][1]);
		msg.SerializeFloat(newFrame->bounds[0][2]);
		msg.SerializeFloat(newFrame->bounds[1][0]);
		msg.SerializeFloat(newFrame->bounds[1][2]);
		msg.SerializeFloat(newFrame->bounds[1][2]);
		msg.SerializeFloat(newFrame->radius);
		msg.SerializeFloat(newFrame->delta[0]);
		msg.SerializeFloat(newFrame->delta[1]);
		msg.SerializeFloat(newFrame->delta[2]);
		msg.SerializeFloat(newFrame->angleDelta);
		newFrame->pChannels = NULL;
		newFrame++;
	}

	msg.SerializeFloat(bounds[0][0]);
	msg.SerializeFloat(bounds[0][1]);
	msg.SerializeFloat(bounds[0][2]);
	msg.SerializeFloat(bounds[1][0]);
	msg.SerializeFloat(bounds[1][2]);
	msg.SerializeFloat(bounds[1][2]);
	ReadEncodedFramesEx(&msg);
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
