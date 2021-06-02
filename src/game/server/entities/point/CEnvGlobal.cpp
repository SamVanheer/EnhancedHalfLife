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

#include "CEnvGlobal.hpp"

void CEnvGlobal::KeyValue(KeyValueData* pkvd)
{
	pkvd->fHandled = true;

	if (AreStringsEqual(pkvd->szKeyName, "globalstate"))		// State name
		m_globalstate = ALLOC_STRING(pkvd->szValue);
	else if (AreStringsEqual(pkvd->szKeyName, "triggermode")) //TODO: validate input
		m_triggermode = static_cast<GlobalTriggerMode>(atoi(pkvd->szValue));
	else if (AreStringsEqual(pkvd->szKeyName, "initialstate")) //TODO: validate input
		m_initialstate = static_cast<GlobalEntState>(atoi(pkvd->szValue));
	else
		CPointEntity::KeyValue(pkvd);
}

void CEnvGlobal::Spawn()
{
	if (IsStringNull(m_globalstate))
	{
		UTIL_RemoveNow(this);
		return;
	}
	if (IsBitSet(pev->spawnflags, SF_GLOBAL_SET))
	{
		if (!gGlobalState.EntityInTable(m_globalstate))
			gGlobalState.EntityAdd(m_globalstate, gpGlobals->mapname, m_initialstate);
	}
}

void CEnvGlobal::Use(const UseInfo& info)
{
	const GlobalEntState oldState = gGlobalState.EntityGetState(m_globalstate);
	GlobalEntState newState;

	switch (m_triggermode)
	{
	case GlobalTriggerMode::Off:
		newState = GlobalEntState::Off;
		break;

	case GlobalTriggerMode::On:
		newState = GlobalEntState::On;
		break;

	case GlobalTriggerMode::Dead:
		newState = GlobalEntState::Dead;
		break;

	default:
	case GlobalTriggerMode::Toggle:
		if (oldState == GlobalEntState::On)
			newState = GlobalEntState::Off;
		else if (oldState == GlobalEntState::Off)
			newState = GlobalEntState::On;
		else
			newState = oldState;
	}

	if (gGlobalState.EntityInTable(m_globalstate))
		gGlobalState.EntitySetState(m_globalstate, newState);
	else
		gGlobalState.EntityAdd(m_globalstate, gpGlobals->mapname, newState);
}
