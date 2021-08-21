#include <Shared.h>
#include "SkelPrivate.h"

using namespace MOHPC;

intptr_t SkeletonChannelList::GlobalChannel(intptr_t localchannel) const
{
	return m_chanGlobalFromLocal[localchannel];
}

intptr_t SkeletonChannelList::GetGlobalFromLocal(intptr_t channel) const
{
	if (channel >= 0 && channel < (intptr_t)m_chanGlobalFromLocal.size()) {
		return GlobalChannel(channel);
	} else {
		return -1;
	}
}

intptr_t SkeletonChannelList::GetLocalFromGlobal(intptr_t globalChannel) const
{
	const auto it = m_chanLocalFromGlobal.find(globalChannel);
	if (it != m_chanLocalFromGlobal.end()) {
		return it->second;
	}

	return -1;
}

intptr_t SkeletonChannelList::NumChannels( void ) const
{
	return m_chanGlobalFromLocal.size();
}

void SkeletonChannelList::ZeroChannels()
{
	m_chanLocalFromGlobal.clear();
	m_chanGlobalFromLocal.clear();
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

	m_chanGlobalFromLocal.shrink_to_fit();
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
	m_chanGlobalFromLocal.clear();
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
		iLocalChannel = m_chanGlobalFromLocal.size();
		m_chanGlobalFromLocal.push_back(-1);
		return iLocalChannel;
	}

	iLocalChannel = m_chanGlobalFromLocal.size();
	// try to insert the element
	auto pair = m_chanLocalFromGlobal.emplace(newGlobalChannelNum, iLocalChannel);
	if (pair.second)
	{
		// got it
		m_chanGlobalFromLocal.push_back(newGlobalChannelNum);
	}

	return iLocalChannel;
}

bool SkeletonChannelList::HasChannel(const SkeletonChannelNameTable *nameTable, const char *channelName) const
{
	intptr_t iGlobalChannel = nameTable->FindNameLookup( channelName );

	if( iGlobalChannel >= 0 )
	{
		const auto it = m_chanLocalFromGlobal.find(iGlobalChannel);
		if (it != m_chanLocalFromGlobal.end()) {
			return ~it->second >> 31;
		}

		return true;
	}
	else
	{
		return false;
	}
}


const char *SkeletonChannelList::ChannelName(const SkeletonChannelNameTable *nameTable, intptr_t localChannelNum) const
{
	if (localChannelNum >= 0 && localChannelNum < (intptr_t)m_chanGlobalFromLocal.size()) {
		return nameTable->FindName(m_chanGlobalFromLocal[localChannelNum]);
	} else {
		return nullptr;
	}
}
