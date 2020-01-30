#include <Shared.h>
#include <MOHPC/Script/MEM_TempAlloc.h>
#include <stdlib.h>

using namespace MOHPC;

MEM_TempAlloc::MEM_TempAlloc()
{
	m_CurrentMemoryBlock = NULL;
}

void *MEM_TempAlloc::Alloc( size_t len )
{
	unsigned char *prev_block = m_CurrentMemoryBlock;
	unsigned char *result;

	if( m_CurrentMemoryBlock && m_CurrentMemoryPos + len <= 65536 )
	{
		result = m_CurrentMemoryBlock + m_CurrentMemoryPos + sizeof( unsigned char * );
		m_CurrentMemoryPos += len;
	}
	else
	{
		m_CurrentMemoryPos = len;

		if( len < 65536 )
			len = 65536;

		m_CurrentMemoryBlock = ( unsigned char * )malloc( len + sizeof( unsigned char * ) );
		*( unsigned char ** )m_CurrentMemoryBlock = prev_block;
		result = m_CurrentMemoryBlock + sizeof( unsigned char * );
	}

	return result;
}

void MEM_TempAlloc::FreeAll( void )
{
	unsigned char *prev_block;

	while( m_CurrentMemoryBlock )
	{
		prev_block = *( unsigned char ** )m_CurrentMemoryBlock;
		free( m_CurrentMemoryBlock );
		m_CurrentMemoryBlock = prev_block;
	}
}
