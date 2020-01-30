#pragma once

#include "../../Global.h"
#include <MOHPC/Math.h>
#include <string.h>

class MOHPC_EXPORTS SkelMat3 {
public:
	float val[ 3 ][ 3 ];

protected:
	void			copy( const SkelMat3& m );

public:
	void			MakeZero();
	void			MakeIdentity();

	SkelMat3( const SkelVec3& x, const SkelVec3& y, const SkelVec3& z );
	SkelMat3( const float *mat[ 3 ] );
	SkelMat3();

	SkelVec3			*XAxis();
	SkelVec3			*YAxis();
	SkelVec3			*ZAxis();

	operator float *( );
	operator float *( ) const;

	float		*operator[]( int index );
	float		*operator[]( int index ) const;

	void			GetEulerAngles( float *vec ) const;
	void			GetScale( float *vec ) const;
	bool			IsOrthonormal() const;
	bool			IsValid() const;

	void			Multiply( const float mat1[ 3 ][ 3 ], const float mat2[ 3 ][ 3 ] );
	float			det() const;
	float			trace() const;
	SkelVec3		TransformVector( const SkelVec3 *skel );
	void			Transpose();
};

inline
SkelMat3::SkelMat3( const SkelVec3& x, const SkelVec3& y, const SkelVec3& z )
{
	VecCopy( x, val[ 0 ] );
	VecCopy( y, val[ 1 ] );
	VecCopy( z, val[ 2 ] );
}

inline
SkelMat3::SkelMat3( const float *mat[ 3 ] )
{
	memcpy( &val, mat, sizeof( val ) );
}

inline
SkelMat3::SkelMat3()
{
	MakeIdentity();
}

inline
void SkelMat3::copy( const SkelMat3& m )
{
	AxisCopy( m.val, val );
}

inline
void SkelMat3::MakeZero()
{
	AxisClear( val );
}

inline
void SkelMat3::MakeIdentity()
{
	MakeZero();
	val[ 0 ][ 0 ] = 1.0f;
	val[ 1 ][ 1 ] = 1.0f;
	val[ 2 ][ 2 ] = 1.0f;
}

inline
SkelVec3 *SkelMat3::XAxis()
{
	return ( SkelVec3 * )&val[ 0 ];
}

inline
SkelVec3 *SkelMat3::YAxis()
{
	return ( SkelVec3 * )&val[ 1 ];
}

inline
SkelVec3 *SkelMat3::ZAxis()
{
	return ( SkelVec3 * )&val[ 2 ];
}

inline
SkelMat3::operator float *( )
{
	return &val[ 0 ][ 0 ];
}

inline
SkelMat3::operator float *( ) const
{
	return ( float * )&val[ 0 ][ 0 ];
}

inline
float *SkelMat3::operator[]( int index )
{
	return val[ index ];
}

inline
float *SkelMat3::operator[]( int index ) const
{
	return ( float * )val[ index ];
}

inline
void SkelMat3::GetEulerAngles( float *vec ) const
{
	MatrixToEulerAngles( val, vec );
}

inline
void SkelMat3::GetScale( float *vec ) const
{
	// FIXME: stub
}

inline
bool SkelMat3::IsOrthonormal() const
{
	// FIXME: stub
	return false;
}

inline
bool SkelMat3::IsValid() const
{
	// FIXME: stub
	return false;
}

inline
void SkelMat3::Multiply( const float mat1[ 3 ][ 3 ], const float mat2[ 3 ][ 3 ] )
{
	MatrixMultiply( mat1, mat2, val );
}

inline
float SkelMat3::det() const
{
	return	( val[ 2 ][ 1 ] * val[ 1 ][ 0 ] - val[ 2 ][ 0 ] * val[ 1 ][ 1 ] ) * val[ 0 ][ 2 ]
		  + ( val[ 1 ][ 1 ] * val[ 2 ][ 2 ] - val[ 1 ][ 2 ] * val[ 2 ][ 1 ] ) * val[ 0 ][ 0 ]
		  - ( val[ 2 ][ 2 ] * val[ 1 ][ 0 ] - val[ 1 ][ 2 ] * val[ 2 ][ 0 ] ) * val[ 0 ][ 1 ];
}

inline
float SkelMat3::trace() const
{
	return val[ 0 ][ 0 ] + val[ 1 ][ 1 ] + val[ 2 ][ 2 ];
}

inline
SkelVec3 SkelMat3::TransformVector( const SkelVec3 *skel )
{
	SkelVec3 out;
	MatrixTransformVector( *skel, val, out );
	return out;
}

inline
void SkelMat3::Transpose()
{
	SkelMat3 out;
	TransposeMatrix( val, out.val );
	memcpy( val, out, sizeof( SkelMat3 ) );
}
