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

#include "CFuncPlat.hpp"
#include "CPlatTrigger.hpp"

LINK_ENTITY_TO_CLASS(func_plat, CFuncPlat);

void CFuncPlat::Setup()
{
	//m_iszMovingSound = MAKE_STRING("plats/platmove1.wav");
	//m_iszArrivedSound = MAKE_STRING("plats/platstop1.wav");

	SetAbsAngles(vec3_origin);

	SetSolidType(Solid::BSP);
	SetMovetype(Movetype::Push);

	SetAbsOrigin(GetAbsOrigin());		// set size and link into world
	SetSize(pev->mins, pev->maxs);
	SetModel(STRING(pev->model));

	// vecPosition1 is the top position, vecPosition2 is the bottom
	m_vecPosition1 = GetAbsOrigin();
	m_vecPosition2 = GetAbsOrigin();
	if (m_flHeight != 0)
		m_vecPosition2.z = GetAbsOrigin().z - m_flHeight;
	else
		m_vecPosition2.z = GetAbsOrigin().z - pev->size.z + 8;
	if (pev->speed == 0)
		pev->speed = 150;

	if (m_volume == 0)
		m_volume = 0.85;
}

void CFuncPlat::Precache()
{
	CBasePlatTrain::Precache();
	//PRECACHE_SOUND("plats/platmove1.wav");
	//PRECACHE_SOUND("plats/platstop1.wav");
	if (!IsTogglePlat())
		GetClassPtr((CPlatTrigger*)nullptr)->SpawnInsideTrigger(this);		// the "start moving" trigger
}

void CFuncPlat::Spawn()
{
	Setup();

	Precache();

	// If this platform is the target of some button, it starts at the TOP position,
	// and is brought down by that button.  Otherwise, it starts at BOTTOM.
	if (!IsStringNull(pev->targetname))
	{
		SetAbsOrigin(m_vecPosition1);
		m_toggle_state = ToggleState::AtTop;
		SetUse(&CFuncPlat::PlatUse);
	}
	else
	{
		SetAbsOrigin(m_vecPosition2);
		m_toggle_state = ToggleState::AtBottom;
	}
}

void CFuncPlat::PlatUse(const UseInfo& info)
{
	if (IsTogglePlat())
	{
		// Top is off, bottom is on
		const bool on = m_toggle_state == ToggleState::AtBottom;

		if (!ShouldToggle(info.GetUseType(), on))
			return;

		if (m_toggle_state == ToggleState::AtTop)
			GoDown();
		else if (m_toggle_state == ToggleState::AtBottom)
			GoUp();
	}
	else
	{
		SetUse(nullptr);

		if (m_toggle_state == ToggleState::AtTop)
			GoDown();
	}
}

void CFuncPlat::GoDown()
{
	if (!IsStringNull(m_iszMovingSound))
		EmitSound(SoundChannel::Static, STRING(m_iszMovingSound), m_volume);

	ASSERT(m_toggle_state == ToggleState::AtTop || m_toggle_state == ToggleState::GoingUp);
	m_toggle_state = ToggleState::GoingDown;
	SetMoveDone(&CFuncPlat::CallHitBottom);
	LinearMove(m_vecPosition2, pev->speed);
}

void CFuncPlat::HitBottom()
{
	if (!IsStringNull(m_iszMovingSound))
		StopSound(SoundChannel::Static, STRING(m_iszMovingSound));

	if (!IsStringNull(m_iszArrivedSound))
		EmitSound(SoundChannel::Weapon, STRING(m_iszArrivedSound), m_volume);

	ASSERT(m_toggle_state == ToggleState::GoingDown);
	m_toggle_state = ToggleState::AtBottom;
}

void CFuncPlat::GoUp()
{
	if (!IsStringNull(m_iszMovingSound))
		EmitSound(SoundChannel::Static, STRING(m_iszMovingSound), m_volume);

	ASSERT(m_toggle_state == ToggleState::AtBottom || m_toggle_state == ToggleState::GoingDown);
	m_toggle_state = ToggleState::GoingUp;
	SetMoveDone(&CFuncPlat::CallHitTop);
	LinearMove(m_vecPosition1, pev->speed);
}

void CFuncPlat::HitTop()
{
	if (!IsStringNull(m_iszMovingSound))
		StopSound(SoundChannel::Static, STRING(m_iszMovingSound));

	if (!IsStringNull(m_iszArrivedSound))
		EmitSound(SoundChannel::Weapon, STRING(m_iszArrivedSound), m_volume);

	ASSERT(m_toggle_state == ToggleState::GoingUp);
	m_toggle_state = ToggleState::AtTop;

	if (!IsTogglePlat())
	{
		// After a delay, the platform will automatically start going down again.
		SetThink(&CFuncPlat::CallGoDown);
		pev->nextthink = pev->ltime + 3;
	}
}

void CFuncPlat::Blocked(CBaseEntity* pOther)
{
	ALERT(at_aiconsole, "%s Blocked by %s\n", GetClassname(), pOther->GetClassname());
	// Hurt the blocker a little
	pOther->TakeDamage({this, this, 1, DMG_CRUSH});

	if (!IsStringNull(m_iszMovingSound))
		StopSound(SoundChannel::Static, STRING(m_iszMovingSound));

	// Send the platform back where it came from
	ASSERT(m_toggle_state == ToggleState::GoingUp || m_toggle_state == ToggleState::GoingDown);
	if (m_toggle_state == ToggleState::GoingUp)
		GoDown();
	else if (m_toggle_state == ToggleState::GoingDown)
		GoUp();
}
