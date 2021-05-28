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

#include "CAutoTrigger.hpp"

LINK_ENTITY_TO_CLASS(trigger_auto, CAutoTrigger);

void CAutoTrigger::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "globalstate"))
	{
		m_globalstate = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "triggerstate"))
	{
		triggerType = UTIL_TriggerStateToTriggerType(static_cast<TriggerState>(atoi(pkvd->szValue)));
		pkvd->fHandled = true;
	}
	else
		BaseClass::KeyValue(pkvd);
}

void CAutoTrigger::Spawn()
{
	Precache();
}

void CAutoTrigger::Precache()
{
	pev->nextthink = gpGlobals->time + 0.1;
}

void CAutoTrigger::Think()
{
	if (IsStringNull(m_globalstate) || gGlobalState.EntityGetState(m_globalstate) == GlobalEntState::On)
	{
		SUB_UseTargets(this, triggerType, 0);
		if (pev->spawnflags & SF_AUTO_FIREONCE)
			UTIL_Remove(this);
	}
}
