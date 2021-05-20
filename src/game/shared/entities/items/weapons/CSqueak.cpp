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
#include "CSoundEnt.hpp"
#include "gamerules.h"
#include "CSqueak.hpp"

LINK_ENTITY_TO_CLASS(weapon_snark, CSqueak);

void CSqueak::Spawn()
{
	Precache();
	FallInit();//get ready to fall down.

	pev->sequence = 1;
	pev->animtime = gpGlobals->time;
	pev->framerate = 1.0;
}

void CSqueak::Precache()
{
	CBaseWeapon::Precache();

	PRECACHE_MODEL("models/w_sqknest.mdl");
	PRECACHE_MODEL("models/v_squeak.mdl");
	PRECACHE_MODEL("models/p_squeak.mdl");
	PRECACHE_SOUND("squeek/sqk_hunt2.wav");
	PRECACHE_SOUND("squeek/sqk_hunt3.wav");
	UTIL_PrecacheOther("monster_snark");

	m_usSnarkFire = PRECACHE_EVENT(1, "events/snarkfire.sc");
}

bool CSqueak::GetWeaponInfo(WeaponInfo& p)
{
	p.pszName = GetClassname();
	p.pszAmmo1 = "Snarks";
	p.iMaxAmmo1 = SNARK_MAX_CARRY;
	p.pszAmmo2 = nullptr;
	p.iMaxAmmo2 = -1;
	p.iMaxClip = WEAPON_NOCLIP;
	p.iSlot = 4;
	p.iPosition = 3;
	p.iId = m_iId = WEAPON_SNARK;
	p.iWeight = SNARK_WEIGHT;
	p.iFlags = WEAPON_FLAG_LIMITINWORLD | WEAPON_FLAG_EXHAUSTIBLE;

	return true;
}

bool CSqueak::Deploy()
{
	auto player = m_hPlayer.Get();

	// play hunt sound
	const float flRndSound = RANDOM_FLOAT(0, 1);

	if (flRndSound <= 0.5)
		EmitSound(SoundChannel::Voice, "squeek/sqk_hunt2.wav");
	else
		EmitSound(SoundChannel::Voice, "squeek/sqk_hunt3.wav");

	player->m_iWeaponVolume = QUIET_GUN_VOLUME;

	const bool result = DefaultDeploy("models/v_squeak.mdl", "models/p_squeak.mdl", SQUEAK_UP, "squeak");

	if (result)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7;
	}

	return result;
}

void CSqueak::Holster()
{
	auto player = m_hPlayer.Get();

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if (!player->m_rgAmmo[m_iPrimaryAmmoType])
	{
		player->pev->weapons &= ~(1 << WEAPON_SNARK);
		SetThink(&CSqueak::DestroyWeapon);
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}

	SendWeaponAnim(SQUEAK_DOWN);
	player->EmitSound(SoundChannel::Weapon, "common/null.wav");
}

void CSqueak::PrimaryAttack()
{
	auto player = m_hPlayer.Get();

	if (player->m_rgAmmo[m_iPrimaryAmmoType])
	{
		UTIL_MakeVectors(player->pev->v_angle);

		// HACK HACK:  Ugly hacks to handle change in origin based on new physics code for players
		// Move origin up if crouched and start trace a bit outside of body ( 20 units instead of 16 )
		Vector trace_origin = player->GetAbsOrigin();
		if (player->pev->flags & FL_DUCKING)
		{
			trace_origin = trace_origin - (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);
		}

		// find place to toss monster
		TraceResult tr;
		UTIL_TraceLine(trace_origin + gpGlobals->v_forward * 20, trace_origin + gpGlobals->v_forward * 64, IgnoreMonsters::No, nullptr, &tr);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		UTIL_PlaybackEvent(flags, player, m_usSnarkFire);

		if (tr.fAllSolid == 0 && tr.fStartSolid == 0 && tr.flFraction > 0.25)
		{
			// player "shoot" animation
			player->SetAnimation(PlayerAnim::Attack1);

#ifndef CLIENT_DLL
			CBaseEntity* pSqueak = CBaseEntity::Create("monster_snark", tr.vecEndPos, player->pev->v_angle, player);
			pSqueak->SetAbsVelocity(gpGlobals->v_forward * 200 + player->GetAbsVelocity());
#endif

			// play hunt sound
			const float flRndSound = RANDOM_FLOAT(0, 1);

			if (flRndSound <= 0.5)
				EmitSound(SoundChannel::Voice, "squeek/sqk_hunt2.wav", VOL_NORM, ATTN_NORM, 105);
			else
				EmitSound(SoundChannel::Voice, "squeek/sqk_hunt3.wav", VOL_NORM, ATTN_NORM, 105);

			player->m_iWeaponVolume = QUIET_GUN_VOLUME;

			player->m_rgAmmo[m_iPrimaryAmmoType]--;

			m_fJustThrown = true;

			m_flNextPrimaryAttack = GetNextAttackDelay(0.3);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
		}
	}
}

void CSqueak::SecondaryAttack()
{
}

void CSqueak::WeaponIdle()
{
	auto player = m_hPlayer.Get();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_fJustThrown)
	{
		m_fJustThrown = false;

		if (!player->m_rgAmmo[PrimaryAmmoIndex()])
		{
			RetireWeapon();
			return;
		}

		SendWeaponAnim(SQUEAK_UP);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
		return;
	}

	int iAnim;
	const float flRand = UTIL_SharedRandomFloat(player->random_seed, 0, 1);
	if (flRand <= 0.75)
	{
		iAnim = SQUEAK_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16 * (2);
	}
	else if (flRand <= 0.875)
	{
		iAnim = SQUEAK_FIDGETFIT;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70.0 / 16.0;
	}
	else
	{
		iAnim = SQUEAK_FIDGETNIP;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 80.0 / 16.0;
	}
	SendWeaponAnim(iAnim);
}
