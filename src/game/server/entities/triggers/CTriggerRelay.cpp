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

#include "CTriggerRelay.hpp"

void CTriggerRelay::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "triggerstate"))
	{
		triggerType = UTIL_TriggerStateToTriggerType(static_cast<TriggerState>(atoi(pkvd->szValue)));
		pkvd->fHandled = true;
	}
	else
		BaseClass::KeyValue(pkvd);
}

void CTriggerRelay::Spawn()
{
}

void CTriggerRelay::Use(const UseInfo& info)
{
	SUB_UseTargets(this, triggerType, 0);
	if (pev->spawnflags & SF_RELAY_FIREONCE)
		UTIL_Remove(this);
}
