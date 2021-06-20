#include <Shared.h>
#include "SkelPrivate.h"

using namespace MOHPC;

SkeletonChannelNameTable::SkeletonChannelNameTable()
{
}

void SkeletonChannelNameTable::CopyChannel(SkeletonChannelName *dest, const SkeletonChannelName *source ) const
{
	*dest = *source;
}

void SkeletonChannelNameTable::SetChannelName(SkeletonChannelName *channel, const char *newName ) const
{
	channel->name = newName;
}

void SkeletonChannelNameTable::PrintContents() const
{
	str channelList;
	int i;

	intptr_t numChannels = m_Channels.NumObjects();
	for( i = 0; i < numChannels; i++ )
	{
		if( !m_Channels[ i ].name[ 0 ] )
		{
			continue;
		}

		channelList += str("c") + str(m_Channels[i].channelNum) + str(":") + str(m_Channels[i].name) + "\n";
	}
}

bool SkeletonChannelNameTable::FindIndexFromName( const char *name, intptr_t *indexPtr ) const
{
	int sortValue;
	intptr_t lowerBound;
	intptr_t upperBound;
	intptr_t index;

	intptr_t numChannels = m_Channels.NumObjects();

	lowerBound = 0;
	upperBound = numChannels - 1;
	while( lowerBound <= upperBound )
	{
		index = ( lowerBound + upperBound ) / 2;
		sortValue = stricmp( name, m_Channels[ index ].name.c_str() );
		if( !sortValue )
		{
			if( indexPtr )
				*indexPtr = index;
			return true;
		}
		if( sortValue <= 0 )
		{
			upperBound = index - 1;
		}
		else
		{
			lowerBound = index + 1;
		}
	}

	if( indexPtr )
		*indexPtr = lowerBound;
	return false;
}

intptr_t SkeletonChannelNameTable::FindNameLookup( const char *name ) const
{
	intptr_t index;

	if( FindIndexFromName( name, &index ) )
	{
		return m_Channels[ index ].channelNum;
	}
	else
	{
		return -1;
	}
}


const char *SkeletonChannelNameTable::FindName(intptr_t index ) const
{
	return FindNameFromLookup( m_lookup[ index ] );
}


void SkeletonChannelNameTable::SortIntoTable(intptr_t index)
{
	SkeletonChannelName tempName;

	intptr_t numChannels = m_Channels.NumObjects() - 1;

	CopyChannel(&tempName, &m_Channels[numChannels]);

	for(intptr_t i = numChannels - 1; i >= index; i--)
	{
		m_lookup[m_Channels[i].channelNum] = i + 1;
		CopyChannel(&m_Channels[i + 1], &m_Channels[i]);
	}

	m_lookup[tempName.channelNum] = index;
	CopyChannel(&m_Channels[ index ], &tempName);
}

static const char *bogusNameTable[] =
{
	"Bip01 Spine pos",
	"Bip01 Spine1 pos",
	"Bip01 Spine2 pos",
	"Bip01 Neck pos",
	"Bip01 Head pos",
	"helmet bone pos",
	"helmet bone rot",
	"Bip01 R Clavicle pos",
	"Bip01 R UpperArm pos",
	"Bip01 R Forearm pos",
	"Bip01 R Hand pos",
	"Bip01 R Finger0 pos",
	"Bip01 R Finger01 pos",
	"Bip01 R Finger02 pos",
	"Bip01 R Finger1 pos",
	"Bip01 R Finger11 pos",
	"Bip01 R Finger12 pos",
	"Bip01 R Finger2 pos",
	"Bip01 R Finger21 pos",
	"Bip01 R Finger22 pos",
	"Bip01 R Finger3 pos",
	"Bip01 R Finger31 pos",
	"Bip01 R Finger32 pos",
	"Bip01 R Finger4 pos",
	"Bip01 R Finger41 pos",
	"Bip01 R Finger42 pos",
	"Bip01 L Clavicle pos",
	"Bip01 L UpperArm pos",
	"Bip01 L Forearm pos",
	"Bip01 L Hand pos",
	"Bip01 L Finger0 pos",
	"Bip01 L Finger01 pos",
	"Bip01 L Finger02 pos",
	"Bip01 L Finger1 pos",
	"Bip01 L Finger11 pos",
	"Bip01 L Finger12 pos",
	"Bip01 L Finger2 pos",
	"Bip01 L Finger21 pos",
	"Bip01 L Finger22 pos",
	"Bip01 L Finger3 pos",
	"Bip01 L Finger31 pos",
	"Bip01 L Finger32 pos",
	"Bip01 L Finger4 pos",
	"Bip01 L Finger41 pos",
	"Bip01 L Finger42 pos",
	"Bip01 L Toe0 pos",
	"Bip01 R Toe0 pos",
	"buckle bone pos",
	"buckle bone rot",
	"belt attachments bone pos",
	"belt attachments bone rot",
	"backpack bone pos",
	"backpack bone rot",
	"helper Lelbow pos",
	"helper Lelbow rot",
	"helper Lshoulder pos",
	"helper Lshoulder rot",
	"helper Lshoulder01 pos",
	"helper Lshoulder01 rot",
	"helper Rshoulder pos",
	"helper Rshoulder rot",
	"helper Lshoulder02 pos",
	"helper Lshoulder02 rot",
	"helper Relbow pos",
	"helper Relbow rot",
	"helper Lankle pos",
	"helper Lankle rot",
	"helper Lknee pos",
	"helper Lknee rot",
	"helper Rankle pos",
	"helper Rankle rot",
	"helper Rknee pos",
	"helper Rknee rot",
	"helper Lhip pos",
	"helper Lhip rot",
	"helper Lhip01 pos",
	"helper Lhip01 rot",
	"helper Rhip pos",
	"helper Rhip rot",
	"helper Rhip01 pos",
	"helper Rhip01 rot",
	"target_left_hand pos",
	"target_left_hand rot",
	"target_right_hand pos",
	"target_right_hand rot",
	"JAW open-closed pos",
	"JAW open-closed rot",
	"BROW_worry$",
	"EYES_down$",
	"EYES_Excited_$",
	"EYES_L_squint$",
	"EYES_left$",
	"EYES_right$",
	"EYES_smile$",
	"EYES_up_$",
	"JAW_open-open$",
	"MOUTH_L_smile_open$",
	"VISEME_Eat$",
	"right foot placeholder pos"
	"right foot placeholder rot"
	"left foot placeholder pos"
	"left foot placeholder rot"
};

bool SkeletonChannelNameTable::IsBogusChannelName( const char *name )
{
	int i;

	for( i = 0; i < sizeof( bogusNameTable  )/ sizeof( bogusNameTable[ 0 ] ); i++ )
	{
		if( !stricmp( name, bogusNameTable[ i ] ) )
		{
			return true;
		}
	}

	if( strstr( name, "Bip0" ) && !strstr( name, "Bip01" ) && !strstr( name, "Footsteps" ) )
	{
		return true;
	}

	return false;
}

int SkeletonChannelNameTable::GetChannelTypeFromName( const char *name )
{
	int i;
	intptr_t len;

	if( !name )
	{
		return 2;
	}

	for( i = 0; i < sizeof( bogusNameTable ) / sizeof( bogusNameTable[ 0 ] ); i++ )
	{
		if( !stricmp( name, bogusNameTable[ i ] ) )
		{
			return 2;
		}
	}

	if( strstr( name, "Bip0" ) && !strstr( name, "Bip01" ) && !strstr( name, "Footsteps" ) )
	{
		return 2;
	}

	len = strlen( name );

	if( len >= 4 )
	{
		if( !memcmp( name + len - 4, " rot", 5 ) )
		{
			return 0;
		}
		else if( !memcmp( name + len - 4, " pos", 5 ) )
		{
			return 1;
		}
		else if( len >= 6 )
		{
			if( !memcmp( name + len - 4, " rotFK", 7 ) )
			{
				return 2;
			}
			else
			{
				return 3;
			}
		}
		else
		{
			return 3;
		}
	}
	else
	{
		return 3;
	}

	return false;
}

intptr_t SkeletonChannelNameTable::RegisterChannel( const char *name )
{
	if( IsBogusChannelName( name ) )
	{
		return -1;
	}

	intptr_t index;
	if( FindIndexFromName( name, &index ) )
	{
		return m_Channels[index].channelNum;
	}

	intptr_t numChannels = m_Channels.NumObjects();

	m_Channels.SetNumObjects(numChannels + 1);
	m_lookup.resize(numChannels + 1);

	SetChannelName(&m_Channels[numChannels], name);
	m_Channels[numChannels].channelNum = numChannels;
	SortIntoTable(index);
	return numChannels;
}

const char *SkeletonChannelNameTable::FindNameFromLookup( intptr_t index ) const
{
	if (index < (intptr_t)m_Channels.NumObjects())
	{
		return m_Channels[index].name.c_str();
	}
	else
	{
		return NULL;
	}
}
