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

#include "CRevertSaved.hpp"
#include "shake.hpp"

void CRevertSaved::KeyValue(KeyValueData* pkvd)
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
	else if (AreStringsEqual(pkvd->szKeyName, "messagetime"))
	{
		SetMessageTime(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "loadtime"))
	{
		SetLoadTime(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CRevertSaved::Use(const UseInfo& info)
{
	UTIL_ScreenFadeAll(GetRenderColor(), Duration(), HoldTime(), GetRenderAmount(), FFADE_OUT);
	pev->nextthink = gpGlobals->time + MessageTime();
	SetThink(&CRevertSaved::MessageThink);
}

void CRevertSaved::MessageThink()
{
	UTIL_ShowMessageAll(STRING(pev->message));
	const float nextThink = LoadTime() - MessageTime();
	if (nextThink > 0)
	{
		pev->nextthink = gpGlobals->time + nextThink;
		SetThink(&CRevertSaved::LoadThink);
	}
	else
		LoadThink();
}

void CRevertSaved::LoadThink()
{
	if (g_pGameRules->AreSaveGamesSupported())
	{
		SERVER_COMMAND("reload\n");
	}
}
