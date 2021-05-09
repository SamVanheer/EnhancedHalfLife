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
#include "UserMessages.h"

#ifndef CLIENT_DLL
constexpr int BOLT_AIR_VELOCITY = 2000;
constexpr int BOLT_WATER_VELOCITY = 1000;

// UNDONE: Save/restore this?  Don't forget to set classname and LINK_ENTITY_TO_CLASS()
// 
// OVERLOADS SOME ENTVARS:
//
// speed - the ideal magnitude of my velocity
class CCrossbowBolt : public CBaseEntity
{
	void Spawn() override;
	void Precache() override;
	int  Classify() override;
	void EXPORT BubbleThink();
	void EXPORT BoltTouch(CBaseEntity* pOther);
	void EXPORT ExplodeThink();

	int m_iTrail = 0;

public:
	static CCrossbowBolt* BoltCreate();
};
LINK_ENTITY_TO_CLASS(crossbow_bolt, CCrossbowBolt);

CCrossbowBolt* CCrossbowBolt::BoltCreate()
{
	// Create a new entity with CCrossbowBolt private data
	CCrossbowBolt* pBolt = GetClassPtr((CCrossbowBolt*)nullptr);
	pBolt->SetClassname("bolt");
	pBolt->Spawn();

	return pBolt;
}

void CCrossbowBolt::Spawn()
{
	Precache();
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	pev->gravity = 0.5;

	SetModel("models/crossbow_bolt.mdl");

	SetAbsOrigin(GetAbsOrigin());
	SetSize(vec3_origin, vec3_origin);

	SetTouch(&CCrossbowBolt::BoltTouch);
	SetThink(&CCrossbowBolt::BubbleThink);
	pev->nextthink = gpGlobals->time + 0.2;
}

void CCrossbowBolt::Precache()
{
	PRECACHE_MODEL("models/crossbow_bolt.mdl");
	PRECACHE_SOUND("weapons/xbow_hitbod1.wav");
	PRECACHE_SOUND("weapons/xbow_hitbod2.wav");
	PRECACHE_SOUND("weapons/xbow_fly1.wav");
	PRECACHE_SOUND("weapons/xbow_hit1.wav");
	PRECACHE_SOUND("fvox/beep.wav");
	m_iTrail = PRECACHE_MODEL("sprites/streak.spr");
}

int	CCrossbowBolt::Classify()
{
	return	CLASS_NONE;
}

void CCrossbowBolt::BoltTouch(CBaseEntity* pOther)
{
	SetTouch(nullptr);
	SetThink(nullptr);

	if (pOther->pev->takedamage)
	{
		TraceResult tr = UTIL_GetGlobalTrace();

		auto owner = GetOwner();

		ClearMultiDamage();

		if (pOther->IsPlayer())
		{
			pOther->TraceAttack({owner, gSkillData.plrDmgCrossbowClient, GetAbsVelocity().Normalize(), tr, DMG_NEVERGIB});
		}
		else
		{
			pOther->TraceAttack({owner, gSkillData.plrDmgCrossbowMonster, GetAbsVelocity().Normalize(), tr, DMG_BULLET | DMG_NEVERGIB});
		}

		ApplyMultiDamage(this, owner);

		SetAbsVelocity(vec3_origin);
		// play body "thwack" sound
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EmitSound(SoundChannel::Body, "weapons/xbow_hitbod1.wav"); break;
		case 1:
			EmitSound(SoundChannel::Body, "weapons/xbow_hitbod2.wav"); break;
		}

		if (!g_pGameRules->IsMultiplayer())
		{
			Killed({this, this, GibType::Never});
		}
	}
	else
	{
		EmitSound(SoundChannel::Body, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 98 + RANDOM_LONG(0, 7));

		SetThink(&CCrossbowBolt::SUB_Remove);
		pev->nextthink = gpGlobals->time;// this will get changed below if the bolt is allowed to stick in what it hit.

		if (pOther->ClassnameIs("worldspawn"))
		{
			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity().Normalize();
			SetAbsOrigin(GetAbsOrigin() - vecDir * 12);
			SetSolidType(Solid::Not);
			SetMovetype(Movetype::Fly);
			SetAbsVelocity(vec3_origin);
			pev->avelocity.z = 0;

			Vector myAngles = VectorAngles(vecDir);
			myAngles.z = RANDOM_LONG(0, 360);
			SetAbsAngles(myAngles);

			pev->nextthink = gpGlobals->time + 10.0;
		}

		if (UTIL_PointContents(GetAbsOrigin()) != Contents::Water)
		{
			UTIL_Sparks(GetAbsOrigin());
		}
	}

	if (g_pGameRules->IsMultiplayer())
	{
		SetThink(&CCrossbowBolt::ExplodeThink);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CCrossbowBolt::BubbleThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->waterlevel == WaterLevel::Dry)
		return;

	UTIL_BubbleTrail(GetAbsOrigin() - GetAbsVelocity() * 0.1, GetAbsOrigin(), 1);
}

void CCrossbowBolt::ExplodeThink()
{
	const Contents iContents = UTIL_PointContents(GetAbsOrigin());

	pev->dmg = 40;

	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, GetAbsOrigin());
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(GetAbsOrigin().x);
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	if (iContents != Contents::Water)
	{
		WRITE_SHORT(g_sModelIndexFireball);
	}
	else
	{
		WRITE_SHORT(g_sModelIndexWExplosion);
	}
	WRITE_BYTE(10); // scale * 10
	WRITE_BYTE(15); // framerate
	WRITE_BYTE(TE_EXPLFLAG_NONE);
	MESSAGE_END();

	auto oldOwner = GetOwner();

	SetOwner(nullptr); // can't traceline attack owner if this is set

	::RadiusDamage(GetAbsOrigin(), this, oldOwner, pev->dmg, 128, CLASS_NONE, DMG_BLAST | DMG_ALWAYSGIB);

	UTIL_Remove(this);
}
#endif

LINK_ENTITY_TO_CLASS(weapon_crossbow, CCrossbow);

void CCrossbow::Spawn()
{
	Precache();
	FallInit();// get ready to fall down.
}

bool CCrossbow::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
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
	CBasePlayerWeapon::Precache();

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

class CCrossbowAmmo : public CBasePlayerAmmo
{
public:
	void OnConstruct() override
	{
		CBasePlayerAmmo::OnConstruct();
		SetModelName("models/w_crossbow_clip.mdl");
		m_iAmount = AMMO_CROSSBOWCLIP_GIVE;
		m_iszAmmoName = MAKE_STRING("bolts");
	}
};
LINK_ENTITY_TO_CLASS(ammo_crossbow, CCrossbowAmmo);
