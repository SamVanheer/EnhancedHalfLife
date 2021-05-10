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
#include "player.h"
#include "gamerules.h"

constexpr int CROWBAR_BODYHIT_VOLUME = 128;
constexpr int CROWBAR_WALLHIT_VOLUME = 512;

LINK_ENTITY_TO_CLASS(weapon_crowbar, CCrowbar);

void CCrowbar::Spawn()
{
	Precache();
	FallInit();// get ready to fall down.
}

void CCrowbar::Precache()
{
	CBaseWeapon::Precache();

	PRECACHE_MODEL("models/v_crowbar.mdl");
	PRECACHE_MODEL("models/w_crowbar.mdl");
	PRECACHE_MODEL("models/p_crowbar.mdl");
	PRECACHE_SOUND("weapons/cbar_hit1.wav");
	PRECACHE_SOUND("weapons/cbar_hit2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	m_usCrowbar = PRECACHE_EVENT(1, "events/crowbar.sc");
}

bool CCrowbar::GetWeaponInfo(WeaponInfo& p)
{
	p.pszName = GetClassname();
	p.pszAmmo1 = nullptr;
	p.iMaxAmmo1 = -1;
	p.pszAmmo2 = nullptr;
	p.iMaxAmmo2 = -1;
	p.iMaxClip = WEAPON_NOCLIP;
	p.iSlot = 0;
	p.iPosition = 0;
	p.iId = WEAPON_CROWBAR;
	p.iWeight = CROWBAR_WEIGHT;
	return true;
}

bool CCrowbar::Deploy()
{
	return DefaultDeploy("models/v_crowbar.mdl", "models/p_crowbar.mdl", CROWBAR_DRAW, "crowbar");
}

void CCrowbar::Holster()
{
	auto player = m_hPlayer.Get();

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(CROWBAR_HOLSTER);
}

void FindHullIntersection(const Vector& vecSrc, TraceResult& tr, const Vector& mins, const Vector& maxs, CBaseEntity* pEntity)
{
	const Vector vecHullEnd = vecSrc + ((tr.vecEndPos - vecSrc) * 2);

	TraceResult tmpTrace;
	UTIL_TraceLine(vecSrc, vecHullEnd, IgnoreMonsters::No, pEntity, &tmpTrace);

	if (tmpTrace.flFraction < 1.0)
	{
		tr = tmpTrace;
		return;
	}

	const Vector* const minmaxs[2] = {&mins, &maxs};

	float distance = 1e6f;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				const Vector vecEnd
				{
					vecHullEnd.x + minmaxs[i]->x,
					vecHullEnd.y + minmaxs[j]->y,
					vecHullEnd.z + minmaxs[k]->z
				};

				UTIL_TraceLine(vecSrc, vecEnd, IgnoreMonsters::No, pEntity, &tmpTrace);
				if (tmpTrace.flFraction < 1.0)
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if (thisDistance < distance)
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}

void CCrowbar::PrimaryAttack()
{
	if (!Swing(true))
	{
		SetThink(&CCrowbar::SwingAgain);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CCrowbar::Smack()
{
	DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
}

void CCrowbar::SwingAgain()
{
	Swing(false);
}

bool CCrowbar::Swing(bool fFirst)
{
	auto player = m_hPlayer.Get();

	UTIL_MakeVectors(player->pev->v_angle);
	const Vector vecSrc = player->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, IgnoreMonsters::No, player, &tr);

#ifndef CLIENT_DLL
	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, IgnoreMonsters::No, Hull::Head, player, &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			if (CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit); !pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, player);
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	UTIL_PlaybackEvent(FEV_NOTHOST, player, m_usCrowbar);

	bool fDidHit = false;

	if (tr.flFraction >= 1.0)
	{
		if (fFirst)
		{
			// miss
			m_flNextPrimaryAttack = GetNextAttackDelay(0.5);

			// player "shoot" animation
			player->SetAnimation(PlayerAnim::Attack1);
		}
	}
	else
	{
		switch (((m_iSwing++) % 2) + 1)
		{
		case 0:
			SendWeaponAnim(CROWBAR_ATTACK1HIT); break;
		case 1:
			SendWeaponAnim(CROWBAR_ATTACK2HIT); break;
		case 2:
			SendWeaponAnim(CROWBAR_ATTACK3HIT); break;
		}

		// player "shoot" animation
		player->SetAnimation(PlayerAnim::Attack1);

#ifndef CLIENT_DLL

		// hit
		fDidHit = true;
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage();

		if ((m_flNextPrimaryAttack + 1 < UTIL_WeaponTimeBase()) || g_pGameRules->IsMultiplayer())
		{
			// first swing does full damage
			pEntity->TraceAttack({player, gSkillData.plrDmgCrowbar, gpGlobals->v_forward, tr, DMG_CLUB});
		}
		else
		{
			// subsequent swings do half
			pEntity->TraceAttack({player, gSkillData.plrDmgCrowbar / 2, gpGlobals->v_forward, tr, DMG_CLUB});
		}
		ApplyMultiDamage(player, player);

#endif

		m_flNextPrimaryAttack = GetNextAttackDelay(0.25);

#ifndef CLIENT_DLL
		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool fHitWorld = true;

		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				// play thwack or smack sound
				switch (RANDOM_LONG(0, 2))
				{
				case 0:
					player->EmitSound(SoundChannel::Item, "weapons/cbar_hitbod1.wav"); break;
				case 1:
					player->EmitSound(SoundChannel::Item, "weapons/cbar_hitbod2.wav"); break;
				case 2:
					player->EmitSound(SoundChannel::Item, "weapons/cbar_hitbod3.wav"); break;
				}
				player->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
					return true;
				else
					flVol = 0.1;

				fHitWorld = false;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (fHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

			if (g_pGameRules->IsMultiplayer())
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play crowbar strike
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				player->EmitSound(SoundChannel::Item, "weapons/cbar_hit1.wav", fvolbar, ATTN_NORM, 98 + RANDOM_LONG(0, 3));
				break;
			case 1:
				player->EmitSound(SoundChannel::Item, "weapons/cbar_hit2.wav", fvolbar, ATTN_NORM, 98 + RANDOM_LONG(0, 3));
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		player->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;
#endif
		SetThink(&CCrowbar::Smack);
		pev->nextthink = gpGlobals->time + 0.2;
	}
	return fDidHit;
}