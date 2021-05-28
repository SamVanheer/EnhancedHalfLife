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

#include "CBaseTrigger.hpp"

//TODO: is this even needed? there's no spawn method for this so it just creates an empty entity
LINK_ENTITY_TO_CLASS(trigger, CBaseTrigger);

void CBaseTrigger::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "activate_sound"))
	{
		m_ActivateSound = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CBaseTrigger::InitTrigger()
{
	// trigger angles are used for one-way touches.  An angle of 0 is assumed
	// to mean no restrictions, so use a yaw of 360 instead.
	if (GetAbsAngles() != vec3_origin)
		SetMovedir(this);
	SetSolidType(Solid::Trigger);
	SetMovetype(Movetype::None);
	SetModel(STRING(pev->model));    // set size and link into world
	if (CVAR_GET_FLOAT("showtriggers") == 0)
		SetBits(pev->effects, EF_NODRAW);
}

void CBaseTrigger::ActivateMultiTrigger(CBaseEntity* pActivator)
{
	if (pev->nextthink > gpGlobals->time)
		return;         // still waiting for reset time

	if (!UTIL_IsMasterTriggered(m_iszMaster, pActivator))
		return;

	if (!IsStringNull(m_ActivateSound))
		EmitSound(SoundChannel::Voice, STRING(m_ActivateSound));

	m_hActivator = pActivator;
	SUB_UseTargets(m_hActivator, UseType::Toggle, 0);

	if (!IsStringNull(pev->message) && pActivator->IsPlayer())
	{
		UTIL_ShowMessage(STRING(pev->message), static_cast<CBasePlayer*>(pActivator));
	}

	if (m_flWait > 0)
	{
		SetThink(&CBaseTrigger::MultiWaitOver);
		pev->nextthink = gpGlobals->time + m_flWait;
	}
	else
	{
		// we can't just remove (self) here, because this is a touch function
		// called while C code is looping through area links...
		SetTouch(nullptr);
		pev->nextthink = gpGlobals->time + 0.1;
		SetThink(&CBaseTrigger::SUB_Remove);
	}
}

void CBaseTrigger::MultiWaitOver()
{
	SetThink(nullptr);
}

void CBaseTrigger::ToggleUse(const UseInfo& info)
{
	if (GetSolidType() == Solid::Not)
	{// if the trigger is off, turn it on
		SetSolidType(Solid::Trigger);

		// Force retouch
		gpGlobals->force_retouch++;
	}
	else
	{// turn the trigger off
		SetSolidType(Solid::Not);
	}
	SetAbsOrigin(GetAbsOrigin());
}
