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
#include "monsters.h"
#include "weapons.h"
#include "CBasePlayer.hpp"
#include "weaponinfo.h"
#include "CGrenade.hpp"
#include "CHandGrenade.hpp"

constexpr int HANDGRENADE_PRIMARY_VOLUME = 450;

LINK_ENTITY_TO_CLASS(weapon_handgrenade, CHandGrenade);

void CHandGrenade::Spawn()
{
	Precache();

#ifndef CLIENT_DLL
	pev->dmg = gSkillData.plrDmgHandGrenade;
#endif

	FallInit();// get ready to fall down.
}

void CHandGrenade::Precache()
{
	CBaseWeapon::Precache();

	PRECACHE_MODEL("models/w_grenade.mdl");
	PRECACHE_MODEL("models/v_grenade.mdl");
	PRECACHE_MODEL("models/p_grenade.mdl");
}

bool CHandGrenade::GetWeaponInfo(WeaponInfo& p)
{
	p.pszName = GetClassname();
	p.pszAmmo1 = "Hand Grenade";
	p.iMaxAmmo1 = HANDGRENADE_MAX_CARRY;
	p.pszAmmo2 = nullptr;
	p.iMaxAmmo2 = -1;
	p.iMaxClip = WEAPON_NOCLIP;
	p.iSlot = 4;
	p.iPosition = 0;
	p.iId = m_iId = WEAPON_HANDGRENADE;
	p.iWeight = HANDGRENADE_WEIGHT;
	p.iFlags = WEAPON_FLAG_LIMITINWORLD | WEAPON_FLAG_EXHAUSTIBLE;

	return true;
}

bool CHandGrenade::Deploy()
{
	m_flReleaseThrow = -1;
	return DefaultDeploy("models/v_grenade.mdl", "models/p_grenade.mdl", HANDGRENADE_DRAW, "crowbar");
}

bool CHandGrenade::CanHolster()
{
	// can only holster hand grenades when not primed!
	return m_flStartThrow == 0;
}

void CHandGrenade::Holster()
{
	auto player = m_hPlayer.Get();

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if (player->m_rgAmmo[m_iPrimaryAmmoType])
	{
		SendWeaponAnim(HANDGRENADE_HOLSTER);
	}
	else
	{
		// no more grenades!
		player->pev->weapons &= ~(1 << WEAPON_HANDGRENADE);
		SetThink(&CHandGrenade::DestroyWeapon);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	player->EmitSound(SoundChannel::Weapon, "common/null.wav");
}

void CHandGrenade::PrimaryAttack()
{
	auto player = m_hPlayer.Get();

	if (!m_flStartThrow && player->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0;

		SendWeaponAnim(HANDGRENADE_PINPULL);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CHandGrenade::WeaponIdle()
{
	auto player = m_hPlayer.Get();

	if (m_flReleaseThrow == 0 && m_flStartThrow)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flStartThrow)
	{
		Vector angThrow = player->pev->v_angle + player->pev->punchangle;

		if (angThrow.x < 0)
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0f);
		else
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0f);

		const float flVel = std::min(500.0f, (90 - angThrow.x) * 4);

		UTIL_MakeVectors(angThrow);

		Vector vecSrc = player->GetAbsOrigin() + player->pev->view_ofs + gpGlobals->v_forward * 16;

		Vector vecThrow = gpGlobals->v_forward * flVel + player->GetAbsVelocity();

		// alway explode 3 seconds after the pin was pulled
		const float time = std::max(0.0f, m_flStartThrow - gpGlobals->time + 3.0f);

		CGrenade::ShootTimed(player, vecSrc, vecThrow, time);

		if (flVel < 500)
		{
			SendWeaponAnim(HANDGRENADE_THROW1);
		}
		else if (flVel < 1000)
		{
			SendWeaponAnim(HANDGRENADE_THROW2);
		}
		else
		{
			SendWeaponAnim(HANDGRENADE_THROW3);
		}

		// player "shoot" animation
		player->SetAnimation(PlayerAnim::Attack1);

		//m_flReleaseThrow = 0;
		m_flStartThrow = 0;
		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;

		player->m_rgAmmo[m_iPrimaryAmmoType]--;

		if (!player->m_rgAmmo[m_iPrimaryAmmoType])
		{
			// just threw last grenade
			// set attack times in the future, and weapon idle in the future so we can see the whole throw
			// animation, weapon idle will automatically retire the weapon for us.
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = GetNextAttackDelay(0.5);// ensure that the animation can finish playing
		}
		return;
	}
	else if (m_flReleaseThrow > 0)
	{
		// we've finished the throw, restart.
		m_flStartThrow = 0;

		if (player->m_rgAmmo[m_iPrimaryAmmoType])
		{
			SendWeaponAnim(HANDGRENADE_DRAW);
		}
		else
		{
			RetireWeapon();
			return;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
		m_flReleaseThrow = -1;
		return;
	}

	if (player->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;
		const float flRand = UTIL_SharedRandomFloat(player->random_seed, 0, 1);
		if (flRand <= 0.75)
		{
			iAnim = HANDGRENADE_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);// how long till we do this again.
		}
		else
		{
			iAnim = HANDGRENADE_FIDGET;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
		}

		SendWeaponAnim(iAnim);
	}
}

void CHandGrenade::GetWeaponData(weapon_data_t& data)
{
	data.fuser2 = m_flStartThrow;
	data.fuser3 = m_flReleaseThrow;
}

void CHandGrenade::SetWeaponData(const weapon_data_t& data)
{
	m_flStartThrow = data.fuser2;
	m_flReleaseThrow = data.fuser3;
}
