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
#include "CFuncTrain.hpp"
#include "CPathCorner.hpp"

void CFuncTrain::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBasePlatTrain::KeyValue(pkvd);
}

void CFuncTrain::Blocked(CBaseEntity* pOther)
{
	if (gpGlobals->time < m_flActivateFinished)
		return;

	m_flActivateFinished = gpGlobals->time + 0.5;

	pOther->TakeDamage({this, this, pev->dmg, DMG_CRUSH});
}

void CFuncTrain::Use(const UseInfo& info)
{
	if (pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER)
	{
		// Move toward my target
		pev->spawnflags &= ~SF_TRAIN_WAIT_RETRIGGER;
		Next();
	}
	else
	{
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
		// Pop back to last target if it's available
		if (CBaseEntity* last = m_hCurrentTarget; last)
			SetTarget(last->GetTargetname());
		pev->nextthink = 0;
		SetAbsVelocity(vec3_origin);
		if (!IsStringNull(m_iszArrivedSound))
			EmitSound(SoundChannel::Voice, STRING(m_iszArrivedSound), m_volume);
	}
}

void CFuncTrain::Wait()
{
	//TODO: can be null if killtargeted
	CBaseEntity* pTarget = m_hCurrentTarget;

	// Fire the pass target if there is one
	if (!IsStringNull(pTarget->pev->message))
	{
		FireTargets(STRING(pTarget->pev->message), this, this, UseType::Toggle, 0);
		if (IsBitSet(pTarget->pev->spawnflags, SF_CORNER_FIREONCE))
			pTarget->pev->message = string_t::Null;
	}

	// need pointer to LAST target.
	if (IsBitSet(pTarget->pev->spawnflags, SF_TRAIN_WAIT_RETRIGGER) || (pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER))
	{
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
		// clear the sound channel.
		if (!IsStringNull(m_iszMovingSound))
			StopSound(SoundChannel::Static, STRING(m_iszMovingSound));
		if (!IsStringNull(m_iszArrivedSound))
			EmitSound(SoundChannel::Voice, STRING(m_iszArrivedSound), m_volume);
		pev->nextthink = 0;
		return;
	}

	// ALERT ( at_console, "%f\n", m_flWait );

	if (m_flWait != 0)
	{// -1 wait will wait forever!		
		pev->nextthink = pev->ltime + m_flWait;
		if (!IsStringNull(m_iszMovingSound))
			StopSound(SoundChannel::Static, STRING(m_iszMovingSound));
		if (!IsStringNull(m_iszArrivedSound))
			EmitSound(SoundChannel::Voice, STRING(m_iszArrivedSound), m_volume);
		SetThink(&CFuncTrain::Next);
	}
	else
	{
		Next();// do it RIGHT now!
	}
}

void CFuncTrain::Next()
{
	// now find our next target
	CBaseEntity* pTarg = GetNextTarget();

	if (!pTarg)
	{
		if (!IsStringNull(m_iszMovingSound))
			StopSound(SoundChannel::Static, STRING(m_iszMovingSound));
		// Play stop sound
		if (!IsStringNull(m_iszArrivedSound))
			EmitSound(SoundChannel::Voice, STRING(m_iszArrivedSound), m_volume);
		return;
	}

	// Save last target in case we need to find it again
	pev->message = MAKE_STRING(GetTarget());

	SetTarget(pTarg->GetTarget());
	m_flWait = pTarg->GetDelay();

	if (CBaseEntity* pCurrentTarget = m_hCurrentTarget; pCurrentTarget && pCurrentTarget->pev->speed != 0)
	{// don't copy speed from target if it is 0 (uninitialized)
		pev->speed = pCurrentTarget->pev->speed;
		ALERT(at_aiconsole, "Train %s speed to %4.2f\n", GetTargetname(), pev->speed);
	}
	m_hCurrentTarget = pTarg;// keep track of this since path corners change our target for us.

	if (IsBitSet(pTarg->pev->spawnflags, SF_CORNER_TELEPORT))
	{
		// Path corner has indicated a teleport to the next corner.
		SetBits(pev->effects, EF_NOINTERP);
		SetAbsOrigin(pTarg->GetAbsOrigin() - (pev->mins + pev->maxs) * 0.5);
		Wait(); // Get on with doing the next path corner.
	}
	else
	{
		// Normal linear move.

		// CHANGED this from SoundChannel::Voice to SoundChannel::Static around OEM beta time because trains should
		// use SoundChannel::Static for their movement sounds to prevent sound field problems.
		// this is not a hack or temporary fix, this is how things should be. (sjb).
		if (!IsStringNull(m_iszMovingSound))
			StopSound(SoundChannel::Static, STRING(m_iszMovingSound));
		if (!IsStringNull(m_iszMovingSound))
			EmitSound(SoundChannel::Static, STRING(m_iszMovingSound), m_volume);
		ClearBits(pev->effects, EF_NOINTERP);
		SetMoveDone(&CFuncTrain::Wait);
		LinearMove(pTarg->GetAbsOrigin() - (pev->mins + pev->maxs) * 0.5, pev->speed);
	}
}

void CFuncTrain::Activate()
{
	// Not yet active, so teleport to first target
	if (!m_activated)
	{
		m_activated = true;
		CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, GetTarget());

		//TODO: currently mimics old behavior where it uses the world by default. Needs to handle null targets better
		if (!pTarget)
		{
			pTarget = UTIL_GetWorld();
		}

		SetTarget(pTarget->GetTarget());
		m_hCurrentTarget = pTarget;// keep track of this since path corners change our target for us.

		SetAbsOrigin(pTarget->GetAbsOrigin() - (pev->mins + pev->maxs) * 0.5);

		if (!HasTargetname())
		{	// not triggered, so start immediately
			pev->nextthink = pev->ltime + 0.1;
			SetThink(&CFuncTrain::Next);
		}
		else
			pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
	}
}

void CFuncTrain::Spawn()
{
	Precache();
	if (pev->speed == 0)
		pev->speed = 100;

	if (!HasTarget())
		ALERT(at_console, "FuncTrain with no target");

	if (pev->dmg == 0)
		pev->dmg = 2;

	SetMovetype(Movetype::Push);

	//TODO: use separate constant for this spawnflag
	if (IsBitSet(pev->spawnflags, SF_TRACKTRAIN_PASSABLE))
		SetSolidType(Solid::Not);
	else
		SetSolidType(Solid::BSP);

	SetModel(STRING(pev->model));
	SetSize(pev->mins, pev->maxs);
	SetAbsOrigin(GetAbsOrigin());

	m_activated = false;

	if (m_volume == 0)
		m_volume = 0.85;
}

void CFuncTrain::OverrideReset()
{
	// Are we moving?
	if (GetAbsVelocity() != vec3_origin && pev->nextthink != 0)
	{
		SetTargetDirect(pev->message);
		// now find our next target
		CBaseEntity* pTarg = GetNextTarget();
		if (!pTarg)
		{
			pev->nextthink = 0;
			SetAbsVelocity(vec3_origin);
		}
		else	// Keep moving for 0.1 secs, then find path_corner again and restart
		{
			SetThink(&CFuncTrain::Next);
			pev->nextthink = pev->ltime + 0.1;
		}
	}
}
