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

#define noiseMoving noise1
#define noiseArrived noise2

void CBaseDoor::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "skin"))//skin is used for content type
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "movesnd"))
	{
		m_bMoveSnd = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "stopsnd"))
	{
		m_bStopSnd = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "healthvalue"))
	{
		m_bHealthValue = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "locked_sound"))
	{
		m_bLockedSound = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "locked_sentence"))
	{
		m_bLockedSentence = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "unlocked_sound"))
	{
		m_bUnlockedSound = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "unlocked_sentence"))
	{
		m_bUnlockedSentence = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "WaveHeight"))
	{
		pev->scale = atof(pkvd->szValue) * (1.0 / 8.0);
		pkvd->fHandled = true;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(func_door, CBaseDoor);

/**
*	@brief func_water - same as a door.
*/
LINK_ENTITY_TO_CLASS(func_water, CBaseDoor);

void CBaseDoor::Spawn()
{
	Precache();
	SetMovedir(this);

	if (pev->skin == 0)
	{//normal door
		if (IsBitSet(pev->spawnflags, SF_DOOR_PASSABLE))
			SetSolidType(Solid::Not);
		else
			SetSolidType(Solid::BSP);
	}
	else
	{// special contents
		SetSolidType(Solid::Not);
		SetBits(pev->spawnflags, SF_DOOR_SILENT);	// water is silent for now
	}

	SetMovetype(Movetype::Push);
	SetAbsOrigin(GetAbsOrigin());
	SetModel(STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 100;

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

	m_toggle_state = ToggleState::AtBottom;

	// if the door is flagged for USE button activation only, use nullptr touch function
	if (IsBitSet(pev->spawnflags, SF_DOOR_USE_ONLY))
	{
		SetTouch(nullptr);
	}
	else // touchable button
		SetTouch(&CBaseDoor::DoorTouch);
}

void CBaseDoor::Precache()
{
	// set the door's "in-motion" sound
	switch (m_bMoveSnd)
	{
	case	0:
		pev->noiseMoving = ALLOC_STRING("common/null.wav");
		break;
	case	1:
		PRECACHE_SOUND("doors/doormove1.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove1.wav");
		break;
	case	2:
		PRECACHE_SOUND("doors/doormove2.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove2.wav");
		break;
	case	3:
		PRECACHE_SOUND("doors/doormove3.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove3.wav");
		break;
	case	4:
		PRECACHE_SOUND("doors/doormove4.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove4.wav");
		break;
	case	5:
		PRECACHE_SOUND("doors/doormove5.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove5.wav");
		break;
	case	6:
		PRECACHE_SOUND("doors/doormove6.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove6.wav");
		break;
	case	7:
		PRECACHE_SOUND("doors/doormove7.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove7.wav");
		break;
	case	8:
		PRECACHE_SOUND("doors/doormove8.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove8.wav");
		break;
	case	9:
		PRECACHE_SOUND("doors/doormove9.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove9.wav");
		break;
	case	10:
		PRECACHE_SOUND("doors/doormove10.wav");
		pev->noiseMoving = ALLOC_STRING("doors/doormove10.wav");
		break;
	default:
		pev->noiseMoving = ALLOC_STRING("common/null.wav");
		break;
	}

	// set the door's 'reached destination' stop sound
	switch (m_bStopSnd)
	{
	case	0:
		pev->noiseArrived = ALLOC_STRING("common/null.wav");
		break;
	case	1:
		PRECACHE_SOUND("doors/doorstop1.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop1.wav");
		break;
	case	2:
		PRECACHE_SOUND("doors/doorstop2.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop2.wav");
		break;
	case	3:
		PRECACHE_SOUND("doors/doorstop3.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop3.wav");
		break;
	case	4:
		PRECACHE_SOUND("doors/doorstop4.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop4.wav");
		break;
	case	5:
		PRECACHE_SOUND("doors/doorstop5.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop5.wav");
		break;
	case	6:
		PRECACHE_SOUND("doors/doorstop6.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop6.wav");
		break;
	case	7:
		PRECACHE_SOUND("doors/doorstop7.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop7.wav");
		break;
	case	8:
		PRECACHE_SOUND("doors/doorstop8.wav");
		pev->noiseArrived = ALLOC_STRING("doors/doorstop8.wav");
		break;
	default:
		pev->noiseArrived = ALLOC_STRING("common/null.wav");
		break;
	}

	// get door button sounds, for doors which are directly 'touched' to open

	if (m_bLockedSound)
	{
		const char* pszSound = ButtonSound((int)m_bLockedSound);
		PRECACHE_SOUND(pszSound);
		m_ls.sLockedSound = ALLOC_STRING(pszSound);
	}

	if (m_bUnlockedSound)
	{
		const char* pszSound = ButtonSound((int)m_bUnlockedSound);
		PRECACHE_SOUND(pszSound);
		m_ls.sUnlockedSound = ALLOC_STRING(pszSound);
	}

	// get sentence group names, for doors which are directly 'touched' to open

	switch (m_bLockedSentence)
	{
	case 1: m_ls.sLockedSentence = ALLOC_STRING("NA"); break; // access denied
	case 2: m_ls.sLockedSentence = ALLOC_STRING("ND"); break; // security lockout
	case 3: m_ls.sLockedSentence = ALLOC_STRING("NF"); break; // blast door
	case 4: m_ls.sLockedSentence = ALLOC_STRING("NFIRE"); break; // fire door
	case 5: m_ls.sLockedSentence = ALLOC_STRING("NCHEM"); break; // chemical door
	case 6: m_ls.sLockedSentence = ALLOC_STRING("NRAD"); break; // radiation door
	case 7: m_ls.sLockedSentence = ALLOC_STRING("NCON"); break; // gen containment
	case 8: m_ls.sLockedSentence = ALLOC_STRING("NH"); break; // maintenance door
	case 9: m_ls.sLockedSentence = ALLOC_STRING("NG"); break; // broken door

	default: m_ls.sLockedSentence = iStringNull; break;
	}

	switch (m_bUnlockedSentence)
	{
	case 1: m_ls.sUnlockedSentence = ALLOC_STRING("EA"); break; // access granted
	case 2: m_ls.sUnlockedSentence = ALLOC_STRING("ED"); break; // security door
	case 3: m_ls.sUnlockedSentence = ALLOC_STRING("EF"); break; // blast door
	case 4: m_ls.sUnlockedSentence = ALLOC_STRING("EFIRE"); break; // fire door
	case 5: m_ls.sUnlockedSentence = ALLOC_STRING("ECHEM"); break; // chemical door
	case 6: m_ls.sUnlockedSentence = ALLOC_STRING("ERAD"); break; // radiation door
	case 7: m_ls.sUnlockedSentence = ALLOC_STRING("ECON"); break; // gen containment
	case 8: m_ls.sUnlockedSentence = ALLOC_STRING("EH"); break; // maintenance door

	default: m_ls.sUnlockedSentence = iStringNull; break;
	}
}

void CBaseDoor::DoorTouch(CBaseEntity* pOther)
{
	// Ignore touches by anything but players
	if (!pOther->IsPlayer())
		return;

	// If door has master, and it's not ready to trigger, 
	// play 'locked' sound

	if (!IsStringNull(m_sMaster) && !UTIL_IsMasterTriggered(m_sMaster, pOther))
		PlayLockSounds(this, &m_ls, true, false);

	// If door is somebody's target, then touching does nothing.
	// You have to activate the owner (e.g. button).

	if (!IsStringNull(pev->targetname))
	{
		// play locked sound
		PlayLockSounds(this, &m_ls, true, false);
		return;
	}

	m_hActivator = pOther;// remember who activated the door

	if (DoorActivate())
		SetTouch(nullptr); // Temporarily disable the touch function, until movement is finished.
}

void CBaseDoor::Use(const UseInfo& info)
{
	m_hActivator = info.GetActivator();
	// if not ready to be used, ignore "use" command.
	if (m_toggle_state == ToggleState::AtBottom || IsBitSet(pev->spawnflags, SF_DOOR_NO_AUTO_RETURN) && m_toggle_state == ToggleState::AtTop)
		DoorActivate();
}

bool CBaseDoor::DoorActivate()
{
	if (!UTIL_IsMasterTriggered(m_sMaster, m_hActivator))
		return false;

	if (IsBitSet(pev->spawnflags, SF_DOOR_NO_AUTO_RETURN) && m_toggle_state == ToggleState::AtTop)
	{// door should close
		DoorGoDown();
	}
	else
	{// door should open

		if (m_hActivator != nullptr && m_hActivator->IsPlayer())
		{// give health if player opened the door (medikit)
		// VARS( m_eoActivator )->health += m_bHealthValue;

			m_hActivator->GiveHealth(m_bHealthValue, DMG_GENERIC);

		}

		// play door unlock sounds
		PlayLockSounds(this, &m_ls, false, false);

		DoorGoUp();
	}

	return true;
}

void CBaseDoor::DoorGoUp()
{
	// It could be going-down, if blocked.
	ASSERT(m_toggle_state == ToggleState::AtBottom || m_toggle_state == ToggleState::GoingDown);

	// emit door moving and stop sounds on SoundChannel::Static so that the multicast doesn't
	// filter them out and leave a client stuck with looping door sounds!
	if (!IsBitSet(pev->spawnflags, SF_DOOR_SILENT))
	{
		if (m_toggle_state != ToggleState::GoingUp && m_toggle_state != ToggleState::GoingDown)
			EmitSound(SoundChannel::Static, STRING(pev->noiseMoving));
	}

	m_toggle_state = ToggleState::GoingUp;

	SetMoveDone(&CBaseDoor::DoorHitTop);
	if (ClassnameIs("func_door_rotating"))		// !!! BUGBUG Triggered doors don't work with this yet
	{
		float sign = 1.0;

		if (m_hActivator != nullptr)
		{
			CBaseEntity* pActivator = m_hActivator;

			if (!IsBitSet(pev->spawnflags, SF_DOOR_ONEWAY) && pev->movedir.y) 		// Y axis rotation, move away from the player
			{
				const Vector vec = pActivator->GetAbsOrigin() - GetAbsOrigin();
				const Vector angles{0, pActivator->GetAbsAngles().y, 0};
				UTIL_MakeVectors(angles);
				//			Vector vnext = (pActivator->GetAbsOrigin() + (pActivator->GetAbsVelocity() * 10)) - GetAbsOrigin();
				UTIL_MakeVectors(pActivator->GetAbsAngles());
				const Vector vnext = (pActivator->GetAbsOrigin() + (gpGlobals->v_forward * 10)) - GetAbsOrigin();
				if ((vec.x * vnext.y - vec.y * vnext.x) < 0)
					sign = -1.0;
			}
		}
		AngularMove(m_vecAngle2 * sign, pev->speed);
	}
	else
		LinearMove(m_vecPosition2, pev->speed);
}

void CBaseDoor::DoorHitTop()
{
	if (!IsBitSet(pev->spawnflags, SF_DOOR_SILENT))
	{
		StopSound(SoundChannel::Static, STRING(pev->noiseMoving));
		EmitSound(SoundChannel::Static, STRING(pev->noiseArrived));
	}

	ASSERT(m_toggle_state == ToggleState::GoingUp);
	m_toggle_state = ToggleState::AtTop;

	// toggle-doors don't come down automatically, they wait for refire.
	if (IsBitSet(pev->spawnflags, SF_DOOR_NO_AUTO_RETURN))
	{
		// Re-instate touch method, movement is complete
		if (!IsBitSet(pev->spawnflags, SF_DOOR_USE_ONLY))
			SetTouch(&CBaseDoor::DoorTouch);
	}
	else
	{
		// In flWait seconds, DoorGoDown will fire, unless wait is -1, then door stays open
		pev->nextthink = pev->ltime + m_flWait;
		SetThink(&CBaseDoor::DoorGoDown);

		if (m_flWait == -1)
		{
			pev->nextthink = -1;
		}
	}

	// Fire the close target (if startopen is set, then "top" is closed) - netname is the close target
	if (!IsStringNull(pev->netname) && (pev->spawnflags & SF_DOOR_START_OPEN))
		FireTargets(STRING(pev->netname), m_hActivator, this, UseType::Toggle, 0);

	SUB_UseTargets(m_hActivator, UseType::Toggle, 0); // this isn't finished
}

void CBaseDoor::DoorGoDown()
{
	if (!IsBitSet(pev->spawnflags, SF_DOOR_SILENT))
	{
		if (m_toggle_state != ToggleState::GoingUp && m_toggle_state != ToggleState::GoingDown)
			EmitSound(SoundChannel::Static, STRING(pev->noiseMoving));
	}

#ifdef DOOR_ASSERT
	ASSERT(m_toggle_state == ToggleState::AtTop);
#endif // DOOR_ASSERT
	m_toggle_state = ToggleState::GoingDown;

	SetMoveDone(&CBaseDoor::DoorHitBottom);
	if (ClassnameIs("func_door_rotating"))//rotating door
		AngularMove(m_vecAngle1, pev->speed);
	else
		LinearMove(m_vecPosition1, pev->speed);
}

void CBaseDoor::DoorHitBottom()
{
	if (!IsBitSet(pev->spawnflags, SF_DOOR_SILENT))
	{
		StopSound(SoundChannel::Static, STRING(pev->noiseMoving));
		EmitSound(SoundChannel::Static, STRING(pev->noiseArrived));
	}

	ASSERT(m_toggle_state == ToggleState::GoingDown);
	m_toggle_state = ToggleState::AtBottom;

	// Re-instate touch method, cycle is complete
	if (IsBitSet(pev->spawnflags, SF_DOOR_USE_ONLY))
	{// use only door
		SetTouch(nullptr);
	}
	else // touchable door
		SetTouch(&CBaseDoor::DoorTouch);

	SUB_UseTargets(m_hActivator, UseType::Toggle, 0); // this isn't finished

	// Fire the close target (if startopen is set, then "top" is closed) - netname is the close target
	if (!IsStringNull(pev->netname) && !(pev->spawnflags & SF_DOOR_START_OPEN))
		FireTargets(STRING(pev->netname), m_hActivator, this, UseType::Toggle, 0);
}

void CBaseDoor::Blocked(CBaseEntity* pOther)
{
	// Hurt the blocker a little.
	if (pev->dmg)
		pOther->TakeDamage({this, this, pev->dmg, DMG_CRUSH});

	// if a door has a negative wait, it would never come back if blocked,
	// so let it just squash the object to death real fast

	if (m_flWait >= 0)
	{
		if (m_toggle_state == ToggleState::GoingDown)
		{
			DoorGoUp();
		}
		else
		{
			DoorGoDown();
		}
	}

	// Block all door pieces with the same targetname here.
	if (!IsStringNull(pev->targetname))
	{
		CBaseEntity* pTarget = nullptr;

		while ((pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->targetname))) != nullptr)
		{
			if (pTarget->pev != pev)
			{
				if (pTarget->ClassnameIs("func_door") || pTarget->ClassnameIs("func_door_rotating"))
				{
					CBaseDoor* pDoor = (CBaseDoor*)pTarget;

					if (pDoor->m_flWait >= 0)
					{
						if (pDoor->GetAbsVelocity() == GetAbsVelocity() && pDoor->pev->avelocity == GetAbsVelocity())
						{
							// this is the most hacked, evil, bastardized thing I've ever seen. kjb
							if (pDoor->ClassnameIs("func_door"))
							{// set origin to realign normal doors
								pDoor->SetAbsOrigin(GetAbsOrigin());
								pDoor->SetAbsVelocity(vec3_origin);// stop!
							}
							else
							{// set angles to realign rotating doors
								pDoor->SetAbsAngles(GetAbsAngles());
								pDoor->pev->avelocity = vec3_origin;
							}
						}

						if (!IsBitSet(pev->spawnflags, SF_DOOR_SILENT))
							StopSound(SoundChannel::Static, STRING(pev->noiseMoving));

						if (pDoor->m_toggle_state == ToggleState::GoingDown)
							pDoor->DoorGoUp();
						else
							pDoor->DoorGoDown();
					}
				}
			}
		}
	}
}
