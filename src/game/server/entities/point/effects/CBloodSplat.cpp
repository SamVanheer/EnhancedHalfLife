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

#include "CBloodSplat.hpp"

//TODO: should tie this to an entity

void CBloodSplat::Spawn(CBaseEntity* pOwner)
{
	SetAbsOrigin(pOwner->GetAbsOrigin() + Vector(0, 0, 32));
	SetAbsAngles(pOwner->pev->v_angle);
	SetOwner(pOwner);

	SetThink(&CBloodSplat::Spray);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CBloodSplat::Spray()
{
	if (g_Language != LANGUAGE_GERMAN)
	{
		TraceResult	tr;
		UTIL_MakeVectors(GetAbsAngles());
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + gpGlobals->v_forward * 128, IgnoreMonsters::Yes, GetOwner(), &tr);

		UTIL_BloodDecalTrace(&tr, BLOOD_COLOR_RED);
	}
	SetThink(&CBloodSplat::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}
