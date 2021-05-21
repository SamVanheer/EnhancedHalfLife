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

#include "CFuncPlatRot.hpp"

LINK_ENTITY_TO_CLASS(func_platrot, CFuncPlatRot);

void CFuncPlatRot::SetupRotation()
{
	if (m_vecFinalAngle.x != 0)		// This plat rotates too!
	{
		CBaseToggle::AxisDir(this);
		m_start = GetAbsAngles();
		m_end = GetAbsAngles() + pev->movedir * m_vecFinalAngle.x;
	}
	else
	{
		m_start = vec3_origin;
		m_end = vec3_origin;
	}
	if (!IsStringNull(pev->targetname))	// Start at top
	{
		SetAbsAngles(m_end);
	}
}

void CFuncPlatRot::Spawn()
{
	CFuncPlat::Spawn();
	SetupRotation();
}

void CFuncPlatRot::GoDown()
{
	CFuncPlat::GoDown();
	RotMove(m_start, pev->nextthink - pev->ltime);
}

void CFuncPlatRot::HitBottom()
{
	CFuncPlat::HitBottom();
	pev->avelocity = vec3_origin;
	SetAbsAngles(m_start);
}

void CFuncPlatRot::GoUp()
{
	CFuncPlat::GoUp();
	RotMove(m_end, pev->nextthink - pev->ltime);
}

void CFuncPlatRot::HitTop()
{
	CFuncPlat::HitTop();
	pev->avelocity = vec3_origin;
	SetAbsAngles(m_end);
}

void CFuncPlatRot::RotMove(const Vector& destAngle, float time)
{
	// set destdelta to the vector needed to move
	const Vector vecDestDelta = destAngle - GetAbsAngles();

	// Travel time is so short, we're practically there already;  so make it so.
	if (time >= 0.1)
		pev->avelocity = vecDestDelta / time;
	else
	{
		pev->avelocity = vecDestDelta;
		pev->nextthink = pev->ltime + 1;
	}
}
