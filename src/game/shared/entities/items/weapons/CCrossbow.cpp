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
#include "CCrossbow.hpp"

#ifndef CLIENT_DLL
#include "CCrossbowBolt.hpp"
#endif

void CCrossbow::Spawn()
{
	Precache();
	FallInit();// get ready to fall down.
}

bool CCrossbow::AddToPlayer(CBasePlayer* pPlayer)
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

void CCrossbow::Precache()
{
	CBaseWeapon::Precache();

	PRECACHE_MODEL("models/w_crossbow.mdl");
	PRECACHE_MODEL("models/v_crossbow.mdl");
	PRECACHE_MODEL("models/p_crossbow.mdl");

	PRECACHE_SOUND("weapons/xbow_fire1.wav");
	PRECACHE_SOUND("weapons/xbow_reload1.wav");

	UTIL_PrecacheOther("crossbow_bolt");

	m_usCrossbow = PRECACHE_EVENT(1, "events/crossbow1.sc");
	m_usCrossbow2 = PRECACHE_EVENT(1, "events/crossbow2.sc");
}

bool CCrossbow::GetWeaponInfo(WeaponInfo& p)
{
	p.pszName = GetClassname();
	p.pszAmmo1 = "bolts";
	p.iMaxAmmo1 = BOLT_MAX_CARRY;
	p.pszAmmo2 = nullptr;
	p.iMaxAmmo2 = -1;
	p.iMaxClip = CROSSBOW_MAX_CLIP;
	p.iSlot = 2;
	p.iPosition = 2;
	p.iId = WEAPON_CROSSBOW;
	p.iFlags = 0;
	p.iWeight = CROSSBOW_WEIGHT;
	return true;
}

bool CCrossbow::Deploy()
{
	if (m_iClip)
		return DefaultDeploy("models/v_crossbow.mdl", "models/p_crossbow.mdl", CROSSBOW_DRAW1, "bow");
	return DefaultDeploy("models/v_crossbow.mdl", "models/p_crossbow.mdl", CROSSBOW_DRAW2, "bow");
}

void CCrossbow::Holster()
{
	auto player = m_hPlayer.Get();

	m_fInReload = false;// cancel any reload in progress.

	if (player->m_iFOV != 0)
	{
		SecondaryAttack();
	}

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	if (m_iClip)
		SendWeaponAnim(CROSSBOW_HOLSTER1);
	else
		SendWeaponAnim(CROSSBOW_HOLSTER2);
}

void CCrossbow::PrimaryAttack()
{
	auto player = m_hPlayer.Get();

#ifdef CLIENT_DLL
	if (player->m_iFOV != 0 && bIsMultiplayer())
#else
	if (player->m_iFOV != 0 && g_pGameRules->IsMultiplayer())
#endif
	{
		FireSniperBolt();
		return;
	}

	FireBolt();
}

// this function only gets called in multiplayer
void CCrossbow::FireSniperBolt()
{
	auto player = m_hPlayer.Get();

	m_flNextPrimaryAttack = GetNextAttackDelay(0.75);

	if (m_iClip == 0)
	{
		PlayEmptySound();
		return;
	}

	player->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_iClip--;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	UTIL_PlaybackEvent(flags, player, m_usCrossbow2, {.iparam1 = m_iClip, .iparam2 = player->m_rgAmmo[m_iPrimaryAmmoType]});

	// player "shoot" animation
	player->SetAnimation(PlayerAnim::Attack1);

	const Vector anglesAim = player->pev->v_angle + player->pev->punchangle;
	UTIL_MakeVectors(anglesAim);
	const Vector vecSrc = player->GetGunPosition() - gpGlobals->v_up * 2;
	const Vector vecDir = gpGlobals->v_forward;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + vecDir * WORLD_SIZE, IgnoreMonsters::No, player, &tr);

#ifndef CLIENT_DLL
	if (tr.pHit->v.takedamage)
	{
		ClearMultiDamage();
		CBaseEntity::Instance(tr.pHit)->TraceAttack({player, 120, vecDir, tr, DMG_BULLET | DMG_NEVERGIB});
		ApplyMultiDamage(this, player);
	}
#endif
}

void CCrossbow::FireBolt()
{
	auto player = m_hPlayer.Get();

	if (m_iClip == 0)
	{
		PlayEmptySound();
		return;
	}

	player->m_iWeaponVolume = QUIET_GUN_VOLUME;

	m_iClip--;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	UTIL_PlaybackEvent(flags, player, m_usCrossbow, {.iparam1 = m_iClip, .iparam2 = player->m_rgAmmo[m_iPrimaryAmmoType]});

	// player "shoot" animation
	player->SetAnimation(PlayerAnim::Attack1);

	Vector anglesAim = player->pev->v_angle + player->pev->punchangle;
	UTIL_MakeVectors(anglesAim);

	anglesAim.x = -anglesAim.x;
	const Vector vecSrc = player->GetGunPosition() - gpGlobals->v_up * 2;
	const Vector vecDir = gpGlobals->v_forward;

#ifndef CLIENT_DLL
	CCrossbowBolt* pBolt = CCrossbowBolt::BoltCreate();
	pBolt->SetAbsOrigin(vecSrc);
	pBolt->SetAbsAngles(anglesAim);
	pBolt->SetOwner(player);

	if (player->pev->waterlevel == WaterLevel::Head)
	{
		pBolt->SetAbsVelocity(vecDir * BOLT_WATER_VELOCITY);
		pBolt->pev->speed = BOLT_WATER_VELOCITY;
	}
	else
	{
		pBolt->SetAbsVelocity(vecDir * BOLT_AIR_VELOCITY);
		pBolt->pev->speed = BOLT_AIR_VELOCITY;
	}
	pBolt->pev->avelocity.z = 10;
#endif

	if (!m_iClip && player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		player->SetSuitUpdate("!HEV_AMO0", SuitSoundType::Sentence, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.75);

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75;

	if (m_iClip != 0)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;
}

void CCrossbow::SecondaryAttack()
{
	auto player = m_hPlayer.Get();

	if (player->m_iFOV != 0)
	{
		player->m_iFOV = 0; // 0 means reset to default fov
	}
	else if (player->m_iFOV != 20)
	{
		player->m_iFOV = 20;
	}

	pev->nextthink = UTIL_WeaponTimeBase() + 0.1;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
}

void CCrossbow::Reload()
{
	auto player = m_hPlayer.Get();

	if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (player->m_iFOV != 0)
	{
		SecondaryAttack();
	}

	if (DefaultReload(CROSSBOW_MAX_CLIP, CROSSBOW_RELOAD, 4.5))
	{
		player->EmitSound(SoundChannel::Item, "weapons/xbow_reload1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 93 + RANDOM_LONG(0, 0xF));
	}
}

void CCrossbow::WeaponIdle()
{
	auto player = m_hPlayer.Get();

	player->GetAutoaimVector(AUTOAIM_2DEGREES);  // get the autoaim vector but ignore it;  used for autoaim crosshair in DM

	ResetEmptySound();

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		float flRand = UTIL_SharedRandomFloat(player->random_seed, 0, 1);
		if (flRand <= 0.75)
		{
			if (m_iClip)
			{
				SendWeaponAnim(CROSSBOW_IDLE1);
			}
			else
			{
				SendWeaponAnim(CROSSBOW_IDLE2);
			}
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
		}
		else
		{
			if (m_iClip)
			{
				SendWeaponAnim(CROSSBOW_FIDGET1);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 30.0;
			}
			else
			{
				SendWeaponAnim(CROSSBOW_FIDGET2);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 80.0 / 30.0;
			}
		}
	}
}
