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

#include "CMiniTurret.hpp"

LINK_ENTITY_TO_CLASS(monster_miniturret, CMiniTurret);

void CMiniTurret::Spawn()
{
	Precache();
	SetModel("models/miniturret.mdl");
	pev->health = gSkillData.miniturretHealth;
	m_HackedGunPos = Vector(0, 0, 12.75);
	m_flMaxSpin = 0;
	pev->view_ofs.z = 12.75;

	CBaseTurret::Spawn();
	m_iRetractHeight = 16;
	m_iDeployHeight = 32;
	m_iMinPitch = -15;
	SetSize(Vector(-16, -16, -m_iRetractHeight), Vector(16, 16, m_iRetractHeight));

	SetThink(&CMiniTurret::Initialize);
	pev->nextthink = gpGlobals->time + 0.3;
}

void CMiniTurret::Precache()
{
	CBaseTurret::Precache();
	PRECACHE_MODEL("models/miniturret.mdl");
	PRECACHE_SOUND("weapons/hks1.wav");
	PRECACHE_SOUND("weapons/hks2.wav");
	PRECACHE_SOUND("weapons/hks3.wav");
}

void CMiniTurret::Shoot(const Vector& vecSrc, const Vector& vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_9MM, 1);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EmitSound(SoundChannel::Weapon, "weapons/hks1.wav"); break;
	case 1: EmitSound(SoundChannel::Weapon, "weapons/hks2.wav"); break;
	case 2: EmitSound(SoundChannel::Weapon, "weapons/hks3.wav"); break;
	}
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}
