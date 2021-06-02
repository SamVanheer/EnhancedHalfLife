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

#include "CXenHull.hpp"
#include "CXenSporeLarge.hpp"

/**
*	@brief I just eyeballed these -- fill in hulls for the legs
*/
const Vector CXenSporeLarge::m_hullSizes[] =
{
	Vector(90, -25, 0),
	Vector(25, 75, 0),
	Vector(-15, -100, 0),
	Vector(-90, -35, 0),
	Vector(-90, 60, 0),
};

void CXenSporeLarge::Spawn()
{
	pev->skin = 2;
	CXenSpore::Spawn();
	SetSize(Vector(-48, -48, 110), Vector(48, 48, 240));

	Vector forward, right;

	AngleVectors(GetAbsAngles(), &forward, &right, nullptr);

	// Rotate the leg hulls into position
	for (std::size_t i = 0; i < ArraySize(m_hullSizes); i++)
		CXenHull::CreateHull(this, Vector(-12, -12, 0), Vector(12, 12, 120), (m_hullSizes[i].x * forward) + (m_hullSizes[i].y * right));
}
