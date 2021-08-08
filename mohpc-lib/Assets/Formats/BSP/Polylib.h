#pragma once

namespace MOHPC
{
	typedef struct
	{
		int		numpoints;
		vec3_t	p[4];		// variable sized
	} winding_t;


static constexpr long MAX_MAP_BOUNDS = 16384;
static constexpr long MIN_MAP_BOUNDS = (-MAX_MAP_BOUNDS);
static constexpr unsigned long MAP_SIZE = (MAX_MAP_BOUNDS - MIN_MAP_BOUNDS);

static constexpr unsigned long MAX_POINTS_ON_WINDING = 64;

static constexpr unsigned long SIDE_FRONT = 0;
static constexpr unsigned long SIDE_BACK = 1;
static constexpr unsigned long SIDE_ON = 2;
static constexpr unsigned long SIDE_CROSS = 3;

#define	CLIP_EPSILON	0.1f

	// you can define on_epsilon in the makefile as tighter
#ifndef	ON_EPSILON
#define	ON_EPSILON	0.1f
#endif

	winding_t	*AllocWinding(int points);
	vec_t	WindingArea(winding_t *w);
	void	WindingCenter(winding_t *w, vec3_t center);
	void	ClipWindingEpsilon(winding_t *in, vec3_t normal, vec_t dist,
		vec_t epsilon, winding_t **front, winding_t **back);
	winding_t	*ChopWinding(winding_t *in, vec3_t normal, vec_t dist);
	winding_t	*CopyWinding(winding_t *w);
	winding_t	*ReverseWinding(winding_t *w);
	winding_t	*BaseWindingForPlane(vec3_t normal, vec_t dist);
	void	CheckWinding(winding_t *w);
	void	WindingPlane(winding_t *w, vec3_t normal, vec_t *dist);
	void	RemoveColinearPoints(winding_t *w);
	int		WindingOnPlaneSide(winding_t *w, vec3_t normal, vec_t dist);
	void	FreeWinding(winding_t *w);
	void	WindingBounds(winding_t *w, vec3_t mins, vec3_t maxs);

	void	AddWindingToConvexHull(winding_t *w, winding_t **hull, vec3_t normal);

	void	ChopWindingInPlace(winding_t **w, vec3_t normal, vec_t dist, vec_t epsilon);
	// frees the original if clipped

	void pw(winding_t *w);
}
