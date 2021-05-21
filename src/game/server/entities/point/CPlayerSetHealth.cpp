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

#pragma once

#include "CPlayerSetHealth.hpp"

LINK_ENTITY_TO_CLASS(player_sethealth, CPlayerSetHealth);

void CPlayerSetHealth::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "all_players"))
	{
		if (atoi(pkvd->szValue) != 0)
		{
			SetBits(m_Flags, PLAYERSETHEALTH_ALLPLAYERS);
		}
		else
		{
			ClearBits(m_Flags, PLAYERSETHEALTH_ALLPLAYERS);
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "set_health"))
	{
		if (atoi(pkvd->szValue) != 0)
		{
			SetBits(m_Flags, PLAYERSETHEALTH_SETHEALTH);
		}
		else
		{
			ClearBits(m_Flags, PLAYERSETHEALTH_SETHEALTH);
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "set_armor"))
	{
		if (atoi(pkvd->szValue) != 0)
		{
			SetBits(m_Flags, PLAYERSETHEALTH_SETARMOR);
		}
		else
		{
			ClearBits(m_Flags, PLAYERSETHEALTH_SETARMOR);
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "health_to_set"))
	{
		//Clamp to 1 so players don't end up in an invalid state
		m_Health = std::max(atoi(pkvd->szValue), 1);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "armor_to_set"))
	{
		m_Armor = std::max(atoi(pkvd->szValue), 0);
		pkvd->fHandled = true;
	}
	else
	{
		CPointEntity::KeyValue(pkvd);
	}
}

void CPlayerSetHealth::Use(const UseInfo& info)
{
	if (m_Flags & PLAYERSETHEALTH_ALLPLAYERS)
	{
		for (auto player : UTIL_AllPlayers())
		{
			ApplyToTarget(player);
		}
	}
	else
	{
		CBasePlayer* player = nullptr;

		if (auto activator = info.GetActivator(); activator && activator->IsPlayer())
		{
			player = static_cast<CBasePlayer*>(activator);
		}
		else if (!g_pGameRules->IsMultiplayer())
		{
			player = UTIL_GetLocalPlayer();
		}

		if (player)
		{
			ApplyToTarget(player);
		}
	}
}

void CPlayerSetHealth::ApplyToTarget(CBasePlayer* player)
{
	if (m_Flags & PLAYERSETHEALTH_SETHEALTH)
	{
		//Clamp it to the current max health
		const int healthToSet = std::min(static_cast<int>(std::floor(player->pev->max_health)), m_Health);
		player->pev->health = healthToSet;
	}

	if (m_Flags & PLAYERSETHEALTH_SETARMOR)
	{
		//Clamp it now, in case future changes allow for custom armor maximum
		const int armorToSet = std::min(MAX_NORMAL_BATTERY, m_Armor);
		player->pev->armorvalue = armorToSet;
	}
}
