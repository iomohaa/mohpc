#include <Shared.h>
#include "SkelPrivate.h"

using namespace MOHPC;

intptr_t SkeletonChannelList::GlobalChannel(intptr_t localchannel) const
{
	return m_chanGlobalFromLocal[localchannel];
}

intptr_t SkeletonChannelList::LocalChannel(intptr_t globalChannel) const
{
	const intptr_t* channel = m_chanLocalFromGlobal.findKeyValue(globalChannel);
	return channel ? *channel : -1;
}

intptr_t SkeletonChannelList::GetGlobalFromLocal(intptr_t channel) const
{
	if (channel >= 0 && channel < (intptr_t)m_chanGlobalFromLocal.NumObjects())
	{
		return GlobalChannel(channel);
	}
	else
	{
		return -1;
	}
}

intptr_t SkeletonChannelList::GetLocalFromGlobal(intptr_t globalChannel) const
{
	const intptr_t* channel = m_chanLocalFromGlobal.findKeyValue(globalChannel);
	return channel ? *channel : -1;
}

intptr_t SkeletonChannelList::NumChannels( void ) const
{
	//return m_numChannels;
	return m_chanGlobalFromLocal.NumObjects();
}

void SkeletonChannelList::ZeroChannels()
{
	/*
	int i;

	m_numChannels = 0;
	m_chanLocalFromGlobal = new short[MAX_CHANNELS];
	m_numLocalFromGlobal = MAX_CHANNELS;

	for( i = 0; i < MAX_CHANNELS; i++ )
	{
		m_chanLocalFromGlobal[ i ] = -1;
	*/

	m_chanLocalFromGlobal.clear();
	m_chanGlobalFromLocal.ClearObjectList();
}

void SkeletonChannelList::PackChannels()
{
	/*
	m_numLocalFromGlobal = MAX_CHANNELS - 1;

	if( m_chanLocalFromGlobal[ m_numLocalFromGlobal ] == -1 )
	{
		do
		{
			m_numLocalFromGlobal--;
		} while( m_chanLocalFromGlobal[ m_numLocalFromGlobal ] == -1 && m_numLocalFromGlobal >= 0 );
	}

	if( m_numLocalFromGlobal < MAX_CHANNELS )
	{
		m_numLocalFromGlobal++;
	}

	if( m_numLocalFromGlobal <= 0 )
	{
		delete[] m_chanLocalFromGlobal;
		m_chanLocalFromGlobal = NULL;
		m_numLocalFromGlobal = -1;
	}
	else
	{
		short *old_array = m_chanLocalFromGlobal;
		m_chanLocalFromGlobal = new short[m_numLocalFromGlobal];
		memcpy( m_chanLocalFromGlobal, old_array, m_numLocalFromGlobal * sizeof( m_chanLocalFromGlobal[ 0 ] ) );
		delete[] old_array;
	}
	*/
}

void SkeletonChannelList::CleanUpChannels()
{
	/*
	if( m_chanLocalFromGlobal )
	{
		free( m_chanLocalFromGlobal );
		m_chanLocalFromGlobal = NULL;
	}

	m_numLocalFromGlobal = 0;
	*/

	m_chanLocalFromGlobal.clear();
	m_chanGlobalFromLocal.ClearObjectList();
}

void SkeletonChannelList::InitChannels()
{
	/*
	m_chanLocalFromGlobal = NULL;
	m_numLocalFromGlobal = 0;
	*/
}

intptr_t SkeletonChannelList::AddChannel(intptr_t newGlobalChannelNum )
{
	intptr_t iLocalChannel;

	if (newGlobalChannelNum == -1)
	{
		//return m_numChannels++;
		iLocalChannel = m_chanGlobalFromLocal.NumObjects();
		//m_chanGlobalFromLocal.push_back(-1);
		m_chanGlobalFromLocal.AddObject(-1);
		return iLocalChannel;
	}

	iLocalChannel = GetLocalFromGlobal( newGlobalChannelNum );

	if( iLocalChannel < 0 )
	{
		//iLocalChannel = m_numChannels++;
		//m_chanGlobalFromLocal[ iLocalChannel ] = newGlobalChannelNum;
		iLocalChannel = m_chanGlobalFromLocal.NumObjects();
		//m_chanGlobalFromLocal.push_back(newGlobalChannelNum);
		m_chanGlobalFromLocal.AddObject(newGlobalChannelNum);
		m_chanLocalFromGlobal.addKeyValue(newGlobalChannelNum) = iLocalChannel;
	}

	return iLocalChannel;
}

bool SkeletonChannelList::HasChannel(const SkeletonChannelNameTable *nameTable, const char *channelName) const
{
	intptr_t iGlobalChannel = nameTable->FindNameLookup( channelName );

	if( iGlobalChannel >= 0 )
	{
		const intptr_t* channel = m_chanLocalFromGlobal.findKeyValue(iGlobalChannel);
		return channel ? (~*channel >> 31) : true;
	}
	else
	{
		return false;
	}
}


const char *SkeletonChannelList::ChannelName(const SkeletonChannelNameTable *nameTable, intptr_t localChannelNum) const
{
	if (localChannelNum >= 0 && localChannelNum < (intptr_t)m_chanGlobalFromLocal.NumObjects())
	{
		return nameTable->FindName(m_chanGlobalFromLocal[localChannelNum]);
	}
	else
	{
		return NULL;
	}
}
