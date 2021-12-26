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

#include "CSqueakGrenade.hpp"

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
	SetAbsOrigin(GetAbsOrigin());

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
	pev->model = string_t::Null;// make invisible
	SetThink(&CSqueakGrenade::SUB_Remove);
	SetTouch(nullptr);
	pev->nextthink = gpGlobals->time + 0.1;

	// since squeak grenades never leave a body behind, clear out their takedamage now.
	// Squeaks do a bit of radius damage when they pop, and that radius damage will
	// continue to call this function unless we acknowledge the Squeak's death now. (sjb)
	SetDamageMode(DamageMode::No);

	// play squeek blast
	EmitSound(SoundChannel::Item, "squeek/sqk_blast1.wav", VOL_NORM, 0.5, PITCH_NORM);

	CSoundEnt::InsertSound(bits_SOUND_COMBAT, GetAbsOrigin(), SMALL_EXPLOSION_VOLUME, 3.0);

	UTIL_BloodDrips(GetAbsOrigin(), vec3_origin, BloodColor(), 80);

	if (m_hOwner != nullptr)
		RadiusDamage(this, m_hOwner, pev->dmg, CLASS_NONE, DMG_BLAST);
	else
		RadiusDamage(this, this, pev->dmg, CLASS_NONE, DMG_BLAST);

	// reset owner so death message happens
	if (m_hOwner != nullptr)
		SetOwner(m_hOwner);

	CBaseMonster::Killed({info.GetInflictor(), info.GetAttacker(), GibType::Always});
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
		g_vecAttackDir = GetAbsVelocity().Normalize();
		pev->health = -1;
		Killed({this, this, GibType::Normal});
		return;
	}

	// float
	if (pev->waterlevel != WaterLevel::Dry)
	{
		if (GetMovetype() == Movetype::Bounce)
		{
			SetMovetype(Movetype::Fly);
		}

		Vector velocity = GetAbsVelocity() * 0.9;
		velocity.z += 8;
		SetAbsVelocity(velocity);
	}
	else //if (SetMovetype(Movetype::Fly); GetMovetype() != Movetype::None)
	{
		SetMovetype(Movetype::Bounce);
	}

	// return if not time to hunt
	if (m_flNextHunt > gpGlobals->time)
		return;

	m_flNextHunt = gpGlobals->time + 2.0;

	Vector vecFlat = GetAbsVelocity();
	vecFlat.z = 0;
	vecFlat = vecFlat.Normalize();

	UTIL_MakeVectors(GetAbsAngles());

	if (m_hEnemy == nullptr || !m_hEnemy->IsAlive())
	{
		// find target, bounce a bit towards it.
		Look(512);
		m_hEnemy = BestVisibleEnemy();
	}

	// squeek if it's about time blow up
	if ((m_flDie - gpGlobals->time <= 0.5f) && (m_flDie - gpGlobals->time >= 0.3f))
	{
		EmitSound(SoundChannel::Voice, "squeek/sqk_die1.wav", VOL_NORM, ATTN_NORM, PITCH_NORM + RANDOM_LONG(0, 0x3F));
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, GetAbsOrigin(), 256, 0.25);
	}

	if (m_hEnemy != nullptr)
	{
		if (IsVisible(m_hEnemy))
		{
			const Vector vecDir = m_hEnemy->EyePosition() - GetAbsOrigin();
			m_vecTarget = vecDir.Normalize();
		}

		const float flVel = GetAbsVelocity().Length();
		const float flAdj = std::min(1.2f, 50.0f / (flVel + 10.0f));

		// ALERT( at_console, "think : enemy\n");

		// ALERT( at_console, "%.0f %.2f %.2f %.2f\n", flVel, m_vecTarget.x, m_vecTarget.y, m_vecTarget.z );

		SetAbsVelocity(GetAbsVelocity() * flAdj + m_vecTarget * 300);
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

	if ((GetAbsOrigin() - m_posPrev).Length() < 1.0)
	{
		SetAbsVelocity({RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), GetAbsVelocity().z});
	}
	m_posPrev = GetAbsOrigin();

	SetAbsAngles({0, VectorAngles(GetAbsVelocity()).y, 0});
}

void CSqueakGrenade::SuperBounceTouch(CBaseEntity* pOther)
{
	// don't hit the guy that launched this grenade
	if (auto owner = GetOwner(); owner && pOther == owner)
		return;

	// at least until we've bounced once
	SetOwner(nullptr);

	SetAbsAngles({0, GetAbsAngles().y, 0});

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
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, GetAbsOrigin(), 256, 0.25);
	}
	else
	{
		// skittering sound
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, GetAbsOrigin(), 100, 0.1);
	}

	m_flNextBounceSoundTime = gpGlobals->time + 0.5;// half second.
}
