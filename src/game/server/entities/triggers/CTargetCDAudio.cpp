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

#include "CTargetCDAudio.hpp"
#include "CTriggerCDAudio.hpp"

LINK_ENTITY_TO_CLASS(target_cdaudio, CTargetCDAudio);

void CTargetCDAudio::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "radius"))
	{
		pev->scale = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CTargetCDAudio::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);

	if (pev->scale > 0)
		pev->nextthink = gpGlobals->time + 1.0;
}

void CTargetCDAudio::Use(const UseInfo& info)
{
	Play();
}

void CTargetCDAudio::Think()
{
	const float radiusSquared = pev->scale * pev->scale;

	for (auto player : UTIL_AllPlayers())
	{
		if ((player->GetAbsOrigin() - GetAbsOrigin()).LengthSquared() <= radiusSquared)
		{
			Play();
			return;
		}
	}

	//No players were in range, check again later
	pev->nextthink = gpGlobals->time + 0.5;
}

void CTargetCDAudio::Play()
{
	PlayCDTrack((int)pev->health);
	UTIL_Remove(this);
}
