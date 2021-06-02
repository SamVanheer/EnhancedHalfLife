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

#include "CFade.hpp"
#include "shake.hpp"

void CFade::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->effects = 0;
	pev->frame = 0;
}

void CFade::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "duration"))
	{
		SetDuration(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "holdtime"))
	{
		SetHoldTime(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CFade::Use(const UseInfo& info)
{
	int fadeFlags = 0;

	if (!(pev->spawnflags & SF_FADE_IN))
		fadeFlags |= FFADE_OUT;

	if (pev->spawnflags & SF_FADE_MODULATE)
		fadeFlags |= FFADE_MODULATE;

	if (pev->spawnflags & SF_FADE_ONLYONE)
	{
		if (info.GetActivator()->IsNetClient())
		{
			UTIL_ScreenFade(static_cast<CBasePlayer*>(info.GetActivator()), GetRenderColor(), Duration(), HoldTime(), GetRenderAmount(), fadeFlags);
		}
	}
	else
	{
		UTIL_ScreenFadeAll(GetRenderColor(), Duration(), HoldTime(), GetRenderAmount(), fadeFlags);
	}
	SUB_UseTargets(this, UseType::Toggle, 0);
}
