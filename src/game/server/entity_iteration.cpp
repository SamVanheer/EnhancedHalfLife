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

#include "entity_iteration.hpp"

CBaseEntity* UTIL_EntityByIndex(int index)
{
	if (auto edict = g_engfuncs.pfnPEntityOfEntIndex(index); edict)
	{
		return reinterpret_cast<CBaseEntity*>(edict->pvPrivateData);
	}

	return nullptr;
}

CBasePlayer* UTIL_PlayerByIndex(int playerIndex)
{
	if (playerIndex > 0 && playerIndex <= gpGlobals->maxClients)
	{
		CBaseEntity* pPlayerEntity = UTIL_EntityByIndex(playerIndex);
		if (pPlayerEntity && !pPlayerEntity->edict()->free)
		{
			return static_cast<CBasePlayer*>(pPlayerEntity);
		}
	}

	return nullptr;
}

CBasePlayer* UTIL_GetLocalPlayer()
{
	if (gpGlobals->maxClients > 1)
	{
		ALERT(at_warning, "UTIL_GetLocalPlayer called in a multiplayer game\n");
		return nullptr;
	}

	return UTIL_PlayerByIndex(1);
}

CBasePlayer* FindPlayerByName(const char* pTestName)
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		if (CBasePlayer* pEnt = UTIL_PlayerByIndex(i); pEnt && pEnt->IsPlayer())
		{
			const char* pNetName = STRING(pEnt->pev->netname);
			if (stricmp(pNetName, pTestName) == 0)
			{
				return (CBasePlayer*)pEnt;
			}
		}
	}

	return nullptr;
}

int UTIL_CountPlayers()
{
	int	num = 0;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pEnt = UTIL_PlayerByIndex(i);

		if (pEnt)
		{
			++num;
		}
	}

	return num;
}

CBaseEntity* UTIL_FindEntityInSphere(CBaseEntity* pStartEntity, const Vector& vecCenter, float flRadius)
{
	edict_t* pentEntity;

	if (pStartEntity)
		pentEntity = pStartEntity->edict();
	else
		pentEntity = nullptr;

	pentEntity = g_engfuncs.pfnFindEntityInSphere(pentEntity, vecCenter, flRadius);

	if (!IsNullEnt(pentEntity))
		return CBaseEntity::Instance(pentEntity);
	return nullptr;
}

CBaseEntity* UTIL_FindEntityByString(CBaseEntity* pStartEntity, const char* szKeyword, const char* szValue)
{
	edict_t* pentEntity;

	if (pStartEntity)
		pentEntity = pStartEntity->edict();
	else
		pentEntity = nullptr;

	pentEntity = g_engfuncs.pfnFindEntityByString(pentEntity, szKeyword, szValue);

	if (!IsNullEnt(pentEntity))
		return CBaseEntity::Instance(pentEntity);
	return nullptr;
}

CBaseEntity* UTIL_FindEntityByClassname(CBaseEntity* pStartEntity, const char* szName)
{
	return UTIL_FindEntityByString(pStartEntity, "classname", szName);
}

CBaseEntity* UTIL_FindEntityByTargetname(CBaseEntity* pStartEntity, const char* szName)
{
	return UTIL_FindEntityByString(pStartEntity, "targetname", szName);
}

CBaseEntity* UTIL_FindEntityByTarget(CBaseEntity* pStartEntity, const char* szName)
{
	return UTIL_FindEntityByString(pStartEntity, "target", szName);
}

CBaseEntity* UTIL_FindEntityGeneric(const char* szWhatever, const Vector& vecSrc, float flRadius)
{
	CBaseEntity* pEntity = nullptr;

	pEntity = UTIL_FindEntityByTargetname(nullptr, szWhatever);
	if (pEntity)
		return pEntity;

	CBaseEntity* pSearch = nullptr;
	float flMaxDist2 = flRadius * flRadius;
	while ((pSearch = UTIL_FindEntityByClassname(pSearch, szWhatever)) != nullptr)
	{
		float flDist2 = (pSearch->GetAbsOrigin() - vecSrc).Length();
		flDist2 = flDist2 * flDist2;
		if (flMaxDist2 > flDist2)
		{
			pEntity = pSearch;
			flMaxDist2 = flDist2;
		}
	}
	return pEntity;
}

CBasePlayer* UTIL_FindClientInPVS(CBaseEntity* pPVSEntity)
{
	return static_cast<CBasePlayer*>(CBaseEntity::InstanceOrNull(g_engfuncs.pfnFindClientInPVS(CBaseEntity::EdictOrNull(pPVSEntity))));
}

int UTIL_MonstersInSphere(CBaseEntity** pList, int listMax, const Vector& center, float radius)
{
	float distance, delta;

	int count = 0;
	const float radiusSquared = radius * radius;

	for (int i = 1; i < gpGlobals->maxEntities; i++)
	{
		auto entity = UTIL_EntityByIndex(i);

		if (!entity || entity->edict()->free) // Not in use
			continue;

		if (!(entity->pev->flags & (FL_CLIENT | FL_MONSTER))) // Not a client/monster ?
			continue;

		// Use origin for X & Y since they are centered for all monsters
		// Now X
		delta = center.x - entity->GetAbsOrigin().x;//(pEdict->v.absmin.x + pEdict->v.absmax.x)*0.5;
		delta *= delta;

		if (delta > radiusSquared)
			continue;
		distance = delta;

		// Now Y
		delta = center.y - entity->GetAbsOrigin().y;//(pEdict->v.absmin.y + pEdict->v.absmax.y)*0.5;
		delta *= delta;

		distance += delta;
		if (distance > radiusSquared)
			continue;

		// Now Z
		delta = center.z - (entity->pev->absmin.z + entity->pev->absmax.z) * 0.5f;
		delta *= delta;

		distance += delta;
		if (distance > radiusSquared)
			continue;

		pList[count] = entity;
		count++;

		if (count >= listMax)
			return count;
	}

	return count;
}

int UTIL_EntitiesInBox(CBaseEntity** pList, int listMax, const Vector& mins, const Vector& maxs, int flagMask)
{
	int count = 0;

	for (int i = 1; i < gpGlobals->maxEntities; i++)
	{
		auto entity = UTIL_EntityByIndex(i);
		if (!entity || entity->edict()->free) // Not in use
			continue;

		if (flagMask && !(entity->pev->flags & flagMask)) // Does it meet the criteria?
			continue;

		if (mins.x > entity->pev->absmax.x ||
			mins.y > entity->pev->absmax.y ||
			mins.z > entity->pev->absmax.z ||
			maxs.x < entity->pev->absmin.x ||
			maxs.y < entity->pev->absmin.y ||
			maxs.z < entity->pev->absmin.z)
			continue;

		pList[count] = entity;
		count++;

		if (count >= listMax)
			return count;
	}

	return count;
}
