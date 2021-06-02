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

#include "CGamePlayerTeam.hpp"

const char* CGamePlayerTeam::TargetTeamName(const char* pszTargetName)
{
	CBaseEntity* pTeamEntity = nullptr;

	while ((pTeamEntity = UTIL_FindEntityByTargetname(pTeamEntity, pszTargetName)) != nullptr)
	{
		if (pTeamEntity->ClassnameIs("game_team_master"))
			return pTeamEntity->TeamID();
	}

	return nullptr;
}

void CGamePlayerTeam::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	if (info.GetActivator()->IsPlayer())
	{
		const char* pszTargetTeam = TargetTeamName(GetTarget());
		if (pszTargetTeam)
		{
			CBasePlayer* pPlayer = (CBasePlayer*)info.GetActivator();
			g_pGameRules->ChangePlayerTeam(pPlayer, pszTargetTeam, ShouldKillPlayer(), ShouldGibPlayer());
		}
	}

	if (RemoveOnFire())
	{
		UTIL_Remove(this);
	}
}
