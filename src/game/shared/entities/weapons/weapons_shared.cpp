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

#ifdef CLIENT_DLL
#include "hud.h"
#include "cl_util.h"
#include "com_weapons.h"

extern int g_irunninggausspred;
#endif

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

void CBasePlayerWeapon::SendWeaponAnim(int iAnim, int body)
{
	auto player = m_hPlayer.Get();

	player->pev->weaponanim = iAnim;

#ifdef CLIENT_DLL
	HUD_SendWeaponAnim(iAnim, body, false);
#else
#if defined( CLIENT_WEAPONS )
	const bool skiplocal = UseDecrement();

	if (skiplocal && g_engfuncs.pfnCanSkipPlayer(player->edict()))
		return;
#endif

	MESSAGE_BEGIN(MessageDest::One, SVC_WEAPONANIM, player);
	WRITE_BYTE(iAnim);						// sequence number
	WRITE_BYTE(pev->body);					// weaponmodel bodygroup.
	MESSAGE_END();
#endif
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

void CBasePlayerWeapon::Holster()
{
	auto player = m_hPlayer.Get();

	m_fInReload = false; // cancel any reload in progress.
	player->pev->viewmodel = iStringNull;
	player->pev->weaponmodel = iStringNull;

#ifdef CLIENT_DLL
	g_irunninggausspred = false;
#endif
}

int CBasePlayerWeapon::PrimaryAmmoIndex()
{
	return m_iPrimaryAmmoType;
}

int CBasePlayerWeapon::SecondaryAmmoIndex()
{
	return m_iSecondaryAmmoType;
}

bool CBasePlayerWeapon::DefaultDeploy(const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, int body)
{
	if (!CanDeploy())
		return false;

	auto player = m_hPlayer.Get();

	player->pev->viewmodel = MAKE_STRING(szViewModel);
	player->pev->weaponmodel = MAKE_STRING(szWeaponModel);
	safe_strcpy(player->m_szAnimExtension, szAnimExt);
	SendWeaponAnim(iAnim, body);

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
	m_flLastFireTime = 0.0;

#ifdef CLIENT_DLL
	LoadVModel(szViewModel, player);
	g_irunninggausspred = false;
#endif

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

bool CBasePlayerWeapon::PlayEmptySound()
{
	if (m_iPlayEmptySound)
	{
#ifdef CLIENT_DLL
		HUD_PlaySound("weapons/357_cock1.wav", 0.8);
#else
		m_hPlayer->EmitSound(SoundChannel::Weapon, "weapons/357_cock1.wav", 0.8);
#endif
		m_iPlayEmptySound = false;
		return false; //TODO: incorrect?
	}
	return false;
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
