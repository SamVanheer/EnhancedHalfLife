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

#include "spawnpoints.hpp"

/**
*	@brief checks if the spot is clear of players
*/
bool IsSpawnPointValid(CBaseEntity* pPlayer, CBaseEntity* pSpot)
{
	if (!pSpot->IsTriggered(pPlayer))
	{
		return false;
	}

	CBaseEntity* ent = nullptr;

	while ((ent = UTIL_FindEntityInSphere(ent, pSpot->GetAbsOrigin(), 128)) != nullptr)
	{
		// if ent is a client, don't spawn on 'em
		if (ent->IsPlayer() && ent != pPlayer)
			return false;
	}

	return true;
}

/**
*	@brief Find a spawn point for the player
*/
static CBaseEntity* FindSpawnPoint(CBaseEntity* pPlayer, CBaseEntity* lastSpawn)
{
	CBaseEntity* pSpot;

	//TODO: maybe delegate parts of this to gamerules instead of checking manually
	// choose a info_player_deathmatch point
	if (g_pGameRules->IsCoOp())
	{
		pSpot = UTIL_FindEntityByClassname(lastSpawn, "info_player_coop");
		if (!IsNullEnt(pSpot))
			return pSpot;
		pSpot = UTIL_FindEntityByClassname(lastSpawn, "info_player_start");
		if (!IsNullEnt(pSpot))
			return pSpot;
	}
	else if (g_pGameRules->IsDeathmatch())
	{
		pSpot = lastSpawn;
		// Randomize the start spot
		for (int i = RANDOM_LONG(1, 5); i > 0; i--)
			pSpot = UTIL_FindEntityByClassname(pSpot, "info_player_deathmatch");
		if (IsNullEnt(pSpot))  // skip over the null point
			pSpot = UTIL_FindEntityByClassname(pSpot, "info_player_deathmatch");

		CBaseEntity* pFirstSpot = pSpot;

		do
		{
			if (pSpot)
			{
				// check if pSpot is valid
				if (IsSpawnPointValid(pPlayer, pSpot))
				{
					if (pSpot->GetAbsOrigin() == vec3_origin)
					{
						pSpot = UTIL_FindEntityByClassname(pSpot, "info_player_deathmatch");
						continue;
					}

					// if so, go to pSpot
					return pSpot;
				}
			}
			// increment pSpot
			pSpot = UTIL_FindEntityByClassname(pSpot, "info_player_deathmatch");
		}
		while (pSpot != pFirstSpot); // loop if we're not back to the start

	 // we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
		if (!IsNullEnt(pSpot))
		{
			CBaseEntity* ent = nullptr;
			while ((ent = UTIL_FindEntityInSphere(ent, pSpot->GetAbsOrigin(), 128)) != nullptr)
			{
				// if ent is a client, kill em (unless they are ourselves)
				if (ent->IsPlayer() && ent != pPlayer)
					ent->TakeDamage({UTIL_GetWorld(), UTIL_GetWorld(), 300, DMG_GENERIC});
			}
			return pSpot;
		}
	}

	// If startspot is set, (re)spawn there.
	if (IsStringNull(gpGlobals->startspot) || !strlen(STRING(gpGlobals->startspot)))
	{
		pSpot = UTIL_FindEntityByClassname(nullptr, "info_player_start");
		if (!IsNullEnt(pSpot))
			return pSpot;
	}
	else
	{
		pSpot = UTIL_FindEntityByTargetname(nullptr, STRING(gpGlobals->startspot));
		if (!IsNullEnt(pSpot))
			return pSpot;
	}

	return nullptr;
}

CBaseEntity* EntSelectSpawnPoint(CBaseEntity* pPlayer)
{
	CBaseEntity* pSpot = FindSpawnPoint(pPlayer, g_pLastSpawn);

	if (IsNullEnt(pSpot))
	{
		ALERT(at_error, "PutClientInServer: no info_player_start on level");
		return UTIL_GetWorld();
	}

	g_pLastSpawn = pSpot;
	return pSpot;
}

