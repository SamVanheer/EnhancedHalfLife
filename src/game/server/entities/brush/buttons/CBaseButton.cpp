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

#include "effects/CEnvSpark.hpp"

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

bool CBaseButton::TakeDamage(const TakeDamageInfo& info)
{
	const ButtonCode code = ButtonResponseToTouch();

	if (code == ButtonCode::Nothing)
		return false;
	// Temporarily disable the touch function, until movement is finished.
	SetTouch(nullptr);

	m_hActivator = info.GetAttacker();
	if (m_hActivator == nullptr)
		return false;

	if (code == ButtonCode::Return)
	{
		EmitSound(SoundChannel::Voice, STRING(pev->noise));

		// Toggle buttons fire when they get back to their "home" position
		if (!(pev->spawnflags & SF_BUTTON_TOGGLE))
			SUB_UseTargets(m_hActivator, UseType::Toggle, 0);
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

	SetMovedir(this);

	SetMovetype(Movetype::Push);
	SetSolidType(Solid::BSP);
	SetModel(STRING(pev->model));

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
	m_vecPosition1 = GetAbsOrigin();
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

void CBaseButton::ButtonSpark()
{
	SetThink(&CBaseButton::ButtonSpark);
	pev->nextthink = pev->ltime + (0.1 + RANDOM_FLOAT(0, 1.5));// spark again at random interval

	DoSpark(this, pev->mins);
}

void CBaseButton::ButtonUse(const UseInfo& info)
{
	// Ignore touches if button is moving, or pushed-in and waiting to auto-come-out.
	// UNDONE: Should this use ButtonResponseToTouch() too?
	if (m_toggle_state == ToggleState::GoingUp || m_toggle_state == ToggleState::GoingDown)
		return;

	m_hActivator = info.GetActivator();
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
		SUB_UseTargets(m_hActivator, UseType::Toggle, 0);
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


	SUB_UseTargets(m_hActivator, UseType::Toggle, 0);
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

		SUB_UseTargets(m_hActivator, UseType::Toggle, 0);
	}


	if (!IsStringNull(pev->target))
	{
		CBaseEntity* pTarget = nullptr;

		while ((pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target))) != nullptr)
		{
			if (!pTarget->ClassnameIs("multisource"))
				continue;

			pTarget->Use({m_hActivator, this, UseType::Toggle});
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
