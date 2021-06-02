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

#include "CGameScore.hpp"

void CGameScore::Spawn()
{
	CRulePointEntity::Spawn();
}

void CGameScore::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "points"))
	{
		SetPoints(atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else
		CRulePointEntity::KeyValue(pkvd);
}

void CGameScore::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	// Only players can use this
	if (info.GetActivator()->IsPlayer())
	{
		auto player = static_cast<CBasePlayer*>(info.GetActivator());
		if (AwardToTeam())
		{
			player->AddPointsToTeam(Points(), AllowNegativeScore());
		}
		else
		{
			player->AddPoints(Points(), AllowNegativeScore());
		}
	}
}
