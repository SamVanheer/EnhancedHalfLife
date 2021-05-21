/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#include "Platform.hpp"
#include "mathlib.hpp"
#include "view_utils.hpp"

void UTIL_DropPunchAngle(float frametime, Vector& ev_punchangle)
{
	float len = VectorNormalize(ev_punchangle);
	len -= (10.0 + len * 0.5) * frametime;
	len = std::max(len, 0.0f);
	ev_punchangle = ev_punchangle * len;
}

float UTIL_CalcRoll(const Vector& angles, const Vector& velocity, float rollangle, float rollspeed)
{
	Vector forward, right, up;
	AngleVectors(angles, forward, right, up);

	float side = DotProduct(velocity, right);

	const float sign = side < 0 ? -1 : 1;

	side = fabs(side);

	const float value = rollangle;

	if (side < rollspeed)
	{
		side = side * value / rollspeed;
	}
	else
	{
		side = value;
	}

	return side * sign;
}
