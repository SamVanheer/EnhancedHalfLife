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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "decals.hpp"
#include "gamerules.h"

//
// This must match the list in decals.hpp
//
DLL_DECALLIST gDecals[] = {
	{ "{shot1",	0 },		// DECAL_GUNSHOT1 
	{ "{shot2",	0 },		// DECAL_GUNSHOT2
	{ "{shot3",0 },			// DECAL_GUNSHOT3
	{ "{shot4",	0 },		// DECAL_GUNSHOT4
	{ "{shot5",	0 },		// DECAL_GUNSHOT5
	{ "{lambda01", 0 },		// DECAL_LAMBDA1
	{ "{lambda02", 0 },		// DECAL_LAMBDA2
	{ "{lambda03", 0 },		// DECAL_LAMBDA3
	{ "{lambda04", 0 },		// DECAL_LAMBDA4
	{ "{lambda05", 0 },		// DECAL_LAMBDA5
	{ "{lambda06", 0 },		// DECAL_LAMBDA6
	{ "{scorch1", 0 },		// DECAL_SCORCH1
	{ "{scorch2", 0 },		// DECAL_SCORCH2
	{ "{blood1", 0 },		// DECAL_BLOOD1
	{ "{blood2", 0 },		// DECAL_BLOOD2
	{ "{blood3", 0 },		// DECAL_BLOOD3
	{ "{blood4", 0 },		// DECAL_BLOOD4
	{ "{blood5", 0 },		// DECAL_BLOOD5
	{ "{blood6", 0 },		// DECAL_BLOOD6
	{ "{yblood1", 0 },		// DECAL_YBLOOD1
	{ "{yblood2", 0 },		// DECAL_YBLOOD2
	{ "{yblood3", 0 },		// DECAL_YBLOOD3
	{ "{yblood4", 0 },		// DECAL_YBLOOD4
	{ "{yblood5", 0 },		// DECAL_YBLOOD5
	{ "{yblood6", 0 },		// DECAL_YBLOOD6
	{ "{break1", 0 },		// DECAL_GLASSBREAK1
	{ "{break2", 0 },		// DECAL_GLASSBREAK2
	{ "{break3", 0 },		// DECAL_GLASSBREAK3
	{ "{bigshot1", 0 },		// DECAL_BIGSHOT1
	{ "{bigshot2", 0 },		// DECAL_BIGSHOT2
	{ "{bigshot3", 0 },		// DECAL_BIGSHOT3
	{ "{bigshot4", 0 },		// DECAL_BIGSHOT4
	{ "{bigshot5", 0 },		// DECAL_BIGSHOT5
	{ "{spit1", 0 },		// DECAL_SPIT1
	{ "{spit2", 0 },		// DECAL_SPIT2
	{ "{bproof1", 0 },		// DECAL_BPROOF1
	{ "{gargstomp", 0 },	// DECAL_GARGSTOMP1
	{ "{smscorch1", 0 },	// DECAL_SMALLSCORCH1
	{ "{smscorch2", 0 },	// DECAL_SMALLSCORCH2
	{ "{smscorch3", 0 },	// DECAL_SMALLSCORCH3
	{ "{mommablob", 0 },	// DECAL_MOMMABIRTH
	{ "{mommablob", 0 },	// DECAL_MOMMASPLAT
};

void InitializeDecalsList()
{
	for (std::size_t i = 0; i < ArraySize(gDecals); i++)
		gDecals[i].index = DECAL_INDEX(gDecals[i].name);
}

constexpr int SF_DECAL_NOTINDEATHMATCH = 2048;

class CDecal : public CBaseEntity
{
public:
	void	Spawn() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	EXPORT StaticDecal();
	void	EXPORT TriggerDecal(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};

LINK_ENTITY_TO_CLASS(infodecal, CDecal);

// UNDONE:  These won't get sent to joining players in multi-player
void CDecal::Spawn()
{
	if (pev->skin < 0 || (g_pGameRules->IsDeathmatch() && IsBitSet(pev->spawnflags, SF_DECAL_NOTINDEATHMATCH)))
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	if (IsStringNull(pev->targetname))
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

void CDecal::TriggerDecal(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// this is set up as a USE function for infodecals that have targetnames, so that the
	// decal doesn't get applied until it is fired. (usually by a scripted sequence)
	TraceResult trace;
	int			entityIndex;

	UTIL_TraceLine(pev->origin - Vector(5, 5, 5), pev->origin + Vector(5, 5, 5), IgnoreMonsters::Yes, ENT(pev), &trace);

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BSPDECAL);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT((int)pev->skin);
	entityIndex = (short)ENTINDEX(trace.pHit);
	WRITE_SHORT(entityIndex);
	if (entityIndex)
		WRITE_SHORT((int)VARS(trace.pHit)->modelindex);
	MESSAGE_END();

	SetThink(&CDecal::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CDecal::StaticDecal()
{
	TraceResult trace;
	int			entityIndex, modelIndex;

	UTIL_TraceLine(pev->origin - Vector(5, 5, 5), pev->origin + Vector(5, 5, 5), IgnoreMonsters::Yes, ENT(pev), &trace);

	entityIndex = (short)ENTINDEX(trace.pHit);
	if (entityIndex)
		modelIndex = (int)VARS(trace.pHit)->modelindex;
	else
		modelIndex = 0;

	g_engfuncs.pfnStaticDecal(pev->origin, (int)pev->skin, entityIndex, modelIndex);

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
