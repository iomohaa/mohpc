#include <MOHPC/Collision/Collision.h>
#include <MOHPC/Managers/ShaderManager.h>

using namespace MOHPC;

#define	SURFACE_CLIP_EPSILON (0.125f)

bool ter_usesphere = true;

/*
====================
CollisionWorld::CM_CheckTerrainPlane
====================
*/
float CollisionWorld::CM_CheckTerrainPlane(vec4_t plane)
{
	float	d1, d2;
	float	f;

	d1 = DotProduct(this->g_trace.vStart, plane) - plane[3];
	d2 = DotProduct(this->g_trace.vEnd, plane) - plane[3];

	// if completely in front of face, no intersection with the entire brush
	if (d1 > 0 && (d2 >= SURFACE_CLIP_EPSILON || d2 >= d1)) {
		return 1;
	}

	if (d1 <= 0 && d2 <= 0)
	{
		if (d1 >= -32) {
			return 0;
		}

		if (d2 >= -32) {
			return 0;
		}
		return 1;
	}

	if (d1 <= d2) {
		return 1;
	}

	f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
	if (f < 0) {
		f = 0;
	}
	return f;
}

/*
====================
CollisionWorld::CM_CheckTerrainTriSpherePoint
====================
*/
float CollisionWorld::CM_CheckTerrainTriSpherePoint(vec3_t v)
{
	vec3_t	vDelta, vDir;
	float	fLenSq;
	float	fRadSq;
	float	fA, fB;
	float	fDiscr;
	float	fFrac;
	float	fSq;
	float	f;

	VecSubtract(this->g_trace.vStart, v, vDir);

	fRadSq = sphere.radius * sphere.radius;
	fLenSq = VectorLengthSquared(vDir);

	if (fLenSq <= fRadSq) {
		this->g_trace.tw->trace.startsolid = true;
		this->g_trace.tw->trace.allsolid = true;
		return 0;
	}

	VecSubtract(this->g_trace.vEnd, this->g_trace.vStart, vDelta);

	fA = VectorLengthSquared(vDelta);
	fB = DotProduct(vDelta, vDir);
	fDiscr = fB * fB - (fLenSq - fRadSq) * fA;

	if (fDiscr <= 0.0f) {
		return this->g_trace.tw->trace.fraction;
	}

	fSq = sqrtf(fDiscr);

	if (fA > 0)
	{
		fFrac = (-fB - fSq) / fA - this->g_trace.fSurfaceClipEpsilon;

		if (fFrac >= 0.0f && fFrac <= this->g_trace.tw->trace.fraction) {
			return fFrac;
		}


		fFrac = -fB + fSq;
	}
	else
	{
		fFrac = (-fB + fSq) / fA - this->g_trace.fSurfaceClipEpsilon;

		if (fFrac >= 0.0f && fFrac <= this->g_trace.tw->trace.fraction) {
			return fFrac;
		}

		fFrac = -fB - fSq;
	}

	f = fFrac / fA - this->g_trace.fSurfaceClipEpsilon;
	if (f < 0 || f > this->g_trace.tw->trace.fraction) {
		f = this->g_trace.tw->trace.fraction;
	}

	return f;
}

/*
====================
CollisionWorld::CM_CheckTerrainTriSphereCorner
====================
*/
float CollisionWorld::CM_CheckTerrainTriSphereCorner(vec4_t plane, float x0, float y0, int i, int j)
{
	vec3_t	v;

	v[0] = ((i << 6) + x0);
	v[1] = ((j << 6) + y0);
	v[2] = (plane[3] - (v[1] * plane[1] + v[0] * plane[0])) / plane[2];

	return CollisionWorld::CM_CheckTerrainTriSpherePoint(v);
}

/*
====================
CollisionWorld::CM_CheckTerrainTriSphereEdge
====================
*/
float CollisionWorld::CM_CheckTerrainTriSphereEdge(float* plane, float x0, float y0, int i0, int j0, int i1, int j1)
{
	vec3_t	v0, v1;
	float	fScale;
	float	fRadSq;
	float	S, T;
	vec3_t	vDeltaStart;
	vec3_t	vDirEdge;
	vec3_t	vDirTrace;
	float	fSFromT_Const;
	float	fSFromT_Scale;
	vec3_t	vRFromT_Const;
	vec3_t	vRFromT_Scale;

	// junk variable(s) as usual
	float	fLengthSq, fDot;
	float	fFrac, fFracClip;

	fScale = 1.0f / plane[2];

	v0[0] = (i0 << 6) + x0;
	v0[1] = (j0 << 6) + y0;
	v0[2] = (plane[3] - (v0[0] * plane[0] + v0[1] * plane[1])) * fScale;

	v1[0] = (i1 << 6) + x0;
	v1[1] = (j1 << 6) + y0;
	v1[2] = (plane[3] - (v1[0] * plane[0] + v1[1] * plane[1])) * fScale;

	VecSubtract(this->g_trace.vStart, v0, vDirTrace);
	VecSubtract(v1, v0, vDirEdge);
	VecSubtract(this->g_trace.vEnd, this->g_trace.vStart, vDeltaStart);

	fScale = 1.0f / VectorLengthSquared(vDirEdge);
	S = DotProduct(vDirTrace, vDirEdge) * fScale;
	T = DotProduct(vDeltaStart, vDirEdge) * fScale;

	VectorMA(vDirTrace, -S, vDirEdge, vRFromT_Const);
	VectorMA(vDeltaStart, -T, vDirEdge, vRFromT_Scale);

	fRadSq = sphere.radius * sphere.radius;
	fLengthSq = VectorLengthSquared(vRFromT_Const);

	if (fLengthSq <= fRadSq)
	{
		if (S < 0 || S > 1) {
			return CollisionWorld::CM_CheckTerrainTriSpherePoint(v0);
		}

		this->g_trace.tw->trace.startsolid = true;
		this->g_trace.tw->trace.allsolid = true;
		return 1;
	}

	fDot = DotProduct(vRFromT_Scale, vRFromT_Const);
	fSFromT_Scale = VectorLengthSquared(vRFromT_Scale);
	fSFromT_Const = fDot * fDot - (fLengthSq - fRadSq) * fSFromT_Scale;

	if (fSFromT_Const <= 0) {
		return this->g_trace.tw->trace.fraction;
	}

	if (fSFromT_Scale > 0) {
		fFrac = (-fDot - sqrtf(fSFromT_Const)) / fSFromT_Scale;
	}
	else {
		fFrac = (-fDot + sqrtf(fSFromT_Const)) / fSFromT_Scale;
	}

	fFracClip = fFrac - this->g_trace.fSurfaceClipEpsilon;
	if (fFrac <= 0 || fFracClip >= this->g_trace.tw->trace.fraction) {
		return this->g_trace.tw->trace.fraction;
	}

	fFrac = fFrac * T + S;

	if (fFrac < 0) {
		return CollisionWorld::CM_CheckTerrainTriSpherePoint(v0);
	}

	if (fFrac > 1) {
		return CollisionWorld::CM_CheckTerrainTriSpherePoint(v1);
	}

	if (fFracClip < 0) {
		fFracClip = 0;
	}
	return fFracClip;
}

/*
====================
CollisionWorld::CM_CheckTerrainTriSphere
====================
*/
float CollisionWorld::CM_CheckTerrainTriSphere(float x0, float y0, int iPlane)
{
	float* plane;
	float	fMaxFraction;
	float	d1, d2;
	float	fSpherePlane;
	int		eMode;
	bool	bFitsX, bFitsY;
	bool	bFitsDiag;
	int		iX[3];
	int		iY[3];

	plane = this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].plane[iPlane];
	d1 = DotProduct(this->g_trace.vStart, plane) - plane[3];
	d2 = DotProduct(this->g_trace.vEnd, plane) - plane[3];

	if (d1 > sphere.radius)
	{
		if (d2 >= sphere.radius + SURFACE_CLIP_EPSILON) {
			return this->g_trace.tw->trace.fraction;
		}

		if (d2 >= d1) {
			return this->g_trace.tw->trace.fraction;
		}
	}

	if (d1 <= -sphere.radius && d2 <= -sphere.radius) {
		return this->g_trace.tw->trace.fraction;
	}

	if (d1 <= d2) {
		return this->g_trace.tw->trace.fraction;
	}

	fMaxFraction = SURFACE_CLIP_EPSILON / (d1 - d2);
	this->g_trace.fSurfaceClipEpsilon = fMaxFraction;
	fSpherePlane = (d1 - sphere.radius) / (d1 - d2) - fMaxFraction;

	if (fSpherePlane < 0) {
		fSpherePlane = 0;
	}

	if (fSpherePlane >= this->g_trace.tw->trace.fraction) {
		return this->g_trace.tw->trace.fraction;
	}

	d1 = (this->g_trace.vEnd[0] - this->g_trace.vStart[0]) * fSpherePlane + this->g_trace.vEnd[0] - sphere.radius * plane[0] - x0;
	d2 = (this->g_trace.vEnd[1] - this->g_trace.vStart[1]) * fSpherePlane + this->g_trace.vEnd[1] - sphere.radius * plane[1] - y0;

	eMode = this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].eMode;

	if (eMode == 1 || eMode == 2)
	{
		if ((this->g_trace.i + this->g_trace.j) & 1) {
			eMode = iPlane ? 6 : 3;
		}
		else {
			eMode = iPlane ? 5 : 4;
		}
	}

	switch (eMode)
	{
	case 3:
		if (d1 > 64) {
			bFitsX = false;
		}
		else {
			bFitsX = true;
		}
		if (d2 > 64) {
			bFitsY = false;
		}
		else {
			bFitsY = true;
		}

		if (d1 < 64 - d2) {
			bFitsDiag = false;
		}
		else {
			bFitsDiag = true;
		}
		iX[0] = 1;
		iX[1] = 1;
		iX[2] = 1;
		iY[0] = 0;
		iY[1] = 1;
		iY[2] = 0;
		break;
	case 4:
		if (d1 < 0) {
			bFitsX = false;
		}
		else {
			bFitsX = true;
		}
		if (d2 > 64) {
			bFitsY = false;
		}
		else {
			bFitsY = true;
		}

		if (d1 > d2) {
			bFitsDiag = false;
		}
		else {
			bFitsDiag = true;
		}
		iX[0] = 0;
		iX[1] = 1;
		iX[2] = 1;
		iY[0] = 1;
		iY[1] = 0;
		iY[2] = 0;
		break;
	case 5:
		if (d1 > 64) {
			bFitsX = false;
		}
		else {
			bFitsX = true;
		}
		if (d2 < 0) {
			bFitsY = false;
		}
		else {
			bFitsY = true;
		}

		if (d1 < d2) {
			bFitsDiag = false;
		}
		else {
			bFitsDiag = true;
		}
		iX[0] = 1;
		iX[1] = 0;
		iX[2] = 0;
		iY[0] = 0;
		iY[1] = 1;
		iY[2] = 1;
		break;
	case 6:
		if (d1 < 0) {
			bFitsX = false;
		}
		else {
			bFitsX = true;
		}
		if (d2 < 0) {
			bFitsY = false;
		}
		else {
			bFitsY = true;
		}

		if (d1 > 64 - d2) {
			bFitsDiag = false;
		}
		else {
			bFitsDiag = true;
		}
		iX[0] = 0;
		iX[1] = 0;
		iX[2] = 0;
		iY[0] = 1;
		iY[1] = 0;
		iY[2] = 1;
		break;
	default:
		return 0;
	}

	if (bFitsX)
	{
		if (bFitsY)
		{
			if (bFitsDiag) {
				return fSpherePlane;
			}
			else {
				return CollisionWorld::CM_CheckTerrainTriSphereEdge(plane, x0, y0, iY[0], iX[1], iY[1], iY[2]);
			}
		}
		else if (bFitsDiag) {
			return CollisionWorld::CM_CheckTerrainTriSphereEdge(plane, x0, y0, iX[0], iX[2], iY[0], iX[1]);
		}
		else {
			return CollisionWorld::CM_CheckTerrainTriSphereCorner(plane, x0, y0, iY[0], iX[1]);
		}
	}
	else if (bFitsY)
	{
		if (bFitsDiag) {
			return CollisionWorld::CM_CheckTerrainTriSphereEdge(plane, x0, y0, iX[0], iX[2], iY[1], iY[2]);
		}
		else {
			return CollisionWorld::CM_CheckTerrainTriSphereCorner(plane, x0, y0, iY[1], iY[2]);
		}
	}
	else
	{
		if (bFitsDiag) {
			return CollisionWorld::CM_CheckTerrainTriSphereCorner(plane, x0, y0, iX[0], iX[2]);
		}
		else {
			return this->g_trace.tw->trace.fraction;
		}
	}
}

/*
====================
CollisionWorld::CM_ValidateTerrainCollidePointSquare
====================
*/
bool CollisionWorld::CM_ValidateTerrainCollidePointSquare(float frac)
{
	float f;

	f = this->g_trace.vStart[0] + frac * (this->g_trace.vEnd[0] - this->g_trace.vStart[0])
		- ((this->g_trace.i << 6) + this->g_trace.tc->vBounds[0][0]);

	if (f >= 0 && f <= 64)
	{
		f = this->g_trace.vStart[1] + frac * (this->g_trace.vEnd[1] - this->g_trace.vStart[1])
			- ((this->g_trace.j << 6) + this->g_trace.tc->vBounds[0][1]);

		if (f >= 0 && f <= 64) {
			return true;
		}
	}

	return false;
}

/*
====================
CollisionWorld::CM_ValidateTerrainCollidePointTri
====================
*/
bool CollisionWorld::CM_ValidateTerrainCollidePointTri(int eMode, float frac)
{
	float	x0, y0;
	float	x, y;
	float	dx, dy;

	x0 = (this->g_trace.i << 6) + this->g_trace.tc->vBounds[0][0];
	dx = this->g_trace.vStart[0] + (this->g_trace.vEnd[0] - this->g_trace.vStart[0]) * frac;
	x = x0 + 64;

	if (x0 > dx) {
		return false;
	}

	if (x < dx) {
		return false;
	}

	y0 = (this->g_trace.j << 6) + this->g_trace.tc->vBounds[0][1];
	dy = this->g_trace.vStart[1] + (this->g_trace.vEnd[1] - this->g_trace.vStart[1]) * frac;
	y = y0 + 64;

	if (y0 > dy) {
		return false;
	}

	if (y < dy) {
		return false;
	}

	switch (eMode)
	{
	case 3:
		return (dx - x0) >= (64 - (dy - y0));
	case 4:
		return (dx - x0) <= (dy - y0);
	case 5:
		return (dx - x0) >= (dy - y0);
	case 6:
		return (dx - x0) <= (64 - (dy - y0));
	default:
		return true;
	}
}

/*
====================
CollisionWorld::CM_TestTerrainCollideSquare
====================
*/
bool CollisionWorld::CM_TestTerrainCollideSquare(void)
{
	float* plane;
	float	frac0;
	float	enterFrac;
	int		eMode;

	eMode = this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].eMode;

	if (!eMode) {
		return false;
	}

	if (eMode >= 0 && eMode <= 2)
	{
		enterFrac = CollisionWorld::CM_CheckTerrainPlane(this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].plane[0]);

		plane = this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].plane[1];
		frac0 = CollisionWorld::CM_CheckTerrainPlane(plane);

		if (eMode == 2)
		{
			if (enterFrac > frac0) {
				enterFrac = frac0;
			}
		}
		else
		{
			if (enterFrac < frac0) {
				enterFrac = frac0;
			}
		}

		if (enterFrac < this->g_trace.tw->trace.fraction && CollisionWorld::CM_ValidateTerrainCollidePointSquare(enterFrac))
		{
			this->g_trace.tw->trace.fraction = enterFrac;
			VecCopy(plane, this->g_trace.tw->trace.plane.normal);
			this->g_trace.tw->trace.plane.dist = plane[3];
			return true;
		}
	}
	else
	{
		plane = this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].plane[0];
		enterFrac = CollisionWorld::CM_CheckTerrainPlane(plane);

		if (enterFrac < this->g_trace.tw->trace.fraction
			&& CollisionWorld::CM_ValidateTerrainCollidePointTri(this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].eMode, enterFrac))
		{
			this->g_trace.tw->trace.fraction = enterFrac;
			VecCopy(plane, this->g_trace.tw->trace.plane.normal);
			this->g_trace.tw->trace.plane.dist = plane[3];
			return true;
		}
	}

	return false;
}

/*
====================
CollisionWorld::CM_CheckStartInsideTerrain
====================
*/
bool CollisionWorld::CM_CheckStartInsideTerrain(int i, int j, float fx, float fy)
{
	float* plane;
	float	fDot;

	if (i < 0 || i > 7) {
		return false;
	}
	if (j < 0 || j > 7) {
		return false;
	}

	if (!this->g_trace.tc->squares[i][j].eMode) {
		return false;
	}

	if ((i + j) & 1)
	{
		if (fx + fy >= 1)
		{
			if (this->g_trace.tc->squares[i][j].eMode == 6) {
				return false;
			}
			plane = this->g_trace.tc->squares[i][j].plane[0];
		}
		else
		{
			if (this->g_trace.tc->squares[i][j].eMode == 3) {
				return false;
			}
			plane = this->g_trace.tc->squares[i][j].plane[1];
		}
	}
	else
	{
		if (fy >= fx)
		{
			if (this->g_trace.tc->squares[i][j].eMode == 5) {
				return false;
			}
			plane = this->g_trace.tc->squares[i][j].plane[0];
		}
		else
		{
			if (this->g_trace.tc->squares[i][j].eMode == 4) {
				return false;
			}
			plane = this->g_trace.tc->squares[i][j].plane[1];
		}
	}

	fDot = DotProduct(this->g_trace.vStart, plane);
	if (fDot <= plane[3] && fDot + 32.0f >= plane[3]) {
		return true;
	}

	return false;
}

/*
====================
CollisionWorld::CM_PositionTestPointInTerrainCollide
====================
*/
bool CollisionWorld::CM_PositionTestPointInTerrainCollide(void)
{
	int		i0, j0;
	float	fx, fy;

	fx = (this->g_trace.vStart[0] - this->g_trace.tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8);
	fy = (this->g_trace.vStart[1] - this->g_trace.tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8);

	i0 = (int)floor(fx);
	j0 = (int)floor(fy);

	return CollisionWorld::CM_CheckStartInsideTerrain(i0, j0, fx - i0, fy - j0);
}

/*
====================
CollisionWorld::CM_TracePointThroughTerrainCollide
====================
*/
void CollisionWorld::CM_TracePointThroughTerrainCollide(void)
{
	int i0, j0, i1, j1;
	int di, dj;
	int d1, d2;
	//int nTotal;
	float fx, fy;
	float dx, dy, dx2, dy2;

	fx = (this->g_trace.vStart[0] - this->g_trace.tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8);
	fy = (this->g_trace.vStart[1] - this->g_trace.tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8);
	i0 = (int)floor(fx);
	j0 = (int)floor(fy);
	i1 = (int)floor((this->g_trace.vEnd[0] - this->g_trace.tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8));
	j1 = (int)floor((this->g_trace.vEnd[1] - this->g_trace.tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8));

	if (CollisionWorld::CM_CheckStartInsideTerrain(i0, j0, fx - i0, fy - j0))
	{
		this->g_trace.tw->trace.startsolid = true;
		this->g_trace.tw->trace.allsolid = true;
		this->g_trace.tw->trace.fraction = 0;
		return;
	}

	if (i0 == i1)
	{
		if (i0 < 0 || i0 > 7) {
			return;
		}

		if (j0 == j1)
		{
			if (j0 < 0 || j0 > 7) {
				return;
			}

			this->g_trace.i = i0;
			this->g_trace.j = j0;
			CollisionWorld::CM_TestTerrainCollideSquare();
		}
		else if (j0 >= j1)
		{
			if (j0 > 7)
				j0 = 7;
			if (j1 < 0)
				j1 = 0;

			this->g_trace.i = i0;
			for (this->g_trace.j = j0; this->g_trace.j >= j1; this->g_trace.j--) {
				if (CollisionWorld::CM_TestTerrainCollideSquare()) {
					return;
				}
			}
		}
		else
		{
			if (j0 < 0)
				j0 = 0;
			if (j1 > 7)
				j1 = 7;

			this->g_trace.i = i0;
			for (this->g_trace.j = j0; this->g_trace.j <= j1; this->g_trace.j++) {
				if (CollisionWorld::CM_TestTerrainCollideSquare()) {
					return;
				}
			}
		}
	}
	else if (j0 == j1)
	{
		if (j0 < 0 || j0 > 7) {
			return;
		}

		if (i0 >= i1)
		{
			if (i0 > 7)
				i0 = 7;
			if (i1 < 0)
				i1 = 0;

			this->g_trace.j = j0;
			for (this->g_trace.i = i0; this->g_trace.i >= i1; this->g_trace.i--) {
				if (CollisionWorld::CM_TestTerrainCollideSquare()) {
					break;
				}
			}
		}
		else
		{
			if (i0 < 0)
				i0 = 0;
			if (i1 > 7)
				i1 = 7;

			this->g_trace.j = j0;
			for (this->g_trace.i = i0; this->g_trace.i <= i1; this->g_trace.i++) {
				if (CollisionWorld::CM_TestTerrainCollideSquare()) {
					break;
				}
			}
		}
	}
	else
	{
		dx = this->g_trace.vEnd[0] - this->g_trace.vStart[0];
		dy = this->g_trace.vEnd[1] - this->g_trace.vStart[1];

		if (dx > 0)
		{
			d1 = 1;
			di = i1 - i0;
			dx2 = (i0 + 1 - fx) * dy;
		}
		else
		{
			d1 = -1;
			di = i0 - i1;
			dx = -dx;
			dx2 = (fx - i0) * dy;
		}

		if (dy > 0)
		{
			d2 = 1;
			dj = di + j1 - j0 + 1;
			dy2 = (j0 + 1 - fy) * dx;
		}
		else
		{
			d2 = -1;
			dy = -dy;
			dj = di + j0 - j1 + 1;
			dy2 = (fy - j0) * dx;
			dx2 = -dx2;
		}

		this->g_trace.i = i0;
		this->g_trace.j = j0;

		while (1)
		{
			if (this->g_trace.i >= 0 && this->g_trace.i <= 7 && this->g_trace.j >= 0 && this->g_trace.j <= 7)
			{
				if (CollisionWorld::CM_TestTerrainCollideSquare()) {
					return;
				}
			}

			dj--;
			if (!dj) {
				break;
			}

			if (dx2 < dy2)
			{
				dy2 -= dx2;
				dx2 = dy;
				this->g_trace.i += d1;
			}
			else
			{
				dx2 -= dy2;
				dy2 = dx;
				this->g_trace.j += d2;
			}
		}
	}
}

/*
====================
CollisionWorld::CM_TraceCylinderThroughTerrainCollide
====================
*/
void CollisionWorld::CM_TraceCylinderThroughTerrainCollide(traceWork_t* tw, const terrainCollide_t* tc)
{
	int i0, j0, i1, j1;
	float x0, y0;
	float enterFrac;

	i0 = (int)(floor(tw->bounds[0][0] - tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8));
	i1 = (int)(floor(tw->bounds[1][0] - tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8));
	j0 = (int)(floor(tw->bounds[0][1] - tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8));
	j1 = (int)(floor(tw->bounds[1][1] - tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8));

	if (i0 < 0)
		i0 = 0;
	if (j0 < 0)
		j0 = 0;
	if (i1 > 7)
		i1 = 7;
	if (j1 > 7)
		j1 = 7;

	y0 = (j0 << 6) + tc->vBounds[0][1];
	for (this->g_trace.j = j0; this->g_trace.j <= j1; this->g_trace.j++)
	{
		x0 = (i0 << 6) + tc->vBounds[0][0];
		for (this->g_trace.i = i0; this->g_trace.i <= i1; this->g_trace.i++)
		{
			switch (tc->squares[this->g_trace.i][this->g_trace.j].eMode)
			{
			case 1:
			case 2:
				enterFrac = CollisionWorld::CM_CheckTerrainTriSphere(x0, y0, 0);
				if (enterFrac < 0)
					enterFrac = 0;
				if (enterFrac < this->g_trace.tw->trace.fraction)
				{
					this->g_trace.tw->trace.fraction = enterFrac;
					VecCopy(this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].plane[0], this->g_trace.tw->trace.plane.normal);
					this->g_trace.tw->trace.plane.dist = this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].plane[0][3];
				}
				enterFrac = CollisionWorld::CM_CheckTerrainTriSphere(x0, y0, 1);
				if (enterFrac < 0)
					enterFrac = 0;
				if (enterFrac < this->g_trace.tw->trace.fraction)
				{
					this->g_trace.tw->trace.fraction = enterFrac;
					VecCopy(this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].plane[1], this->g_trace.tw->trace.plane.normal);
					this->g_trace.tw->trace.plane.dist = this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].plane[1][3];
				}
				break;
			case 3:
			case 4:
				enterFrac = CollisionWorld::CM_CheckTerrainTriSphere(x0, y0, 0);
				if (enterFrac < 0)
					enterFrac = 0;
				if (enterFrac < this->g_trace.tw->trace.fraction)
				{
					this->g_trace.tw->trace.fraction = enterFrac;
					VecCopy(this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].plane[0], this->g_trace.tw->trace.plane.normal);
					this->g_trace.tw->trace.plane.dist = this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].plane[0][3];
				}
				break;
			case 5:
			case 6:
				enterFrac = CollisionWorld::CM_CheckTerrainTriSphere(x0, y0, 1);
				if (enterFrac < 0)
					enterFrac = 0;
				if (enterFrac < this->g_trace.tw->trace.fraction)
				{
					this->g_trace.tw->trace.fraction = enterFrac;
					VecCopy(this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].plane[1], this->g_trace.tw->trace.plane.normal);
					this->g_trace.tw->trace.plane.dist = this->g_trace.tc->squares[this->g_trace.i][this->g_trace.j].plane[1][3];
				}
				break;
			default:
				break;
			}
			x0 += 64;
		}

		y0 += 64;
	}
}

/*
====================
CollisionWorld::CM_TraceThroughTerrainCollide
====================
*/
void CollisionWorld::CM_TraceThroughTerrainCollide(traceWork_t* tw, terrainCollide_t* tc) {
	int i;

	if (tw->bounds[0][0] >= tc->vBounds[1][0] ||
		tw->bounds[0][1] >= tc->vBounds[1][1] ||
		tw->bounds[0][2] >= tc->vBounds[1][2] ||
		tw->bounds[1][0] <= tc->vBounds[0][0] ||
		tw->bounds[1][1] <= tc->vBounds[0][1] ||
		tw->bounds[1][2] <= tc->vBounds[0][2])
	{
		return;
	}

	this->g_trace.tw = tw;
	this->g_trace.tc = tc;
	VecCopy(tw->start, this->g_trace.vStart);
	VecCopy(tw->end, this->g_trace.vEnd);

	if (sphere.use && ter_usesphere)
	{
		VecSubtract(tw->start, sphere.offset, this->g_trace.vStart);
		VecSubtract(tw->end, sphere.offset, this->g_trace.vEnd);
		CollisionWorld::CM_TraceCylinderThroughTerrainCollide(tw, tc);
	}
	else if (tw->isPoint)
	{
		VecCopy(tw->start, this->g_trace.vStart);
		VecCopy(tw->end, this->g_trace.vEnd);
		CollisionWorld::CM_TracePointThroughTerrainCollide();
	}
	else
	{
		if (tc->squares[0][0].plane[0][2] >= 0)
		{
			for (i = 0; i < 4; i++)
			{
				VecAdd(tw->start, tw->offsets[i], this->g_trace.vStart);
				VecAdd(tw->end, tw->offsets[i], this->g_trace.vEnd);

				CollisionWorld::CM_TracePointThroughTerrainCollide();
				if (tw->trace.allsolid) {
					return;
				}
			}
		}
		else
		{
			for (i = 4; i < 8; i++)
			{
				VecAdd(tw->start, tw->offsets[i], this->g_trace.vStart);
				VecAdd(tw->end, tw->offsets[i], this->g_trace.vEnd);

				CollisionWorld::CM_TracePointThroughTerrainCollide();
				if (tw->trace.allsolid) {
					return;
				}
			}
		}
	}
}

/*
====================
CollisionWorld::CM_PositionTestInTerrainCollide
====================
*/
bool CollisionWorld::CM_PositionTestInTerrainCollide(traceWork_t* tw, terrainCollide_t* tc) {
	int i;

	if (tw->bounds[0][0] >= tc->vBounds[1][0] ||
		tw->bounds[0][1] >= tc->vBounds[1][1] ||
		tw->bounds[0][2] >= tc->vBounds[1][2] ||
		tw->bounds[1][0] <= tc->vBounds[0][0] ||
		tw->bounds[1][1] <= tc->vBounds[0][1] ||
		tw->bounds[1][2] <= tc->vBounds[0][2])
	{
		return false;
	}

	this->g_trace.tw = tw;
	this->g_trace.tc = tc;
	VecCopy(tw->start, this->g_trace.vStart);
	VecCopy(tw->end, this->g_trace.vEnd);

	if (sphere.use && ter_usesphere)
	{
		VecSubtract(tw->start, sphere.offset, this->g_trace.vStart);
		VecSubtract(tw->end, sphere.offset, this->g_trace.vEnd);
		CollisionWorld::CM_TraceCylinderThroughTerrainCollide(tw, tc);
		return tw->trace.startsolid;
	}
	else if (tw->isPoint)
	{
		VecCopy(tw->start, this->g_trace.vStart);
		VecCopy(tw->end, this->g_trace.vEnd);
		return CollisionWorld::CM_PositionTestPointInTerrainCollide();
	}
	else
	{
		if (tc->squares[0][0].plane[0][2] >= 0)
		{
			for (i = 0; i < 4; i++)
			{
				VecAdd(tw->start, tw->offsets[i], this->g_trace.vStart);
				VecAdd(tw->end, tw->offsets[i], this->g_trace.vEnd);

				if (CollisionWorld::CM_PositionTestPointInTerrainCollide()) {
					return true;
				}
			}
		}
		else
		{
			for (i = 4; i < 8; i++)
			{
				VecAdd(tw->start, tw->offsets[i], this->g_trace.vStart);
				VecAdd(tw->end, tw->offsets[i], this->g_trace.vEnd);

				if (CollisionWorld::CM_PositionTestPointInTerrainCollide()) {
					return true;
				}
			}
		}
	}

	return false;
}

/*
====================
CollisionWorld::CM_SightTracePointThroughTerrainCollide
====================
*/
bool CollisionWorld::CM_SightTracePointThroughTerrainCollide(void)
{
	int		i0, j0;
	int		i1, j1;
	int		di, dj;
	float	fx, fy;
	float	dx, dy, dx2, dy2;
	float	d1, d2;

	fx = (this->g_trace.vStart[0] - this->g_trace.tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8);
	fy = (this->g_trace.vStart[1] - this->g_trace.tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8);
	i0 = (int)floor(fx);
	j0 = (int)floor(fy);
	i1 = (int)floor((this->g_trace.vEnd[0] - this->g_trace.tc->vBounds[0][0]) * (SURFACE_CLIP_EPSILON / 8));
	j1 = (int)floor((this->g_trace.vEnd[1] - this->g_trace.tc->vBounds[0][1]) * (SURFACE_CLIP_EPSILON / 8));

	if (CollisionWorld::CM_CheckStartInsideTerrain(i0, j0, fx - i0, fy - j0)) {
		return false;
	}

	if (i0 == i1)
	{
		if (i0 < 0 || i0 > 7) {
			return true;
		}

		if (j0 == j1)
		{
			if (j0 < 0 || j0 > 7) {
				return true;
			}

			this->g_trace.i = i0;
			this->g_trace.j = j0;
			return !CollisionWorld::CM_TestTerrainCollideSquare();
		}
		else if (j0 >= j1)
		{
			if (j0 > 7)
				j0 = 7;
			if (j1 < 0)
				j1 = 0;

			this->g_trace.i = i0;
			for (this->g_trace.j = j0; this->g_trace.j >= j1; this->g_trace.j--) {
				if (CollisionWorld::CM_TestTerrainCollideSquare()) {
					return false;
				}
			}
		}
		else
		{
			if (j0 < 0)
				j0 = 0;
			if (j1 > 7)
				j1 = 7;

			this->g_trace.i = i0;
			for (this->g_trace.j = j0; this->g_trace.j <= j1; this->g_trace.j++) {
				if (CollisionWorld::CM_TestTerrainCollideSquare()) {
					return false;
				}
			}
		}
	}
	else if (j0 == j1)
	{
		if (j0 < 0 || j0 > 7) {
			return true;
		}

		if (i0 >= i1)
		{
			if (i0 > 7)
				i0 = 7;
			if (i1 < 0)
				i1 = 0;

			this->g_trace.j = j0;
			for (this->g_trace.i = i0; this->g_trace.i >= i1; this->g_trace.i--) {
				if (CollisionWorld::CM_TestTerrainCollideSquare()) {
					return false;
				}
			}
		}
		else
		{
			if (i0 < 0)
				i0 = 0;
			if (i1 > 7)
				i1 = 7;

			this->g_trace.j = j0;
			for (this->g_trace.i = i0; this->g_trace.i <= i1; this->g_trace.i++) {
				if (CollisionWorld::CM_TestTerrainCollideSquare()) {
					return false;
				}
			}
		}
	}
	else
	{
		dx = this->g_trace.vEnd[0] - this->g_trace.vStart[0];
		dy = this->g_trace.vEnd[1] - this->g_trace.vStart[1];

		if (dx > 0)
		{
			d1 = 1;
			di = i1 - i0;
			dx2 = (i0 + 1 - fx) * dy;
		}
		else
		{
			d1 = -1;
			di = i0 - i1;
			dx = -dx;
			dx2 = (fx - i0) * dy;
		}

		if (dy > 0)
		{
			d2 = 1;
			dj = di + j1 - j0 + 1;
			dy2 = (j0 + 1 - fy) * dx;
		}
		else
		{
			d2 = -1;
			dy = -dy;
			dj = di + j0 - j1 + 1;
			dy2 = (fy - j0) * dx;
			dx2 = -dx2;
		}

		this->g_trace.i = i0;
		this->g_trace.j = j0;

		while (1)
		{
			if (this->g_trace.i >= 0 && this->g_trace.i <= 7 && this->g_trace.j >= 0 && this->g_trace.j <= 7)
			{
				if (CollisionWorld::CM_TestTerrainCollideSquare()) {
					return false;
				}
			}

			dj--;
			if (!dj) {
				break;
			}

			if (dx2 < dy2)
			{
				dy2 -= dx2;
				dx2 = dy;
				this->g_trace.i += (int)d1;
			}
			else
			{
				dx2 -= dy2;
				dy2 = dx;
				this->g_trace.j += (int)d2;
			}
		}
	}

	return true;
}

/*
====================
CollisionWorld::CM_SightTraceThroughTerrainCollide
====================
*/
bool CollisionWorld::CM_SightTraceThroughTerrainCollide(traceWork_t* tw, terrainCollide_t* tc)
{
	int i;

	if (tw->bounds[0][0] >= tc->vBounds[1][0] ||
		tw->bounds[0][1] >= tc->vBounds[1][1] ||
		tw->bounds[0][2] >= tc->vBounds[1][2] ||
		tw->bounds[1][0] <= tc->vBounds[0][0] ||
		tw->bounds[1][1] <= tc->vBounds[0][1] ||
		tw->bounds[1][2] <= tc->vBounds[0][2])
	{
		return true;
	}

	this->g_trace.tw = tw;
	this->g_trace.tc = tc;
	VecCopy(tw->start, this->g_trace.vStart);
	VecCopy(tw->end, this->g_trace.vEnd);

	if (tw->isPoint)
	{
		VecCopy(tw->start, this->g_trace.vStart);
		VecCopy(tw->end, this->g_trace.vEnd);
		return CollisionWorld::CM_SightTracePointThroughTerrainCollide();
	}
	else
	{
		if (tc->squares[0][0].plane[0][2] >= 0)
		{
			for (i = 0; i < 4; i++)
			{
				VecAdd(tw->start, tw->offsets[i], this->g_trace.vStart);
				VecAdd(tw->end, tw->offsets[i], this->g_trace.vEnd);

				if (!CollisionWorld::CM_SightTracePointThroughTerrainCollide()) {
					return false;
				}
			}
		}
		else
		{
			for (i = 4; i < 8; i++)
			{
				VecAdd(tw->start, tw->offsets[i], this->g_trace.vStart);
				VecAdd(tw->end, tw->offsets[i], this->g_trace.vEnd);

				if (!CollisionWorld::CM_SightTracePointThroughTerrainCollide()) {
					return false;
				}
			}
		}
	}

	return true;
}
