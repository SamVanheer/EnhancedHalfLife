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

#include "CBaseAmmo.hpp"

void CBaseAmmo::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "ammo_amount"))
	{
		m_iAmount = std::max(0, atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "pickup_sound"))
	{
		m_iszPickupSound = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
	{
		CBaseItem::KeyValue(pkvd);
	}
}

void CBaseAmmo::Precache()
{
	CBaseItem::Precache();

	if (!IsStringNull(m_iszPickupSound))
	{
		PRECACHE_SOUND(STRING(m_iszPickupSound));
	}
}

ItemApplyResult CBaseAmmo::DefaultGiveAmmo(CBasePlayer* player, int amount, const char* ammoName, bool playSound)
{
	if (ammoName && *ammoName && player->GiveAmmo(amount, ammoName) != -1)
	{
		if (auto sound = STRING(m_iszPickupSound); playSound && *sound)
		{
			EmitSound(SoundChannel::Item, sound);
		}

		return {ItemApplyAction::Used};
	}
	return {ItemApplyAction::NotUsed};
}

ItemApplyResult CBaseAmmo::Apply(CBasePlayer* player)
{
	return DefaultGiveAmmo(player, m_iAmount, STRING(m_iszAmmoName), true);
}
