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

#include "CGamePlayerZone.hpp"

LINK_ENTITY_TO_CLASS(game_zone_player, CGamePlayerZone);

void CGamePlayerZone::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "intarget"))
	{
		m_iszInTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "outtarget"))
	{
		m_iszOutTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "incount"))
	{
		m_iszInCount = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "outcount"))
	{
		m_iszOutCount = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CRuleBrushEntity::KeyValue(pkvd);
}

void CGamePlayerZone::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	int playersInCount = 0;
	int playersOutCount = 0;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		auto pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			TraceResult trace;
			Hull hullNumber;

			hullNumber = Hull::Human;
			if (pPlayer->pev->flags & FL_DUCKING)
			{
				hullNumber = Hull::Head;
			}

			UTIL_TraceModel(pPlayer->GetAbsOrigin(), pPlayer->GetAbsOrigin(), hullNumber, this, &trace);

			if (trace.fStartSolid)
			{
				playersInCount++;
				if (!IsStringNull(m_iszInTarget))
				{
					FireTargets(STRING(m_iszInTarget), pPlayer, info.GetActivator(), info.GetUseType(), info.GetValue());
				}
			}
			else
			{
				playersOutCount++;
				if (!IsStringNull(m_iszOutTarget))
				{
					FireTargets(STRING(m_iszOutTarget), pPlayer, info.GetActivator(), info.GetUseType(), info.GetValue());
				}
			}
		}
	}

	if (!IsStringNull(m_iszInCount))
	{
		FireTargets(STRING(m_iszInCount), info.GetActivator(), this, UseType::Set, playersInCount);
	}

	if (!IsStringNull(m_iszOutCount))
	{
		FireTargets(STRING(m_iszOutCount), info.GetActivator(), this, UseType::Set, playersOutCount);
	}
}
