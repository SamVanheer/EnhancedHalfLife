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

#include "CBaseDoor.hpp"
#include "CMomentaryDoor.hpp"

LINK_ENTITY_TO_CLASS(momentary_door, CMomentaryDoor);

void CMomentaryDoor::Spawn()
{
	SetMovedir(this);

	SetSolidType(Solid::BSP);
	SetMovetype(Movetype::Push);

	SetAbsOrigin(GetAbsOrigin());
	SetModel(STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 100;
	if (pev->dmg == 0)
		pev->dmg = 2;

	m_vecPosition1 = GetAbsOrigin();
	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	m_vecPosition2 = m_vecPosition1 + (pev->movedir * (fabs(pev->movedir.x * (pev->size.x - 2)) + fabs(pev->movedir.y * (pev->size.y - 2)) + fabs(pev->movedir.z * (pev->size.z - 2)) - m_flLip));
	ASSERTSZ(m_vecPosition1 != m_vecPosition2, "door start/end positions are equal");

	if (IsBitSet(pev->spawnflags, SF_DOOR_START_OPEN))
	{	// swap pos1 and pos2, put door at pos2
		SetAbsOrigin(m_vecPosition2);
		m_vecPosition2 = m_vecPosition1;
		m_vecPosition1 = GetAbsOrigin();
	}
	SetTouch(nullptr);

	Precache();
}

void CMomentaryDoor::Precache()
{
	// set the door's "in-motion" sound
	switch (m_bMoveSnd)
	{
	case	0:
		m_iszMovingSound = ALLOC_STRING("common/null.wav");
		break;
	case	1:
		PRECACHE_SOUND("doors/doormove1.wav");
		m_iszMovingSound = ALLOC_STRING("doors/doormove1.wav");
		break;
	case	2:
		PRECACHE_SOUND("doors/doormove2.wav");
		m_iszMovingSound = ALLOC_STRING("doors/doormove2.wav");
		break;
	case	3:
		PRECACHE_SOUND("doors/doormove3.wav");
		m_iszMovingSound = ALLOC_STRING("doors/doormove3.wav");
		break;
	case	4:
		PRECACHE_SOUND("doors/doormove4.wav");
		m_iszMovingSound = ALLOC_STRING("doors/doormove4.wav");
		break;
	case	5:
		PRECACHE_SOUND("doors/doormove5.wav");
		m_iszMovingSound = ALLOC_STRING("doors/doormove5.wav");
		break;
	case	6:
		PRECACHE_SOUND("doors/doormove6.wav");
		m_iszMovingSound = ALLOC_STRING("doors/doormove6.wav");
		break;
	case	7:
		PRECACHE_SOUND("doors/doormove7.wav");
		m_iszMovingSound = ALLOC_STRING("doors/doormove7.wav");
		break;
	case	8:
		PRECACHE_SOUND("doors/doormove8.wav");
		m_iszMovingSound = ALLOC_STRING("doors/doormove8.wav");
		break;
	default:
		m_iszMovingSound = ALLOC_STRING("common/null.wav");
		break;
	}
}

void CMomentaryDoor::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "movesnd"))
	{
		m_bMoveSnd = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "stopsnd"))
	{
		//		m_bStopSnd = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "healthvalue"))
	{
		//		m_bHealthValue = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CMomentaryDoor::Use(const UseInfo& info)
{
	if (info.GetUseType() != UseType::Set)		// Momentary buttons will pass down a float in here
		return;

	const float value = std::clamp(info.GetValue(), 0.0f, 1.0f);

	const Vector move = m_vecPosition1 + (value * (m_vecPosition2 - m_vecPosition1));

	const Vector delta = move - GetAbsOrigin();
	//float speed = delta.Length() * 10;
	const float speed = delta.Length() / 0.1; // move there in 0.1 sec
	if (speed == 0)
		return;

	// This entity only thinks when it moves, so if it's thinking, it's in the process of moving
	// play the sound when it starts moving (not yet thinking)
	if (pev->nextthink < pev->ltime || pev->nextthink == 0)
		EmitSound(SoundChannel::Static, STRING(m_iszMovingSound));
	// If we already moving to designated point, return
	else if (move == m_vecFinalDest)
		return;

	SetMoveDone(&CMomentaryDoor::DoorMoveDone);
	LinearMove(move, speed);
}

void CMomentaryDoor::DoorMoveDone()
{
	StopSound(SoundChannel::Static, STRING(m_iszMovingSound));
	EmitSound(SoundChannel::Static, STRING(m_iszArrivedSound)); //TODO: arrived sound is never set
}
