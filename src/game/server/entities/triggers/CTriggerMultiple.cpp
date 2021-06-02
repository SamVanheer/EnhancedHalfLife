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

#include "CTriggerMultiple.hpp"

void CTriggerMultiple::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "activate_sound"))
	{
		m_ActivateSound = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		BaseClass::KeyValue(pkvd);
}

void CTriggerMultiple::Spawn()
{
	if (m_flWait == 0)
		m_flWait = 0.2;

	InitTrigger();

	ASSERTSZ(pev->health == 0, "trigger_multiple with health");
	SetTouch(&CTriggerMultiple::MultiTouch);
}

void CTriggerMultiple::MultiTouch(CBaseEntity* pOther)
{
	// Only touch clients, monsters, or pushables (depending on flags)
	if (((pOther->pev->flags & FL_CLIENT) && !(pev->spawnflags & SF_TRIGGER_NOCLIENTS)) ||
		((pOther->pev->flags & FL_MONSTER) && (pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS)) ||
		(pev->spawnflags & SF_TRIGGER_PUSHABLES) && pOther->ClassnameIs("func_pushable"))
	{

#if 0
		// if the trigger has an angles field, check player's facing direction
		if (pev->movedir != vec3_origin)
		{
			UTIL_MakeVectors(pOther->GetAbsAngles());
			if (DotProduct(gpGlobals->v_forward, pev->movedir) < 0)
				return;         // not facing the right way
		}
#endif

		ActivateMultiTrigger(pOther);
	}
}

void CTriggerMultiple::ActivateMultiTrigger(CBaseEntity* pActivator)
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
		SetThink(&CTriggerMultiple::MultiWaitOver);
		pev->nextthink = gpGlobals->time + m_flWait;
	}
	else
	{
		// we can't just remove (self) here, because this is a touch function
		// called while C code is looping through area links...
		SetTouch(nullptr);
		pev->nextthink = gpGlobals->time + 0.1;
		SetThink(&CTriggerMultiple::SUB_Remove);
	}
}

void CTriggerMultiple::MultiWaitOver()
{
	SetThink(nullptr);
}
