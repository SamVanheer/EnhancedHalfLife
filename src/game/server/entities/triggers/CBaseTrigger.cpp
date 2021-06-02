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

#include "CBaseTrigger.hpp"

void CBaseTrigger::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		BaseClass::KeyValue(pkvd);
}

void CBaseTrigger::InitTrigger()
{
	// trigger angles are used for one-way touches.  An angle of 0 is assumed
	// to mean no restrictions, so use a yaw of 360 instead.
	if (GetAbsAngles() != vec3_origin)
		SetMovedir(this);
	SetSolidType(Solid::Trigger);
	SetMovetype(Movetype::None);
	SetModel(STRING(pev->model));    // set size and link into world
	if (CVAR_GET_FLOAT("showtriggers") == 0)
		SetBits(pev->effects, EF_NODRAW);
}

void CBaseTrigger::ToggleUse(const UseInfo& info)
{
	if (GetSolidType() == Solid::Not)
	{// if the trigger is off, turn it on
		SetSolidType(Solid::Trigger);

		// Force retouch
		gpGlobals->force_retouch++;
	}
	else
	{// turn the trigger off
		SetSolidType(Solid::Not);
	}
	SetAbsOrigin(GetAbsOrigin());
}
