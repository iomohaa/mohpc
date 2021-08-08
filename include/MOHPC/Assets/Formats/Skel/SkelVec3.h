#pragma once

#include "../../AssetsGlobal.h"
#include "../../../Common/Math.h"
#include "../../../Common/Vector.h"

using namespace MOHPC;

typedef enum { svX, svY, svZ, svW } SkelVec_Axis;
typedef enum { Vec3YAW, Vec3PITCH, Vec3ROLL } YPR_Axes;

class SkelVec3 {
public:
	union {
		float val[ 3 ];
		struct {
			float x;
			float y;
			float z;
		};
	};
protected:
	void		copy( const SkelVec3& skel );

public:

	SkelVec3( float x, float y, float z );
	SkelVec3(const vec3r_t vec);
	SkelVec3();

	operator float *( );
	operator float *( ) const;

	float&		operator[] ( int index );
	float		operator[] ( int index ) const;

	SkelVec3& operator=(const_vec3r_t vec);
	const SkelVec3&		operator+=( const SkelVec3 &a );
	const SkelVec3&		operator+=( float a[3] );

	bool		IsZero() const;
	bool		IsUnit() const;
	void		set( float x, float y, float z );

	float		Normalize();
	void		NormalizeFast();

	void		SetZero();
	void		SetXAxis();
	void		SetYAxis();
	void		SetZAxis();
	void		RotateYaw( float yaw, float deg );
};

inline
SkelVec3::SkelVec3( float x, float y, float z )
{
	set( x, y, z );
}

inline
SkelVec3::SkelVec3(const vec3r_t vec)
{
	this->x = vec[0];
	this->y = vec[1];
	this->z = vec[2];
}

inline
SkelVec3::SkelVec3()
{
	SetZero();
}

inline
SkelVec3& SkelVec3::operator=(const_vec3r_t vec)
{
	this->x = vec[0];
	this->y = vec[1];
	this->z = vec[2];
	return *this;
}

inline
SkelVec3::operator float *( )
{
	return val;
}

inline
SkelVec3::operator float *( ) const
{
	return ( float * )val;
}

inline
bool SkelVec3::IsZero() const
{
	return ( x == 0.0f ) && ( y == 0.0f ) && ( z == 0.0f );
}

inline
bool SkelVec3::IsUnit() const
{
	// FIXME: stub
	return false;
}

inline
void SkelVec3::set( float a, float b, float c )
{
	this->x = a;
	this->y = b;
	this->z = c;
}

inline
float SkelVec3::Normalize()
{
	return VectorNormalize(val);
}

inline
void SkelVec3::NormalizeFast()
{
	VectorNormalizeFast(val);
}

inline
void SkelVec3::SetZero()
{
	this->x = 0.0f;
	this->y = 0.0f;
	this->z = 0.0f;
}

inline
void SkelVec3::SetXAxis()
{
	x = 0.0f;
}

inline
void SkelVec3::SetYAxis()
{
	y = 0.0f;
}

inline
void SkelVec3::SetZAxis()
{
	z = 0.0f;
}

inline
void SkelVec3::RotateYaw( float yaw, float deg )
{
	// FIXME: stub
}

inline
float& SkelVec3::operator[]( int index )
{
	return val[ index ];
}

inline
float SkelVec3::operator[]( int index ) const
{
	return val[ index ];
}

inline
const SkelVec3& SkelVec3::operator+=( const SkelVec3 &a )
{
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}

inline
const SkelVec3& SkelVec3::operator+=( float a[3] )
{
	x += a[ 0 ];
	y += a[ 1 ];
	z += a[ 2 ];

	return *this;
}
