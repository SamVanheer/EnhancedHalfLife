/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#pragma once

/**
*	@brief used for many pathfinding and many other operations that are treated as planar rather than 3d.
*/
class Vector2D
{
public:
	constexpr Vector2D()
		: x{}
		, y{}
	{
	}

	constexpr Vector2D(float X, float Y)
		: x(X)
		, y(Y)
	{
	}

	constexpr Vector2D operator+(const Vector2D& v) const { return Vector2D(x + v.x, y + v.y); }
	constexpr Vector2D operator-(const Vector2D& v) const { return Vector2D(x - v.x, y - v.y); }
	constexpr Vector2D operator*(float fl) const { return Vector2D(x * fl, y * fl); }
	constexpr Vector2D operator/(float fl) const { return Vector2D(x / fl, y / fl); }

	float Length() const { return static_cast<float>(sqrt(x * x + y * y)); }

	Vector2D Normalize() const
	{
		float flLen = Length();
		if (flLen == 0)
		{
			return Vector2D(0, 0);
		}
		else
		{
			flLen = 1 / flLen;
			return Vector2D(x * flLen, y * flLen);
		}
	}

	vec_t x, y;
};

constexpr float DotProduct(const Vector2D& a, const Vector2D& b) { return(a.x * b.x + a.y * b.y); }
constexpr Vector2D operator*(float fl, const Vector2D& v) { return v * fl; }

/**
*	@brief same data-layout as engine's vec3_t, which is a vec_t[3]
*/
class Vector
{
public:
	constexpr Vector()
		: x{}
		, y{}
		, z{}
	{
	}

	constexpr Vector(float X, float Y, float Z)
		: x(X)
		, y(Y)
		, z(Z)
	{
	}

	constexpr Vector(const Vector& v)
		: x(v.x)
		, y(v.y)
		, z(v.z)
	{
	}

	// Operators
	constexpr Vector operator-() const { return Vector(-x, -y, -z); }
	constexpr bool operator==(const Vector& v) const { return x == v.x && y == v.y && z == v.z; }
	constexpr bool operator!=(const Vector& v) const { return !(*this == v); }
	constexpr Vector operator+(const Vector& v) const { return Vector(x + v.x, y + v.y, z + v.z); }
	constexpr Vector operator-(const Vector& v) const { return Vector(x - v.x, y - v.y, z - v.z); }
	constexpr Vector operator*(float fl) const { return Vector(x * fl, y * fl, z * fl); }
	constexpr Vector operator/(float fl) const { return Vector(x / fl, y / fl, z / fl); }

	// Methods
	float Length() const { return static_cast<float>(sqrt(x * x + y * y + z * z)); }

	operator float* () { return &x; } // Vectors will now automatically convert to float * when needed
	operator const float* () const { return &x; } // Vectors will now automatically convert to float * when needed

	Vector Normalize() const
	{
		float flLen = Length();
		if (flLen == 0) return Vector(0, 0, 1); // ????
		flLen = 1 / flLen;
		return Vector(x * flLen, y * flLen, z * flLen);
	}

	constexpr Vector2D Make2D() const
	{
		Vector2D	Vec2;

		Vec2.x = x;
		Vec2.y = y;

		return Vec2;
	}

	float Length2D() const { return static_cast<float>(sqrt(x * x + y * y)); }

	// Members
	vec_t x, y, z;
};

constexpr Vector operator*(float fl, const Vector& v) { return v * fl; }
constexpr float DotProduct(const Vector& a, const Vector& b) { return(a.x * b.x + a.y * b.y + a.z * b.z); }
constexpr Vector CrossProduct(const Vector& a, const Vector& b) { return Vector(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
