#include <Shared.h>
// Basic Hash functions

#include <MOHPC/Script/con_set.h>
#include <MOHPC/Script/str.h>
#include <MOHPC/Vector.h>

using namespace MOHPC;

template<>
intptr_t MOHPC::HashCode< const char * >( const char * const& key )
{
	const char *p;
	int hash = 0;

	for( p = key; *p != 0; p++ )
	{
		hash = *p + 31 * hash;
	}

	return hash;
}

template<>
intptr_t MOHPC::HashCode< void * >( void * const& key )
{
	return 0;
}

template<>
intptr_t MOHPC::HashCode< const void * >( const void * const& key )
{
	return 0;
}

template<>
intptr_t MOHPC::HashCode< char >( const char& key )
{
	return key;
}

template<>
intptr_t MOHPC::HashCode< unsigned char >( const unsigned char& key )
{
	return key;
}

template<>
intptr_t MOHPC::HashCode< unsigned char * >( unsigned char * const& key )
{
	return (intptr_t)key;
}

template<>
intptr_t MOHPC::HashCode< short >( const short& key )
{
	return key;
}

template<>
intptr_t MOHPC::HashCode< unsigned short >( const unsigned short& key )
{
	return key;
}

/*
template<>
int MOHPC::HashCode< short3 >( const short3& key )
{
	return key;
}

template<>
int MOHPC::HashCode< unsigned_short3 >( const unsigned_short3& key )
{
	return key;
}
*/

template<>
intptr_t MOHPC::HashCode<int>( const int& key )
{
	return key;
}

template<>
intptr_t MOHPC::HashCode<unsigned int>( const unsigned int& key )
{
	return key;
}

template<>
intptr_t MOHPC::HashCode<long long>(const long long& key)
{
	return key;
}

template<>
intptr_t MOHPC::HashCode<unsigned long long>(const unsigned long long& key)
{
	return key;
}

template<>
intptr_t MOHPC::HashCode< float >( const float& key )
{
	return *( int * )&key;
}

template<>
intptr_t MOHPC::HashCode< double >( const double& key )
{
	return *( int * )&key;
}

template<>
intptr_t MOHPC::HashCode< str >( const str& key )
{
	return MOHPC::HashCode< const char * >( key.c_str() );
}

template<>
intptr_t MOHPC::HashCode< Vector >( const Vector& key )
{
	return ( int )( ( key[ 0 ] + key[ 1 ] + key[ 2 ] ) / 3 );
}
