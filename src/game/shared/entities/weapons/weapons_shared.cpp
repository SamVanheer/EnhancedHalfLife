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
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"

int giAmmoIndex = 0;

// Precaches the ammo and queues the ammo info for sending to clients
void AddAmmoNameToAmmoRegistry(const char* szAmmoname)
{
	// make sure it's not already in the registry
	for (int i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if (!CBasePlayerItem::AmmoInfoArray[i].pszName)
			continue;

		if (stricmp(CBasePlayerItem::AmmoInfoArray[i].pszName, szAmmoname) == 0)
			return; // ammo already in registry, just quite
	}

	giAmmoIndex++;
	ASSERT(giAmmoIndex < MAX_AMMO_TYPES);
	if (giAmmoIndex >= MAX_AMMO_TYPES)
		giAmmoIndex = 0;

	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].pszName = szAmmoname;
	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].iId = giAmmoIndex;   // yes, this info is redundant
}

bool CBasePlayerWeapon::CanDeploy()
{
	bool bHasAmmo = false;

	if (!Ammo1Name())
	{
		// this weapon doesn't use ammo, can always deploy.
		return true;
	}

	if (Ammo1Name())
	{
		bHasAmmo |= (m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0);
	}
	if (Ammo2Name())
	{
		bHasAmmo |= (m_hPlayer->m_rgAmmo[m_iSecondaryAmmoType] != 0);
	}
	if (m_iClip > 0)
	{
		bHasAmmo |= true;
	}
	if (!bHasAmmo)
	{
		return false;
	}

	return true;
}

bool CBasePlayerWeapon::DefaultReload(int iClipSize, int iAnim, float fDelay, int body)
{
	if (m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return false;

	const int j = std::min(iClipSize - m_iClip, m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

	if (j == 0)
		return false;

	m_hPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + fDelay;

	//!!UNDONE -- reload sound goes here !!!
	SendWeaponAnim(iAnim, body);

	m_fInReload = true;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
	return true;
}

void CBasePlayerWeapon::ResetEmptySound()
{
	m_iPlayEmptySound = true;
}

bool CanAttack(float attack_time, float curtime, bool isPredicted)
{
#if defined( CLIENT_WEAPONS )
	if (!isPredicted)
#else
	if (1)
#endif
	{
		return attack_time <= curtime;
	}
	else
	{
		return (static_cast<int>(std::floor(attack_time * 1000.0)) * 1000.0) <= 0.0;
	}
}

void CBasePlayerWeapon::ItemPostFrame()
{
	auto player = m_hPlayer.Get();

	if ((m_fInReload) && (player->m_flNextAttack <= UTIL_WeaponTimeBase()))
	{
		// complete the reload. 
		const int j = std::min(MaxClip() - m_iClip, player->m_rgAmmo[m_iPrimaryAmmoType]);

		// Add them to the clip
		m_iClip += j;
		player->m_rgAmmo[m_iPrimaryAmmoType] -= j;

		m_fInReload = false;
	}

	if (!(player->pev->button & IN_ATTACK))
	{
		m_flLastFireTime = 0.0f;
	}

	if ((player->pev->button & IN_ATTACK2) && CanAttack(m_flNextSecondaryAttack, gpGlobals->time, UseDecrement()))
	{
		if (Ammo2Name() && !player->m_rgAmmo[SecondaryAmmoIndex()])
		{
			m_fFireOnEmpty = true;
		}

		SecondaryAttack();
		player->pev->button &= ~IN_ATTACK2;
	}
	else if ((player->pev->button & IN_ATTACK) && CanAttack(m_flNextPrimaryAttack, gpGlobals->time, UseDecrement()))
	{
		if ((m_iClip == 0 && Ammo1Name()) || (MaxClip() == -1 && !player->m_rgAmmo[PrimaryAmmoIndex()]))
		{
			m_fFireOnEmpty = true;
		}

		PrimaryAttack();
	}
	else if (player->pev->button & IN_RELOAD && MaxClip() != WEAPON_NOCLIP && !m_fInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}
	else if (!(player->pev->button & (IN_ATTACK | IN_ATTACK2)))
	{
		// no fire buttons down

		m_fFireOnEmpty = false;

#ifndef CLIENT_DLL
		if (!IsUseable() && m_flNextPrimaryAttack < (UseDecrement() ? 0.0 : gpGlobals->time))
		{
			// weapon isn't useable, switch.
			if (!(Flags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && g_pGameRules->GetNextBestWeapon(player, this))
			{
				m_flNextPrimaryAttack = (UseDecrement() ? 0.0 : gpGlobals->time) + 0.3;
				return;
			}
		}
		else
#endif
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if (m_iClip == 0 && !(Flags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < (UseDecrement() ? 0.0 : gpGlobals->time))
			{
				Reload();
				return;
			}
		}

		WeaponIdle();
		return;
	}

	// catch all
	if (ShouldWeaponIdle())
	{
		WeaponIdle();
	}
}

void CBasePlayer::SelectLastItem()
{
	if (!m_hLastItem)
	{
		return;
	}

	auto activeItem = m_hActiveItem.Get();

	if (activeItem && !activeItem->CanHolster())
	{
		return;
	}

	ResetAutoaim();

	// FIX, this needs to queue them up and delay
	if (activeItem)
		activeItem->Holster();

	CBasePlayerItem* pTemp = activeItem;
	activeItem = m_hActiveItem = m_hLastItem;
	m_hLastItem = pTemp;
	activeItem->Deploy();
	activeItem->UpdateItemInfo();
}
