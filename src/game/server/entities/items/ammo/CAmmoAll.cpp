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

#include "CAmmoAll.hpp"
#include "CBaseWeapon.hpp"

void CAmmoAll::KeyValue(KeyValueData* pkvd)
{
	//Override this to allow -1 for refill
	if (AreStringsEqual(pkvd->szKeyName, "ammo_amount"))
	{
		m_iAmount = std::max(AMMOALL_REFILLAMMO, atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else
	{
		CBaseAmmo::KeyValue(pkvd);
	}
}

ItemApplyResult CAmmoAll::Apply(CBasePlayer* player)
{
	ItemApplyAction action = ItemApplyAction::NotUsed;

	for (int i = 1; i < MAX_AMMO_TYPES; ++i)
	{
		const auto& info = CBaseWeapon::AmmoInfoArray[i];

		if (info.pszName)
		{
			if (DefaultGiveAmmo(player, m_iAmount == AMMOALL_REFILLAMMO ? info.MaxCarry : m_iAmount, info.pszName, false).Action != ItemApplyAction::NotUsed)
			{
				action = ItemApplyAction::Used;
			}
		}
	}

	if (action != ItemApplyAction::NotUsed)
	{
		if (auto sound = STRING(m_iszPickupSound); *sound)
		{
			EmitSound(SoundChannel::Item, sound);
		}
	}

	return {action};
}
