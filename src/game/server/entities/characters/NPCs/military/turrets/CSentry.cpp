/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#include "CSentry.hpp"

LINK_ENTITY_TO_CLASS(monster_sentry, CSentry);

void CSentry::Precache()
{
	CBaseTurret::Precache();
	PRECACHE_MODEL("models/sentry.mdl");
}

void CSentry::Spawn()
{
	Precache();
	SetModel("models/sentry.mdl");
	pev->health = gSkillData.sentryHealth;
	m_HackedGunPos = Vector(0, 0, 48);
	pev->view_ofs.z = 48;
	m_flMaxWait = 1E6;
	m_flMaxSpin = 1E6;

	CBaseTurret::Spawn();
	m_iRetractHeight = 64;
	m_iDeployHeight = 64;
	m_iMinPitch = -60;
	SetSize(Vector(-16, -16, -m_iRetractHeight), Vector(16, 16, m_iRetractHeight));

	SetTouch(&CSentry::SentryTouch);
	SetThink(&CSentry::Initialize);
	pev->nextthink = gpGlobals->time + 0.3;
}

void CSentry::Shoot(const Vector& vecSrc, const Vector& vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_MP5, 1);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EmitSound(SoundChannel::Weapon, "weapons/hks1.wav"); break;
	case 1: EmitSound(SoundChannel::Weapon, "weapons/hks2.wav"); break;
	case 2: EmitSound(SoundChannel::Weapon, "weapons/hks3.wav"); break;
	}
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}

bool CSentry::TakeDamage(const TakeDamageInfo& info)
{
	if (!pev->takedamage)
		return false;

	if (!m_iOn)
	{
		SetThink(&CSentry::Deploy);
		SetUse(nullptr);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	pev->health -= info.GetDamage();
	if (pev->health <= 0)
	{
		pev->health = 0;
		SetDamageMode(DamageMode::No);
		pev->dmgtime = gpGlobals->time;

		ClearBits(pev->flags, FL_MONSTER); // why are they set in the first place???

		SetUse(nullptr);
		SetThink(&CSentry::SentryDeath);
		SUB_UseTargets(this, UseType::On, 0); // wake up others
		pev->nextthink = gpGlobals->time + 0.1;

		return false;
	}

	return true;
}

void CSentry::SentryTouch(CBaseEntity* pOther)
{
	if (pOther && (pOther->IsPlayer() || (pOther->pev->flags & FL_MONSTER)))
	{
		TakeDamage({pOther, pOther, 0, 0});
	}
}

void CSentry::SentryDeath()
{
	bool iActive = false;

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->deadflag != DeadFlag::Dead)
	{
		pev->deadflag = DeadFlag::Dead;

		const float flRndSound = RANDOM_FLOAT(0, 1);

		if (flRndSound <= 0.33)
			EmitSound(SoundChannel::Body, "turret/tu_die.wav");
		else if (flRndSound <= 0.66)
			EmitSound(SoundChannel::Body, "turret/tu_die2.wav");
		else
			EmitSound(SoundChannel::Body, "turret/tu_die3.wav");

		StopSound(SoundChannel::Static, "turret/tu_active2.wav");

		SetBoneController(0, 0);
		SetBoneController(1, 0);

		SetTurretAnim(TurretAnim::Die);

		SetSolidType(Solid::Not);
		Vector myAngles = GetAbsAngles();
		myAngles.y = UTIL_AngleMod(myAngles.y + RANDOM_LONG(0, 2) * 120);
		SetAbsAngles(myAngles);
		EyeOn();
	}

	EyeOff();

	Vector vecSrc, vecAng;
	GetAttachment(1, vecSrc, vecAng);

	if (pev->dmgtime + RANDOM_FLOAT(0, 2) > gpGlobals->time)
	{
		// lots of smoke
		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(vecSrc.x + RANDOM_FLOAT(-16, 16));
		WRITE_COORD(vecSrc.y + RANDOM_FLOAT(-16, 16));
		WRITE_COORD(vecSrc.z - 32);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE(15); // scale * 10
		WRITE_BYTE(8); // framerate
		MESSAGE_END();
	}

	if (pev->dmgtime + RANDOM_FLOAT(0, 8) > gpGlobals->time)
	{
		UTIL_Sparks(vecSrc);
	}

	if (m_fSequenceFinished && pev->dmgtime + 5 < gpGlobals->time)
	{
		pev->framerate = 0;
		SetThink(nullptr);
	}
}
