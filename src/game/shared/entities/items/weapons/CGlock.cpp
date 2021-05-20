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
#include "CBaseMonster.monsters.hpp"
#include "weapons.h"
#include "CBasePlayer.hpp"
#include "CGlock.hpp"

LINK_ENTITY_TO_CLASS(weapon_glock, CGlock);
LINK_ENTITY_TO_CLASS(weapon_9mmhandgun, CGlock);

void CGlock::Spawn()
{
	SetClassname("weapon_9mmhandgun"); // hack to allow for old names
	Precache();
	FallInit();// get ready to fall down.
}

void CGlock::Precache()
{
	CBaseWeapon::Precache();

	PRECACHE_MODEL("models/v_9mmhandgun.mdl");
	PRECACHE_MODEL("models/w_9mmhandgun.mdl");
	PRECACHE_MODEL("models/p_9mmhandgun.mdl");

	m_iShell = PRECACHE_MODEL("models/shell.mdl");// brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND("weapons/pl_gun1.wav");//silenced handgun
	PRECACHE_SOUND("weapons/pl_gun2.wav");//silenced handgun
	PRECACHE_SOUND("weapons/pl_gun3.wav");//handgun

	m_usFireGlock1 = PRECACHE_EVENT(1, "events/glock1.sc");
	m_usFireGlock2 = PRECACHE_EVENT(1, "events/glock2.sc");
}

bool CGlock::GetWeaponInfo(WeaponInfo& p)
{
	p.pszName = GetClassname();
	p.pszAmmo1 = "9mm";
	p.iMaxAmmo1 = _9MM_MAX_CARRY;
	p.pszAmmo2 = nullptr;
	p.iMaxAmmo2 = -1;
	p.iMaxClip = GLOCK_MAX_CLIP;
	p.iSlot = 1;
	p.iPosition = 0;
	p.iFlags = 0;
	p.iId = m_iId = WEAPON_GLOCK;
	p.iWeight = GLOCK_WEIGHT;

	return true;
}

bool CGlock::Deploy()
{
	// pev->body = 1;
	return DefaultDeploy("models/v_9mmhandgun.mdl", "models/p_9mmhandgun.mdl", GLOCK_DRAW, "onehanded");
}

void CGlock::SecondaryAttack()
{
	GlockFire(0.1, 0.2, false);
}

void CGlock::PrimaryAttack()
{
	GlockFire(0.01, 0.3, true);
}

void CGlock::GlockFire(float flSpread, float flCycleTime, bool fUseAutoAim)
{
	auto player = m_hPlayer.Get();

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = GetNextAttackDelay(0.2);
		}

		return;
	}

	m_iClip--;

	player->pev->effects = (int)(player->pev->effects) | EF_MUZZLEFLASH;

	int flags;

#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	// player "shoot" animation
	player->SetAnimation(PlayerAnim::Attack1);

	// silenced
	if (pev->body == 1)
	{
		player->m_iWeaponVolume = QUIET_GUN_VOLUME;
		player->m_iWeaponFlash = DIM_GUN_FLASH;
	}
	else
	{
		// non-silenced
		player->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		player->m_iWeaponFlash = NORMAL_GUN_FLASH;
	}

	const Vector vecSrc = player->GetGunPosition();
	Vector vecAiming;

	if (fUseAutoAim)
	{
		vecAiming = player->GetAutoaimVector(AUTOAIM_10DEGREES);
	}
	else
	{
		vecAiming = gpGlobals->v_forward;
	}

	const Vector vecDir = player->FireBulletsPlayer(1, vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), WORLD_SIZE, BULLET_PLAYER_9MM, 0);

	UTIL_PlaybackEvent(flags, player, fUseAutoAim ? m_usFireGlock1 : m_usFireGlock2, {.fparam1 = vecDir.x, .fparam2 = vecDir.y, .bparam1 = m_iClip == 0});

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(flCycleTime);

	if (!m_iClip && player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		player->SetSuitUpdate("!HEV_AMO0", SuitSoundType::Sentence, 0);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
}

void CGlock::Reload()
{
	auto player = m_hPlayer.Get();

	if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	int iResult;

	if (m_iClip == 0)
		iResult = DefaultReload(GLOCK_MAX_CLIP, GLOCK_RELOAD, 1.5);
	else
		iResult = DefaultReload(GLOCK_MAX_CLIP, GLOCK_RELOAD_NOT_EMPTY, 1.5);

	if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
	}
}

void CGlock::WeaponIdle()
{
	auto player = m_hPlayer.Get();

	ResetEmptySound();

	player->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		const float flRand = UTIL_SharedRandomFloat(player->random_seed, 0.0, 1.0);

		if (flRand <= 0.3 + 0 * 0.75)
		{
			iAnim = GLOCK_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16;
		}
		else if (flRand <= 0.6 + 0 * 0.875)
		{
			iAnim = GLOCK_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
		}
		else
		{
			iAnim = GLOCK_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
		}
		SendWeaponAnim(iAnim);
	}
}
