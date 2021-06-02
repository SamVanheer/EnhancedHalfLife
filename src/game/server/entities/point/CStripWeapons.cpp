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

#pragma once

#include "CStripWeapons.hpp"

void CStripWeapons::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "strip_weapons"))
	{
		if (atoi(pkvd->szValue) != 0)
		{
			SetBits(m_Flags, WEAPONSTRIP_REMOVEWEAPONS);
		}
		else
		{
			ClearBits(m_Flags, WEAPONSTRIP_REMOVEWEAPONS);
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "strip_suit"))
	{
		if (atoi(pkvd->szValue) != 0)
		{
			SetBits(m_Flags, WEAPONSTRIP_REMOVESUIT);
		}
		else
		{
			ClearBits(m_Flags, WEAPONSTRIP_REMOVESUIT);
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "strip_longjump"))
	{
		if (atoi(pkvd->szValue) != 0)
		{
			SetBits(m_Flags, WEAPONSTRIP_REMOVELONGJUMP);
		}
		else
		{
			ClearBits(m_Flags, WEAPONSTRIP_REMOVELONGJUMP);
		}

		pkvd->fHandled = true;
	}
	else
	{
		CPointEntity::KeyValue(pkvd);
	}
}

void CStripWeapons::Use(const UseInfo& info)
{
	CBasePlayer* pPlayer = nullptr;

	if (auto activator = info.GetActivator(); activator && activator->IsPlayer())
	{
		pPlayer = (CBasePlayer*)activator;
	}
	else if (!g_pGameRules->IsMultiplayer())
	{
		pPlayer = UTIL_GetLocalPlayer();
	}

	if (pPlayer)
	{
		if (m_Flags & WEAPONSTRIP_REMOVEWEAPONS)
		{
			pPlayer->RemoveAllItems(false);
		}

		if (m_Flags & WEAPONSTRIP_REMOVESUIT)
		{
			pPlayer->SetHasSuit(false);
		}

		if (m_Flags & WEAPONSTRIP_REMOVELONGJUMP)
		{
			pPlayer->SetHasLongJump(false);
		}
	}
}
