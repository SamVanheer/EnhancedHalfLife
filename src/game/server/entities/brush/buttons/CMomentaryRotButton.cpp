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

#include "CMomentaryRotButton.hpp"

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
	CBaseToggle::AxisDir(this);

	if (pev->speed == 0)
		pev->speed = 100;

	if (m_flMoveDistance < 0)
	{
		m_start = GetAbsAngles() + pev->movedir * m_flMoveDistance;
		m_end = GetAbsAngles();
		m_direction = 1;		// This will toggle to -1 on the first use()
		m_flMoveDistance = -m_flMoveDistance;
	}
	else
	{
		m_start = GetAbsAngles();
		m_end = GetAbsAngles() + pev->movedir * m_flMoveDistance;
		m_direction = -1;		// This will toggle to +1 on the first use()
	}

	if (pev->spawnflags & SF_MOMENTARY_DOOR)
		SetSolidType(Solid::BSP);
	else
		SetSolidType(Solid::Not);

	SetMovetype(Movetype::Push);
	SetAbsOrigin(GetAbsOrigin());
	SetModel(STRING(pev->model));

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
void CMomentaryRotButton::Use(const UseInfo& info)
{
	pev->ideal_yaw = CBaseToggle::AxisDelta(pev->spawnflags, GetAbsAngles(), m_start) / m_flMoveDistance;

	UpdateAllButtons(pev->ideal_yaw, true);

	// Calculate destination angle and use it to predict value, this prevents sending target in wrong direction on retriggering
	const Vector dest = GetAbsAngles() + pev->avelocity * (pev->nextthink - pev->ltime);
	const float value1 = CBaseToggle::AxisDelta(pev->spawnflags, dest, m_start) / m_flMoveDistance;
	UpdateTarget(value1);
}

void CMomentaryRotButton::UpdateAllButtons(float value, bool start)
{
	// Update all rot buttons attached to the same target
	CBaseEntity* pTarget = nullptr;

	while ((pTarget = UTIL_FindEntityByTarget(pTarget, STRING(pev->target))) != nullptr)
	{
		if (pTarget->ClassnameIs("momentary_rot_button"))
		{
			auto pEntity = static_cast<CMomentaryRotButton*>(pTarget);

			if (start)
				pEntity->UpdateSelf(value);
			else
				pEntity->UpdateSelfReturn(value);
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
		SetAbsAngles(m_end);
		return;
	}
	else if (m_direction < 0 && value <= 0)
	{
		pev->avelocity = vec3_origin;
		SetAbsAngles(m_start);
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
		CBaseEntity* pTarget = nullptr;

		while ((pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target))) != nullptr)
		{
			pTarget->Use({this, this, UseType::Set, value});
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
	const float value = CBaseToggle::AxisDelta(pev->spawnflags, GetAbsAngles(), m_start) / m_flMoveDistance;

	UpdateAllButtons(value, false);	// This will end up calling UpdateSelfReturn() n times, but it still works right
	if (value > 0)
		UpdateTarget(value);
}

void CMomentaryRotButton::UpdateSelfReturn(float value)
{
	if (value <= 0)
	{
		pev->avelocity = vec3_origin;
		SetAbsAngles(m_start);
		pev->nextthink = -1;
		SetThink(nullptr);
	}
	else
	{
		pev->avelocity = -m_returnSpeed * pev->movedir;
		pev->nextthink = pev->ltime + 0.1;
	}
}
