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

#include "CTurret.hpp"

LINK_ENTITY_TO_CLASS(monster_turret, CTurret);

constexpr std::string_view TURRET_GLOW_SPRITE{"sprites/flare3.spr"};

void CTurret::Spawn()
{
	Precache();
	SetModel("models/turret.mdl");
	pev->health = gSkillData.turretHealth;
	m_HackedGunPos = Vector(0, 0, 12.75);
	m_flMaxSpin = TURRET_MAXSPIN;
	pev->view_ofs.z = 12.75;

	CBaseTurret::Spawn();

	m_iRetractHeight = 16;
	m_iDeployHeight = 32;
	m_iMinPitch = -15;
	SetSize(Vector(-32, -32, -m_iRetractHeight), Vector(32, 32, m_iRetractHeight));

	SetThink(&CTurret::Initialize);

	auto glow = m_hEyeGlow = CSprite::SpriteCreate(TURRET_GLOW_SPRITE.data(), GetAbsOrigin(), false);
	glow->SetTransparency(RenderMode::Glow, {255, 0, 0}, 0, RenderFX::NoDissipation);
	glow->SetAttachment(this, 2);
	m_eyeBrightness = 0;

	pev->nextthink = gpGlobals->time + 0.3;
}

void CTurret::Precache()
{
	CBaseTurret::Precache();
	PRECACHE_MODEL("models/turret.mdl");
	PRECACHE_MODEL(TURRET_GLOW_SPRITE.data());
}

void CTurret::Shoot(const Vector& vecSrc, const Vector& vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_12MM, 1);
	EmitSound(SoundChannel::Weapon, "turret/tu_fire1.wav", VOL_NORM, 0.6);
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}

void CTurret::SpinUpCall()
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	// Are we already spun up? If not start the two stage process.
	if (!m_iSpin)
	{
		SetTurretAnim(TurretAnim::Spin);
		// for the first pass, spin up the the barrel
		if (!m_iStartSpin)
		{
			pev->nextthink = gpGlobals->time + 1.0; // spinup delay
			EmitSound(SoundChannel::Body, "turret/tu_spinup.wav", TURRET_MACHINE_VOLUME);
			m_iStartSpin = true;
			pev->framerate = 0.1;
		}
		// after the barrel is spun up, turn on the hum
		else if (pev->framerate >= 1.0)
		{
			pev->nextthink = gpGlobals->time + 0.1; // retarget delay
			EmitSound(SoundChannel::Static, "turret/tu_active2.wav", TURRET_MACHINE_VOLUME);
			SetThink(&CTurret::ActiveThink);
			m_iStartSpin = false;
			m_iSpin = true;
		}
		else
		{
			pev->framerate += 0.075;
		}
	}

	if (m_iSpin)
	{
		SetThink(&CTurret::ActiveThink);
	}
}

void CTurret::SpinDownCall()
{
	if (m_iSpin)
	{
		SetTurretAnim(TurretAnim::Spin);
		if (pev->framerate == 1.0)
		{
			StopSound(SoundChannel::Static, "turret/tu_active2.wav");
			EmitSound(SoundChannel::Item, "turret/tu_spindown.wav", TURRET_MACHINE_VOLUME);
		}
		pev->framerate -= 0.02;
		if (pev->framerate <= 0)
		{
			pev->framerate = 0;
			m_iSpin = false;
		}
	}
}
