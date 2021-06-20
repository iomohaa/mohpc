#pragma once

class MOHPC_ASSETS_EXPORTS SkelVec4 {
public:
	union {
		float val[ 4 ];
		struct {
			float x;
			float y;
			float z;
			float w;
		} xyzw;
	};

protected:
	void copy( class SkelVec4 const & );

public:
	SkelVec4( float x, float y, float z, float w );
	SkelVec4( const SkelVec3& vec3, float w );
	SkelVec4( const float *vec4 );

	operator float *( );
	operator float *( ) const;

	void set( float x, float y, float z, float w );
};
