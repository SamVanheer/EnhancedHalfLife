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
#include "soundent.h"
#include "gamerules.h"

enum w_squeak_e
{
	WSQUEAK_IDLE1 = 0,
	WSQUEAK_FIDGET,
	WSQUEAK_JUMP,
	WSQUEAK_RUN,
};

#ifndef CLIENT_DLL
class CSqueakGrenade : public CGrenade
{
	void Spawn() override;
	void Precache() override;
	int  Classify() override;
	void EXPORT SuperBounceTouch(CBaseEntity* pOther);
	void EXPORT HuntThink();
	int  BloodColor() override { return BLOOD_COLOR_YELLOW; }
	void Killed(const KilledInfo& info) override;
	void GibMonster() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	static inline float m_flNextBounceSoundTime = 0;

	// CBaseEntity *m_pTarget;
	float m_flDie;
	Vector m_vecTarget;
	float m_flNextHunt;
	float m_flNextHit;
	Vector m_posPrev;
	EHANDLE m_hOwner;
	int  m_iMyClass;
};

LINK_ENTITY_TO_CLASS(monster_snark, CSqueakGrenade);
TYPEDESCRIPTION	CSqueakGrenade::m_SaveData[] =
{
	DEFINE_FIELD(CSqueakGrenade, m_flDie, FIELD_TIME),
	DEFINE_FIELD(CSqueakGrenade, m_vecTarget, FIELD_VECTOR),
	DEFINE_FIELD(CSqueakGrenade, m_flNextHunt, FIELD_TIME),
	DEFINE_FIELD(CSqueakGrenade, m_flNextHit, FIELD_TIME),
	DEFINE_FIELD(CSqueakGrenade, m_posPrev, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CSqueakGrenade, m_hOwner, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CSqueakGrenade, CGrenade);

constexpr float SQUEEK_DETONATE_DELAY = 15.0;

int CSqueakGrenade::Classify()
{
	if (m_iMyClass != 0)
		return m_iMyClass; // protect against recursion

	if (m_hEnemy != nullptr)
	{
		m_iMyClass = CLASS_INSECT; // no one cares about it
		switch (m_hEnemy->Classify())
		{
		case CLASS_PLAYER:
		case CLASS_HUMAN_PASSIVE:
		case CLASS_HUMAN_MILITARY:
			m_iMyClass = 0;
			return CLASS_ALIEN_MILITARY; // barney's get mad, grunts get mad at it
		}
		m_iMyClass = 0;
	}

	return CLASS_ALIEN_BIOWEAPON;
}

void CSqueakGrenade::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Bounce);
	SetSolidType(Solid::BBox);

	SetModel("models/w_squeak.mdl");
	SetSize(Vector(-4, -4, 0), Vector(4, 4, 8));
	SetAbsOrigin(pev->origin);

	SetTouch(&CSqueakGrenade::SuperBounceTouch);
	SetThink(&CSqueakGrenade::HuntThink);
	pev->nextthink = gpGlobals->time + 0.1;
	m_flNextHunt = gpGlobals->time + 1E6;

	pev->flags |= FL_MONSTER;
	SetDamageMode(DamageMode::Aim);
	pev->health = gSkillData.snarkHealth;
	pev->gravity = 0.5;
	pev->friction = 0.5;

	pev->dmg = gSkillData.snarkDmgPop;

	m_flDie = gpGlobals->time + SQUEEK_DETONATE_DELAY;

	m_flFieldOfView = 0; // 180 degrees

	if (auto owner = GetOwner(); owner)
		m_hOwner = owner;

	m_flNextBounceSoundTime = gpGlobals->time;// reset each time a snark is spawned.

	pev->sequence = WSQUEAK_RUN;
	ResetSequenceInfo();
}

void CSqueakGrenade::Precache()
{
	PRECACHE_MODEL("models/w_squeak.mdl");
	PRECACHE_SOUND("squeek/sqk_blast1.wav");
	PRECACHE_SOUND("common/bodysplat.wav");
	PRECACHE_SOUND("squeek/sqk_die1.wav");
	PRECACHE_SOUND("squeek/sqk_hunt1.wav");
	PRECACHE_SOUND("squeek/sqk_hunt2.wav");
	PRECACHE_SOUND("squeek/sqk_hunt3.wav");
	PRECACHE_SOUND("squeek/sqk_deploy1.wav");
}

void CSqueakGrenade::Killed(const KilledInfo& info)
{
	pev->model = iStringNull;// make invisible
	SetThink(&CSqueakGrenade::SUB_Remove);
	SetTouch(nullptr);
	pev->nextthink = gpGlobals->time + 0.1;

	// since squeak grenades never leave a body behind, clear out their takedamage now.
	// Squeaks do a bit of radius damage when they pop, and that radius damage will
	// continue to call this function unless we acknowledge the Squeak's death now. (sjb)
	SetDamageMode(DamageMode::No);

	// play squeek blast
	EmitSound(SoundChannel::Item, "squeek/sqk_blast1.wav", VOL_NORM, 0.5, PITCH_NORM);

	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, SMALL_EXPLOSION_VOLUME, 3.0);

	UTIL_BloodDrips(pev->origin, vec3_origin, BloodColor(), 80);

	if (m_hOwner != nullptr)
		RadiusDamage(this, m_hOwner, pev->dmg, CLASS_NONE, DMG_BLAST);
	else
		RadiusDamage(this, this, pev->dmg, CLASS_NONE, DMG_BLAST);

	// reset owner so death message happens
	if (m_hOwner != nullptr)
		SetOwner(m_hOwner);

	CBaseMonster::Killed({info.GetAttacker(), GibType::Always});
}

void CSqueakGrenade::GibMonster()
{
	EmitSound(SoundChannel::Voice, "common/bodysplat.wav", 0.75, ATTN_NORM, 200);
}

void CSqueakGrenade::HuntThink()
{
	// ALERT( at_console, "think\n" );

	if (!IsInWorld())
	{
		SetTouch(nullptr);
		UTIL_Remove(this);
		return;
	}

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	// explode when ready
	if (gpGlobals->time >= m_flDie)
	{
		g_vecAttackDir = pev->velocity.Normalize();
		pev->health = -1;
		Killed({this, GibType::Normal});
		return;
	}

	// float
	if (pev->waterlevel != WaterLevel::Dry)
	{
		if (GetMovetype() == Movetype::Bounce)
		{
			SetMovetype(Movetype::Fly);
		}
		pev->velocity = pev->velocity * 0.9;
		pev->velocity.z += 8.0;
	}
	else //if (SetMovetype(Movetype::Fly); GetMovetype() != Movetype::None)
	{
		SetMovetype(Movetype::Bounce);
	}

	// return if not time to hunt
	if (m_flNextHunt > gpGlobals->time)
		return;

	m_flNextHunt = gpGlobals->time + 2.0;

	Vector vecFlat = pev->velocity;
	vecFlat.z = 0;
	vecFlat = vecFlat.Normalize();

	UTIL_MakeVectors(pev->angles);

	if (m_hEnemy == nullptr || !m_hEnemy->IsAlive())
	{
		// find target, bounce a bit towards it.
		Look(512);
		m_hEnemy = BestVisibleEnemy();
	}

	// squeek if it's about time blow up
	if ((m_flDie - gpGlobals->time <= 0.5) && (m_flDie - gpGlobals->time >= 0.3))
	{
		EmitSound(SoundChannel::Voice, "squeek/sqk_die1.wav", VOL_NORM, ATTN_NORM, PITCH_NORM + RANDOM_LONG(0, 0x3F));
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 256, 0.25);
	}

	if (m_hEnemy != nullptr)
	{
		if (IsVisible(m_hEnemy))
		{
			const Vector vecDir = m_hEnemy->EyePosition() - pev->origin;
			m_vecTarget = vecDir.Normalize();
		}

		const float flVel = pev->velocity.Length();
		const float flAdj = std::min(1.2f, 50.0f / (flVel + 10.0f));

		// ALERT( at_console, "think : enemy\n");

		// ALERT( at_console, "%.0f %.2f %.2f %.2f\n", flVel, m_vecTarget.x, m_vecTarget.y, m_vecTarget.z );

		pev->velocity = pev->velocity * flAdj + m_vecTarget * 300;
	}

	if (pev->flags & FL_ONGROUND)
	{
		pev->avelocity = vec3_origin;
	}
	else
	{
		if (pev->avelocity == vec3_origin)
		{
			pev->avelocity.x = RANDOM_FLOAT(-100, 100);
			pev->avelocity.z = RANDOM_FLOAT(-100, 100);
		}
	}

	if ((pev->origin - m_posPrev).Length() < 1.0)
	{
		pev->velocity.x = RANDOM_FLOAT(-100, 100);
		pev->velocity.y = RANDOM_FLOAT(-100, 100);
	}
	m_posPrev = pev->origin;

	pev->angles = VectorAngles(pev->velocity);
	pev->angles.z = 0;
	pev->angles.x = 0;
}

void CSqueakGrenade::SuperBounceTouch(CBaseEntity* pOther)
{
	// don't hit the guy that launched this grenade
	if (auto owner = GetOwner(); owner && pOther == owner)
		return;

	// at least until we've bounced once
	SetOwner(nullptr);

	pev->angles.x = 0;
	pev->angles.z = 0;

	// avoid bouncing too much
	if (m_flNextHit > gpGlobals->time)
		return;

	TraceResult tr = UTIL_GetGlobalTrace();

	// higher pitch as squeeker gets closer to detonation time
	const float flpitch = 155.0 - 60.0 * ((m_flDie - gpGlobals->time) / SQUEEK_DETONATE_DELAY);

	if (pOther->pev->takedamage && m_flNextAttack < gpGlobals->time)
	{
		// attack!

		// make sure it's me who has touched them
		if (InstanceOrNull(tr.pHit) == pOther)
		{
			// and it's not another squeakgrenade
			if (tr.pHit->v.modelindex != pev->modelindex)
			{
				// ALERT( at_console, "hit enemy\n");
				ClearMultiDamage();
				pOther->TraceAttack({this, gSkillData.snarkDmgBite, gpGlobals->v_forward, tr, DMG_SLASH});
				if (m_hOwner != nullptr)
					ApplyMultiDamage(this, m_hOwner);
				else
					ApplyMultiDamage(this, this);

				pev->dmg += gSkillData.snarkDmgPop; // add more explosion damage
				// m_flDie += 2.0; // add more life

				// make bite sound
				EmitSound(SoundChannel::Weapon, "squeek/sqk_deploy1.wav", VOL_NORM, ATTN_NORM, (int)flpitch);
				m_flNextAttack = gpGlobals->time + 0.5;
			}
		}
		else
		{
			// ALERT( at_console, "been hit\n");
		}
	}

	m_flNextHit = gpGlobals->time + 0.1;
	m_flNextHunt = gpGlobals->time;

	if (g_pGameRules->IsMultiplayer())
	{
		// in multiplayer, we limit how often snarks can make their bounce sounds to prevent overflows.
		if (gpGlobals->time < m_flNextBounceSoundTime)
		{
			// too soon!
			return;
		}
	}

	if (!(pev->flags & FL_ONGROUND))
	{
		// play bounce sound
		const float flRndSound = RANDOM_FLOAT(0, 1);

		if (flRndSound <= 0.33)
			EmitSound(SoundChannel::Voice, "squeek/sqk_hunt1.wav", VOL_NORM, ATTN_NORM, (int)flpitch);
		else if (flRndSound <= 0.66)
			EmitSound(SoundChannel::Voice, "squeek/sqk_hunt2.wav", VOL_NORM, ATTN_NORM, (int)flpitch);
		else
			EmitSound(SoundChannel::Voice, "squeek/sqk_hunt3.wav", VOL_NORM, ATTN_NORM, (int)flpitch);
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 256, 0.25);
	}
	else
	{
		// skittering sound
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 100, 0.1);
	}

	m_flNextBounceSoundTime = gpGlobals->time + 0.5;// half second.
}
#endif

LINK_ENTITY_TO_CLASS(weapon_snark, CSqueak);

void CSqueak::Spawn()
{
	Precache();
	m_iId = WEAPON_SNARK;
	SetModel("models/w_sqknest.mdl");

	FallInit();//get ready to fall down.

	m_iDefaultAmmo = SNARK_DEFAULT_GIVE;

	pev->sequence = 1;
	pev->animtime = gpGlobals->time;
	pev->framerate = 1.0;
}

void CSqueak::Precache()
{
	PRECACHE_MODEL("models/w_sqknest.mdl");
	PRECACHE_MODEL("models/v_squeak.mdl");
	PRECACHE_MODEL("models/p_squeak.mdl");
	PRECACHE_SOUND("squeek/sqk_hunt2.wav");
	PRECACHE_SOUND("squeek/sqk_hunt3.wav");
	UTIL_PrecacheOther("monster_snark");

	m_usSnarkFire = PRECACHE_EVENT(1, "events/snarkfire.sc");
}

bool CSqueak::GetItemInfo(ItemInfo* p)
{
	p->pszName = GetClassname();
	p->pszAmmo1 = "Snarks";
	p->iMaxAmmo1 = SNARK_MAX_CARRY;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_SNARK;
	p->iWeight = SNARK_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return true;
}

bool CSqueak::Deploy()
{
	// play hunt sound
	const float flRndSound = RANDOM_FLOAT(0, 1);

	if (flRndSound <= 0.5)
		EmitSound(SoundChannel::Voice, "squeek/sqk_hunt2.wav");
	else
		EmitSound(SoundChannel::Voice, "squeek/sqk_hunt3.wav");

	m_hPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	const bool result = DefaultDeploy("models/v_squeak.mdl", "models/p_squeak.mdl", SQUEAK_UP, "squeak");

	if (result)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7;
	}

	return result;
}

void CSqueak::Holster()
{
	m_hPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if (!m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_hPlayer->pev->weapons &= ~(1 << WEAPON_SNARK);
		SetThink(&CSqueak::DestroyItem);
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}

	SendWeaponAnim(SQUEAK_DOWN);
	m_hPlayer->EmitSound(SoundChannel::Weapon, "common/null.wav");
}

void CSqueak::PrimaryAttack()
{
	if (m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		UTIL_MakeVectors(m_hPlayer->pev->v_angle);

		// HACK HACK:  Ugly hacks to handle change in origin based on new physics code for players
		// Move origin up if crouched and start trace a bit outside of body ( 20 units instead of 16 )
		Vector trace_origin = m_hPlayer->pev->origin;
		if (m_hPlayer->pev->flags & FL_DUCKING)
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

		UTIL_PlaybackEvent(flags, m_hPlayer, m_usSnarkFire);

		if (tr.fAllSolid == 0 && tr.fStartSolid == 0 && tr.flFraction > 0.25)
		{
			// player "shoot" animation
			m_hPlayer->SetAnimation(PlayerAnim::Attack1);

#ifndef CLIENT_DLL
			CBaseEntity* pSqueak = CBaseEntity::Create("monster_snark", tr.vecEndPos, m_hPlayer->pev->v_angle, m_hPlayer);
			pSqueak->pev->velocity = gpGlobals->v_forward * 200 + m_hPlayer->pev->velocity;
#endif

			// play hunt sound
			const float flRndSound = RANDOM_FLOAT(0, 1);

			if (flRndSound <= 0.5)
				EmitSound(SoundChannel::Voice, "squeek/sqk_hunt2.wav", VOL_NORM, ATTN_NORM, 105);
			else
				EmitSound(SoundChannel::Voice, "squeek/sqk_hunt3.wav", VOL_NORM, ATTN_NORM, 105);

			m_hPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

			m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

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
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_fJustThrown)
	{
		m_fJustThrown = false;

		if (!m_hPlayer->m_rgAmmo[PrimaryAmmoIndex()])
		{
			RetireWeapon();
			return;
		}

		SendWeaponAnim(SQUEAK_UP);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_hPlayer->random_seed, 10, 15);
		return;
	}

	int iAnim;
	const float flRand = UTIL_SharedRandomFloat(m_hPlayer->random_seed, 0, 1);
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
