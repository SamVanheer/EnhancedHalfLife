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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"

#define noiseMoving noise1
#define noiseArrived noise2

/**
*	@details if two doors touch, they are assumed to be connected and operate as a unit.
*	TOGGLE causes the door to wait in both the start and end states for a trigger event.
*	START_OPEN causes the door to move to its destination when spawned, and operate in reverse.
*	It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
*	"movedir"        determines the opening direction
*	"targetname"	if set, no touch function will be set and a remote button or trigger field activates the door.
*	"speed"         movement speed (100 default)
*	"wait"          wait before returning (0 default, -1 = never return)
*	"lip"           lip remaining at end of move (0 default)
*	"dmg"           damage to inflict when blocked (0 default)
*/
class CBaseDoor : public CBaseToggle
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Use(const UseInfo& info) override;
	void Blocked(CBaseEntity* pOther) override;


	int	ObjectCaps() override
	{
		if (pev->spawnflags & SF_ITEM_USE_ONLY)
			return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_IMPULSE_USE;
		else
			return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION);
	}
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	/**
	*	@brief used to selectivly override defaults
	*	@details Doors not tied to anything (e.g. button, another door) can be touched, to make them activate.
	*/
	void EXPORT DoorTouch(CBaseEntity* pOther);

	// local functions
	/**
	*	@brief Causes the door to "do its thing", i.e. start moving, and cascade activation.
	*/
	bool DoorActivate();

	/**
	*	@brief Starts the door going to its "up" position (simply m_vecPosition2)
	*/
	void EXPORT DoorGoUp();

	/**
	*	@brief Starts the door going to its "down" position (simply m_vecPosition1).
	*/
	void EXPORT DoorGoDown();

	/**
	*	@brief The door has reached the "up" position. Either go back down, or wait for another activation.
	*/
	void EXPORT DoorHitTop();

	/**
	*	@brief The door has reached the "down" position. Back to quiescence.
	*/
	void EXPORT DoorHitBottom();

	byte	m_bHealthValue;//!< some doors are medi-kit doors, they give players health

	byte	m_bMoveSnd;			//!< sound a door makes while moving
	byte	m_bStopSnd;			//!< sound a door makes when it stops

	locksound_t m_ls;			//!< door lock sounds

	byte	m_bLockedSound;		//!< ordinals from entity selection
	byte	m_bLockedSentence;
	byte	m_bUnlockedSound;
	byte	m_bUnlockedSentence;
};

TYPEDESCRIPTION	CBaseDoor::m_SaveData[] =
{
	DEFINE_FIELD(CBaseDoor, m_bHealthValue, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseDoor, m_bMoveSnd, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseDoor, m_bStopSnd, FIELD_CHARACTER),

	DEFINE_FIELD(CBaseDoor, m_bLockedSound, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseDoor, m_bLockedSentence, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseDoor, m_bUnlockedSound, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseDoor, m_bUnlockedSentence, FIELD_CHARACTER),

};

IMPLEMENT_SAVERESTORE(CBaseDoor, CBaseToggle);

constexpr int DOOR_SENTENCEWAIT = 6;
constexpr int DOOR_SOUNDWAIT = 3;
constexpr float BUTTON_SOUNDWAIT = 0.5;

void PlayLockSounds(CBaseEntity* entity, locksound_t* pls, int flocked, int fbutton)
{
	// LOCKED SOUND

	// CONSIDER: consolidate the locksound_t struct (all entries are duplicates for lock/unlock)
	// CONSIDER: and condense this code.
	float flsoundwait;

	if (fbutton)
		flsoundwait = BUTTON_SOUNDWAIT;
	else
		flsoundwait = DOOR_SOUNDWAIT;

	if (flocked)
	{
		const bool fplaysound = (!IsStringNull(pls->sLockedSound) && gpGlobals->time > pls->flwaitSound);
		const bool fplaysentence = (!IsStringNull(pls->sLockedSentence) && !pls->bEOFLocked && gpGlobals->time > pls->flwaitSentence);
		float fvol;

		if (fplaysound && fplaysentence)
			fvol = 0.25;
		else
			fvol = 1.0;

		// if there is a locked sound, and we've debounced, play sound
		if (fplaysound)
		{
			// play 'door locked' sound
			entity->EmitSound(SoundChannel::Item, STRING(pls->sLockedSound), fvol);
			pls->flwaitSound = gpGlobals->time + flsoundwait;
		}

		// if there is a sentence, we've not played all in list, and we've debounced, play sound
		if (fplaysentence)
		{
			// play next 'door locked' sentence in group
			const int iprev = pls->iLockedSentence;

			pls->iLockedSentence = SENTENCEG_PlaySequentialSz(entity, STRING(pls->sLockedSentence),
				0.85, ATTN_NORM, PITCH_NORM, pls->iLockedSentence, false);
			pls->iUnlockedSentence = 0;

			// make sure we don't keep calling last sentence in list
			pls->bEOFLocked = (iprev == pls->iLockedSentence);

			pls->flwaitSentence = gpGlobals->time + DOOR_SENTENCEWAIT;
		}
	}
	else
	{
		// UNLOCKED SOUND

		const bool fplaysound = (!IsStringNull(pls->sUnlockedSound) && gpGlobals->time > pls->flwaitSound);
		const bool fplaysentence = (!IsStringNull(pls->sUnlockedSentence) && !pls->bEOFUnlocked && gpGlobals->time > pls->flwaitSentence);
		float fvol;

		// if playing both sentence and sound, lower sound volume so we hear sentence
		if (fplaysound && fplaysentence)
			fvol = 0.25;
		else
			fvol = 1.0;

		// play 'door unlocked' sound if set
		if (fplaysound)
		{
			entity->EmitSound(SoundChannel::Item, STRING(pls->sUnlockedSound), fvol);
			pls->flwaitSound = gpGlobals->time + flsoundwait;
		}

		// play next 'door unlocked' sentence in group
		if (fplaysentence)
		{
			const int iprev = pls->iUnlockedSentence;

			pls->iUnlockedSentence = SENTENCEG_PlaySequentialSz(entity, STRING(pls->sUnlockedSentence),
				0.85, ATTN_NORM, PITCH_NORM, pls->iUnlockedSentence, false);
			pls->iLockedSentence = 0;

			// make sure we don't keep calling last sentence in list
			pls->bEOFUnlocked = (iprev == pls->iUnlockedSentence);
			pls->flwaitSentence = gpGlobals->time + DOOR_SENTENCEWAIT;
		}
	}
}

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
	SetMovedir(pev);

	if (pev->skin == 0)
	{//normal door
		if (IsBitSet(pev->spawnflags, SF_DOOR_PASSABLE))
			pev->solid = Solid::Not;
		else
			pev->solid = Solid::BSP;
	}
	else
	{// special contents
		pev->solid = Solid::Not;
		SetBits(pev->spawnflags, SF_DOOR_SILENT);	// water is silent for now
	}

	pev->movetype = Movetype::Push;
	SetAbsOrigin(pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 100;

	m_vecPosition1 = pev->origin;
	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	m_vecPosition2 = m_vecPosition1 + (pev->movedir * (fabs(pev->movedir.x * (pev->size.x - 2)) + fabs(pev->movedir.y * (pev->size.y - 2)) + fabs(pev->movedir.z * (pev->size.z - 2)) - m_flLip));
	ASSERTSZ(m_vecPosition1 != m_vecPosition2, "door start/end positions are equal");
	if (IsBitSet(pev->spawnflags, SF_DOOR_START_OPEN))
	{	// swap pos1 and pos2, put door at pos2
		SetAbsOrigin(m_vecPosition2);
		m_vecPosition2 = m_vecPosition1;
		m_vecPosition1 = pev->origin;
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
	if (ClassnameIs(pev, "func_door_rotating"))		// !!! BUGBUG Triggered doors don't work with this yet
	{
		float sign = 1.0;

		if (m_hActivator != nullptr)
		{
			CBaseEntity* pActivator = m_hActivator;

			if (!IsBitSet(pev->spawnflags, SF_DOOR_ONEWAY) && pev->movedir.y) 		// Y axis rotation, move away from the player
			{
				const Vector vec = pActivator->pev->origin - pev->origin;
				const Vector angles{0, pActivator->pev->angles.y, 0};
				UTIL_MakeVectors(angles);
				//			Vector vnext = (pevToucher->origin + (pevToucher->velocity * 10)) - pev->origin;
				UTIL_MakeVectors(pActivator->pev->angles);
				const Vector vnext = (pActivator->pev->origin + (gpGlobals->v_forward * 10)) - pev->origin;
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
		FireTargets(STRING(pev->netname), m_hActivator, this, USE_TOGGLE, 0);

	SUB_UseTargets(m_hActivator, USE_TOGGLE, 0); // this isn't finished
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
	if (ClassnameIs(pev, "func_door_rotating"))//rotating door
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

	SUB_UseTargets(m_hActivator, USE_TOGGLE, 0); // this isn't finished

	// Fire the close target (if startopen is set, then "top" is closed) - netname is the close target
	if (!IsStringNull(pev->netname) && !(pev->spawnflags & SF_DOOR_START_OPEN))
		FireTargets(STRING(pev->netname), m_hActivator, this, USE_TOGGLE, 0);
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
				if (ClassnameIs(pTarget->pev, "func_door") || ClassnameIs(pTarget->pev, "func_door_rotating"))
				{
					CBaseDoor* pDoor = (CBaseDoor*) pTarget;

					if (pDoor->m_flWait >= 0)
					{
						if (pDoor->pev->velocity == pev->velocity && pDoor->pev->avelocity == pev->velocity)
						{
							// this is the most hacked, evil, bastardized thing I've ever seen. kjb
							if (ClassnameIs(pDoor->pev, "func_door"))
							{// set origin to realign normal doors
								pDoor->pev->origin = pev->origin;
								pDoor->pev->velocity = vec3_origin;// stop!
							}
							else
							{// set angles to realign rotating doors
								pDoor->pev->angles = pev->angles;
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

/**
*	@details if two doors touch, they are assumed to be connected and operate as a unit.
*	TOGGLE causes the door to wait in both the start and end states for a trigger event.
*	START_OPEN causes the door to move to its destination when spawned, and operate in reverse.
*	It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
*	You need to have an origin brush as part of this entity.
*	The center of that brush will be the point around which it is rotated.
*	It will rotate around the Z axis by default.
*	You can check either the X_AXIS or Y_AXIS box to change that.
* 
*	"distance" is how many degrees the door will be rotated.
*	"speed" determines how fast the door moves; default value is 100.
*	REVERSE will cause the door to rotate in the opposite direction.
*	"movedir"		determines the opening direction
*	"targetname"	if set, no touch field will be spawned and a remote button or trigger field activates the door.
*	"speed"			movement speed (100 default)
*	"wait"			wait before returning (0 default, -1 = never return)
*/
class CRotDoor : public CBaseDoor
{
public:
	void Spawn() override;
};

LINK_ENTITY_TO_CLASS(func_door_rotating, CRotDoor);

void CRotDoor::Spawn()
{
	Precache();
	// set the axis of rotation
	CBaseToggle::AxisDir(this);

	// check for clockwise rotation
	if (IsBitSet(pev->spawnflags, SF_DOOR_ROTATE_BACKWARDS))
		pev->movedir = pev->movedir * -1;

	//m_flWait			= 2; who the hell did this? (sjb)
	m_vecAngle1 = pev->angles;
	m_vecAngle2 = pev->angles + pev->movedir * m_flMoveDistance;

	ASSERTSZ(m_vecAngle1 != m_vecAngle2, "rotating door start/end positions are equal");

	if (IsBitSet(pev->spawnflags, SF_DOOR_PASSABLE))
		pev->solid = Solid::Not;
	else
		pev->solid = Solid::BSP;

	pev->movetype = Movetype::Push;
	SetAbsOrigin(pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 100;

	// DOOR_START_OPEN is to allow an entity to be lighted in the closed position
	// but spawn in the open position
	if (IsBitSet(pev->spawnflags, SF_DOOR_START_OPEN))
	{	// swap pos1 and pos2, put door at pos2, invert movement direction
		//TODO: swapping incorrectly?
		pev->angles = m_vecAngle2;
		const Vector vecSav = m_vecAngle1;
		m_vecAngle2 = m_vecAngle1;
		m_vecAngle1 = vecSav;
		pev->movedir = pev->movedir * -1;
	}

	m_toggle_state = ToggleState::AtBottom;

	if (IsBitSet(pev->spawnflags, SF_DOOR_USE_ONLY))
	{
		SetTouch(nullptr);
	}
	else // touchable button
		SetTouch(&CRotDoor::DoorTouch);
}

class CMomentaryDoor : public CBaseToggle
{
public:
	void	Spawn() override;
	void Precache() override;

	void	KeyValue(KeyValueData* pkvd) override;
	void	Use(const UseInfo& info) override;
	int	ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	/**
	*	@brief The door has reached needed position.
	*/
	void EXPORT DoorMoveDone();

	byte	m_bMoveSnd;			// sound a door makes while moving	
};

LINK_ENTITY_TO_CLASS(momentary_door, CMomentaryDoor);

TYPEDESCRIPTION	CMomentaryDoor::m_SaveData[] =
{
	DEFINE_FIELD(CMomentaryDoor, m_bMoveSnd, FIELD_CHARACTER),
};

IMPLEMENT_SAVERESTORE(CMomentaryDoor, CBaseToggle);

void CMomentaryDoor::Spawn()
{
	SetMovedir(pev);

	pev->solid = Solid::BSP;
	pev->movetype = Movetype::Push;

	SetAbsOrigin(pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 100;
	if (pev->dmg == 0)
		pev->dmg = 2;

	m_vecPosition1 = pev->origin;
	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	m_vecPosition2 = m_vecPosition1 + (pev->movedir * (fabs(pev->movedir.x * (pev->size.x - 2)) + fabs(pev->movedir.y * (pev->size.y - 2)) + fabs(pev->movedir.z * (pev->size.z - 2)) - m_flLip));
	ASSERTSZ(m_vecPosition1 != m_vecPosition2, "door start/end positions are equal");

	if (IsBitSet(pev->spawnflags, SF_DOOR_START_OPEN))
	{	// swap pos1 and pos2, put door at pos2
		SetAbsOrigin(m_vecPosition2);
		m_vecPosition2 = m_vecPosition1;
		m_vecPosition1 = pev->origin;
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
	default:
		pev->noiseMoving = ALLOC_STRING("common/null.wav");
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
	if (info.GetUseType() != USE_SET)		// Momentary buttons will pass down a float in here
		return;

	const float value = std::clamp(info.GetValue(), 0.0f, 1.0f);

	const Vector move = m_vecPosition1 + (value * (m_vecPosition2 - m_vecPosition1));

	const Vector delta = move - pev->origin;
	//float speed = delta.Length() * 10;
	const float speed = delta.Length() / 0.1; // move there in 0.1 sec
	if (speed == 0)
		return;

	// This entity only thinks when it moves, so if it's thinking, it's in the process of moving
	// play the sound when it starts moving (not yet thinking)
	if (pev->nextthink < pev->ltime || pev->nextthink == 0)
		EmitSound(SoundChannel::Static, STRING(pev->noiseMoving));
	// If we already moving to designated point, return
	else if (move == m_vecFinalDest)
		return;

	SetMoveDone(&CMomentaryDoor::DoorMoveDone);
	LinearMove(move, speed);
}

void CMomentaryDoor::DoorMoveDone()
{
	StopSound(SoundChannel::Static, STRING(pev->noiseMoving));
	EmitSound(SoundChannel::Static, STRING(pev->noiseArrived));
}
