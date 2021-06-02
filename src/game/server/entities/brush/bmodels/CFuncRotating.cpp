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

#include "CFuncRotating.hpp"

void CFuncRotating::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "fanfriction"))
	{
		m_flFanFriction = atof(pkvd->szValue) / 100;
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "Volume"))
	{
		m_flVolume = atof(pkvd->szValue) / 10.0;

		if (m_flVolume > 1.0)
			m_flVolume = 1.0;
		if (m_flVolume < 0.0)
			m_flVolume = 0.0;
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "spawnorigin"))
	{
		const Vector tmp = UTIL_StringToVector(pkvd->szValue);
		if (tmp != vec3_origin)
			SetAbsOrigin(tmp);
	}
	else if (AreStringsEqual(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CFuncRotating::Spawn()
{
	// set final pitch.  Must not be PITCH_NORM, since we
	// plan on pitch shifting later.

	m_pitch = PITCH_NORM - 1;

	// maintain compatibility with previous maps
	if (m_flVolume == 0.0)
		m_flVolume = 1.0;

	// if the designer didn't set a sound attenuation, default to one.
	m_flAttenuation = ATTN_NORM;

	if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_SMALLRADIUS))
	{
		m_flAttenuation = ATTN_IDLE;
	}
	else if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_MEDIUMRADIUS))
	{
		m_flAttenuation = ATTN_STATIC;
	}
	else if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_LARGERADIUS))
	{
		m_flAttenuation = ATTN_NORM;
	}

	// prevent divide by zero if level designer forgets friction!
	if (m_flFanFriction == 0)
	{
		m_flFanFriction = 1;
	}

	if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_Z_AXIS))
		pev->movedir = vec3_up;
	else if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_X_AXIS))
		pev->movedir = vec3_forward;
	else
		pev->movedir = vec3_right;	// y-axis

	// check for reverse rotation
	if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_BACKWARDS))
		pev->movedir = pev->movedir * -1;

	// some rotating objects like fake volumetric lights will not be solid.
	if (IsBitSet(pev->spawnflags, SF_ROTATING_NOT_SOLID))
	{
		SetSolidType(Solid::Not);
		pev->skin = static_cast<int>(Contents::Empty);
		SetMovetype(Movetype::Push);
	}
	else
	{
		SetSolidType(Solid::BSP);
		SetMovetype(Movetype::Push);
	}

	SetAbsOrigin(GetAbsOrigin());
	SetModel(STRING(pev->model));

	SetUse(&CFuncRotating::RotatingUse);
	// did level designer forget to assign speed?
	if (pev->speed <= 0)
		pev->speed = 0;

	// Removed this per level designers request.  -- JAY
	//	if (pev->dmg == 0)
	//		pev->dmg = 2;

	// instant-use brush?
	if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_INSTANT))
	{
		SetThink(&CFuncRotating::SUB_CallUseToggle);
		pev->nextthink = pev->ltime + 1.5;	// leave a magic delay for client to start up
	}
	// can this brush inflict pain?
	if (IsBitSet(pev->spawnflags, SF_BRUSH_HURT))
	{
		SetTouch(&CFuncRotating::HurtTouch);
	}

	Precache();
}

void CFuncRotating::Precache()
{
	const char* szSoundFile = STRING(pev->message);

	// set up fan sounds

	if (!IsStringNull(pev->message) && strlen(szSoundFile) > 0)
	{
		// if a path is set for a wave, use it

		PRECACHE_SOUND(szSoundFile);

		m_iszRunningSound = ALLOC_STRING(szSoundFile);
	}
	else
	{
		// otherwise use preset sound
		switch (m_sounds)
		{
		case 1:
			PRECACHE_SOUND("fans/fan1.wav");
			m_iszRunningSound = ALLOC_STRING("fans/fan1.wav");
			break;
		case 2:
			PRECACHE_SOUND("fans/fan2.wav");
			m_iszRunningSound = ALLOC_STRING("fans/fan2.wav");
			break;
		case 3:
			PRECACHE_SOUND("fans/fan3.wav");
			m_iszRunningSound = ALLOC_STRING("fans/fan3.wav");
			break;
		case 4:
			PRECACHE_SOUND("fans/fan4.wav");
			m_iszRunningSound = ALLOC_STRING("fans/fan4.wav");
			break;
		case 5:
			PRECACHE_SOUND("fans/fan5.wav");
			m_iszRunningSound = ALLOC_STRING("fans/fan5.wav");
			break;

		case 0:
		default:
			if (!IsStringNull(pev->message) && strlen(szSoundFile) > 0)
			{
				PRECACHE_SOUND(szSoundFile);

				m_iszRunningSound = ALLOC_STRING(szSoundFile);
				break;
			}
			else
			{
				m_iszRunningSound = ALLOC_STRING("common/null.wav");
				break;
			}
		}
	}

	if (pev->avelocity != vec3_origin)
	{
		// if fan was spinning, and we went through transition or save/restore,
		// make sure we restart the sound.  1.5 sec delay is magic number. KDB

		SetThink(&CFuncRotating::SpinUp);
		pev->nextthink = pev->ltime + 1.5;
	}
}

void CFuncRotating::HurtTouch(CBaseEntity* pOther)
{
	// we can't hurt this thing, so we're not concerned with it
	if (!pOther->pev->takedamage)
		return;

	// calculate damage based on rotation speed
	pev->dmg = pev->avelocity.Length() / 10;

	pOther->TakeDamage({this, this, pev->dmg, DMG_CRUSH});

	pOther->SetAbsVelocity((pOther->GetAbsOrigin() - GetBrushModelOrigin(this)).Normalize() * pev->dmg);
}

constexpr int FANPITCHMIN = 30;
constexpr int FANPITCHMAX = 100;

void CFuncRotating::RampPitchVol(int fUp) //TODO: bool
{
	const Vector vecAVel = pev->avelocity;

	// get current angular velocity

	const vec_t vecCur = fabs(vecAVel.x != 0 ? vecAVel.x : (vecAVel.y != 0 ? vecAVel.y : vecAVel.z));

	// get target angular velocity

	vec_t vecFinal = (pev->movedir.x != 0 ? pev->movedir.x : (pev->movedir.y != 0 ? pev->movedir.y : pev->movedir.z));
	vecFinal *= pev->speed;
	vecFinal = fabs(vecFinal);

	// calc volume and pitch as % of final vol and pitch

	const float fpct = vecCur / vecFinal;
	//	if (fUp)
	//		fvol = m_flVolume * (0.5 + fpct/2.0); // spinup volume ramps up from 50% max vol
	//	else
	const float fvol = m_flVolume * fpct;			  // slowdown volume ramps down to 0

	const float fpitch = FANPITCHMIN + (FANPITCHMAX - FANPITCHMIN) * fpct;

	int pitch = (int)fpitch;
	if (pitch == PITCH_NORM)
		pitch = PITCH_NORM - 1;

	// change the fan's vol and pitch

	EmitSound(SoundChannel::Static, STRING(m_iszRunningSound),
		fvol, m_flAttenuation, pitch, SND_CHANGE_PITCH | SND_CHANGE_VOL);
}

void CFuncRotating::SpinUp()
{
	pev->nextthink = pev->ltime + 0.1;
	pev->avelocity = pev->avelocity + (pev->movedir * (pev->speed * m_flFanFriction));

	const Vector vecAVel = pev->avelocity;// cache entity's rotational velocity

	// if we've met or exceeded target speed, set target speed and stop thinking
	if (fabs(vecAVel.x) >= fabs(pev->movedir.x * pev->speed) &&
		fabs(vecAVel.y) >= fabs(pev->movedir.y * pev->speed) &&
		fabs(vecAVel.z) >= fabs(pev->movedir.z * pev->speed))
	{
		pev->avelocity = pev->movedir * pev->speed;// set speed in case we overshot
		EmitSound(SoundChannel::Static, STRING(m_iszRunningSound),
			m_flVolume, m_flAttenuation, FANPITCHMAX, SND_CHANGE_PITCH | SND_CHANGE_VOL);

		SetThink(&CFuncRotating::Rotate);
		Rotate();
	}
	else
	{
		RampPitchVol(true);
	}
}

void CFuncRotating::SpinDown()
{
	pev->nextthink = pev->ltime + 0.1;

	pev->avelocity = pev->avelocity - (pev->movedir * (pev->speed * m_flFanFriction));//spin down slower than spinup

	const Vector vecAVel = pev->avelocity;// cache entity's rotational velocity

	vec_t vecdir;
	if (pev->movedir.x != 0)
		vecdir = pev->movedir.x;
	else if (pev->movedir.y != 0)
		vecdir = pev->movedir.y;
	else
		vecdir = pev->movedir.z;

	// if we've met or exceeded target speed, set target speed and stop thinking
	// (note: must check for movedir > 0 or < 0)
	if (((vecdir > 0) && (vecAVel.x <= 0 && vecAVel.y <= 0 && vecAVel.z <= 0)) ||
		((vecdir < 0) && (vecAVel.x >= 0 && vecAVel.y >= 0 && vecAVel.z >= 0)))
	{
		pev->avelocity = vec3_origin;// set speed in case we overshot

		// stop sound, we're done
		StopSound(SoundChannel::Static, STRING(m_iszRunningSound /* Stop */));

		SetThink(&CFuncRotating::Rotate);
		Rotate();
	}
	else
	{
		RampPitchVol(false);
	}
}

void CFuncRotating::Rotate()
{
	pev->nextthink = pev->ltime + 10;
}

void CFuncRotating::RotatingUse(const UseInfo& info)
{
	//TODO: clean this up
	// is this a brush that should accelerate and decelerate when turned on/off (fan)?
	if (IsBitSet(pev->spawnflags, SF_BRUSH_ACCDCC))
	{
		// fan is spinning, so stop it.
		if (pev->avelocity != vec3_origin)
		{
			SetThink(&CFuncRotating::SpinDown);
			//EmitSound(SoundChannel::Weapon, STRING(m_iszStopSound), 
			//	m_flVolume, m_flAttenuation, m_pitch);

			pev->nextthink = pev->ltime + 0.1;
		}
		else// fan is not moving, so start it
		{
			SetThink(&CFuncRotating::SpinUp);
			EmitSound(SoundChannel::Static, STRING(m_iszRunningSound),
				0.01, m_flAttenuation, FANPITCHMIN);

			pev->nextthink = pev->ltime + 0.1;
		}
	}
	else if (!IsBitSet(pev->spawnflags, SF_BRUSH_ACCDCC))//this is a normal start/stop brush.
	{
		if (pev->avelocity != vec3_origin)
		{
			// play stopping sound here
			SetThink(&CFuncRotating::SpinDown);

			// EmitSound(SoundChannel::Weapon, STRING(m_iszStopSound), 
			//	m_flVolume, m_flAttenuation, m_pitch);

			pev->nextthink = pev->ltime + 0.1;
			// pev->avelocity = vec3_origin;
		}
		else
		{
			EmitSound(SoundChannel::Static, STRING(m_iszRunningSound),
				m_flVolume, m_flAttenuation, FANPITCHMAX);
			pev->avelocity = pev->movedir * pev->speed;

			SetThink(&CFuncRotating::Rotate);
			Rotate();
		}
	}
}

void CFuncRotating::Blocked(CBaseEntity* pOther)
{
	pOther->TakeDamage({this, this, pev->dmg, DMG_CRUSH});
}
