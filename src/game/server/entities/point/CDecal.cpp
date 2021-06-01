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

#include "CDecal.hpp"

LINK_ENTITY_TO_CLASS(infodecal, CDecal);

// UNDONE:  These won't get sent to joining players in multi-player
void CDecal::Spawn()
{
	if (pev->skin < 0 || (g_pGameRules->IsDeathmatch() && IsBitSet(pev->spawnflags, SF_DECAL_NOTINDEATHMATCH)))
	{
		UTIL_RemoveNow(this);
		return;
	}

	if (!HasTargetname())
	{
		SetThink(&CDecal::StaticDecal);
		// if there's no targetname, the decal will spray itself on as soon as the world is done spawning.
		pev->nextthink = gpGlobals->time;
	}
	else
	{
		// if there IS a targetname, the decal sprays itself on when it is triggered.
		SetThink(&CDecal::SUB_DoNothing);
		SetUse(&CDecal::TriggerDecal);
	}
}

void CDecal::TriggerDecal(const UseInfo& info)
{
	// this is set up as a USE function for infodecals that have targetnames, so that the
	// decal doesn't get applied until it is fired. (usually by a scripted sequence)
	TraceResult trace;
	int			entityIndex;

	UTIL_TraceLine(GetAbsOrigin() - Vector(5, 5, 5), GetAbsOrigin() + Vector(5, 5, 5), IgnoreMonsters::Yes, this, &trace);

	auto hit = InstanceOrWorld(trace.pHit);

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BSPDECAL);
	WRITE_COORD(GetAbsOrigin().x);
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	WRITE_SHORT((int)pev->skin);
	entityIndex = (short)hit->entindex();
	WRITE_SHORT(entityIndex);
	if (entityIndex)
		WRITE_SHORT((int)hit->pev->modelindex);
	MESSAGE_END();

	SetThink(&CDecal::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CDecal::StaticDecal()
{
	TraceResult trace;
	UTIL_TraceLine(GetAbsOrigin() - Vector(5, 5, 5), GetAbsOrigin() + Vector(5, 5, 5), IgnoreMonsters::Yes, this, &trace);

	auto hit = InstanceOrWorld(trace.pHit);

	const int entityIndex = (short)hit->entindex();
	const int modelIndex = entityIndex ? hit->pev->modelindex : 0;

	g_engfuncs.pfnStaticDecal(GetAbsOrigin(), pev->skin, entityIndex, modelIndex);

	SUB_Remove();
}

void CDecal::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "texture"))
	{
		pev->skin = DECAL_INDEX(pkvd->szValue);

		// Found
		if (pev->skin >= 0)
			return;
		ALERT(at_console, "Can't find decal %s\n", pkvd->szValue);
	}
	else
		CBaseEntity::KeyValue(pkvd);
}
