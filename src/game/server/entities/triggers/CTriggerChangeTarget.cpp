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

#include "CTriggerChangeTarget.hpp"

LINK_ENTITY_TO_CLASS(trigger_changetarget, CTriggerChangeTarget);

void CTriggerChangeTarget::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "m_iszNewTarget"))
	{
		m_iszNewTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		BaseClass::KeyValue(pkvd);
}

void CTriggerChangeTarget::Spawn()
{
}

void CTriggerChangeTarget::Use(const UseInfo& info)
{
	CBaseEntity* pTarget = UTIL_FindEntityByString(nullptr, "targetname", GetTarget());

	if (pTarget)
	{
		pTarget->SetTargetDirect(m_iszNewTarget);
		CBaseMonster* pMonster = pTarget->MyMonsterPointer();
		if (pMonster)
		{
			pMonster->m_hGoalEnt = nullptr;
		}
	}
}
