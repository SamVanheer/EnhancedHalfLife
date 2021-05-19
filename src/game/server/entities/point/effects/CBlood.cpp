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

#include "CBlood.hpp"

LINK_ENTITY_TO_CLASS(env_blood, CBlood);

void CBlood::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->effects = 0;
	pev->frame = 0;
	SetMovedir(this);
}

void CBlood::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "color"))
	{
		const int color = atoi(pkvd->szValue);
		switch (color)
		{
		case 1:
			SetColor(BLOOD_COLOR_YELLOW);
			break;
		default:
			SetColor(BLOOD_COLOR_RED);
			break;
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "amount"))
	{
		SetBloodAmount(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

Vector CBlood::Direction()
{
	if (pev->spawnflags & SF_BLOOD_RANDOM)
		return UTIL_RandomBloodVector();

	return pev->movedir;
}

Vector CBlood::BloodPosition(CBaseEntity* pActivator)
{
	if (pev->spawnflags & SF_BLOOD_PLAYER)
	{
		CBaseEntity* pPlayer = nullptr;

		if (pActivator && pActivator->IsPlayer())
		{
			pPlayer = pActivator;
		}
		else if (!g_pGameRules->IsMultiplayer())
			pPlayer = UTIL_GetLocalPlayer();
		if (pPlayer)
			return (pPlayer->GetAbsOrigin() + pPlayer->pev->view_ofs) + Vector(RANDOM_FLOAT(-10, 10), RANDOM_FLOAT(-10, 10), RANDOM_FLOAT(-10, 10));
	}

	return GetAbsOrigin();
}

void CBlood::Use(const UseInfo& info)
{
	if (pev->spawnflags & SF_BLOOD_STREAM)
		UTIL_BloodStream(BloodPosition(info.GetActivator()), Direction(), (Color() == BLOOD_COLOR_RED) ? 70 : Color(), BloodAmount());
	else
		UTIL_BloodDrips(BloodPosition(info.GetActivator()), Direction(), Color(), BloodAmount());

	if (pev->spawnflags & SF_BLOOD_DECAL)
	{
		const Vector forward = Direction();
		const Vector start = BloodPosition(info.GetActivator());
		TraceResult tr;

		UTIL_TraceLine(start, start + forward * BloodAmount() * 2, IgnoreMonsters::Yes, nullptr, &tr);
		if (tr.flFraction != 1.0)
			UTIL_BloodDecalTrace(&tr, Color());
	}
}
