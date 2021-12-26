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

#include "extdll.hpp"
#include "util.hpp"
#include "cbase.hpp"
#include "CBaseMonster.monsters.hpp"
#include "weapons.hpp"
#include "CBasePlayer.hpp"
#include "gamerules.hpp"
#include "UserMessages.hpp"
#include "CRpg.hpp"

#ifndef CLIENT_DLL
#include "CRpgRocket.hpp"
#endif

void CRpg::Reload()
{
	auto player = GetPlayerOwner();

	if (m_iClip == 1)
	{
		// don't bother with any of this if don't need to reload.
		return;
	}

	if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	// because the RPG waits to autoreload when no missiles are active while  the LTD is on, the
	// weapons code is constantly calling into this function, but is often denied because 
	// a) missiles are in flight, but the LTD is on
	// or
	// b) player is totally out of ammo and has nothing to switch to, and should be allowed to
	//    shine the designator around
	//
	// Set the next attack time into the future so that WeaponIdle will get called more often
	// than reload, allowing the RPG LTD to be updated

	m_flNextPrimaryAttack = GetNextAttackDelay(0.5);

	if (m_cActiveRockets && m_fSpotActive)
	{
		// no reloading when there are active missiles tracking the designator.
		// ward off future autoreload attempts by setting next attack time into the future for a bit. 
		return;
	}

#ifndef CLIENT_DLL
	if (m_hSpot && m_fSpotActive)
	{
		m_hSpot->Suspend(2.1);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.1;
	}
#endif

	if (m_iClip == 0)
	{
		if (DefaultReload(RPG_MAX_CLIP, RPG_RELOAD, 2))
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
	}
}

void CRpg::OnRemove()
{
#ifndef CLIENT_DLL
	m_hSpot.Remove();
#endif
	CBaseWeapon::OnRemove();
}

void CRpg::Spawn()
{
	Precache();

	m_fSpotActive = true;

	FallInit();// get ready to fall down.
}

void CRpg::Precache()
{
	CBaseWeapon::Precache();

	PRECACHE_MODEL("models/w_rpg.mdl");
	PRECACHE_MODEL("models/v_rpg.mdl");
	PRECACHE_MODEL("models/p_rpg.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	UTIL_PrecacheOther("laser_spot");
	UTIL_PrecacheOther("rpg_rocket");

	PRECACHE_SOUND("weapons/rocketfire1.wav");
	PRECACHE_SOUND("weapons/glauncher.wav"); // alternative fire sound

	m_usRpg = PRECACHE_EVENT(1, "events/rpg.sc");
}

bool CRpg::GetWeaponInfo(WeaponInfo& p)
{
	p.pszName = GetClassname();
	p.pszAmmo1 = "rockets";
	p.iMaxAmmo1 = ROCKET_MAX_CARRY;
	p.pszAmmo2 = nullptr;
	p.iMaxAmmo2 = -1;
	p.iMaxClip = RPG_MAX_CLIP;
	p.iSlot = 3;
	p.iPosition = 0;
	p.iId = m_iId = WEAPON_RPG;
	p.iFlags = 0;
	p.iWeight = RPG_WEIGHT;

	return true;
}

bool CRpg::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBaseWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MessageDest::One, gmsgWeapPickup, pPlayer);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return true;
	}
	return false;
}

bool CRpg::Deploy()
{
	if (m_iClip == 0)
	{
		return DefaultDeploy("models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW_UL, "rpg");
	}

	return DefaultDeploy("models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW1, "rpg");
}

bool CRpg::CanHolster()
{
	if (m_fSpotActive && m_cActiveRockets)
	{
		// can't put away while guiding a missile.
		return false;
	}

	return true;
}

void CRpg::Holster()
{
	auto player = GetPlayerOwner();

	m_fInReload = false;// cancel any reload in progress.

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(RPG_HOLSTER1);

#ifndef CLIENT_DLL
	m_hSpot.Remove();
#endif
}

void CRpg::PrimaryAttack()
{
	if (m_iClip)
	{
		auto player = GetPlayerOwner();

		player->m_iWeaponVolume = LOUD_GUN_VOLUME;
		player->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		// player "shoot" animation
		player->SetAnimation(PlayerAnim::Attack1);

		UTIL_MakeVectors(player->pev->v_angle);
		Vector vecSrc = player->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;

		CRpgRocket* pRocket = CRpgRocket::CreateRpgRocket(vecSrc, player->pev->v_angle, player, this);

		UTIL_MakeVectors(player->pev->v_angle);// RpgRocket::Create stomps on globals, so remake.
		pRocket->SetAbsVelocity(pRocket->GetAbsVelocity() + gpGlobals->v_forward * DotProduct(player->GetAbsVelocity(), gpGlobals->v_forward));
#endif

		// firing RPG no longer turns on the designator. ALT fire is a toggle switch for the LTD.
		// Ken signed up for this as a global change (sjb)

		int flags;
#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		UTIL_PlaybackEvent(flags, player, m_usRpg);

		m_iClip--;

		m_flNextPrimaryAttack = GetNextAttackDelay(1.5);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
	}
	else
	{
		PlayEmptySound();
	}
	UpdateSpot();
}

void CRpg::SecondaryAttack()
{
	m_fSpotActive = !m_fSpotActive;

#ifndef CLIENT_DLL
	if (!m_fSpotActive)
	{
		m_hSpot.Remove();
	}
#endif

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
}

void CRpg::WeaponIdle()
{
	auto player = GetPlayerOwner();

	UpdateSpot();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (player->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;
		const float flRand = UTIL_SharedRandomFloat(player->random_seed, 0, 1);
		if (flRand <= 0.75 || m_fSpotActive)
		{
			if (m_iClip == 0)
				iAnim = RPG_IDLE_UL;
			else
				iAnim = RPG_IDLE;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 15.0;
		}
		else
		{
			if (m_iClip == 0)
				iAnim = RPG_FIDGET_UL;
			else
				iAnim = RPG_FIDGET;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.1;
		}

		ResetEmptySound();
		SendWeaponAnim(iAnim);
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
	}
}

void CRpg::UpdateSpot()
{
#ifndef CLIENT_DLL
	if (m_fSpotActive)
	{
		auto player = GetPlayerOwner();

		if (!m_hSpot)
		{
			m_hSpot = CLaserSpot::CreateSpot();
		}

		UTIL_MakeVectors(player->pev->v_angle);
		const Vector vecSrc = player->GetGunPosition();
		const Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecSrc + vecAiming * WORLD_SIZE, IgnoreMonsters::No, player, &tr);

		m_hSpot->SetAbsOrigin(tr.vecEndPos);
	}
#endif
}
