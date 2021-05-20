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
#include "effects/CBeam.hpp"
#include "effects/CSprite.hpp"
#include "gamerules.h"
#include "CTripmine.hpp"

constexpr int	TRIPMINE_PRIMARY_VOLUME = 450;

LINK_ENTITY_TO_CLASS(weapon_tripmine, CTripmine);

void CTripmine::Spawn()
{
	Precache();
	pev->frame = 0;
	pev->body = 3;
	pev->sequence = TRIPMINE_GROUND;
	// ResetSequenceInfo( );
	pev->framerate = 0;

	FallInit();// get ready to fall down

#ifdef CLIENT_DLL
	if (!bIsMultiplayer())
#else
	if (!g_pGameRules->IsMultiplayer())
#endif
	{
		SetSize(Vector(-16, -16, 0), Vector(16, 16, 28));
	}
}

void CTripmine::Precache()
{
	CBaseWeapon::Precache();

	PRECACHE_MODEL("models/v_tripmine.mdl");
	PRECACHE_MODEL("models/p_tripmine.mdl");
	UTIL_PrecacheOther("monster_tripmine");

	m_usTripFire = PRECACHE_EVENT(1, "events/tripfire.sc");
}

bool CTripmine::GetWeaponInfo(WeaponInfo& p)
{
	p.pszName = GetClassname();
	p.pszAmmo1 = "Trip Mine";
	p.iMaxAmmo1 = TRIPMINE_MAX_CARRY;
	p.pszAmmo2 = nullptr;
	p.iMaxAmmo2 = -1;
	p.iMaxClip = WEAPON_NOCLIP;
	p.iSlot = 4;
	p.iPosition = 2;
	p.iId = m_iId = WEAPON_TRIPMINE;
	p.iWeight = TRIPMINE_WEIGHT;
	p.iFlags = WEAPON_FLAG_LIMITINWORLD | WEAPON_FLAG_EXHAUSTIBLE;

	return true;
}

bool CTripmine::Deploy()
{
	pev->body = 0;
	return DefaultDeploy("models/v_tripmine.mdl", "models/p_tripmine.mdl", TRIPMINE_DRAW, "trip");
}

void CTripmine::Holster()
{
	auto player = m_hPlayer.Get();

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if (!player->m_rgAmmo[m_iPrimaryAmmoType])
	{
		// out of mines
		player->pev->weapons &= ~(1 << WEAPON_TRIPMINE);
		SetThink(&CTripmine::DestroyWeapon);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	SendWeaponAnim(TRIPMINE_HOLSTER);
	player->EmitSound(SoundChannel::Weapon, "common/null.wav");
}

void CTripmine::PrimaryAttack()
{
	auto player = m_hPlayer.Get();

	if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	UTIL_MakeVectors(player->pev->v_angle + player->pev->punchangle);
	const Vector vecSrc = player->GetGunPosition();
	const Vector vecAiming = gpGlobals->v_forward;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + vecAiming * 128, IgnoreMonsters::No, player, &tr);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	UTIL_PlaybackEvent(flags, player, m_usTripFire);

	if (tr.flFraction < 1.0)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
		if (pEntity && !(pEntity->pev->flags & FL_CONVEYOR))
		{
			const Vector angles = VectorAngles(tr.vecPlaneNormal);

			CBaseEntity* pEnt = CBaseEntity::Create("monster_tripmine", tr.vecEndPos + tr.vecPlaneNormal * 8, angles, player);

			player->m_rgAmmo[m_iPrimaryAmmoType]--;

			// player "shoot" animation
			player->SetAnimation(PlayerAnim::Attack1);

			if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			{
				// no more mines! 
				RetireWeapon();
				return;
			}
		}
		else
		{
			// ALERT( at_console, "no deploy\n" );
		}
	}
	else
	{
	}

	m_flNextPrimaryAttack = GetNextAttackDelay(0.3);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
}

void CTripmine::WeaponIdle()
{
	auto player = m_hPlayer.Get();

	//If we're here then we're in a player's inventory, and need to use this body
	pev->body = 0;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (player->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		SendWeaponAnim(TRIPMINE_DRAW);
	}
	else
	{
		RetireWeapon();
		return;
	}

	int iAnim;
	const float flRand = UTIL_SharedRandomFloat(player->random_seed, 0, 1);
	if (flRand <= 0.25)
	{
		iAnim = TRIPMINE_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 30.0;
	}
	else if (flRand <= 0.75)
	{
		iAnim = TRIPMINE_IDLE2;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 30.0;
	}
	else
	{
		iAnim = TRIPMINE_FIDGET;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 100.0 / 30.0;
	}

	SendWeaponAnim(iAnim);
}
