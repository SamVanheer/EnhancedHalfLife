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

#include "CGamePlayerHurt.hpp"

void CGamePlayerHurt::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	if (info.GetActivator()->IsPlayer())
	{
		if (pev->dmg < 0)
			info.GetActivator()->GiveHealth(-pev->dmg, DMG_GENERIC);
		else
			info.GetActivator()->TakeDamage({this, this, pev->dmg, DMG_GENERIC});
	}

	SUB_UseTargets(info.GetActivator(), info.GetUseType(), info.GetValue());

	if (RemoveOnFire())
	{
		UTIL_Remove(this);
	}
}
