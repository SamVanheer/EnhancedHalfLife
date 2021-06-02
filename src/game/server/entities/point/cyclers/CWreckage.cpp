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

#include "CWreckage.hpp"

void CWreckage::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->takedamage = 0;
	pev->effects = 0;

	pev->frame = 0;
	pev->nextthink = gpGlobals->time + 0.1;

	if (!IsStringNull(pev->model))
	{
		PRECACHE_MODEL(STRING(pev->model));
		SetModel(STRING(pev->model));
	}
	// pev->scale = 5.0;

	m_flStartTime = gpGlobals->time;
}

void CWreckage::Precache()
{
	if (!IsStringNull(pev->model))
		PRECACHE_MODEL(STRING(pev->model));
}

void CWreckage::Think()
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.2;

	if (pev->dmgtime)
	{
		if (pev->dmgtime < gpGlobals->time)
		{
			UTIL_Remove(this);
			return;
		}
		else if (RANDOM_FLOAT(0, pev->dmgtime - m_flStartTime) > pev->dmgtime - gpGlobals->time)
		{
			return;
		}
	}

	const Vector VecSrc
	{
		RANDOM_FLOAT(pev->absmin.x, pev->absmax.x),
		RANDOM_FLOAT(pev->absmin.y, pev->absmax.y),
		RANDOM_FLOAT(pev->absmin.z, pev->absmax.z)
	};

	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, VecSrc);
	WRITE_BYTE(TE_SMOKE);
	WRITE_COORD(VecSrc.x);
	WRITE_COORD(VecSrc.y);
	WRITE_COORD(VecSrc.z);
	WRITE_SHORT(g_sModelIndexSmoke);
	WRITE_BYTE(RANDOM_LONG(0, 49) + 50); // scale * 10
	WRITE_BYTE(RANDOM_LONG(0, 3) + 8); // framerate
	MESSAGE_END();
}
