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

#include "CGameTeamMaster.hpp"

void CGameTeamMaster::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "teamindex"))
	{
		m_teamIndex = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "triggerstate"))
	{
		triggerType = UTIL_TriggerStateToTriggerType(static_cast<TriggerState>(atoi(pkvd->szValue)));
		pkvd->fHandled = true;
	}
	else
		CRulePointEntity::KeyValue(pkvd);
}

void CGameTeamMaster::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	if (info.GetUseType() == UseType::Set)
	{
		if (info.GetValue() < 0)
		{
			m_teamIndex = -1;
		}
		else
		{
			m_teamIndex = g_pGameRules->GetTeamIndex(info.GetActivator()->TeamID());
		}
		return;
	}

	if (TeamMatch(info.GetActivator()))
	{
		SUB_UseTargets(info.GetActivator(), triggerType, info.GetValue());
		if (RemoveOnFire())
			UTIL_Remove(this);
	}
}

bool CGameTeamMaster::IsTriggered(CBaseEntity* pActivator)
{
	return TeamMatch(pActivator);
}

const char* CGameTeamMaster::TeamID()
{
	if (m_teamIndex < 0)		// Currently set to "no team"
		return "";

	return g_pGameRules->GetIndexedTeamName(m_teamIndex);		// UNDONE: Fill this in with the team from the "teamlist"
}

bool CGameTeamMaster::TeamMatch(CBaseEntity* pActivator)
{
	if (m_teamIndex < 0 && AnyTeam())
		return true;

	if (!pActivator)
		return false;

	return UTIL_TeamsMatch(pActivator->TeamID(), TeamID());
}
