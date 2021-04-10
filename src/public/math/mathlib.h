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
// mathlib.h

#pragma once

#include <cmath>

typedef float vec_t;

#include "vector.h"

// up / down
constexpr int PITCH = 0;
// left / right
constexpr int YAW = 1;
// fall over
constexpr int ROLL = 2;

typedef vec_t vec4_t[4];	// x,y,z,w
typedef vec_t vec5_t[5];

typedef short vec_s_t;
typedef vec_s_t vec3s_t[3];
typedef vec_s_t vec4s_t[4];	// x,y,z,w
typedef vec_s_t vec5s_t[5];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;
#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

struct mplane_t;

constexpr Vector vec3_origin(0, 0, 0);
constexpr int nanmask = 255 << 23;

template<typename T>
constexpr bool IS_NAN(T x)
{
	return (*reinterpret_cast<int*>(&x) & nanmask) == nanmask;
}

#define VectorCopy(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}

float VectorNormalize(float* v);		// returns vector length
void VectorInverse(float* v);
void VectorScale(const float* in, float scale, float* out);
int Q_log2(int val);

void R_ConcatRotations(float in1[3][3], float in2[3][3], float out[3][3]);
void R_ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4]); //TODO: probably the same as ConcatTransforms

void FloorDivMod(double numer, double denom, int* quotient, int* rem);
fixed16_t Invert24To16(fixed16_t val);
int GreatestCommonDivisor(int i1, int i2);

void AngleVectors(const Vector& angles, Vector* forward, Vector* right, Vector* up);

/**
*	@brief overload that takes all outputs by reference
*/
inline void AngleVectors(const Vector& angles, Vector& forward, Vector& right, Vector& up)
{
	AngleVectors(angles, &forward, &right, &up);
}

void AngleVectorsTranspose(const Vector& angles, Vector* forward, Vector* right, Vector* up);

void AngleMatrix(const float* angles, float(*matrix)[4]);
void AngleIMatrix(const Vector& angles, float(*matrix)[4]);
void VectorTransform(const float* in1, float in2[3][4], float* out);

void NormalizeAngles(float* angles);

/**
*	@brief Interpolate Euler angles.
*
*	FIXME:  Use Quaternions to avoid discontinuities
*	Frac is 0.0 to 1.0 ( i.e., should probably be clamped, but doesn't have to be )
*/
void InterpolateAngles(float* start, float* end, float* output, float frac);
void SmoothInterpolateAngles(Vector& startAngle, Vector& endAngle, Vector& finalAngle, float degreesPerSec, float frametime);
float AngleBetweenVectors(const Vector& v1, const Vector& v2);
float UTIL_AngleDiff(float destAngle, float srcAngle);
float UTIL_AngleDistance(float next, float cur);
float UTIL_Approach(float target, float value, float speed);
float UTIL_ApproachAngle(float target, float value, float speed);

// Use for ease-in, ease-out style interpolation (accel/decel)
float UTIL_SplineFraction(float value, float scale);

void VectorMatrix(const Vector& forward, Vector& right, Vector& up);
Vector VectorAngles(const Vector& forward);
float UTIL_VecToYaw(const Vector& vec);
Vector UTIL_ClampVectorToBox(const Vector& input, const Vector& clampSize);

int InvertMatrix(const float* m, float* out);

int BoxOnPlaneSide(const Vector& emins, const Vector& emaxs, mplane_t* plane);
float anglemod(float a);
float UTIL_AngleMod(float a);

float Distance(const Vector& v1, const Vector& v2);

void ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4]);

void AngleQuaternion(float* angles, vec4_t quaternion);

void QuaternionSlerp(vec4_t p, vec4_t q, float t, vec4_t qt);

void QuaternionMatrix(vec4_t quaternion, float(*matrix)[4]);

void MatrixCopy(float in[3][4], float out[3][4]);

float MaxAngleBetweenAngles(float* a1, float* a2);

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide( (emins), (emaxs), (p)))
