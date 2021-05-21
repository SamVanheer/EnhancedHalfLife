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

#include "cbase.hpp"
#include "CBasePlayer.hpp"
#include "CBaseWeapon.hpp"

void CBasePlayer::SelectLastWeapon()
{
	if (!m_hLastWeapon)
	{
		return;
	}

	auto activeWeapon = m_hActiveWeapon.Get();

	if (activeWeapon && !activeWeapon->CanHolster())
	{
		return;
	}

	ResetAutoaim();

	// FIX, this needs to queue them up and delay
	if (activeWeapon)
		activeWeapon->Holster();

	CBaseWeapon* pTemp = activeWeapon;
	activeWeapon = m_hActiveWeapon = m_hLastWeapon;
	m_hLastWeapon = pTemp;
	activeWeapon->Deploy();
	activeWeapon->UpdateWeaponInfo();
}

void CBasePlayer::SelectWeapon(const char* pstr)
{
	if (!pstr)
		return;

	CBaseWeapon* weapon = nullptr;

#ifndef CLIENT_DLL
	for (int i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		weapon = m_hPlayerWeapons[i];

		while (weapon)
		{
			if (weapon->ClassnameIs(pstr))
				break;
			weapon = weapon->m_hNext;
		}

		if (weapon)
			break;
	}
#endif

	if (!weapon)
		return;

	if (weapon == m_hActiveWeapon)
		return;

#ifndef CLIENT_DLL
	ResetAutoaim();
#endif

	// FIX, this needs to queue them up and delay
	if (auto activeWeapon = m_hActiveWeapon.Get(); activeWeapon)
		activeWeapon->Holster();

	m_hLastWeapon = m_hActiveWeapon;
	m_hActiveWeapon = weapon;

	if (auto activeWeapon = m_hActiveWeapon.Get(); activeWeapon)
	{
		activeWeapon->Deploy();
#ifndef CLIENT_DLL
		activeWeapon->UpdateWeaponInfo();
#endif
	}
}
