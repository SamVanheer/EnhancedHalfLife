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

#include "CCycler.hpp"

TYPEDESCRIPTION	CCycler::m_SaveData[] =
{
	DEFINE_FIELD(CCycler, m_animate, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CCycler, CBaseMonster);

void CCycler::GenericCyclerSpawn(const char* szModel, const Vector& vecMin, const Vector& vecMax)
{
	if (!szModel || !*szModel)
	{
		ALERT(at_error, "cycler at %.0f %.0f %0.f missing modelname", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
		UTIL_RemoveNow(this);
		return;
	}

	SetClassname("cycler");
	PRECACHE_MODEL(szModel);
	SetModel(szModel);

	CCycler::Spawn();

	SetSize(vecMin, vecMax);
}

void CCycler::Spawn()
{
	InitBoneControllers();
	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::None);
	SetDamageMode(DamageMode::Yes);
	pev->effects = 0;
	pev->health = 80000;// no cycler should die
	pev->yaw_speed = 5;
	pev->ideal_yaw = GetAbsAngles().y;
	ChangeYaw(360);

	m_flFrameRate = 75;
	m_flGroundSpeed = 0;

	pev->nextthink += 1.0;

	ResetSequenceInfo();

	if (pev->sequence != 0 || pev->frame != 0)
	{
		m_animate = false;
		pev->framerate = 0;
	}
	else
	{
		m_animate = true;
	}
}

void CCycler::Think()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (m_animate)
	{
		StudioFrameAdvance();
	}
	if (m_fSequenceFinished && !m_fSequenceLoops)
	{
		// ResetSequenceInfo();
		// hack to avoid reloading model every frame
		pev->animtime = gpGlobals->time;
		pev->framerate = 1.0;
		m_fSequenceFinished = false;
		m_flLastEventCheck = gpGlobals->time;
		pev->frame = 0;
		if (!m_animate)
			pev->framerate = 0.0;	// FIX: don't reset framerate
	}
}

void CCycler::Use(const UseInfo& info)
{
	m_animate = !m_animate;
	if (m_animate)
		pev->framerate = 1.0;
	else
		pev->framerate = 0.0;
}

bool CCycler::TakeDamage(const TakeDamageInfo& info)
{
	if (m_animate)
	{
		pev->sequence++;

		ResetSequenceInfo();

		if (m_flFrameRate == 0.0)
		{
			pev->sequence = 0;
			ResetSequenceInfo();
		}
		pev->frame = 0;
	}
	else
	{
		pev->framerate = 1.0;
		StudioFrameAdvance(0.1);
		pev->framerate = 0;
		ALERT(at_console, "sequence: %d, frame %.0f\n", pev->sequence, pev->frame);
	}

	return false;
}
