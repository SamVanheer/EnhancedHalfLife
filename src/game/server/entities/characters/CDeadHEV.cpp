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

#include "CDeadHEV.hpp"

void CDeadHEV::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

void CDeadHEV::Spawn()
{
	PRECACHE_MODEL("models/player.mdl");
	SetModel("models/player.mdl");

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	SetBodygroup(DEADHEV_BODYGROUP_HEAD, DEADHEV_HEAD_HELMETED);
	m_bloodColor = BLOOD_COLOR_RED;

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);

	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead hevsuit with bad pose\n");
		pev->sequence = 0;
		pev->effects = EF_BRIGHTFIELD;
	}

	// Corpses have less health
	pev->health = 8;

	MonsterInitDead();
}