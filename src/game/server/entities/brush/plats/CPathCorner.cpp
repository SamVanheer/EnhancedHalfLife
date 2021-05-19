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

#include "CPathCorner.hpp"

LINK_ENTITY_TO_CLASS(path_corner, CPathCorner);

// Global Savedata for Delay
TYPEDESCRIPTION	CPathCorner::m_SaveData[] =
{
	DEFINE_FIELD(CPathCorner, m_flWait, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CPathCorner, CPointEntity);

void CPathCorner::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CPathCorner::Spawn()
{
	ASSERTSZ(!IsStringNull(pev->targetname), "path_corner without a targetname");
}

#if 0
void CPathCorner::Touch(CBaseEntity* pOther)
{
	if (IsBitSet(pOther->pev->flags, FL_MONSTER))
	{// monsters don't navigate path corners based on touch anymore
		return;
	}

	// If OTHER isn't explicitly looking for this path_corner, bail out
	if (pOther->m_hGoalEnt != this)
	{
		return;
	}

	// If OTHER has an enemy, this touch is incidental, ignore
	if (!IsNullEnt(pOther->pev->enemy))
	{
		return;		// fighting, not following a path
	}

	// UNDONE: support non-zero flWait
	/*
	if (m_flWait != 0)
		ALERT(at_warning, "Non-zero path-cornder waits NYI");
	*/

	// Find the next "stop" on the path, make it the goal of the "toucher".
	if (IsStringNull(pev->target))
	{
		ALERT(at_warning, "PathCornerTouch: no next stop specified");
	}

	pOther->m_hGoalEnt = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));

	// If "next spot" was not found (does not exist - level design error)
	if (!pOther->m_hGoalEnt)
	{
		ALERT(at_console, "PathCornerTouch--%s couldn't find next stop in path: %s", GetClassname(), STRING(pev->target));
		return;
	}

	// Turn towards the next stop in the path.
	pOther->pev->ideal_yaw = UTIL_VecToYaw(pOther->m_hGoalEnt->GetAbsOrigin() - pOther->GetAbsOrigin());
}
#endif

