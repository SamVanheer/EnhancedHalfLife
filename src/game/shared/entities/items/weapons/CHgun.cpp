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
#include "aliens/CHornet.hpp"
#include "gamerules.h"
#include "UserMessages.h"
#include "CHgun.hpp"

enum firemode_e
{
	FIREMODE_TRACK = 0,
	FIREMODE_FAST
};

LINK_ENTITY_TO_CLASS(weapon_hornetgun, CHgun);

bool CHgun::IsUseable()
{
	return true;
}

void CHgun::Spawn()
{
	Precache();
	
	m_iFirePhase = 0;

	FallInit();// get ready to fall down.
}

void CHgun::Precache()
{
	CBaseWeapon::Precache();

	PRECACHE_MODEL("models/v_hgun.mdl");
	PRECACHE_MODEL("models/w_hgun.mdl");
	PRECACHE_MODEL("models/p_hgun.mdl");

	m_usHornetFire = PRECACHE_EVENT(1, "events/firehornet.sc");

	UTIL_PrecacheOther("hornet");
}

bool CHgun::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBaseWeapon::AddToPlayer(pPlayer))
	{

#ifndef CLIENT_DLL
		if (g_pGameRules->IsMultiplayer())
		{
			// in multiplayer, all hivehands come full. 
			pPlayer->m_rgAmmo[PrimaryAmmoIndex()] = HORNET_MAX_CARRY;
		}
#endif

		MESSAGE_BEGIN(MessageDest::One, gmsgWeapPickup, pPlayer);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return true;
	}
	return false;
}

bool CHgun::GetWeaponInfo(WeaponInfo& p)
{
	p.pszName = GetClassname();
	p.pszAmmo1 = "Hornets";
	p.iMaxAmmo1 = HORNET_MAX_CARRY;
	p.pszAmmo2 = nullptr;
	p.iMaxAmmo2 = -1;
	p.iMaxClip = WEAPON_NOCLIP;
	p.iSlot = 3;
	p.iPosition = 3;
	p.iId = m_iId = WEAPON_HORNETGUN;
	p.iFlags = WEAPON_FLAG_NOAUTOSWITCHEMPTY | WEAPON_FLAG_NOAUTORELOAD;
	p.iWeight = HORNETGUN_WEIGHT;

	return true;
}

bool CHgun::Deploy()
{
	return DefaultDeploy("models/v_hgun.mdl", "models/p_hgun.mdl", HGUN_UP, "hive");
}

void CHgun::Holster()
{
	auto player = m_hPlayer.Get();

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(HGUN_DOWN);

	//!!!HACKHACK - can't select hornetgun if it's empty! no way to get ammo for it, either.
	if (!player->m_rgAmmo[PrimaryAmmoIndex()])
	{
		player->m_rgAmmo[PrimaryAmmoIndex()] = 1;
	}
}

void CHgun::PrimaryAttack()
{
	auto player = m_hPlayer.Get();

	Reload();

	if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		return;
	}

#ifndef CLIENT_DLL
	UTIL_MakeVectors(player->pev->v_angle);

	CBaseEntity* pHornet = CBaseEntity::Create("hornet", player->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -12, player->pev->v_angle, player);
	pHornet->SetAbsVelocity(gpGlobals->v_forward * 300);

	m_flRechargeTime = gpGlobals->time + 0.5;
#endif

	player->m_rgAmmo[m_iPrimaryAmmoType]--;


	player->m_iWeaponVolume = QUIET_GUN_VOLUME;
	player->m_iWeaponFlash = DIM_GUN_FLASH;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	UTIL_PlaybackEvent(flags, player, m_usHornetFire, {.iparam1 = FIREMODE_TRACK});

	// player "shoot" animation
	player->SetAnimation(PlayerAnim::Attack1);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.25);

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.25;
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
}

void CHgun::SecondaryAttack()
{
	auto player = m_hPlayer.Get();

	Reload();

	if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		return;
	}

	//Wouldn't be a bad idea to completely predict these, since they fly so fast...
#ifndef CLIENT_DLL
	UTIL_MakeVectors(player->pev->v_angle);

	Vector vecSrc = player->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -12;

	m_iFirePhase++;
	switch (m_iFirePhase)
	{
	case 1:
		vecSrc = vecSrc + gpGlobals->v_up * 8;
		break;
	case 2:
		vecSrc = vecSrc + gpGlobals->v_up * 8;
		vecSrc = vecSrc + gpGlobals->v_right * 8;
		break;
	case 3:
		vecSrc = vecSrc + gpGlobals->v_right * 8;
		break;
	case 4:
		vecSrc = vecSrc + gpGlobals->v_up * -8;
		vecSrc = vecSrc + gpGlobals->v_right * 8;
		break;
	case 5:
		vecSrc = vecSrc + gpGlobals->v_up * -8;
		break;
	case 6:
		vecSrc = vecSrc + gpGlobals->v_up * -8;
		vecSrc = vecSrc + gpGlobals->v_right * -8;
		break;
	case 7:
		vecSrc = vecSrc + gpGlobals->v_right * -8;
		break;
	case 8:
		vecSrc = vecSrc + gpGlobals->v_up * 8;
		vecSrc = vecSrc + gpGlobals->v_right * -8;
		m_iFirePhase = 0;
		break;
	}

	CBaseEntity* pHornet = CBaseEntity::Create("hornet", vecSrc, player->pev->v_angle, player);
	pHornet->SetAbsVelocity(gpGlobals->v_forward * 1200);
	pHornet->SetAbsAngles(VectorAngles(pHornet->GetAbsVelocity()));

	pHornet->SetThink(&CHornet::StartDart);

	m_flRechargeTime = gpGlobals->time + 0.5;
#endif

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	UTIL_PlaybackEvent(flags, player, m_usHornetFire, {.iparam1 = FIREMODE_FAST});

	player->m_rgAmmo[m_iPrimaryAmmoType]--;
	player->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	player->m_iWeaponFlash = DIM_GUN_FLASH;

	// player "shoot" animation
	player->SetAnimation(PlayerAnim::Attack1);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
}

void CHgun::Reload()
{
	auto player = m_hPlayer.Get();

	if (player->m_rgAmmo[m_iPrimaryAmmoType] >= HORNET_MAX_CARRY)
		return;

	while (player->m_rgAmmo[m_iPrimaryAmmoType] < HORNET_MAX_CARRY && m_flRechargeTime < gpGlobals->time)
	{
		player->m_rgAmmo[m_iPrimaryAmmoType]++;
		m_flRechargeTime += 0.5;
	}
}

void CHgun::WeaponIdle()
{
	auto player = m_hPlayer.Get();

	Reload();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	const float flRand = UTIL_SharedRandomFloat(player->random_seed, 0, 1);
	if (flRand <= 0.75)
	{
		iAnim = HGUN_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16 * (2);
	}
	else if (flRand <= 0.875)
	{
		iAnim = HGUN_FIDGETSWAY;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
	}
	else
	{
		iAnim = HGUN_FIDGETSHAKE;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 35.0 / 16.0;
	}
	SendWeaponAnim(iAnim);
}
