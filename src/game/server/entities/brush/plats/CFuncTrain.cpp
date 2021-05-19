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

LINK_ENTITY_TO_CLASS(func_train, CFuncTrain);

TYPEDESCRIPTION	CFuncTrain::m_SaveData[] =
{
	DEFINE_FIELD(CFuncTrain, m_sounds, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTrain, m_hCurrentTarget, FIELD_EHANDLE),
	DEFINE_FIELD(CFuncTrain, m_activated, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CFuncTrain, CBasePlatTrain);

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
			pev->target = last->pev->targetname;
		pev->nextthink = 0;
		SetAbsVelocity(vec3_origin);
		if (!IsStringNull(pev->noiseStopMoving))
			EmitSound(SoundChannel::Voice, STRING(pev->noiseStopMoving), m_volume);
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
			pTarget->pev->message = iStringNull;
	}

	// need pointer to LAST target.
	if (IsBitSet(pTarget->pev->spawnflags, SF_TRAIN_WAIT_RETRIGGER) || (pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER))
	{
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
		// clear the sound channel.
		if (!IsStringNull(pev->noiseMovement))
			StopSound(SoundChannel::Static, STRING(pev->noiseMovement));
		if (!IsStringNull(pev->noiseStopMoving))
			EmitSound(SoundChannel::Voice, STRING(pev->noiseStopMoving), m_volume);
		pev->nextthink = 0;
		return;
	}

	// ALERT ( at_console, "%f\n", m_flWait );

	if (m_flWait != 0)
	{// -1 wait will wait forever!		
		pev->nextthink = pev->ltime + m_flWait;
		if (!IsStringNull(pev->noiseMovement))
			StopSound(SoundChannel::Static, STRING(pev->noiseMovement));
		if (!IsStringNull(pev->noiseStopMoving))
			EmitSound(SoundChannel::Voice, STRING(pev->noiseStopMoving), m_volume);
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
		if (!IsStringNull(pev->noiseMovement))
			StopSound(SoundChannel::Static, STRING(pev->noiseMovement));
		// Play stop sound
		if (!IsStringNull(pev->noiseStopMoving))
			EmitSound(SoundChannel::Voice, STRING(pev->noiseStopMoving), m_volume);
		return;
	}

	// Save last target in case we need to find it again
	pev->message = pev->target;

	pev->target = pTarg->pev->target;
	m_flWait = pTarg->GetDelay();

	if (CBaseEntity* pCurrentTarget = m_hCurrentTarget; pCurrentTarget && pCurrentTarget->pev->speed != 0)
	{// don't copy speed from target if it is 0 (uninitialized)
		pev->speed = pCurrentTarget->pev->speed;
		ALERT(at_aiconsole, "Train %s speed to %4.2f\n", STRING(pev->targetname), pev->speed);
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
		if (!IsStringNull(pev->noiseMovement))
			StopSound(SoundChannel::Static, STRING(pev->noiseMovement));
		if (!IsStringNull(pev->noiseMovement))
			EmitSound(SoundChannel::Static, STRING(pev->noiseMovement), m_volume);
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
		CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));

		//TODO: currently mimics old behavior where it uses the world by default. Needs to handle null targets better
		if (!pTarget)
		{
			pTarget = UTIL_GetWorld();
		}

		pev->target = pTarget->pev->target;
		m_hCurrentTarget = pTarget;// keep track of this since path corners change our target for us.

		SetAbsOrigin(pTarget->GetAbsOrigin() - (pev->mins + pev->maxs) * 0.5);

		if (IsStringNull(pev->targetname))
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

	if (IsStringNull(pev->target))
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

void CFuncTrain::Precache()
{
	CBasePlatTrain::Precache();

#if 0  // obsolete
	// otherwise use preset sound
	switch (m_sounds)
	{
	case 0:
		pev->noise = 0;
		pev->noise1 = 0;
		break;

	case 1:
		PRECACHE_SOUND("plats/train2.wav");
		PRECACHE_SOUND("plats/train1.wav");
		pev->noise = MAKE_STRING("plats/train2.wav");
		pev->noise1 = MAKE_STRING("plats/train1.wav");
		break;

	case 2:
		PRECACHE_SOUND("plats/platmove1.wav");
		PRECACHE_SOUND("plats/platstop1.wav");
		pev->noise = MAKE_STRING("plats/platstop1.wav");
		pev->noise1 = MAKE_STRING("plats/platmove1.wav");
		break;
	}
#endif
}

void CFuncTrain::OverrideReset()
{
	// Are we moving?
	if (GetAbsVelocity() != vec3_origin && pev->nextthink != 0)
	{
		pev->target = pev->message;
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
