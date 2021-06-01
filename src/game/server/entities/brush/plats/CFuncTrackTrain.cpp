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

#include "CFuncTrackTrain.hpp"

LINK_ENTITY_TO_CLASS(func_tracktrain, CFuncTrackTrain);

void CFuncTrackTrain::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "wheels"))
	{
		m_length = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "height"))
	{
		m_height = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "startspeed"))
	{
		m_startSpeed = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "volume"))
	{
		m_flVolume = (float)(atoi(pkvd->szValue));
		m_flVolume *= 0.1;
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "bank"))
	{
		m_flBank = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CFuncTrackTrain::NextThink(float thinkTime, bool alwaysThink)
{
	if (alwaysThink)
		pev->flags |= FL_ALWAYSTHINK;
	else
		pev->flags &= ~FL_ALWAYSTHINK;

	pev->nextthink = thinkTime;
}

void CFuncTrackTrain::Blocked(CBaseEntity* pOther)
{
	// Blocker is on-ground on the train
	if (IsBitSet(pOther->pev->flags, FL_ONGROUND) && InstanceOrNull(pOther->pev->groundentity) == this)
	{
		const float deltaSpeed = std::min(std::abs(pev->speed), 50.0f);
		if (!pOther->GetAbsVelocity().z)
			pOther->SetAbsVelocity(pOther->GetAbsVelocity() + Vector{0, 0, deltaSpeed});
		return;
	}
	else
		pOther->SetAbsVelocity((pOther->GetAbsOrigin() - GetAbsOrigin()).Normalize() * pev->dmg);

	ALERT(at_aiconsole, "TRAIN(%s): Blocked by %s (dmg:%.2f)\n", GetTargetname(), pOther->GetClassname(), pev->dmg);
	if (pev->dmg <= 0)
		return;
	// we can't hurt this thing, so we're not concerned with it
	pOther->TakeDamage({this, this, pev->dmg, DMG_CRUSH});
}

void CFuncTrackTrain::Use(const UseInfo& info)
{
	if (info.GetUseType() != UseType::Set)
	{
		if (!ShouldToggle(info.GetUseType(), (pev->speed != 0)))
			return;

		if (pev->speed == 0)
		{
			pev->speed = m_speed * m_dir;

			Next();
		}
		else
		{
			pev->speed = 0;
			SetAbsVelocity(vec3_origin);
			pev->avelocity = vec3_origin;
			StopSound();
			SetThink(nullptr);
		}
	}
	else
	{
		float delta = info.GetValue();

		delta = ((int)(pev->speed * 4) / (int)m_speed) * 0.25 + 0.25 * delta;
		if (delta > 1)
			delta = 1;
		else if (delta < -1)
			delta = -1;
		if (pev->spawnflags & SF_TRACKTRAIN_FORWARDONLY)
		{
			if (delta < 0)
				delta = 0;
		}
		pev->speed = m_speed * delta;
		Next();
		ALERT(at_aiconsole, "TRAIN(%s), speed to %.2f\n", GetTargetname(), pev->speed);
	}
}

constexpr int TRAIN_STARTPITCH = 60;
constexpr int TRAIN_MAXPITCH = 200;
constexpr int TRAIN_MAXSPEED = 1000;	// approx max speed for sound pitch calculation

void CFuncTrackTrain::StopSound()
{
	// if sound playing, stop it
	if (m_soundPlaying && !IsStringNull(m_iszMovingSound))
	{
		const unsigned short us_sound = ((unsigned short)(m_sounds) & 0x0007) << 12;

		const unsigned short us_encode = us_sound;

		UTIL_PlaybackEvent(FEV_RELIABLE | FEV_UPDATE, this, m_usAdjustPitch, {.iparam1 = us_encode, .bparam1 = true});

		/*
		StopSound(SoundChannel::Static, STRING(m_iszMovingSound));
		*/
		EmitSound(SoundChannel::Item, "plats/ttrain_brake1.wav", m_flVolume);
	}

	m_soundPlaying = false;
}

void CFuncTrackTrain::UpdateSound()
{
	if (IsStringNull(m_iszMovingSound))
		return;

	const float flpitch = TRAIN_STARTPITCH + (fabs(pev->speed) * (TRAIN_MAXPITCH - TRAIN_STARTPITCH) / TRAIN_MAXSPEED);

	if (!m_soundPlaying)
	{
		// play startup sound for train
		EmitSound(SoundChannel::Item, "plats/ttrain_start1.wav", m_flVolume);
		EmitSound(SoundChannel::Static, STRING(m_iszMovingSound), m_flVolume, ATTN_NORM, (int)flpitch);
		m_soundPlaying = true;
	}
	else
	{
		/*
				// update pitch
				EmitSound(SoundChannel::Static, STRING(m_iszMovingSound), m_flVolume, ATTN_NORM, (int) flpitch, SND_CHANGE_PITCH);
		*/
		// volume 0.0 - 1.0 - 6 bits
		// m_sounds 3 bits
		// flpitch = 6 bits
		// 15 bits total

		const unsigned short us_sound = ((unsigned short)(m_sounds) & 0x0007) << 12;
		const unsigned short us_pitch = ((unsigned short)(flpitch / 10.0) & 0x003f) << 6;
		const unsigned short us_volume = ((unsigned short)(m_flVolume * 40.0) & 0x003f);

		const unsigned short us_encode = us_sound | us_pitch | us_volume;

		UTIL_PlaybackEvent(FEV_RELIABLE | FEV_UPDATE, this, m_usAdjustPitch, {.iparam1 = us_encode, .bparam1 = false});
	}
}

void CFuncTrackTrain::Next()
{
	if (!pev->speed)
	{
		ALERT(at_aiconsole, "TRAIN(%s): Speed is 0\n", GetTargetname());
		StopSound();
		return;
	}

	/*
	if (!m_hPath)
	{
		m_hPath = CPathTrack::Instance(UTIL_FindEntityByTargetname(nullptr, GetTarget()));
	}
	*/
	if (!m_hPath)
	{
		ALERT(at_aiconsole, "TRAIN(%s): Lost path\n", GetTargetname());
		StopSound();
		return;
	}

	UpdateSound();

	Vector nextPos = GetAbsOrigin();

	nextPos.z -= m_height;
	CPathTrack* pnext = m_hPath->LookAhead(nextPos, pev->speed * 0.1, 1);
	nextPos.z += m_height;

	SetAbsVelocity((nextPos - GetAbsOrigin()) * 10);
	Vector nextFront = GetAbsOrigin();

	nextFront.z -= m_height;
	if (m_length > 0)
		m_hPath->LookAhead(nextFront, m_length, 0);
	else
		m_hPath->LookAhead(nextFront, 100, 0);
	nextFront.z += m_height;

	const Vector delta = nextFront - GetAbsOrigin();
	Vector angles = VectorAngles(delta);
	// The train actually points west
	angles.y += 180;

	// !!!  All of this crap has to be done to make the angles not wrap around, revisit this.
	angles = UTIL_FixupAngles(angles);
	SetAbsAngles(UTIL_FixupAngles(GetAbsAngles()));

	if (!pnext || (delta.x == 0 && delta.y == 0))
		angles = GetAbsAngles();

	float vx;
	if (!(pev->spawnflags & SF_TRACKTRAIN_NOPITCH))
		vx = UTIL_AngleDistance(angles.x, GetAbsAngles().x);
	else
		vx = 0;
	const float vy = UTIL_AngleDistance(angles.y, GetAbsAngles().y);

	pev->avelocity.y = vy * 10;
	pev->avelocity.x = vx * 10;

	if (m_flBank != 0)
	{
		if (pev->avelocity.y < -5)
			pev->avelocity.z = UTIL_AngleDistance(UTIL_ApproachAngle(-m_flBank, GetAbsAngles().z, m_flBank * 2), GetAbsAngles().z);
		else if (pev->avelocity.y > 5)
			pev->avelocity.z = UTIL_AngleDistance(UTIL_ApproachAngle(m_flBank, GetAbsAngles().z, m_flBank * 2), GetAbsAngles().z);
		else
			pev->avelocity.z = UTIL_AngleDistance(UTIL_ApproachAngle(0, GetAbsAngles().z, m_flBank * 4), GetAbsAngles().z) * 4;
	}

	if (pnext)
	{
		if (pnext != m_hPath)
		{
			CPathTrack* pFire;
			if (pev->speed >= 0)
				pFire = pnext;
			else
				pFire = m_hPath;

			m_hPath = pnext;
			// Fire the pass target if there is one
			if (!IsStringNull(pFire->pev->message))
			{
				FireTargets(STRING(pFire->pev->message), this, this, UseType::Toggle, 0);
				if (IsBitSet(pFire->pev->spawnflags, SF_PATH_FIREONCE))
					pFire->pev->message = iStringNull;
			}

			if (pFire->pev->spawnflags & SF_PATH_DISABLE_TRAIN)
				pev->spawnflags |= SF_TRACKTRAIN_NOCONTROL;

			// Don't override speed if under user control
			if (pev->spawnflags & SF_TRACKTRAIN_NOCONTROL)
			{
				if (pFire->pev->speed != 0)
				{// don't copy speed from target if it is 0 (uninitialized)
					pev->speed = pFire->pev->speed;
					ALERT(at_aiconsole, "TrackTrain %s speed to %4.2f\n", GetTargetname(), pev->speed);
				}
			}

		}
		SetThink(&CFuncTrackTrain::Next);
		NextThink(pev->ltime + 0.5f, true);
	}
	else	// end of path, stop
	{
		StopSound();
		SetAbsVelocity(nextPos - GetAbsOrigin());
		pev->avelocity = vec3_origin;
		const float distance = GetAbsVelocity().Length();
		m_oldSpeed = pev->speed;

		pev->speed = 0;

		// Move to the dead end

		// Are we there yet?
		if (distance > 0)
		{
			// no, how long to get there?
			const float time = distance / m_oldSpeed;
			SetAbsVelocity(GetAbsVelocity() * (m_oldSpeed / distance));
			SetThink(&CFuncTrackTrain::DeadEnd);
			NextThink(pev->ltime + time, false);
		}
		else
		{
			DeadEnd();
		}
	}
}

void CFuncTrackTrain::DeadEnd()
{
	// Fire the dead-end target if there is one
	CPathTrack* pTrack = m_hPath;

	ALERT(at_aiconsole, "TRAIN(%s): Dead end ", GetTargetname());
	// Find the dead end path node
	// HACKHACK -- This is bugly, but the train can actually stop moving at a different node depending on its speed
	// so we have to traverse the list to it's end.
	if (pTrack)
	{
		if (m_oldSpeed < 0)
		{
			CPathTrack* pNext;
			do
			{
				pNext = pTrack->ValidPath(pTrack->GetPrevious(), true);
				if (pNext)
					pTrack = pNext;
			}
			while (pNext);
		}
		else
		{
			CPathTrack* pNext;
			do
			{
				pNext = pTrack->ValidPath(pTrack->GetNext(), true);
				if (pNext)
					pTrack = pNext;
			}
			while (pNext);
		}
	}

	SetAbsVelocity(vec3_origin);
	pev->avelocity = vec3_origin;
	if (pTrack)
	{
		ALERT(at_aiconsole, "at %s\n", pTrack->GetTargetname());
		if (!IsStringNull(pTrack->pev->netname))
			FireTargets(STRING(pTrack->pev->netname), this, this, UseType::Toggle, 0);
	}
	else
		ALERT(at_aiconsole, "\n");
}

void CFuncTrackTrain::SetControls(CBaseEntity* pControls)
{
	const Vector offset = pControls->GetAbsOrigin() - pev->oldorigin;

	m_controlMins = pControls->pev->mins + offset;
	m_controlMaxs = pControls->pev->maxs + offset;
}

bool CFuncTrackTrain::OnControls(CBaseEntity* pTest)
{
	const Vector offset = pTest->GetAbsOrigin() - GetAbsOrigin();

	if (pev->spawnflags & SF_TRACKTRAIN_NOCONTROL)
		return false;

	// Transform offset into local coordinates
	UTIL_MakeVectors(GetAbsAngles());
	const Vector local
	{
		DotProduct(offset, gpGlobals->v_forward),
		-DotProduct(offset, gpGlobals->v_right),
		DotProduct(offset, gpGlobals->v_up)
	};

	if (local.x >= m_controlMins.x && local.y >= m_controlMins.y && local.z >= m_controlMins.z &&
		local.x <= m_controlMaxs.x && local.y <= m_controlMaxs.y && local.z <= m_controlMaxs.z)
		return true;

	return false;
}

void CFuncTrackTrain::Find()
{
	auto path = m_hPath = CPathTrack::Instance(UTIL_FindEntityByTargetname(nullptr, GetTarget()));
	if (!path)
		return;

	if (!path->ClassnameIs("path_track"))
	{
		ALERT(at_error, "func_track_train must be on a path of path_track\n");
		m_hPath = nullptr;
		return;
	}

	Vector nextPos = path->GetAbsOrigin();
	nextPos.z += m_height;

	Vector look = nextPos;
	look.z -= m_height;
	path->LookAhead(look, m_length, 0);
	look.z += m_height;

	Vector angles = VectorAngles(look - nextPos);

	// The train actually points west
	angles.y += 180;

	if (pev->spawnflags & SF_TRACKTRAIN_NOPITCH)
		angles.x = 0;

	SetAbsAngles(angles);

	SetAbsOrigin(nextPos);
	NextThink(pev->ltime + 0.1, false);
	SetThink(&CFuncTrackTrain::Next);
	pev->speed = m_startSpeed;

	UpdateSound();
}

void CFuncTrackTrain::NearestPath()
{
	CBaseEntity* pTrack = nullptr;
	CBaseEntity* pNearest = nullptr;
	float closest = 1024;

	while ((pTrack = UTIL_FindEntityInSphere(pTrack, GetAbsOrigin(), 1024)) != nullptr)
	{
		// filter out non-tracks
		if (!(pTrack->pev->flags & (FL_CLIENT | FL_MONSTER)) && pTrack->ClassnameIs("path_track"))
		{
			const float dist = (GetAbsOrigin() - pTrack->GetAbsOrigin()).Length();
			if (dist < closest)
			{
				closest = dist;
				pNearest = pTrack;
			}
		}
	}

	if (!pNearest)
	{
		ALERT(at_console, "Can't find a nearby track !!!\n");
		SetThink(nullptr);
		return;
	}

	ALERT(at_aiconsole, "TRAIN: %s, Nearest track is %s\n", GetTargetname(), pNearest->GetTargetname());
	// If I'm closer to the next path_track on this path, then it's my real path
	pTrack = ((CPathTrack*)pNearest)->GetNext();
	if (pTrack)
	{
		if ((GetAbsOrigin() - pTrack->GetAbsOrigin()).Length() < (GetAbsOrigin() - pNearest->GetAbsOrigin()).Length())
			pNearest = pTrack;
	}

	m_hPath = (CPathTrack*)pNearest;

	if (pev->speed != 0)
	{
		NextThink(pev->ltime + 0.1, false);
		SetThink(&CFuncTrackTrain::Next);
	}
}

void CFuncTrackTrain::OverrideReset()
{
	NextThink(pev->ltime + 0.1, false);
	SetThink(&CFuncTrackTrain::NearestPath);
}

CFuncTrackTrain* CFuncTrackTrain::Instance(CBaseEntity* pent)
{
	if (pent && pent->ClassnameIs("func_tracktrain"))
		return (CFuncTrackTrain*)pent;
	return nullptr;
}

void CFuncTrackTrain::Spawn()
{
	if (pev->speed == 0)
		m_speed = 100;
	else
		m_speed = pev->speed;

	pev->speed = 0;
	SetAbsVelocity(vec3_origin);
	pev->avelocity = vec3_origin;
	pev->impulse = m_speed;

	m_dir = 1;

	if (!HasTarget())
		ALERT(at_console, "FuncTrain with no target");

	if (pev->spawnflags & SF_TRACKTRAIN_PASSABLE)
		SetSolidType(Solid::Not);
	else
		SetSolidType(Solid::BSP);
	SetMovetype(Movetype::Push);

	SetModel(STRING(pev->model));

	SetSize(pev->mins, pev->maxs);
	SetAbsOrigin(GetAbsOrigin());

	// Cache off placed origin for train controls
	pev->oldorigin = GetAbsOrigin();

	m_controlMins = pev->mins;
	m_controlMaxs = pev->maxs;
	m_controlMaxs.z += 72;
	// start trains on the next frame, to make sure their targets have had
	// a chance to spawn/activate
	NextThink(pev->ltime + 0.1, false);
	SetThink(&CFuncTrackTrain::Find);
	Precache();
}

void CFuncTrackTrain::Precache()
{
	if (m_flVolume == 0.0)
		m_flVolume = 1.0;

	switch (m_sounds)
	{
	default:
		// no sound
		m_iszMovingSound = iStringNull;
		break;
	case 1: PRECACHE_SOUND("plats/ttrain1.wav"); m_iszMovingSound = MAKE_STRING("plats/ttrain1.wav"); break;
	case 2: PRECACHE_SOUND("plats/ttrain2.wav"); m_iszMovingSound = MAKE_STRING("plats/ttrain2.wav"); break;
	case 3: PRECACHE_SOUND("plats/ttrain3.wav"); m_iszMovingSound = MAKE_STRING("plats/ttrain3.wav"); break;
	case 4: PRECACHE_SOUND("plats/ttrain4.wav"); m_iszMovingSound = MAKE_STRING("plats/ttrain4.wav"); break;
	case 5: PRECACHE_SOUND("plats/ttrain6.wav"); m_iszMovingSound = MAKE_STRING("plats/ttrain6.wav"); break;
	case 6: PRECACHE_SOUND("plats/ttrain7.wav"); m_iszMovingSound = MAKE_STRING("plats/ttrain7.wav"); break;
	}

	PRECACHE_SOUND("plats/ttrain_brake1.wav");
	PRECACHE_SOUND("plats/ttrain_start1.wav");

	m_usAdjustPitch = PRECACHE_EVENT(1, "events/train.sc");
}
