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

LINK_ENTITY_TO_CLASS(func_plat, CFuncPlat);

void CFuncPlat::Setup()
{
	//pev->noiseMovement = MAKE_STRING("plats/platmove1.wav");
	//pev->noiseStopMoving = MAKE_STRING("plats/platstop1.wav");

	if (m_flTLength == 0)
		m_flTLength = 80;
	if (m_flTWidth == 0)
		m_flTWidth = 10;

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
	if (!IsStringNull(pev->noiseMovement))
		EmitSound(SoundChannel::Static, STRING(pev->noiseMovement), m_volume);

	ASSERT(m_toggle_state == ToggleState::AtTop || m_toggle_state == ToggleState::GoingUp);
	m_toggle_state = ToggleState::GoingDown;
	SetMoveDone(&CFuncPlat::CallHitBottom);
	LinearMove(m_vecPosition2, pev->speed);
}

void CFuncPlat::HitBottom()
{
	if (!IsStringNull(pev->noiseMovement))
		StopSound(SoundChannel::Static, STRING(pev->noiseMovement));

	if (!IsStringNull(pev->noiseStopMoving))
		EmitSound(SoundChannel::Weapon, STRING(pev->noiseStopMoving), m_volume);

	ASSERT(m_toggle_state == ToggleState::GoingDown);
	m_toggle_state = ToggleState::AtBottom;
}

void CFuncPlat::GoUp()
{
	if (!IsStringNull(pev->noiseMovement))
		EmitSound(SoundChannel::Static, STRING(pev->noiseMovement), m_volume);

	ASSERT(m_toggle_state == ToggleState::AtBottom || m_toggle_state == ToggleState::GoingDown);
	m_toggle_state = ToggleState::GoingUp;
	SetMoveDone(&CFuncPlat::CallHitTop);
	LinearMove(m_vecPosition1, pev->speed);
}

void CFuncPlat::HitTop()
{
	if (!IsStringNull(pev->noiseMovement))
		StopSound(SoundChannel::Static, STRING(pev->noiseMovement));

	if (!IsStringNull(pev->noiseStopMoving))
		EmitSound(SoundChannel::Weapon, STRING(pev->noiseStopMoving), m_volume);

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

	if (!IsStringNull(pev->noiseMovement))
		StopSound(SoundChannel::Static, STRING(pev->noiseMovement));

	// Send the platform back where it came from
	ASSERT(m_toggle_state == ToggleState::GoingUp || m_toggle_state == ToggleState::GoingDown);
	if (m_toggle_state == ToggleState::GoingUp)
		GoDown();
	else if (m_toggle_state == ToggleState::GoingDown)
		GoUp();
}

void CPlatTrigger::SpawnInsideTrigger(CFuncPlat* pPlatform)
{
	m_hPlatform = pPlatform;
	// Create trigger entity, "point" it at the owning platform, give it a touch method
	SetSolidType(Solid::Trigger);
	SetMovetype(Movetype::None);
	SetAbsOrigin(pPlatform->GetAbsOrigin());

	// Establish the trigger field's size
	Vector vecTMin = pPlatform->pev->mins + Vector(25, 25, 0);
	Vector vecTMax = pPlatform->pev->maxs + Vector(25, 25, 8);
	vecTMin.z = vecTMax.z - (pPlatform->m_vecPosition1.z - pPlatform->m_vecPosition2.z + 8);
	if (pPlatform->pev->size.x <= 50)
	{
		vecTMin.x = (pPlatform->pev->mins.x + pPlatform->pev->maxs.x) / 2;
		vecTMax.x = vecTMin.x + 1;
	}
	if (pPlatform->pev->size.y <= 50)
	{
		vecTMin.y = (pPlatform->pev->mins.y + pPlatform->pev->maxs.y) / 2;
		vecTMax.y = vecTMin.y + 1;
	}
	SetSize(vecTMin, vecTMax);
}

void CPlatTrigger::Touch(CBaseEntity* pOther)
{
	//Platform was removed, remove trigger
	auto platform = m_hPlatform.Get();

	if (!platform || !platform->pev)
	{
		UTIL_Remove(this);
		return;
	}

	// Ignore touches by non-players
	if (!pOther->IsPlayer())
		return;

	// Ignore touches by corpses
	if (!pOther->IsAlive())
		return;

	// Make linked platform go up/down.
	if (platform->m_toggle_state == ToggleState::AtBottom)
		platform->GoUp();
	else if (platform->m_toggle_state == ToggleState::AtTop)
		platform->pev->nextthink = platform->pev->ltime + 1;// delay going down
}
