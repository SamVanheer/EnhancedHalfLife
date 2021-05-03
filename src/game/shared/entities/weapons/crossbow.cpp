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

	int m_iTrail;

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
	pev->movetype = Movetype::Fly;
	pev->solid = Solid::BBox;

	pev->gravity = 0.5;

	SetModel("models/crossbow_bolt.mdl");

	SetAbsOrigin(pev->origin);
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
			pOther->TraceAttack({owner, gSkillData.plrDmgCrossbowClient, pev->velocity.Normalize(), tr, DMG_NEVERGIB});
		}
		else
		{
			pOther->TraceAttack({owner, gSkillData.plrDmgCrossbowMonster, pev->velocity.Normalize(), tr, DMG_BULLET | DMG_NEVERGIB});
		}

		ApplyMultiDamage(this, owner);

		pev->velocity = vec3_origin;
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
			Killed({this, GibType::Never});
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
			Vector vecDir = pev->velocity.Normalize();
			SetAbsOrigin(pev->origin - vecDir * 12);
			pev->angles = VectorAngles(vecDir);
			pev->solid = Solid::Not;
			pev->movetype = Movetype::Fly;
			pev->velocity = vec3_origin;
			pev->avelocity.z = 0;
			pev->angles.z = RANDOM_LONG(0, 360);
			pev->nextthink = gpGlobals->time + 10.0;
		}

		if (UTIL_PointContents(pev->origin) != Contents::Water)
		{
			UTIL_Sparks(pev->origin);
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

	UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);
}

void CCrossbowBolt::ExplodeThink()
{
	const Contents iContents = UTIL_PointContents(pev->origin);

	pev->dmg = 40;

	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
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

	::RadiusDamage(pev->origin, this, oldOwner, pev->dmg, 128, CLASS_NONE, DMG_BLAST | DMG_ALWAYSGIB);

	UTIL_Remove(this);
}
#endif

LINK_ENTITY_TO_CLASS(weapon_crossbow, CCrossbow);

void CCrossbow::Spawn()
{
	Precache();
	m_iId = WEAPON_CROSSBOW;
	SetModel("models/w_crossbow.mdl");

	m_iDefaultAmmo = CROSSBOW_DEFAULT_GIVE;

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
	PRECACHE_MODEL("models/w_crossbow.mdl");
	PRECACHE_MODEL("models/v_crossbow.mdl");
	PRECACHE_MODEL("models/p_crossbow.mdl");

	PRECACHE_SOUND("weapons/xbow_fire1.wav");
	PRECACHE_SOUND("weapons/xbow_reload1.wav");

	UTIL_PrecacheOther("crossbow_bolt");

	m_usCrossbow = PRECACHE_EVENT(1, "events/crossbow1.sc");
	m_usCrossbow2 = PRECACHE_EVENT(1, "events/crossbow2.sc");
}

bool CCrossbow::GetItemInfo(ItemInfo* p)
{
	p->pszName = GetClassname();
	p->pszAmmo1 = "bolts";
	p->iMaxAmmo1 = BOLT_MAX_CARRY;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = CROSSBOW_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 2;
	p->iId = WEAPON_CROSSBOW;
	p->iFlags = 0;
	p->iWeight = CROSSBOW_WEIGHT;
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
	m_fInReload = false;// cancel any reload in progress.

	if (m_hPlayer->m_iFOV != 0)
	{
		SecondaryAttack();
	}

	m_hPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	if (m_iClip)
		SendWeaponAnim(CROSSBOW_HOLSTER1);
	else
		SendWeaponAnim(CROSSBOW_HOLSTER2);
}

void CCrossbow::PrimaryAttack()
{
#ifdef CLIENT_DLL
	if (m_hPlayer->m_iFOV != 0 && bIsMultiplayer())
#else
	if (m_hPlayer->m_iFOV != 0 && g_pGameRules->IsMultiplayer())
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
	m_flNextPrimaryAttack = GetNextAttackDelay(0.75);

	if (m_iClip == 0)
	{
		PlayEmptySound();
		return;
	}


	m_hPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_iClip--;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	UTIL_PlaybackEvent(flags, m_hPlayer, m_usCrossbow2, {.iparam1 = m_iClip, .iparam2 = m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType]});

	// player "shoot" animation
	m_hPlayer->SetAnimation(PlayerAnim::Attack1);

	const Vector anglesAim = m_hPlayer->pev->v_angle + m_hPlayer->pev->punchangle;
	UTIL_MakeVectors(anglesAim);
	const Vector vecSrc = m_hPlayer->GetGunPosition() - gpGlobals->v_up * 2;
	const Vector vecDir = gpGlobals->v_forward;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + vecDir * WORLD_SIZE, IgnoreMonsters::No, m_hPlayer, &tr);

#ifndef CLIENT_DLL
	if (tr.pHit->v.takedamage)
	{
		ClearMultiDamage();
		CBaseEntity::Instance(tr.pHit)->TraceAttack({m_hPlayer, 120, vecDir, tr, DMG_BULLET | DMG_NEVERGIB});
		ApplyMultiDamage(this, m_hPlayer);
	}
#endif
}

void CCrossbow::FireBolt()
{
	if (m_iClip == 0)
	{
		PlayEmptySound();
		return;
	}

	m_hPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	m_iClip--;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	UTIL_PlaybackEvent(flags, m_hPlayer, m_usCrossbow, {.iparam1 = m_iClip, .iparam2 = m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType]});

	// player "shoot" animation
	m_hPlayer->SetAnimation(PlayerAnim::Attack1);

	Vector anglesAim = m_hPlayer->pev->v_angle + m_hPlayer->pev->punchangle;
	UTIL_MakeVectors(anglesAim);

	anglesAim.x = -anglesAim.x;
	const Vector vecSrc = m_hPlayer->GetGunPosition() - gpGlobals->v_up * 2;
	const Vector vecDir = gpGlobals->v_forward;

#ifndef CLIENT_DLL
	CCrossbowBolt* pBolt = CCrossbowBolt::BoltCreate();
	pBolt->pev->origin = vecSrc;
	pBolt->pev->angles = anglesAim;
	pBolt->SetOwner(m_hPlayer);

	if (m_hPlayer->pev->waterlevel == WaterLevel::Head)
	{
		pBolt->pev->velocity = vecDir * BOLT_WATER_VELOCITY;
		pBolt->pev->speed = BOLT_WATER_VELOCITY;
	}
	else
	{
		pBolt->pev->velocity = vecDir * BOLT_AIR_VELOCITY;
		pBolt->pev->speed = BOLT_AIR_VELOCITY;
	}
	pBolt->pev->avelocity.z = 10;
#endif

	if (!m_iClip && m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_hPlayer->SetSuitUpdate("!HEV_AMO0", SuitSoundType::Sentence, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.75);

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75;

	if (m_iClip != 0)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;
}

void CCrossbow::SecondaryAttack()
{
	if (m_hPlayer->m_iFOV != 0)
	{
		m_hPlayer->m_iFOV = 0; // 0 means reset to default fov
	}
	else if (m_hPlayer->m_iFOV != 20)
	{
		m_hPlayer->m_iFOV = 20;
	}

	pev->nextthink = UTIL_WeaponTimeBase() + 0.1;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
}

void CCrossbow::Reload()
{
	if (m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (m_hPlayer->m_iFOV != 0)
	{
		SecondaryAttack();
	}

	if (DefaultReload(CROSSBOW_MAX_CLIP, CROSSBOW_RELOAD, 4.5))
	{
		m_hPlayer->EmitSound(SoundChannel::Item, "weapons/xbow_reload1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 93 + RANDOM_LONG(0, 0xF));
	}
}

void CCrossbow::WeaponIdle()
{
	m_hPlayer->GetAutoaimVector(AUTOAIM_2DEGREES);  // get the autoaim vector but ignore it;  used for autoaim crosshair in DM

	ResetEmptySound();

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		float flRand = UTIL_SharedRandomFloat(m_hPlayer->random_seed, 0, 1);
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
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_hPlayer->random_seed, 10, 15);
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
	void Spawn() override
	{
		Precache();
		SetModel("models/w_crossbow_clip.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_crossbow_clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBasePlayer* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_CROSSBOWCLIP_GIVE, "bolts", BOLT_MAX_CARRY) != -1)
		{
			EmitSound(SoundChannel::Item, "items/9mmclip1.wav");
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_crossbow, CCrossbowAmmo);
