#pragma once

#include "Global.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <assert.h>
#include "Math.h"

#define VECTOR_FABS ::fabsf

#ifdef _MSC_VER
#define sscanf sscanf_s
#endif

namespace MOHPC
{
	class MOHPC_EXPORTS Vector
	{
	public:
		float		x;
		float		y;
		float		z;

		Vector();
		//Vector(const float x, const float y, const float z);
		constexpr Vector(const float x, const float y, const float z);
		Vector(const float xyz[3]);
		explicit	Vector(const char *text);

		operator float * ();
		operator float const * () const;

		float					pitch(void) const;
		float					yaw(void) const;
		float					roll(void) const;
		float					operator[](const int index) const;
		float &					operator[](const int index);
		void					setPitch(const float x);
		void 					setYaw(const float y);
		void 					setRoll(const float z);
		void 					setXYZ(const float x, const float y, const float z);
		const Vector &			operator=(const Vector &a);
		const Vector &			operator=(const char *a);
		friend Vector			operator+(const Vector &a, const Vector &b);
		const Vector &			operator+=(const Vector &a);
		friend Vector			operator-(const Vector &a, const Vector &b);
		const Vector &			operator-=(const Vector &a);
		friend Vector			operator*(const Vector &a, const float b);
		friend Vector			operator*(const float a, const Vector &b);
		friend float			operator*(const Vector &a, const Vector &b);
		const Vector &			operator*=(const float a);
		friend Vector			operator/(const Vector &a, const float b);
		friend Vector			operator/(const float a, const Vector &b);
		friend float			operator/(const Vector &a, const Vector &b);
		const Vector &			operator/=(const float a);
		friend bool				operator==(const Vector &a, const Vector &b);
		friend bool				operator!=(const Vector &a, const Vector &b);
		int						FuzzyEqual(const Vector &b, const float epsilon) const;
		const Vector &			CrossProduct(const Vector &a, const Vector &b);
		float					length(void) const;
		float					lengthfast(void) const;
		float					lengthSquared(void) const;
		float					lengthXY(void) const;
		float					normalize(void);
		void					normalizefast(void);
		void					EulerNormalize(void);
		void					EulerNormalize360(void);
		static Vector			Clamp(Vector &value, const Vector &min, const Vector &max);
		static Vector			Cross(const Vector &vector1, const Vector &vector2);
		static float			Dot(const Vector &vector1, const Vector &vector2);
		static float			Distance(const Vector &vector1, const Vector &vector2);
		static float			DistanceSquared(const Vector &vector1, const Vector &vector2);
		static float			DistanceXY(const Vector &vector1, const Vector &vector2);
		static Vector			AnglesBetween(const Vector &vector1, const Vector &vector2);
		static float			AngleBetween(const Vector &vector1, const Vector &vector2);
		static bool				CloseEnough(const Vector &vector1, const Vector &vector2, const float epsilon = Vector::Epsilon());
		static bool				SmallEnough(const Vector &vector, const float epsilon = Vector::Epsilon());
		static float			Epsilon(void);
		static Vector &			Identity(void);
		Vector					operator-(void) const;
		static Vector			fabs(const Vector &a);
		float					toYaw(void) const;
		float					toPitch(void) const;
		Vector					toAngles(void) const;
		Vector					AnglesMod(void) const;
		void					AngleVectors(Vector *forward, Vector *right = NULL, Vector *up = NULL)  const;
		void					AngleVectorsLeft(Vector *forward, Vector *right = NULL, Vector *up = NULL)  const;
		static Vector			LerpVector(const Vector& w1, const Vector& w2, const float t);
		static float			MaxValue(const Vector& a);
		Vector					GetRotatedX(float angle) const;
		void					RotateX(float angle);
		Vector					GetRotatedY(float angle) const;
		void					RotateY(float angle);
		Vector					GetRotatedZ(float angle) const;
		void					RotateZ(float angle);
		void					PackTo01();
		Vector					GetPackedTo01() const;
		bool					IsEmpty() const;
	};

	static Vector vec_origin = Vector(0, 0, 0);
	static Vector vec_zero = Vector(0, 0, 0);
	static Vector g_vEyeDir = Vector(0, 0, 0);

	inline float Vector::pitch(void) const { return x; }
	inline float Vector::yaw(void) const { return y; }
	inline float Vector::roll(void) const { return z; }
	inline void  Vector::setPitch(float pitch) { x = pitch; }
	inline void  Vector::setYaw(float yaw) { y = yaw; }
	inline void  Vector::setRoll(float roll) { z = roll; }

	inline float Vector::operator[](const int index) const
	{
		assert((index >= 0) && (index < 3));
		return (&x)[index];
	}

	inline float& Vector::operator[](const int index)
	{
		assert((index >= 0) && (index < 3));
		return (&x)[index];
	}

	inline void Vector::setXYZ(const float new_x, const float new_y, const float new_z)
	{
		x = new_x;
		y = new_y;
		z = new_z;
	}

	inline Vector::Vector() : x(0), y(0), z(0)
	{
	}

	constexpr Vector::Vector(const float init_x, const float init_y, const float init_z) : x(init_x), y(init_y), z(init_z)
	{
	}

	inline Vector::Vector(const float xyz[3]) : x(xyz[0]), y(xyz[1]), z(xyz[2])
	{
	}

	inline Vector::Vector(const char *text) : x(0), y(0), z(0)

	{
		if (text)
		{
			if (text[0] == '"')
				sscanf(text, "\"%f %f %f\"", &x, &y, &z);
			else
				sscanf(text, "%f %f %f", &x, &y, &z);
		}
	}

	inline Vector::operator float * (void)
	{
		return &x;
	}

	inline Vector::operator float const * (void) const
	{
		return &x;
	}

	inline const Vector & Vector::operator=(const Vector &a)
	{
		x = a.x;
		y = a.y;
		z = a.z;

		return *this;
	}

	inline const Vector & Vector::operator=(const char *a)
	{
		if (a)
		{
			if (a[0] == '"')
				sscanf(a, "\"%f %f %f\"", &x, &y, &z);
			else
				sscanf(a, "%f %f %f", &x, &y, &z);
		}

		return *this;
	}

	inline Vector operator+(const Vector &a, const Vector &b)
	{
		return Vector(a.x + b.x, a.y + b.y, a.z + b.z);
	}

	inline const Vector & Vector::operator+=(const Vector &a)
	{
		x += a.x;
		y += a.y;
		z += a.z;

		return *this;
	}

	inline Vector operator-(const Vector &a, const Vector &b)
	{
		return Vector(a.x - b.x, a.y - b.y, a.z - b.z);
	}

	inline const Vector & Vector::operator-=(const Vector &a)
	{
		x -= a.x;
		y -= a.y;
		z -= a.z;

		return *this;
	}

	inline Vector operator*(const Vector &a, const float b)
	{
		return Vector(a.x * b, a.y * b, a.z * b);
	}

	inline Vector operator*(const float a, const Vector &b)
	{
		return b * a;
	}

	inline float operator*(const Vector &a, const Vector &b)
	{
		return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
	}

	inline const Vector& Vector::operator*=(const float a)
	{
		x *= a;
		y *= a;
		z *= a;

		return *this;
	}

	inline Vector operator/(const Vector &a, const float b)
	{
		return Vector(a.x / b, a.y / b, a.z / b);
	}

	inline Vector operator/(const float a, const Vector &b)
	{
		return b / a;
	}

	inline float operator/(const Vector &a, const Vector &b)
	{
		return (a.x / b.x) + (a.y / b.y) + (a.z / b.z);
	}

	inline const Vector &	Vector::operator/=(const float a)
	{
		*this = *this / a;
		return *this;
	}

	inline int Vector::FuzzyEqual(const Vector &b, const float epsilon) const
	{
		return
			(
			(VECTOR_FABS(x - b.x) < epsilon) &&
				(VECTOR_FABS(y - b.y) < epsilon) &&
				(VECTOR_FABS(z - b.z) < epsilon)
				);
	}


	inline bool operator==(const Vector &a, const Vector &b)

	{
		return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
	}

	inline bool operator!=(const Vector &a, const Vector &b)
	{
		return ((a.x != b.x) || (a.y != b.y) || (a.z != b.z));
	}

	inline const Vector & Vector::CrossProduct(const Vector &a, const Vector &b)
	{
		x = (a.y * b.z) - (a.z * b.y);
		y = (a.z * b.x) - (a.x * b.z);
		z = (a.x * b.y) - (a.y * b.x);

		return *this;
	}

	inline Vector Vector::Clamp(Vector &value, const Vector &minimum, const Vector &maximum)
	{
		Vector clamped(value);
		for (int i = 0; i < 3; i++)
		{
			const float min = minimum[i];
			const float max = maximum[i];
			assert(min <= max);

			if (clamped[i] < min)
			{
				clamped[i] = min;
			}
			else if (clamped[i] > max)
			{
				clamped[i] = max;
			}
		}
		return clamped;
	}
	inline Vector Vector::Cross(const Vector &vector1, const Vector &vector2)
	{
		const Vector result(
			(vector1.y * vector2.z) - (vector1.z * vector2.y),
			(vector1.z * vector2.x) - (vector1.x * vector2.z),
			(vector1.x * vector2.y) - (vector1.y * vector2.x)
		);

		return result;
	}


	inline float Vector::Dot(const Vector &vector1, const Vector &vector2)
	{
		return vector1 * vector2;
	}

	//----------------------------------------------------------------
	// Name:				lengthSquared
	// Class:			Vector
	//
	// Description:	Returns squared length of the vector
	//
	// Parameters:		None
	//
	// Returns:			float - squared length
	//----------------------------------------------------------------
	inline float Vector::lengthSquared(void) const
	{
		return (x * x) + (y * y) + (z * z);
	}

	inline float Vector::length(void) const
	{
		return sqrtf(lengthSquared());
	}

	inline float Vector::lengthfast(void) const
	{
		return vrsqrtf(lengthSquared());
	}

	//----------------------------------------------------------------
	// Name:				lengthXY
	// Class:			Vector
	//
	// Description:	Returns length of the vector (using only the x
	//						and y components
	//
	// Parameters:		None
	//
	// Returns:			float - length of the vector in the xy plane
	//----------------------------------------------------------------
	inline float Vector::lengthXY(void) const
	{
		return sqrtf((x * x) + (y * y));
	}

	//----------------------------------------------------------------
	// Name:				normalize
	// Class:			Vector
	//
	// Description:	unitizes the vector
	//
	// Parameters:		None
	//
	// Returns:			float - length of the vector before the function
	//----------------------------------------------------------------
	inline float Vector::normalize(void)
	{
		float	length, ilength;

		length = this->length();
		if (length)
		{
			ilength = 1.0f / length;
			x *= ilength;
			y *= ilength;
			z *= ilength;
		}

		return length;
	}

	//----------------------------------------------------------------
	// Name:				normalizefast
	// Class:			Vector
	//
	// Description:	fast version of normalize
	//
	// Parameters:		None
	//
	// Returns:			float - length of the vector before the function
	//----------------------------------------------------------------
	inline void Vector::normalizefast(void)
	{
		float ilength;

		ilength = this->lengthfast();

		x *= ilength;
		y *= ilength;
		z *= ilength;
	}

	//----------------------------------------------------------------
	// Name:				EulerNormalize
	// Class:			Vector
	//
	// Description:	forces each component of the vector into the
	//						range (-180, +180) by adding or subtracting 360
	//						This is useful when the Vector is being used as
	//						EulerAngles to represent a rotational offset
	//
	// Parameters:		None
	//
	// Returns:			None
	//----------------------------------------------------------------
	inline void Vector::EulerNormalize(void)
	{
		x = AngleNormalize180(x);
		y = AngleNormalize180(y);
		z = AngleNormalize180(z);
	}

	//----------------------------------------------------------------
	// Name:				EulerNormalize360
	// Class:			Vector
	//
	// Description:	forces each component of the vector into the
	//						range (0, +360) by adding or subtracting 360
	//						This is useful when the Vector is being used as
	//						EulerAngles to represent a rotational direction
	//
	// Parameters:		None
	//
	// Returns:			None
	//----------------------------------------------------------------
	inline void Vector::EulerNormalize360(void)
	{
		x = AngleNormalize360(x);
		y = AngleNormalize360(y);
		z = AngleNormalize360(z);
	}

	//----------------------------------------------------------------
	// Name:				Epsilon
	// Class:			Vector
	//
	// Description:	returns a standard 'small' value for the class
	//
	// Parameters:		None
	//
	// Returns:			float - the epsilon constant for the class
	//----------------------------------------------------------------
	inline float Vector::Epsilon(void)
	{
		return 0.000000001f;
	}

	//----------------------------------------------------------------
	// Name:				Identity
	// Class:			Vector
	//
	// Description:	returns the additive identity for the class
	//
	// Parameters:		None
	//
	// Returns:			Vector - the identity for the class
	//----------------------------------------------------------------
	inline Vector & Vector::Identity(void)
	{
		return vec_zero;
	}

	//----------------------------------------------------------------
	// Name:				Distance
	// Class:			Vector
	//
	// Description:	returns the distance between two vectors
	//
	// Parameters:
	//						Vector - first vector
	//						Vector - second vector
	//
	// Returns:			float - distance between the two vectors
	//----------------------------------------------------------------
	inline float Vector::Distance(const Vector &vector1, const Vector &vector2)
	{
		return (vector1 - vector2).length();
	}

	//----------------------------------------------------------------
	// Name:				DistanceSquared
	// Class:			Vector
	//
	// Description:	returns the squared distance between two vectors
	//
	// Parameters:
	//						Vector - first vector
	//						Vector - second vector
	//
	// Returns:			float - distance between the two vectors squared
	//----------------------------------------------------------------
	inline float Vector::DistanceSquared(const Vector &vector1, const Vector &vector2)
	{
		return (vector1 - vector2).lengthSquared();
	}

	//----------------------------------------------------------------
	// Name:				DistanceXY
	// Class:			Vector
	//
	// Description:	returns the distance between two vectors in the
	//						xy plane
	//
	// Parameters:
	//						Vector - first vector
	//						Vector - second vector
	//
	// Returns:			float - distance between the two vectors in the
	//						xy plane
	//----------------------------------------------------------------
	inline float Vector::DistanceXY(const Vector &vector1, const Vector &vector2)
	{
		return (vector1 - vector2).lengthXY();
	}

	inline Vector Vector::toAngles(void) const
	{
		float	forward;
		float	yaw, pitch;

		if ((x == 0.0) && (y == 0.0))
		{
			yaw = 0.0;
			if (z > 0.0)
			{
				pitch = 90.0;
			}
			else
			{
				pitch = 270.0;
			}
		}
		else
		{
			yaw = atan2f(y, x) * 180.0f / M_PI_FLOAT;
			if (yaw < 0.0)
			{
				yaw += 360.0f;
			}

			forward = (float)sqrtf(x * x + y * y);
			pitch = atan2f(z, forward) * 180.0f / M_PI_FLOAT;
			if (pitch < 0.0f)
			{
				pitch += 360.0f;
			}
		}

		return Vector(-pitch, yaw, 0.0);
	}

	//----------------------------------------------------------------
	// Name:				AnglesBetween
	// Class:			Vector
	//
	// Description:	returns the smaller of the angles formed by the
	//						two vectors
	//
	// Parameters:
	//						Vector - first vector
	//						Vector - second vector
	//
	// Returns:			Vector - angles between the vectors
	//----------------------------------------------------------------
	inline Vector Vector::AnglesBetween(const Vector &vector1, const Vector &vector2)
	{
		Vector unitVector1(vector1);
		unitVector1.normalize();
		Vector unitVector2(vector2);
		unitVector2.normalize();
		Vector angles(unitVector1.toAngles() - unitVector2.toAngles());
		angles.EulerNormalize();

		return angles;
	}

	//----------------------------------------------------------------
	// Name:				AngleBetween
	// Class:			Vector
	//
	// Description:	returns the smaller of the angles formed by the
	//						two vectors
	//
	// Parameters:
	//						Vector - first vector
	//						Vector - second vector
	//
	// Returns:			float - angle between the vectors
	//----------------------------------------------------------------
	inline float Vector::AngleBetween(const Vector &vector1, const Vector &vector2)
	{
		Vector unitVector1(vector1);
		unitVector1.normalize();
		Vector unitVector2(vector2);
		unitVector2.normalize();

		return acosf(Vector::Dot(unitVector1, unitVector2));
	}

	//----------------------------------------------------------------
	// Name:				CloseEnough
	// Class:			Vector
	//
	// Description:	tests to see if the two vectors are within
	//						'epsilon' of each other
	//
	// Parameters:
	//						Vector - first vector
	//						Vector - second vector
	//						float  - amount that each component of the
	//						vectors can be apart
	//
	// Returns:			bool	 - the result of the test for closeness
	//----------------------------------------------------------------
	inline bool Vector::CloseEnough(const Vector &vector1, const Vector &vector2, const float epsilon)
	{
		return Distance(vector1, vector2) < epsilon;
	}

	//----------------------------------------------------------------
	// Name:				SmallEnough
	// Class:			Vector
	//
	// Description:	tests to see if the vectors are within
	//						'epsilon' of the origin
	//
	// Parameters:
	//						Vector - vector
	//						float  - amount that each component of the
	//						vectors can be from the origin
	//
	// Returns:			bool	 - the result of the test for smallness
	//----------------------------------------------------------------
	inline bool Vector::SmallEnough(const Vector &vector, const float epsilon)
	{
		return CloseEnough(vector, Vector::Identity(), epsilon);
	}

	inline Vector Vector::operator-() const
	{
		return Vector(-x, -y, -z);
	}

	inline Vector Vector::fabs(const Vector &a)
	{
		return Vector(VECTOR_FABS(a.x), VECTOR_FABS(a.y), VECTOR_FABS(a.z));
	}

	inline float Vector::MaxValue(const Vector &a)
	{
		float maxy;
		float maxz;
		float max;

		max = VECTOR_FABS(a.x);
		maxy = VECTOR_FABS(a.y);
		maxz = VECTOR_FABS(a.z);

		if (maxy > max)
		{
			max = maxy;
		}
		if (maxz > max)
		{
			max = maxz;
		}
		return max;
	}

	inline float Vector::toYaw(void) const
	{
		float yaw;

		if ((y == 0.0) && (x == 0.0))
		{
			yaw = 0.0;
		}
		else
		{
			yaw = (float)((int)(atan2f(y, x) * 180.0 / M_PI_FLOAT));
			if (yaw < 0.0)
			{
				yaw += 360.0;
			}
		}

		return yaw;
	}

	inline float Vector::toPitch(void) const
	{
		float	forward;
		float	pitch;

		if ((x == 0.0) && (y == 0.0))
		{
			if (z > 0.0)
			{
				pitch = 90.0;
			}
			else
			{
				pitch = 270.0;
			}
		}
		else
		{
			forward = (float)sqrtf((x * x) + (y * y));
			pitch = (float)((int)(atan2f(z, forward) * 180.0 / M_PI_FLOAT));
			if (pitch < 0.0)
			{
				pitch += 360.0;
			}
		}

		return pitch;
	}

	inline Vector Vector::AnglesMod(void) const
	{
		return Vector(AngleMod(x), AngleMod(y), AngleMod(z));
	}

	inline void Vector::AngleVectors(Vector *forward, Vector *right, Vector *up) const
	{
		float				angle;
		static float	sr, sp, sy, cr, cp, cy; // static to help MS compiler fp bugs

		angle = yaw() * (M_PI_FLOAT * 2.0f / 360.0f);
		sy = sinf(angle);
		cy = cosf(angle);

		angle = pitch() * (M_PI_FLOAT * 2.0f / 360.0f);
		sp = sinf(angle);
		cp = cosf(angle);

		angle = roll() * (M_PI_FLOAT * 2.0f / 360.0f);
		sr = sinf(angle);
		cr = cosf(angle);

		if (forward)
		{
			forward->setXYZ(cp * cy, cp * sy, -sp);
		}

		if (right)
		{
			right->setXYZ((-1 * sr * sp * cy) + (-1 * cr * -sy), (-1 * sr * sp * sy) + (-1 * cr * cy), -1 * sr * cp);
		}

		if (up)
		{
			up->setXYZ((cr * sp * cy) + (-sr * -sy), (cr * sp * sy) + (-sr * cy), cr * cp);
		}
	}

	inline void Vector::AngleVectorsLeft(Vector *forward, Vector *left, Vector *up) const
	{
		float				angle;
		static float	sr, sp, sy, cr, cp, cy; // static to help MS compiler fp bugs

		angle = yaw() * (M_PI_FLOAT * 2.0f / 360.0f);
		sy = sinf(angle);
		cy = cosf(angle);

		angle = pitch() * (M_PI_FLOAT * 2.0f / 360.0f);
		sp = sinf(angle);
		cp = cosf(angle);

		angle = roll() * (M_PI_FLOAT * 2.0f / 360.0f);
		sr = sinf(angle);
		cr = cosf(angle);

		if (forward)
		{
			forward->setXYZ(cp * cy, cp * sy, -sp);
		}

		if (left)
		{
			left->setXYZ((sr * sp * cy) + (cr * -sy), (sr * sp * sy) + (cr * cy), sr * cp);
		}

		if (up)
		{
			up->setXYZ((cr * sp * cy) + (-sr * -sy), (cr * sp * sy) + (-sr * cy), cr * cp);
		}
	}


#define LERP_DELTA 1e-6
	inline Vector Vector::LerpVector(const Vector &vector1, const Vector &vector2, const float t)
	{
		float	omega, cosom, sinom, scale0, scale1;

		Vector w1(vector1);
		Vector w2(vector2);

		w1.normalize();
		w2.normalize();

		cosom = w1 * w2;
		if ((1.0 - cosom) > LERP_DELTA)
		{
			omega = acosf(cosom);
			sinom = sinf(omega);
			scale0 = sinf((1.0f - t) * omega) / sinom;
			scale1 = sinf(t * omega) / sinom;
		}
		else
		{
			scale0 = 1.0f - t;
			scale1 = t;
		}

		return ((w1 * scale0) + (w2 * scale1));
	}

	class Quat
	{
	public:
		float			x;
		float			y;
		float			z;
		float			w;

		Quat();
		Quat(Vector angles);
		Quat(float scrMatrix[3][3]);
		Quat(const float x, const float y, const float z, const float w);

		float *			vec4(void);
		float			operator[](const int index) const;
		float &			operator[](const int index);
		void 			set(const float x, const float y, const float z, const float w);
		const Quat &	operator=(const Quat &a);
		friend Quat		operator+(const Quat &a, const Quat &b);
		const Quat &	operator+=(const Quat &a);
		friend Quat		operator-(const Quat &a, const Quat &b);
		const Quat &	operator-=(const Quat &a);
		friend Quat		operator*(const Quat &a, const float b);
		friend Quat		operator*(const float a, const Quat &b);
		const Quat &	operator*=(const float a);
		friend bool		operator==(const Quat &a, const Quat &b);
		friend bool		operator!=(const Quat &a, const Quat &b);
		float			length(void) const;
		float			lengthSquared(void) const;
		const Quat &	normalize(void);
		Quat			operator-() const;
		Vector			toAngles(void);
	};

	inline Quat::Quat() : x(0), y(0), z(0), w(0)
	{
	}

	inline Quat::Quat(Vector Angles)
	{
		EulerToQuat(Angles, this->vec4());
	}

	inline Quat::Quat(float srcMatrix[3][3])
	{
		MatToQuat(srcMatrix, this->vec4());
	}

	inline Quat::Quat(const float init_x, const float init_y, const float init_z, const float init_w) : x(init_x), y(init_y), z(init_z), w(init_w)
	{
	}

	inline float Quat::operator[](const int index) const
	{
		assert((index >= 0) && (index < 4));
		return (&x)[index];
	}

	inline float & Quat::operator[](const int index)
	{
		assert((index >= 0) && (index < 4));
		return (&x)[index];
	}

	inline float *Quat::vec4(void)
	{
		return &x;
	}

	inline void  Quat::set(const float new_x, const float new_y, const float new_z, const float new_w)
	{
		x = new_x;
		y = new_y;
		z = new_z;
		w = new_w;
	}


	inline const Quat & Quat::operator=(const Quat &a)
	{
		x = a.x;
		y = a.y;
		z = a.z;
		w = a.w;

		return *this;
	}

	inline Quat operator+(const Quat &a, const Quat &b)
	{
		return Quat(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
	}

	inline const Quat & Quat::operator+=(const Quat &a)
	{
		*this = *this + a;

		return *this;
	}

	inline Quat operator-(const Quat &a, const Quat &b)
	{
		return Quat(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
	}

	inline const Quat & Quat::operator-=(const Quat &a)
	{
		*this = *this - a;

		return *this;
	}

	inline Quat operator*(const Quat &a, const float b)
	{
		return Quat(a.x * b, a.y * b, a.z * b, a.w * b);
	}

	inline Quat operator*(const float a, const Quat &b)
	{
		return b * a;
	}

	inline const Quat & Quat::operator*=(const float a)
	{
		*this = *this * a;

		return *this;
	}

	inline bool operator==(const Quat &a, const Quat &b)
	{
		return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w));
	}

	inline bool operator!=(const Quat &a, const Quat &b)
	{
		return ((a.x != b.x) || (a.y != b.y) || (a.z != b.z) && (a.w != b.w));
	}

	inline float Quat::length(void) const
	{
		float	length;

		length = (x * x) + (y * y) + (z * z) + (w * w);
		return sqrtf(length);
	}

	inline const Quat & Quat::normalize(void)
	{
		float	length, ilength;

		length = this->length();
		if (length)
		{
			ilength = 1.0f / length;
			*this *= ilength;
		}

		return *this;
	}

	inline Quat Quat::operator-() const
	{
		return Quat(-x, -y, -z, -w);
	}

	inline Vector Quat::toAngles(void)
	{
		float m[3][3];
		Vector angles;

		QuatToMat(this->vec4(), m);
		MatrixToEulerAngles(m, angles);
		return angles;
	}

	inline Vector Vector::GetRotatedX(float angle) const
	{
		if (angle == 0.0)
			return (*this);

		float sinAngle = (float)sinf(M_PI_FLOAT*angle / 180);
		float cosAngle = (float)cosf(M_PI_FLOAT*angle / 180);

		return Vector(x,
			y*cosAngle - z*sinAngle,
			y*sinAngle + z*cosAngle);
	}

	inline void Vector::RotateX(float angle)
	{
		(*this) = GetRotatedX(angle);
	}

	inline Vector Vector::GetRotatedY(float angle) const
	{
		if (angle == 0.0)
			return (*this);

		float sinAngle = (float)sinf(M_PI_FLOAT*angle / 180);
		float cosAngle = (float)cosf(M_PI_FLOAT*angle / 180);

		return Vector(x*cosAngle + z*sinAngle,
			y,
			-x*sinAngle + z*cosAngle);
	}

	inline void Vector::RotateY(float angle)
	{
		(*this) = GetRotatedY(angle);
	}

	inline Vector Vector::GetRotatedZ(float angle) const
	{
		if (angle == 0.0)
			return (*this);

		float sinAngle = (float)sinf(M_PI_FLOAT*angle / 180);
		float cosAngle = (float)cosf(M_PI_FLOAT*angle / 180);

		return Vector(x*cosAngle - y*sinAngle,
			x*sinAngle + y*cosAngle,
			z);
	}

	inline void Vector::RotateZ(float angle)
	{
		(*this) = GetRotatedZ(angle);
	}

	inline void Vector::PackTo01()
	{
		(*this) = GetPackedTo01();
	}

	inline Vector Vector::GetPackedTo01() const
	{
		Vector temp(*this);

		temp.normalize();

		temp = temp*0.5 + Vector(0.5, 0.5, 0.5);

		return temp;
	}

	inline bool Vector::IsEmpty() const
	{
		return x == 0 && y == 0 && z == 0;
	}
};

#undef VECTOR_FABS
