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

/**
*	@file
*
*	button-related code
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"

constexpr int SF_BUTTON_DONTMOVE = 1;
constexpr int SF_ROTBUTTON_NOTSOLID = 1;
constexpr int SF_BUTTON_TOGGLE = 32;		// button stays pushed until reactivated
constexpr int SF_BUTTON_SPARK_IF_OFF = 64;	// button sparks in OFF state
constexpr int SF_BUTTON_TOUCH_ONLY = 256;	// button only fires as a result of USE key.

constexpr int SF_GLOBAL_SET = 1;			// Set global state to initial state on spawn

enum class GlobalTriggerMode
{
	Off,
	On,
	Dead,
	Toggle
};

class CEnvGlobal : public CPointEntity
{
public:
	void	Spawn() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	string_t	m_globalstate;
	GlobalTriggerMode m_triggermode;
	GlobalEntState m_initialstate;
};

TYPEDESCRIPTION CEnvGlobal::m_SaveData[] =
{
	DEFINE_FIELD(CEnvGlobal, m_globalstate, FIELD_STRING),
	DEFINE_FIELD(CEnvGlobal, m_triggermode, FIELD_INTEGER),
	DEFINE_FIELD(CEnvGlobal, m_initialstate, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEnvGlobal, CBaseEntity);

LINK_ENTITY_TO_CLASS(env_global, CEnvGlobal);

void CEnvGlobal::KeyValue(KeyValueData* pkvd)
{
	pkvd->fHandled = true;

	if (AreStringsEqual(pkvd->szKeyName, "globalstate"))		// State name
		m_globalstate = ALLOC_STRING(pkvd->szValue);
	else if (AreStringsEqual(pkvd->szKeyName, "triggermode")) //TODO: validate input
		m_triggermode = static_cast<GlobalTriggerMode>(atoi(pkvd->szValue));
	else if (AreStringsEqual(pkvd->szKeyName, "initialstate")) //TODO: validate input
		m_initialstate = static_cast<GlobalEntState>(atoi(pkvd->szValue));
	else
		CPointEntity::KeyValue(pkvd);
}

void CEnvGlobal::Spawn()
{
	if (IsStringNull(m_globalstate))
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}
	if (IsBitSet(pev->spawnflags, SF_GLOBAL_SET))
	{
		if (!gGlobalState.EntityInTable(m_globalstate))
			gGlobalState.EntityAdd(m_globalstate, gpGlobals->mapname, m_initialstate);
	}
}

void CEnvGlobal::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	const GlobalEntState oldState = gGlobalState.EntityGetState(m_globalstate);
	GlobalEntState newState;

	switch (m_triggermode)
	{
	case GlobalTriggerMode::Off:
		newState = GlobalEntState::Off;
		break;

	case GlobalTriggerMode::On:
		newState = GlobalEntState::On;
		break;

	case GlobalTriggerMode::Dead:
		newState = GlobalEntState::Dead;
		break;

	default:
	case GlobalTriggerMode::Toggle:
		if (oldState == GlobalEntState::On)
			newState = GlobalEntState::Off;
		else if (oldState == GlobalEntState::Off)
			newState = GlobalEntState::On;
		else
			newState = oldState;
	}

	if (gGlobalState.EntityInTable(m_globalstate))
		gGlobalState.EntitySetState(m_globalstate, newState);
	else
		gGlobalState.EntityAdd(m_globalstate, gpGlobals->mapname, newState);
}

TYPEDESCRIPTION CMultiSource::m_SaveData[] =
{
	//!!!BUGBUG FIX
	DEFINE_ARRAY(CMultiSource, m_rgEntities, FIELD_EHANDLE, MS_MAX_TARGETS),
	DEFINE_ARRAY(CMultiSource, m_rgTriggered, FIELD_BOOLEAN, MS_MAX_TARGETS),
	DEFINE_FIELD(CMultiSource, m_iTotal, FIELD_INTEGER),
	DEFINE_FIELD(CMultiSource, m_globalstate, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CMultiSource, CBaseEntity);

LINK_ENTITY_TO_CLASS(multisource, CMultiSource);

void CMultiSource::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "style") ||
		AreStringsEqual(pkvd->szKeyName, "height") ||
		AreStringsEqual(pkvd->szKeyName, "killtarget") ||
		AreStringsEqual(pkvd->szKeyName, "value1") ||
		AreStringsEqual(pkvd->szKeyName, "value2") ||
		AreStringsEqual(pkvd->szKeyName, "value3"))
		pkvd->fHandled = true;
	else if (AreStringsEqual(pkvd->szKeyName, "globalstate"))
	{
		m_globalstate = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

constexpr int SF_MULTI_INIT = 1;

void CMultiSource::Spawn()
{
	// set up think for later registration

	pev->solid = Solid::Not;
	pev->movetype = Movetype::None;
	pev->nextthink = gpGlobals->time + 0.1;
	pev->spawnflags |= SF_MULTI_INIT;	// Until it's initialized
	SetThink(&CMultiSource::Register);
}

void CMultiSource::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	//No known sources so do nothing. Prevents out of bounds access.
	if (m_iTotal <= 0)
	{
		return;
	}

	int i = 0;

	// Find the entity in our list
	while (i < m_iTotal)
		if (m_rgEntities[i++] == pCaller)
			break;

	// if we didn't find it, report error and leave
	if (i > m_iTotal)
	{
		ALERT(at_console, "MultiSrc:Used by non member %s.\n", STRING(pCaller->pev->classname));
		return;
	}

	// CONSIDER: a Use input to the multisource always toggles.  Could check useType for ON/OFF/TOGGLE

	m_rgTriggered[i - 1] = !m_rgTriggered[i - 1];

	// 
	if (IsTriggered(pActivator))
	{
		ALERT(at_aiconsole, "Multisource %s enabled (%d inputs)\n", STRING(pev->targetname), m_iTotal);
		USE_TYPE useType = USE_TOGGLE;
		if (!IsStringNull(m_globalstate))
			useType = USE_ON;
		SUB_UseTargets(nullptr, useType, 0);
	}
}

bool CMultiSource::IsTriggered(CBaseEntity*)
{
	// Still initializing?
	if (pev->spawnflags & SF_MULTI_INIT)
		return false;

	// Is everything triggered?
	int i = 0;

	while (i < m_iTotal)
	{
		if (!m_rgTriggered[i])
			break;
		i++;
	}

	if (i == m_iTotal)
	{
		if (IsStringNull(m_globalstate) || gGlobalState.EntityGetState(m_globalstate) == GlobalEntState::On)
			return true;
	}

	return false;
}

void CMultiSource::Register()
{
	m_iTotal = 0;
	memset(m_rgEntities, 0, MS_MAX_TARGETS * sizeof(EHANDLE));

	SetThink(&CMultiSource::SUB_DoNothing);

	// search for all entities which target this multisource (pev->targetname)

	edict_t* pentTarget = FIND_ENTITY_BY_STRING(nullptr, "target", STRING(pev->targetname));

	while (!IsNullEnt(pentTarget) && (m_iTotal < MS_MAX_TARGETS))
	{
		CBaseEntity* pTarget = CBaseEntity::Instance(pentTarget);
		if (pTarget)
			m_rgEntities[m_iTotal++] = pTarget;

		pentTarget = FIND_ENTITY_BY_STRING(pentTarget, "target", STRING(pev->targetname));
	}

	pentTarget = FIND_ENTITY_BY_STRING(nullptr, "classname", "multi_manager");
	while (!IsNullEnt(pentTarget) && (m_iTotal < MS_MAX_TARGETS))
	{
		CBaseEntity* pTarget = CBaseEntity::Instance(pentTarget);
		if (pTarget && pTarget->HasTarget(pev->targetname))
			m_rgEntities[m_iTotal++] = pTarget;

		pentTarget = FIND_ENTITY_BY_STRING(pentTarget, "classname", "multi_manager");
	}

	pev->spawnflags &= ~SF_MULTI_INIT;
}

// CBaseButton
TYPEDESCRIPTION CBaseButton::m_SaveData[] =
{
	DEFINE_FIELD(CBaseButton, m_fStayPushed, FIELD_BOOLEAN),
	DEFINE_FIELD(CBaseButton, m_fRotating, FIELD_BOOLEAN),

	DEFINE_FIELD(CBaseButton, m_sounds, FIELD_INTEGER),
	DEFINE_FIELD(CBaseButton, m_bLockedSound, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseButton, m_bLockedSentence, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseButton, m_bUnlockedSound, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseButton, m_bUnlockedSentence, FIELD_CHARACTER),
	//	DEFINE_FIELD( CBaseButton, m_ls, FIELD_??? ),   // This is restored in Precache()
};

IMPLEMENT_SAVERESTORE(CBaseButton, CBaseToggle);

void CBaseButton::Precache()
{
	if (IsBitSet(pev->spawnflags, SF_BUTTON_SPARK_IF_OFF))// this button should spark in OFF state
	{
		PRECACHE_SOUND("buttons/spark1.wav");
		PRECACHE_SOUND("buttons/spark2.wav");
		PRECACHE_SOUND("buttons/spark3.wav");
		PRECACHE_SOUND("buttons/spark4.wav");
		PRECACHE_SOUND("buttons/spark5.wav");
		PRECACHE_SOUND("buttons/spark6.wav");
	}

	// get door button sounds, for doors which require buttons to open
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
	case 1: m_ls.sLockedSentence = MAKE_STRING("NA"); break; // access denied
	case 2: m_ls.sLockedSentence = MAKE_STRING("ND"); break; // security lockout
	case 3: m_ls.sLockedSentence = MAKE_STRING("NF"); break; // blast door
	case 4: m_ls.sLockedSentence = MAKE_STRING("NFIRE"); break; // fire door
	case 5: m_ls.sLockedSentence = MAKE_STRING("NCHEM"); break; // chemical door
	case 6: m_ls.sLockedSentence = MAKE_STRING("NRAD"); break; // radiation door
	case 7: m_ls.sLockedSentence = MAKE_STRING("NCON"); break; // gen containment
	case 8: m_ls.sLockedSentence = MAKE_STRING("NH"); break; // maintenance door
	case 9: m_ls.sLockedSentence = MAKE_STRING("NG"); break; // broken door

	default: m_ls.sLockedSentence = iStringNull; break;
	}

	switch (m_bUnlockedSentence)
	{
	case 1: m_ls.sUnlockedSentence = MAKE_STRING("EA"); break; // access granted
	case 2: m_ls.sUnlockedSentence = MAKE_STRING("ED"); break; // security door
	case 3: m_ls.sUnlockedSentence = MAKE_STRING("EF"); break; // blast door
	case 4: m_ls.sUnlockedSentence = MAKE_STRING("EFIRE"); break; // fire door
	case 5: m_ls.sUnlockedSentence = MAKE_STRING("ECHEM"); break; // chemical door
	case 6: m_ls.sUnlockedSentence = MAKE_STRING("ERAD"); break; // radiation door
	case 7: m_ls.sUnlockedSentence = MAKE_STRING("ECON"); break; // gen containment
	case 8: m_ls.sUnlockedSentence = MAKE_STRING("EH"); break; // maintenance door

	default: m_ls.sUnlockedSentence = iStringNull; break;
	}
}

void CBaseButton::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "locked_sound"))
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
	else if (AreStringsEqual(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

bool CBaseButton::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	const ButtonCode code = ButtonResponseToTouch();

	if (code == ButtonCode::Nothing)
		return false;
	// Temporarily disable the touch function, until movement is finished.
	SetTouch(nullptr);

	m_hActivator = CBaseEntity::Instance(pevAttacker);
	if (m_hActivator == nullptr)
		return false;

	if (code == ButtonCode::Return)
	{
		EmitSound(SoundChannel::Voice, STRING(pev->noise));

		// Toggle buttons fire when they get back to their "home" position
		if (!(pev->spawnflags & SF_BUTTON_TOGGLE))
			SUB_UseTargets(m_hActivator, USE_TOGGLE, 0);
		ButtonReturn();
	}
	else // code == BUTTON_ACTIVATE
		ButtonActivate();

	return false;
}

LINK_ENTITY_TO_CLASS(func_button, CBaseButton);

void CBaseButton::Spawn()
{
	//----------------------------------------------------
	//determine sounds for buttons
	//a sound of 0 should not make a sound
	//----------------------------------------------------
	const char* pszSound = ButtonSound(m_sounds);
	PRECACHE_SOUND(pszSound);
	pev->noise = ALLOC_STRING(pszSound);

	Precache();

	if (IsBitSet(pev->spawnflags, SF_BUTTON_SPARK_IF_OFF))// this button should spark in OFF state
	{
		SetThink(&CBaseButton::ButtonSpark);
		pev->nextthink = gpGlobals->time + 0.5;// no hurry, make sure everything else spawns
	}

	SetMovedir(pev);

	pev->movetype = Movetype::Push;
	pev->solid = Solid::BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 40;

	if (pev->health > 0)
	{
		SetDamageMode(DamageMode::Yes);
	}

	if (m_flWait == 0)
		m_flWait = 1;
	if (m_flLip == 0)
		m_flLip = 4;

	m_toggle_state = ToggleState::AtBottom;
	m_vecPosition1 = pev->origin;
	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	m_vecPosition2 = m_vecPosition1 + (pev->movedir * (fabs(pev->movedir.x * (pev->size.x - 2)) + fabs(pev->movedir.y * (pev->size.y - 2)) + fabs(pev->movedir.z * (pev->size.z - 2)) - m_flLip));


	// Is this a non-moving button?
	if (((m_vecPosition2 - m_vecPosition1).Length() < 1) || (pev->spawnflags & SF_BUTTON_DONTMOVE))
		m_vecPosition2 = m_vecPosition1;

	m_fStayPushed = m_flWait == -1;
	m_fRotating = false;

	// if the button is flagged for USE button activation only, take away it's touch function and add a use function

	if (IsBitSet(pev->spawnflags, SF_BUTTON_TOUCH_ONLY)) // touchable button
	{
		SetTouch(&CBaseButton::ButtonTouch);
	}
	else
	{
		SetTouch(nullptr);
		SetUse(&CBaseButton::ButtonUse);
	}
}

const char* ButtonSound(int sound)
{
	switch (sound)
	{
	case 0: return "common/null.wav";
	case 1: return "buttons/button1.wav";
	case 2: return "buttons/button2.wav";
	case 3: return "buttons/button3.wav";
	case 4: return "buttons/button4.wav";
	case 5: return "buttons/button5.wav";
	case 6: return "buttons/button6.wav";
	case 7: return "buttons/button7.wav";
	case 8: return "buttons/button8.wav";
	case 9: return "buttons/button9.wav";
	case 10: return "buttons/button10.wav";
	case 11: return "buttons/button11.wav";
	case 12: return "buttons/latchlocked1.wav";
	case 13: return "buttons/latchunlocked1.wav";
	case 14: return "buttons/lightswitch2.wav";

		// next 6 slots reserved for any additional sliding button sounds we may add

	case 21: return "buttons/lever1.wav";
	case 22: return "buttons/lever2.wav";
	case 23: return "buttons/lever3.wav";
	case 24: return "buttons/lever4.wav";
	case 25: return "buttons/lever5.wav";

	default:return "buttons/button9.wav";
	}
}

/**
*	@brief Makes flagged buttons spark when turned off
*/
void DoSpark(CBaseEntity* entity, const Vector& location)
{
	const Vector tmp = location + entity->pev->size * 0.5;
	UTIL_Sparks(tmp);

	const float flVolume = RANDOM_FLOAT(0.25, 0.75) * 0.4;//random volume range
	switch ((int)(RANDOM_FLOAT(0, 1) * 6))
	{
	case 0: entity->EmitSound(SoundChannel::Voice, "buttons/spark1.wav", flVolume); break;
	case 1: entity->EmitSound(SoundChannel::Voice, "buttons/spark2.wav", flVolume); break;
	case 2: entity->EmitSound(SoundChannel::Voice, "buttons/spark3.wav", flVolume); break;
	case 3: entity->EmitSound(SoundChannel::Voice, "buttons/spark4.wav", flVolume); break;
	case 4: entity->EmitSound(SoundChannel::Voice, "buttons/spark5.wav", flVolume); break;
	case 5: entity->EmitSound(SoundChannel::Voice, "buttons/spark6.wav", flVolume); break;
	}
}

void CBaseButton::ButtonSpark()
{
	SetThink(&CBaseButton::ButtonSpark);
	pev->nextthink = pev->ltime + (0.1 + RANDOM_FLOAT(0, 1.5));// spark again at random interval

	DoSpark(this, pev->mins);
}

void CBaseButton::ButtonUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Ignore touches if button is moving, or pushed-in and waiting to auto-come-out.
	// UNDONE: Should this use ButtonResponseToTouch() too?
	if (m_toggle_state == ToggleState::GoingUp || m_toggle_state == ToggleState::GoingDown)
		return;

	m_hActivator = pActivator;
	if (m_toggle_state == ToggleState::AtTop)
	{
		if (!m_fStayPushed && IsBitSet(pev->spawnflags, SF_BUTTON_TOGGLE))
		{
			EmitSound(SoundChannel::Voice, STRING(pev->noise));

			//SUB_UseTargets( m_eoActivator );
			ButtonReturn();
		}
	}
	else
		ButtonActivate();
}

CBaseButton::ButtonCode CBaseButton::ButtonResponseToTouch()
{
	// Ignore touches if button is moving, or pushed-in and waiting to auto-come-out.
	if (m_toggle_state == ToggleState::GoingUp ||
		m_toggle_state == ToggleState::GoingDown ||
		(m_toggle_state == ToggleState::AtTop && !m_fStayPushed && !IsBitSet(pev->spawnflags, SF_BUTTON_TOGGLE)))
		return ButtonCode::Nothing;

	if (m_toggle_state == ToggleState::AtTop)
	{
		if ((IsBitSet(pev->spawnflags, SF_BUTTON_TOGGLE)) && !m_fStayPushed)
		{
			return ButtonCode::Return;
		}
	}
	else
		return ButtonCode::Activate;

	return ButtonCode::Nothing;
}

void CBaseButton::ButtonTouch(CBaseEntity* pOther)
{
	// Ignore touches by anything but players
	if (!pOther->IsPlayer())
		return;

	m_hActivator = pOther;

	const ButtonCode code = ButtonResponseToTouch();

	if (code == ButtonCode::Nothing)
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
	{
		// play button locked sound
		PlayLockSounds(this, &m_ls, true, true);
		return;
	}

	// Temporarily disable the touch function, until movement is finished.
	SetTouch(nullptr);

	if (code == ButtonCode::Return)
	{
		EmitSound(SoundChannel::Voice, STRING(pev->noise));
		SUB_UseTargets(m_hActivator, USE_TOGGLE, 0);
		ButtonReturn();
	}
	else	// code == BUTTON_ACTIVATE
		ButtonActivate();
}

void CBaseButton::ButtonActivate()
{
	EmitSound(SoundChannel::Voice, STRING(pev->noise));

	if (!UTIL_IsMasterTriggered(m_sMaster, m_hActivator))
	{
		// button is locked, play locked sound
		PlayLockSounds(this, &m_ls, true, true);
		return;
	}
	else
	{
		// button is unlocked, play unlocked sound
		PlayLockSounds(this, &m_ls, false, true);
	}

	ASSERT(m_toggle_state == ToggleState::AtBottom);
	m_toggle_state = ToggleState::GoingUp;

	SetMoveDone(&CBaseButton::TriggerAndWait);
	if (!m_fRotating)
		LinearMove(m_vecPosition2, pev->speed);
	else
		AngularMove(m_vecAngle2, pev->speed);
}

void CBaseButton::TriggerAndWait()
{
	ASSERT(m_toggle_state == ToggleState::GoingUp);

	if (!UTIL_IsMasterTriggered(m_sMaster, m_hActivator))
		return;

	m_toggle_state = ToggleState::AtTop;

	// If button automatically comes back out, start it moving out.
	// Else re-instate touch method
	if (m_fStayPushed || IsBitSet(pev->spawnflags, SF_BUTTON_TOGGLE))
	{
		if (!IsBitSet(pev->spawnflags, SF_BUTTON_TOUCH_ONLY)) // this button only works if USED, not touched!
		{
			// ALL buttons are now use only
			SetTouch(nullptr);
		}
		else
			SetTouch(&CBaseButton::ButtonTouch);
	}
	else
	{
		pev->nextthink = pev->ltime + m_flWait;
		SetThink(&CBaseButton::ButtonReturn);
	}

	pev->frame = 1;			// use alternate textures


	SUB_UseTargets(m_hActivator, USE_TOGGLE, 0);
}

void CBaseButton::ButtonReturn()
{
	ASSERT(m_toggle_state == ToggleState::AtTop);
	m_toggle_state = ToggleState::GoingDown;

	SetMoveDone(&CBaseButton::ButtonBackHome);
	if (!m_fRotating)
		LinearMove(m_vecPosition1, pev->speed);
	else
		AngularMove(m_vecAngle1, pev->speed);

	pev->frame = 0;			// use normal textures
}

void CBaseButton::ButtonBackHome()
{
	ASSERT(m_toggle_state == ToggleState::GoingDown);
	m_toggle_state = ToggleState::AtBottom;

	if (IsBitSet(pev->spawnflags, SF_BUTTON_TOGGLE))
	{
		//EmitSound(SoundChannel::Voice, STRING(pev->noise));

		SUB_UseTargets(m_hActivator, USE_TOGGLE, 0);
	}


	if (!IsStringNull(pev->target))
	{
		edict_t* pentTarget = nullptr;
		for (;;)
		{
			pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));

			if (IsNullEnt(pentTarget))
				break;

			if (!ClassnameIs(pentTarget, "multisource"))
				continue;
			CBaseEntity* pTarget = CBaseEntity::Instance(pentTarget);

			if (pTarget)
				pTarget->Use(m_hActivator, this, USE_TOGGLE, 0);
		}
	}

	// Re-instate touch method, movement cycle is complete.
	if (!IsBitSet(pev->spawnflags, SF_BUTTON_TOUCH_ONLY)) // this button only works if USED, not touched!
	{
		// All buttons are now use only	
		SetTouch(nullptr);
	}
	else
		SetTouch(&CBaseButton::ButtonTouch);

	// reset think for a sparking button
	if (IsBitSet(pev->spawnflags, SF_BUTTON_SPARK_IF_OFF))
	{
		SetThink(&CBaseButton::ButtonSpark);
		pev->nextthink = gpGlobals->time + 0.5;// no hurry.
	}
}

/**
*	@brief Rotating button (aka "lever")
*/
class CRotButton : public CBaseButton
{
public:
	void Spawn() override;
};

LINK_ENTITY_TO_CLASS(func_rot_button, CRotButton);

void CRotButton::Spawn()
{
	//----------------------------------------------------
	//determine sounds for buttons
	//a sound of 0 should not make a sound
	//----------------------------------------------------
	const char* pszSound = ButtonSound(m_sounds);
	PRECACHE_SOUND(pszSound);
	pev->noise = ALLOC_STRING(pszSound);

	// set the axis of rotation
	CBaseToggle::AxisDir(pev);

	// check for clockwise rotation
	if (IsBitSet(pev->spawnflags, SF_DOOR_ROTATE_BACKWARDS))
		pev->movedir = pev->movedir * -1;

	pev->movetype = Movetype::Push;

	if (pev->spawnflags & SF_ROTBUTTON_NOTSOLID)
		pev->solid = Solid::Not;
	else
		pev->solid = Solid::BSP;

	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 40;

	if (m_flWait == 0)
		m_flWait = 1;

	if (pev->health > 0)
	{
		SetDamageMode(DamageMode::Yes);
	}

	m_toggle_state = ToggleState::AtBottom;
	m_vecAngle1 = pev->angles;
	m_vecAngle2 = pev->angles + pev->movedir * m_flMoveDistance;
	ASSERTSZ(m_vecAngle1 != m_vecAngle2, "rotating button start/end positions are equal");

	m_fStayPushed = m_flWait == -1;
	m_fRotating = true;

	// if the button is flagged for USE button activation only, take away it's touch function and add a use function
	if (!IsBitSet(pev->spawnflags, SF_BUTTON_TOUCH_ONLY))
	{
		SetTouch(nullptr);
		SetUse(&CRotButton::ButtonUse);
	}
	else // touchable button
		SetTouch(&CRotButton::ButtonTouch);

	//SetTouch( ButtonTouch );
}

/**
*	@brief Make this button behave like a door (HACKHACK)
*	@details This will disable use and make the button solid
*	rotating buttons were made Solid::Not by default since their were some collision problems with them...
*/
constexpr int SF_MOMENTARY_DOOR = 0x0001;
constexpr int SF_MOMENTARY_AUTO_RETURN = 16;

class CMomentaryRotButton : public CBaseToggle
{
public:
	void	Spawn() override;
	void	KeyValue(KeyValueData* pkvd) override;
	int	ObjectCaps() override
	{
		const int flags = CBaseToggle::ObjectCaps() & (~FCAP_ACROSS_TRANSITION);
		if (pev->spawnflags & SF_MOMENTARY_DOOR)
			return flags;
		return flags | FCAP_CONTINUOUS_USE;
	}
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void	EXPORT Off();
	void	EXPORT Return();
	void	UpdateSelf(float value);
	void	UpdateSelfReturn(float value);
	void	UpdateAllButtons(float value, bool start);

	void	PlaySound();
	void	UpdateTarget(float value);

	static CMomentaryRotButton* Instance(edict_t* pent) { return (CMomentaryRotButton*)GET_PRIVATE(pent); }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	bool m_lastUsed;
	int		m_direction;
	float	m_returnSpeed;
	Vector	m_start;
	Vector	m_end;
	int		m_sounds;
};

TYPEDESCRIPTION CMomentaryRotButton::m_SaveData[] =
{
	DEFINE_FIELD(CMomentaryRotButton, m_lastUsed, FIELD_BOOLEAN),
	DEFINE_FIELD(CMomentaryRotButton, m_direction, FIELD_INTEGER),
	DEFINE_FIELD(CMomentaryRotButton, m_returnSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CMomentaryRotButton, m_start, FIELD_VECTOR),
	DEFINE_FIELD(CMomentaryRotButton, m_end, FIELD_VECTOR),
	DEFINE_FIELD(CMomentaryRotButton, m_sounds, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CMomentaryRotButton, CBaseToggle);

LINK_ENTITY_TO_CLASS(momentary_rot_button, CMomentaryRotButton);

void CMomentaryRotButton::Spawn()
{
	CBaseToggle::AxisDir(pev);

	if (pev->speed == 0)
		pev->speed = 100;

	if (m_flMoveDistance < 0)
	{
		m_start = pev->angles + pev->movedir * m_flMoveDistance;
		m_end = pev->angles;
		m_direction = 1;		// This will toggle to -1 on the first use()
		m_flMoveDistance = -m_flMoveDistance;
	}
	else
	{
		m_start = pev->angles;
		m_end = pev->angles + pev->movedir * m_flMoveDistance;
		m_direction = -1;		// This will toggle to +1 on the first use()
	}

	if (pev->spawnflags & SF_MOMENTARY_DOOR)
		pev->solid = Solid::BSP;
	else
		pev->solid = Solid::Not;

	pev->movetype = Movetype::Push;
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	const char* pszSound = ButtonSound(m_sounds);
	PRECACHE_SOUND(pszSound);
	pev->noise = ALLOC_STRING(pszSound);
	m_lastUsed = false;
}

void CMomentaryRotButton::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "returnspeed"))
	{
		m_returnSpeed = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CMomentaryRotButton::PlaySound()
{
	EmitSound(SoundChannel::Voice, STRING(pev->noise));
}

// BUGBUG: This design causes a latency.  When the button is retriggered, the first impulse
// will send the target in the wrong direction because the parameter is calculated based on the
// current, not future position.
void CMomentaryRotButton::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	pev->ideal_yaw = CBaseToggle::AxisDelta(pev->spawnflags, pev->angles, m_start) / m_flMoveDistance;

	UpdateAllButtons(pev->ideal_yaw, true);

	// Calculate destination angle and use it to predict value, this prevents sending target in wrong direction on retriggering
	const Vector dest = pev->angles + pev->avelocity * (pev->nextthink - pev->ltime);
	const float value1 = CBaseToggle::AxisDelta(pev->spawnflags, dest, m_start) / m_flMoveDistance;
	UpdateTarget(value1);
}

void CMomentaryRotButton::UpdateAllButtons(float value, bool start)
{
	// Update all rot buttons attached to the same target
	edict_t* pentTarget = nullptr;
	for (;;)
	{

		pentTarget = FIND_ENTITY_BY_STRING(pentTarget, "target", STRING(pev->target));
		if (IsNullEnt(pentTarget))
			break;

		if (ClassnameIs(VARS(pentTarget), "momentary_rot_button"))
		{
			CMomentaryRotButton* pEntity = CMomentaryRotButton::Instance(pentTarget);
			if (pEntity)
			{
				if (start)
					pEntity->UpdateSelf(value);
				else
					pEntity->UpdateSelfReturn(value);
			}
		}
	}
}

void CMomentaryRotButton::UpdateSelf(float value)
{
	bool fplaysound = false;

	if (!m_lastUsed)
	{
		fplaysound = true;
		m_direction = -m_direction;
	}
	m_lastUsed = true;

	pev->nextthink = pev->ltime + 0.1;
	if (m_direction > 0 && value >= 1.0)
	{
		pev->avelocity = vec3_origin;
		pev->angles = m_end;
		return;
	}
	else if (m_direction < 0 && value <= 0)
	{
		pev->avelocity = vec3_origin;
		pev->angles = m_start;
		return;
	}

	if (fplaysound)
		PlaySound();

	// HACKHACK -- If we're going slow, we'll get multiple player packets per frame, bump nexthink on each one to avoid stalling
	if (pev->nextthink < pev->ltime)
		pev->nextthink = pev->ltime + 0.1;
	else
		pev->nextthink += 0.1;

	pev->avelocity = (m_direction * pev->speed) * pev->movedir;
	SetThink(&CMomentaryRotButton::Off);
}

void CMomentaryRotButton::UpdateTarget(float value)
{
	if (!IsStringNull(pev->target))
	{
		edict_t* pentTarget = nullptr;
		for (;;)
		{
			pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));
			if (IsNullEnt(pentTarget))
				break;
			CBaseEntity* pEntity = CBaseEntity::Instance(pentTarget);
			if (pEntity)
			{
				pEntity->Use(this, this, USE_SET, value);
			}
		}
	}
}

void CMomentaryRotButton::Off()
{
	pev->avelocity = vec3_origin;
	m_lastUsed = false;
	if (IsBitSet(pev->spawnflags, SF_MOMENTARY_AUTO_RETURN) && m_returnSpeed > 0)
	{
		SetThink(&CMomentaryRotButton::Return);
		pev->nextthink = pev->ltime + 0.1;
		m_direction = -1;
	}
	else
		SetThink(nullptr);
}

void CMomentaryRotButton::Return()
{
	const float value = CBaseToggle::AxisDelta(pev->spawnflags, pev->angles, m_start) / m_flMoveDistance;

	UpdateAllButtons(value, false);	// This will end up calling UpdateSelfReturn() n times, but it still works right
	if (value > 0)
		UpdateTarget(value);
}

void CMomentaryRotButton::UpdateSelfReturn(float value)
{
	if (value <= 0)
	{
		pev->avelocity = vec3_origin;
		pev->angles = m_start;
		pev->nextthink = -1;
		SetThink(nullptr);
	}
	else
	{
		pev->avelocity = -m_returnSpeed * pev->movedir;
		pev->nextthink = pev->ltime + 0.1;
	}
}

constexpr int SF_SPARK_TOGGLE = 1 << 5;
constexpr int SF_SPARK_START_ON = 1 << 6;

class CEnvSpark : public CBaseEntity
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	EXPORT SparkThink();
	void	EXPORT SparkStart(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	EXPORT SparkStop(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	KeyValue(KeyValueData* pkvd) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	float	m_flDelay;
};

TYPEDESCRIPTION CEnvSpark::m_SaveData[] =
{
	DEFINE_FIELD(CEnvSpark, m_flDelay, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CEnvSpark, CBaseEntity);

LINK_ENTITY_TO_CLASS(env_spark, CEnvSpark);
LINK_ENTITY_TO_CLASS(env_debris, CEnvSpark);

void CEnvSpark::Spawn()
{
	SetThink(nullptr);
	SetUse(nullptr);

	if (IsBitSet(pev->spawnflags, SF_SPARK_TOGGLE)) // Use for on/off
	{
		if (IsBitSet(pev->spawnflags, SF_SPARK_START_ON)) // Start on
		{
			SetThink(&CEnvSpark::SparkThink);	// start sparking
			SetUse(&CEnvSpark::SparkStop);		// set up +USE to stop sparking
		}
		else
			SetUse(&CEnvSpark::SparkStart);
	}
	else
		SetThink(&CEnvSpark::SparkThink);

	pev->nextthink = gpGlobals->time + (0.1 + RANDOM_FLOAT(0, 1.5));

	if (m_flDelay <= 0)
		m_flDelay = 1.5;

	Precache();
}

void CEnvSpark::Precache()
{
	PRECACHE_SOUND("buttons/spark1.wav");
	PRECACHE_SOUND("buttons/spark2.wav");
	PRECACHE_SOUND("buttons/spark3.wav");
	PRECACHE_SOUND("buttons/spark4.wav");
	PRECACHE_SOUND("buttons/spark5.wav");
	PRECACHE_SOUND("buttons/spark6.wav");
}

void CEnvSpark::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "MaxDelay"))
	{
		m_flDelay = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "style") ||
		AreStringsEqual(pkvd->szKeyName, "height") ||
		AreStringsEqual(pkvd->szKeyName, "killtarget") ||
		AreStringsEqual(pkvd->szKeyName, "value1") ||
		AreStringsEqual(pkvd->szKeyName, "value2") ||
		AreStringsEqual(pkvd->szKeyName, "value3"))
		pkvd->fHandled = true;
	else
		CBaseEntity::KeyValue(pkvd);
}

void EXPORT CEnvSpark::SparkThink()
{
	pev->nextthink = gpGlobals->time + 0.1 + RANDOM_FLOAT(0, m_flDelay);
	DoSpark(this, pev->origin);
}

void EXPORT CEnvSpark::SparkStart(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	SetUse(&CEnvSpark::SparkStop);
	SetThink(&CEnvSpark::SparkThink);
	pev->nextthink = gpGlobals->time + (0.1 + RANDOM_FLOAT(0, m_flDelay));
}

void EXPORT CEnvSpark::SparkStop(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	SetUse(&CEnvSpark::SparkStart);
	SetThink(nullptr);
}

constexpr int SF_BTARGET_USE = 0x0001;
constexpr int SF_BTARGET_ON = 0x0002;

class CButtonTarget : public CBaseEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	int	ObjectCaps() override;
};

LINK_ENTITY_TO_CLASS(button_target, CButtonTarget);

void CButtonTarget::Spawn()
{
	pev->movetype = Movetype::Push;
	pev->solid = Solid::BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));
	SetDamageMode(DamageMode::Yes);

	if (IsBitSet(pev->spawnflags, SF_BTARGET_ON))
		pev->frame = 1;
}

void CButtonTarget::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, (int)pev->frame))
		return;
	pev->frame = 1 - pev->frame;
	if (pev->frame)
		SUB_UseTargets(pActivator, USE_ON, 0);
	else
		SUB_UseTargets(pActivator, USE_OFF, 0);
}

int	CButtonTarget::ObjectCaps()
{
	const int caps = CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;

	if (IsBitSet(pev->spawnflags, SF_BTARGET_USE))
		return caps | FCAP_IMPULSE_USE;
	else
		return caps;
}

bool CButtonTarget::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	Use(Instance(pevAttacker), this, USE_TOGGLE, 0);

	return true;
}
