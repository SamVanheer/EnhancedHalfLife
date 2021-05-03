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
*	spawn and use functions for editor-placed triggers
*/

#include "triggers.hpp"
#include "trains.h"			// trigger_camera has train functionality

LINK_ENTITY_TO_CLASS(func_friction, CFrictionModifier);

// Global Savedata for changelevel friction modifier
TYPEDESCRIPTION	CFrictionModifier::m_SaveData[] =
{
	DEFINE_FIELD(CFrictionModifier, m_frictionFraction, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CFrictionModifier, CBaseEntity);

void CFrictionModifier::Spawn()
{
	SetSolidType(Solid::Trigger);
	SetModel(STRING(pev->model));    // set size and link into world
	SetMovetype(Movetype::None);
	SetTouch(&CFrictionModifier::ChangeFriction);
}

void CFrictionModifier::ChangeFriction(CBaseEntity* pOther)
{
	if (pOther->GetMovetype() != Movetype::BounceMissile && pOther->GetMovetype() != Movetype::Bounce)
		pOther->pev->friction = m_frictionFraction;
}

void CFrictionModifier::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "modifier"))
	{
		m_frictionFraction = atof(pkvd->szValue) / 100.0;
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(trigger_auto, CAutoTrigger);

TYPEDESCRIPTION	CAutoTrigger::m_SaveData[] =
{
	DEFINE_FIELD(CAutoTrigger, m_globalstate, FIELD_STRING),
	DEFINE_FIELD(CAutoTrigger, triggerType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CAutoTrigger, CBaseDelay);

void CAutoTrigger::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "globalstate"))
	{
		m_globalstate = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "triggerstate"))
	{
		triggerType = UTIL_TriggerStateToTriggerType(static_cast<TriggerState>(atoi(pkvd->szValue)));
		pkvd->fHandled = true;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CAutoTrigger::Spawn()
{
	Precache();
}

void CAutoTrigger::Precache()
{
	pev->nextthink = gpGlobals->time + 0.1;
}

void CAutoTrigger::Think()
{
	if (IsStringNull(m_globalstate) || gGlobalState.EntityGetState(m_globalstate) == GlobalEntState::On)
	{
		SUB_UseTargets(this, triggerType, 0);
		if (pev->spawnflags & SF_AUTO_FIREONCE)
			UTIL_Remove(this);
	}
}

LINK_ENTITY_TO_CLASS(trigger_relay, CTriggerRelay);

TYPEDESCRIPTION	CTriggerRelay::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerRelay, triggerType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CTriggerRelay, CBaseDelay);

void CTriggerRelay::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "triggerstate"))
	{
		triggerType = UTIL_TriggerStateToTriggerType(static_cast<TriggerState>(atoi(pkvd->szValue)));
		pkvd->fHandled = true;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CTriggerRelay::Spawn()
{
}

void CTriggerRelay::Use(const UseInfo& info)
{
	SUB_UseTargets(this, triggerType, 0);
	if (pev->spawnflags & SF_RELAY_FIREONCE)
		UTIL_Remove(this);
}

LINK_ENTITY_TO_CLASS(multi_manager, CMultiManager);

// Global Savedata for multi_manager
TYPEDESCRIPTION	CMultiManager::m_SaveData[] =
{
	DEFINE_FIELD(CMultiManager, m_cTargets, FIELD_INTEGER),
	DEFINE_FIELD(CMultiManager, m_index, FIELD_INTEGER),
	DEFINE_FIELD(CMultiManager, m_startTime, FIELD_TIME),
	DEFINE_ARRAY(CMultiManager, m_iTargetName, FIELD_STRING, MAX_MULTI_TARGETS),
	DEFINE_ARRAY(CMultiManager, m_flTargetDelay, FIELD_FLOAT, MAX_MULTI_TARGETS),
};

IMPLEMENT_SAVERESTORE(CMultiManager, CBaseToggle);

void CMultiManager::KeyValue(KeyValueData* pkvd)
{
	// UNDONE: Maybe this should do something like this:
	//CBaseToggle::KeyValue( pkvd );
	// if ( !pkvd->fHandled )
	// ... etc.

	if (AreStringsEqual(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else // add this field to the target list
	{
		// this assumes that additional fields are targetnames and their values are delay values.
		if (m_cTargets < MAX_MULTI_TARGETS)
		{
			char tmp[128];

			UTIL_StripToken(pkvd->szKeyName, tmp);
			m_iTargetName[m_cTargets] = ALLOC_STRING(tmp);
			m_flTargetDelay[m_cTargets] = atof(pkvd->szValue);
			m_cTargets++;
			pkvd->fHandled = true;
		}
	}
}

void CMultiManager::Spawn()
{
	SetSolidType(Solid::Not);
	SetUse(&CMultiManager::ManagerUse);
	SetThink(&CMultiManager::ManagerThink);

	// Sort targets
	// Quick and dirty bubble sort
	bool swapped = true;

	while (swapped)
	{
		swapped = false;
		for (int i = 1; i < m_cTargets; i++)
		{
			if (m_flTargetDelay[i] < m_flTargetDelay[i - 1])
			{
				// Swap out of order elements
				std::swap(m_iTargetName[i - 1], m_iTargetName[i]);
				std::swap(m_flTargetDelay[i - 1], m_flTargetDelay[i]);
				swapped = true;
			}
		}
	}
}

bool CMultiManager::HasTarget(string_t targetname)
{
	for (int i = 0; i < m_cTargets; i++)
		if (AreStringsEqual(STRING(targetname), STRING(m_iTargetName[i])))
			return true;

	return false;
}

// Designers were using this to fire targets that may or may not exist -- 
// so I changed it to use the standard target fire code, made it a little simpler.
void CMultiManager::ManagerThink()
{
	const float time = gpGlobals->time - m_startTime;
	while (m_index < m_cTargets && m_flTargetDelay[m_index] <= time)
	{
		FireTargets(STRING(m_iTargetName[m_index]), m_hActivator, this, UseType::Toggle, 0);
		m_index++;
	}

	if (m_index >= m_cTargets)// have we fired all targets?
	{
		SetThink(nullptr);
		if (IsClone())
		{
			UTIL_Remove(this);
			return;
		}
		SetUse(&CMultiManager::ManagerUse);// allow manager re-use 
	}
	else
		pev->nextthink = m_startTime + m_flTargetDelay[m_index];
}

CMultiManager* CMultiManager::Clone()
{
	CMultiManager* pMulti = GetClassPtr((CMultiManager*)nullptr);

	edict_t* pEdict = pMulti->pev->pContainingEntity;
	memcpy(pMulti->pev, pev, sizeof(*pev));
	pMulti->pev->pContainingEntity = pEdict;

	pMulti->pev->spawnflags |= SF_MULTIMAN_CLONE;
	pMulti->m_cTargets = m_cTargets;
	memcpy(pMulti->m_iTargetName, m_iTargetName, sizeof(m_iTargetName));
	memcpy(pMulti->m_flTargetDelay, m_flTargetDelay, sizeof(m_flTargetDelay));

	return pMulti;
}

// The USE function builds the time table and starts the entity thinking.
void CMultiManager::ManagerUse(const UseInfo& info)
{
	// In multiplayer games, clone the MM and execute in the clone (like a thread)
	// to allow multiple players to trigger the same multimanager
	if (ShouldClone())
	{
		CMultiManager* pClone = Clone();
		pClone->ManagerUse(info);
		return;
	}

	m_hActivator = info.GetActivator();
	m_index = 0;
	m_startTime = gpGlobals->time;

	SetUse(nullptr);// disable use until all targets have fired

	SetThink(&CMultiManager::ManagerThink);
	pev->nextthink = gpGlobals->time;
}

#if _DEBUG
void CMultiManager::ManagerReport()
{
	for (int cIndex = 0; cIndex < m_cTargets; cIndex++)
	{
		ALERT(at_console, "%s %f\n", STRING(m_iTargetName[cIndex]), m_flTargetDelay[cIndex]);
	}
}
#endif

LINK_ENTITY_TO_CLASS(env_render, CRenderFxManager);

void CRenderFxManager::Spawn()
{
	SetSolidType(Solid::Not);
}

void CRenderFxManager::Use(const UseInfo& info)
{
	if (!IsStringNull(pev->target))
	{
		CBaseEntity* pTarget = nullptr;

		while ((pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target))) != nullptr)
		{
			if (!IsBitSet(pev->spawnflags, SF_RENDER_MASKFX))
				pTarget->SetRenderFX(GetRenderFX());
			if (!IsBitSet(pev->spawnflags, SF_RENDER_MASKAMT))
				pTarget->pev->renderamt = pev->renderamt;
			if (!IsBitSet(pev->spawnflags, SF_RENDER_MASKMODE))
				pTarget->SetRenderMode(GetRenderMode());
			if (!IsBitSet(pev->spawnflags, SF_RENDER_MASKCOLOR))
				pTarget->SetRenderColor(GetRenderColor());
		}
	}
}

LINK_ENTITY_TO_CLASS(trigger, CBaseTrigger);

void CBaseTrigger::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "damagetype"))
	{
		m_bitsDamageInflict = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CBaseTrigger::InitTrigger()
{
	// trigger angles are used for one-way touches.  An angle of 0 is assumed
	// to mean no restrictions, so use a yaw of 360 instead.
	if (pev->angles != vec3_origin)
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

	if (!UTIL_IsMasterTriggered(m_sMaster, pActivator))
		return;

	if (!IsStringNull(pev->noise))
		EmitSound(SoundChannel::Voice, STRING(pev->noise));

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

LINK_ENTITY_TO_CLASS(trigger_hurt, CTriggerHurt);

void CTriggerHurt::Spawn()
{
	InitTrigger();
	SetTouch(&CTriggerHurt::HurtTouch);

	if (!IsStringNull(pev->targetname))
	{
		SetUse(&CTriggerHurt::ToggleUse);
	}
	else
	{
		SetUse(nullptr);
	}

	if (m_bitsDamageInflict & DMG_RADIATION)
	{
		SetThink(&CTriggerHurt::RadiationThink);
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.0, 0.5);
	}

	if (IsBitSet(pev->spawnflags, SF_TRIGGER_HURT_START_OFF))// if flagged to Start Turned Off, make trigger nonsolid.
		SetSolidType(Solid::Not);

	SetAbsOrigin(GetAbsOrigin());		// Link into the list
}

void CTriggerHurt::HurtTouch(CBaseEntity* pOther)
{
	if (!pOther->pev->takedamage)
		return;

	if ((pev->spawnflags & SF_TRIGGER_HURT_CLIENTONLYTOUCH) && !pOther->IsPlayer())
	{
		// this trigger is only allowed to touch clients, and this ain't a client.
		return;
	}

	if ((pev->spawnflags & SF_TRIGGER_HURT_NO_CLIENTS) && pOther->IsPlayer())
		return;

	// HACKHACK -- In multiplayer, players touch this based on packet receipt.
	// So the players who send packets later aren't always hurt.  Keep track of
	// how much time has passed and whether or not you've touched that player
	if (g_pGameRules->IsMultiplayer())
	{
		if (pev->dmgtime > gpGlobals->time)
		{
			if (gpGlobals->time != pev->pain_finished)
			{// too early to hurt again, and not same frame with a different entity
				if (pOther->IsPlayer())
				{
					const int playerMask = 1 << (pOther->entindex() - 1);

					// If I've already touched this player (this time), then bail out
					if (pev->impulse & playerMask)
						return;

					// Mark this player as touched
					// BUGBUG - There can be only 32 players!
					pev->impulse |= playerMask;
				}
				else
				{
					return;
				}
			}
		}
		else
		{
			// New clock, "un-touch" all players
			pev->impulse = 0;
			if (pOther->IsPlayer())
			{
				const int playerMask = 1 << (pOther->entindex() - 1);

				// Mark this player as touched
				// BUGBUG - There can be only 32 players!
				pev->impulse |= playerMask;
			}
		}
	}
	else	// Original code -- single player
	{
		if (pev->dmgtime > gpGlobals->time && gpGlobals->time != pev->pain_finished)
		{// too early to hurt again, and not same frame with a different entity
			return;
		}
	}

	// If this is time_based damage (poison, radiation), override the pev->dmg with a 
	// default for the given damage type.  Monsters only take time-based damage
	// while touching the trigger.  Player continues taking damage for a while after
	// leaving the trigger

	const float fldmg = pev->dmg * 0.5; // 0.5 seconds worth of damage, pev->dmg is damage/second

	// JAY: Cut this because it wasn't fully realized.  Damage is simpler now.
#if 0
	switch (m_bitsDamageInflict)
	{
	default: break;
	case DMG_POISON:		fldmg = POISON_DAMAGE / 4; break;
	case DMG_NERVEGAS:		fldmg = NERVEGAS_DAMAGE / 4; break;
	case DMG_RADIATION:		fldmg = RADIATION_DAMAGE / 4; break;
	case DMG_PARALYZE:		fldmg = PARALYZE_DAMAGE / 4; break; // UNDONE: cut this? should slow movement to 50%
	case DMG_ACID:			fldmg = ACID_DAMAGE / 4; break;
	case DMG_SLOWBURN:		fldmg = SLOWBURN_DAMAGE / 4; break;
	case DMG_SLOWFREEZE:	fldmg = SLOWFREEZE_DAMAGE / 4; break;
	}
#endif

	if (fldmg < 0)
		pOther->GiveHealth(-fldmg, m_bitsDamageInflict);
	else
		pOther->TakeDamage({this, this, fldmg, m_bitsDamageInflict});

	// Store pain time so we can get all of the other entities on this frame
	pev->pain_finished = gpGlobals->time;

	// Apply damage every half second
	pev->dmgtime = gpGlobals->time + 0.5;// half second delay until this trigger can hurt toucher again

	if (!IsStringNull(pev->target))
	{
		// trigger has a target it wants to fire. 
		if (pev->spawnflags & SF_TRIGGER_HURT_CLIENTONLYFIRE)
		{
			// if the toucher isn't a client, don't fire the target!
			if (!pOther->IsPlayer())
			{
				return;
			}
		}

		SUB_UseTargets(pOther, UseType::Toggle, 0);
		if (pev->spawnflags & SF_TRIGGER_HURT_TARGETONCE)
			pev->target = iStringNull;
	}
}

void CTriggerHurt::RadiationThink()
{
	// check to see if a player is in pvs
	// if not, continue	

	// set origin to center of trigger so that this check works
	const Vector origin = GetAbsOrigin();
	const Vector view_ofs = pev->view_ofs;

	SetAbsOrigin((pev->absmin + pev->absmax) * 0.5);
	pev->view_ofs = pev->view_ofs * 0.0;

	auto pPlayer = static_cast<CBasePlayer*>(UTIL_FindClientInPVS(this));

	SetAbsOrigin(origin);
	pev->view_ofs = view_ofs;

	// reset origin

	if (!IsNullEnt(pPlayer))
	{
		// get range to player;

		const Vector vecSpot1 = (pev->absmin + pev->absmax) * 0.5;
		const Vector vecSpot2 = (pPlayer->pev->absmin + pPlayer->pev->absmax) * 0.5;

		const Vector vecRange = vecSpot1 - vecSpot2;
		const float flRange = vecRange.Length();

		// if player's current geiger counter range is larger
		// than range to this trigger hurt, reset player's
		// geiger counter range 

		if (pPlayer->m_flgeigerRange >= flRange)
			pPlayer->m_flgeigerRange = flRange;
	}

	pev->nextthink = gpGlobals->time + 0.25;
}

LINK_ENTITY_TO_CLASS(trigger_monsterjump, CTriggerMonsterJump);

void CTriggerMonsterJump::Spawn()
{
	SetMovedir(this);

	InitTrigger();

	pev->nextthink = 0;
	pev->speed = 200;
	m_flHeight = 150;

	if (!IsStringNull(pev->targetname))
	{// if targetted, spawn turned off
		SetSolidType(Solid::Not);
		SetAbsOrigin(GetAbsOrigin()); // Unlink from trigger list
		SetUse(&CTriggerMonsterJump::ToggleUse);
	}
}

void CTriggerMonsterJump::Think()
{
	SetSolidType(Solid::Not);// kill the trigger for now !!!UNDONE
	SetAbsOrigin(GetAbsOrigin()); // Unlink from trigger list
	SetThink(nullptr);
}

void CTriggerMonsterJump::Touch(CBaseEntity* pOther)
{
	if (!IsBitSet(pOther->pev->flags, FL_MONSTER))
	{// touched by a non-monster.
		return;
	}

	pOther->SetAbsOrigin(pOther->GetAbsOrigin() + vec3_up);

	if (IsBitSet(pOther->pev->flags, FL_ONGROUND))
	{// clear the onground so physics don't bitch
		pOther->pev->flags &= ~FL_ONGROUND;
	}

	// toss the monster!
	pOther->pev->velocity = pev->movedir * pev->speed;
	pOther->pev->velocity.z += m_flHeight;
	pev->nextthink = gpGlobals->time;
}

LINK_ENTITY_TO_CLASS(trigger_cdaudio, CTriggerCDAudio);

// !!!HACK - overloaded HEALTH to avoid adding new field
void CTriggerCDAudio::Touch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{// only clients may trigger these events
		return;
	}

	PlayTrack();
}

void CTriggerCDAudio::Spawn()
{
	InitTrigger();
}

void CTriggerCDAudio::Use(const UseInfo& info)
{
	PlayTrack();
}

void PlayCDTrack(int iTrack)
{
	// manually find the single player. 
	//TODO: make this work for all players
	CBaseEntity* pClient = UTIL_EntityByIndex(1);

	// Can't play if the client is not connected!
	if (!pClient)
		return;

	if (iTrack < -1 || iTrack > 30)
	{
		ALERT(at_console, "TriggerCDAudio - Track %d out of range\n");
		return;
	}

	if (iTrack == -1)
	{
		CLIENT_COMMAND(pClient->edict(), "cd stop\n");
	}
	else
	{
		char string[64];

		snprintf(string, sizeof(string), "cd play %3d\n", iTrack);
		CLIENT_COMMAND(pClient->edict(), string);
	}
}

// only plays for ONE client, so only use in single play!
void CTriggerCDAudio::PlayTrack()
{
	PlayCDTrack((int)pev->health);

	SetTouch(nullptr);
	UTIL_Remove(this);
}

LINK_ENTITY_TO_CLASS(target_cdaudio, CTargetCDAudio);

void CTargetCDAudio::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "radius"))
	{
		pev->scale = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CTargetCDAudio::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);

	if (pev->scale > 0)
		pev->nextthink = gpGlobals->time + 1.0;
}

void CTargetCDAudio::Use(const UseInfo& info)
{
	Play();
}

// only plays for ONE client, so only use in single play!
void CTargetCDAudio::Think()
{
	// manually find the single player. 
	CBaseEntity* pClient = UTIL_EntityByIndex(1);

	// Can't play if the client is not connected!
	if (!pClient)
		return;

	pev->nextthink = gpGlobals->time + 0.5;

	if ((pClient->GetAbsOrigin() - GetAbsOrigin()).Length() <= pev->scale)
		Play();
}

void CTargetCDAudio::Play()
{
	PlayCDTrack((int)pev->health);
	UTIL_Remove(this);
}

LINK_ENTITY_TO_CLASS(trigger_multiple, CTriggerMultiple);

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
			UTIL_MakeVectors(pOther->pev->angles);
			if (DotProduct(gpGlobals->v_forward, pev->movedir) < 0)
				return;         // not facing the right way
		}
#endif

		ActivateMultiTrigger(pOther);
	}
}

LINK_ENTITY_TO_CLASS(trigger_once, CTriggerOnce);

void CTriggerOnce::Spawn()
{
	m_flWait = -1;

	CTriggerMultiple::Spawn();
}

LINK_ENTITY_TO_CLASS(trigger_counter, CTriggerCounter);

TYPEDESCRIPTION CTriggerCounter::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerCounter, m_cTriggersLeft, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CTriggerCounter, CBaseTrigger);

void CTriggerCounter::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "count"))
	{
		m_cTriggersLeft = (int)atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseTrigger::KeyValue(pkvd);
}

void CTriggerCounter::Spawn()
{
	// By making the flWait be -1, this counter-trigger will disappear after it's activated
	// (but of course it needs cTriggersLeft "uses" before that happens).
	m_flWait = -1;

	if (m_cTriggersLeft == 0)
		m_cTriggersLeft = 2;
	SetUse(&CTriggerCounter::CounterUse);
}

void CTriggerCounter::CounterUse(const UseInfo& info)
{
	m_cTriggersLeft--;
	m_hActivator = info.GetActivator();

	if (m_cTriggersLeft < 0)
		return;

	const bool fTellActivator =
		(m_hActivator != nullptr) &&
		m_hActivator->IsPlayer() &&
		!IsBitSet(pev->spawnflags, SF_COUNTER_NOMESSAGE);
	if (m_cTriggersLeft != 0)
	{
		if (fTellActivator)
		{
			// UNDONE: I don't think we want these Quakesque messages
			switch (m_cTriggersLeft)
			{
			case 1:		ALERT(at_console, "Only 1 more to go...");		break;
			case 2:		ALERT(at_console, "Only 2 more to go...");		break;
			case 3:		ALERT(at_console, "Only 3 more to go...");		break;
			default:	ALERT(at_console, "There are more to go...");	break;
			}
		}
		return;
	}

	// !!!UNDONE: I don't think we want these Quakesque messages
	if (fTellActivator)
		ALERT(at_console, "Sequence completed!");

	ActivateMultiTrigger(m_hActivator);
}

LINK_ENTITY_TO_CLASS(func_ladder, CLadder);

void CLadder::KeyValue(KeyValueData* pkvd)
{
	CBaseTrigger::KeyValue(pkvd);
}

void CLadder::Precache()
{
	// Do all of this in here because we need to 'convert' old saved games
	SetSolidType(Solid::Not);
	pev->skin = static_cast<int>(Contents::Ladder);
	if (CVAR_GET_FLOAT("showtriggers") == 0)
	{
		SetRenderMode(RenderMode::TransTexture);
		pev->renderamt = 0;
	}
	pev->effects &= ~EF_NODRAW;
}

void CLadder::Spawn()
{
	Precache();

	SetModel(STRING(pev->model));    // set size and link into world
	SetMovetype(Movetype::Push);
}

LINK_ENTITY_TO_CLASS(trigger_push, CTriggerPush);

void CTriggerPush::KeyValue(KeyValueData* pkvd)
{
	CBaseTrigger::KeyValue(pkvd);
}

void CTriggerPush::Spawn()
{
	if (pev->angles == vec3_origin)
		pev->angles.y = 360;
	InitTrigger();

	if (pev->speed == 0)
		pev->speed = 100;

	if (IsBitSet(pev->spawnflags, SF_TRIGGER_PUSH_START_OFF))// if flagged to Start Turned Off, make trigger nonsolid.
		SetSolidType(Solid::Not);

	SetUse(&CTriggerPush::ToggleUse);

	SetAbsOrigin(GetAbsOrigin());		// Link into the list
}

void CTriggerPush::Touch(CBaseEntity* pOther)
{
	// UNDONE: Is there a better way than health to detect things that have physics? (clients/monsters)
	switch (pOther->GetMovetype())
	{
	case Movetype::None:
	case Movetype::Push:
	case Movetype::Noclip:
	case Movetype::Follow:
		return;
	}

	if (pOther->GetSolidType() != Solid::Not && pOther->GetSolidType() != Solid::BSP)
	{
		// Instant trigger, just transfer velocity and remove
		if (IsBitSet(pev->spawnflags, SF_TRIGGER_PUSH_ONCE))
		{
			pOther->pev->velocity = pOther->pev->velocity + (pev->speed * pev->movedir);
			if (pOther->pev->velocity.z > 0)
				pOther->pev->flags &= ~FL_ONGROUND;
			UTIL_Remove(this);
		}
		else
		{	// Push field, transfer to base velocity
			Vector vecPush = (pev->speed * pev->movedir);
			if (pOther->pev->flags & FL_BASEVELOCITY)
				vecPush = vecPush + pOther->pev->basevelocity;

			pOther->pev->basevelocity = vecPush;

			pOther->pev->flags |= FL_BASEVELOCITY;
			//			ALERT( at_console, "Vel %f, base %f\n", pOther->pev->velocity.z, pOther->pev->basevelocity.z );
		}
	}
}

LINK_ENTITY_TO_CLASS(trigger_teleport, CTriggerTeleport);
LINK_ENTITY_TO_CLASS(info_teleport_destination, CPointEntity);

void CTriggerTeleport::Spawn()
{
	InitTrigger();

	SetTouch(&CTriggerTeleport::TeleportTouch);
}

void CTriggerTeleport::TeleportTouch(CBaseEntity* pOther)
{
	// Only teleport monsters or clients
	if (!IsBitSet(pOther->pev->flags, FL_CLIENT | FL_MONSTER))
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
		return;

	if (!(pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS))
	{// no monsters allowed!
		if (IsBitSet(pOther->pev->flags, FL_MONSTER))
		{
			return;
		}
	}

	if ((pev->spawnflags & SF_TRIGGER_NOCLIENTS))
	{// no clients allowed
		if (pOther->IsPlayer())
		{
			return;
		}
	}

	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
	if (IsNullEnt(pTarget))
		return;

	Vector tmp = pTarget->GetAbsOrigin();

	if (pOther->IsPlayer())
	{
		tmp.z -= pOther->pev->mins.z;// make origin adjustments in case the teleportee is a player. (origin in center, not at feet)
	}

	tmp.z++;

	pOther->pev->flags &= ~FL_ONGROUND;

	pOther->SetAbsOrigin(tmp);

	pOther->pev->angles = pTarget->pev->angles;

	if (pOther->IsPlayer())
	{
		pOther->pev->v_angle = pTarget->pev->angles;
	}

	pOther->pev->fixangle = FixAngleMode::Absolute;
	pOther->pev->velocity = pOther->pev->basevelocity = vec3_origin;
}

LINK_ENTITY_TO_CLASS(trigger_gravity, CTriggerGravity);

void CTriggerGravity::Spawn()
{
	InitTrigger();
	SetTouch(&CTriggerGravity::GravityTouch);
}

void CTriggerGravity::GravityTouch(CBaseEntity* pOther)
{
	// Only save on clients
	if (!pOther->IsPlayer())
		return;

	pOther->pev->gravity = pev->gravity;
}

LINK_ENTITY_TO_CLASS(trigger_changetarget, CTriggerChangeTarget);

TYPEDESCRIPTION	CTriggerChangeTarget::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerChangeTarget, m_iszNewTarget, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CTriggerChangeTarget, CBaseDelay);

void CTriggerChangeTarget::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "m_iszNewTarget"))
	{
		m_iszNewTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CTriggerChangeTarget::Spawn()
{
}

void CTriggerChangeTarget::Use(const UseInfo& info)
{
	CBaseEntity* pTarget = UTIL_FindEntityByString(nullptr, "targetname", STRING(pev->target));

	if (pTarget)
	{
		pTarget->pev->target = m_iszNewTarget;
		CBaseMonster* pMonster = pTarget->MyMonsterPointer();
		if (pMonster)
		{
			pMonster->m_hGoalEnt = nullptr;
		}
	}
}

LINK_ENTITY_TO_CLASS(trigger_camera, CTriggerCamera);

// Global Savedata for changelevel friction modifier
TYPEDESCRIPTION	CTriggerCamera::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerCamera, m_hPlayer, FIELD_EHANDLE),
	DEFINE_FIELD(CTriggerCamera, m_hTarget, FIELD_EHANDLE),
	DEFINE_FIELD(CTriggerCamera, m_hEntPath, FIELD_EHANDLE),
	DEFINE_FIELD(CTriggerCamera, m_sPath, FIELD_STRING),
	DEFINE_FIELD(CTriggerCamera, m_flWait, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_flReturnTime, FIELD_TIME),
	DEFINE_FIELD(CTriggerCamera, m_flStopTime, FIELD_TIME),
	DEFINE_FIELD(CTriggerCamera, m_moveDistance, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_targetSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_initialSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_acceleration, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_deceleration, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_state, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CTriggerCamera, CBaseDelay);

void CTriggerCamera::Spawn()
{
	SetMovetype(Movetype::Noclip);
	SetSolidType(Solid::Not);							// Remove model & collisions
	pev->renderamt = 0;								// The engine won't draw this model if this is set to 0 and blending is on
	SetRenderMode(RenderMode::TransTexture);

	m_initialSpeed = pev->speed;
	if (m_acceleration == 0)
		m_acceleration = 500;
	if (m_deceleration == 0)
		m_deceleration = 500;
}

void CTriggerCamera::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "moveto"))
	{
		m_sPath = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "acceleration"))
	{
		m_acceleration = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "deceleration"))
	{
		m_deceleration = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CTriggerCamera::Use(const UseInfo& info)
{
	if (!ShouldToggle(info.GetUseType(), m_state))
		return;

	// Toggle state
	m_state = !m_state;
	if (!m_state)
	{
		m_flReturnTime = gpGlobals->time;
		return;
	}

	auto pActivator = info.GetActivator();

	if (!pActivator || !pActivator->IsPlayer())
	{
		pActivator = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));
	}

	auto player = static_cast<CBasePlayer*>(pActivator);

	m_hPlayer = player;

	m_flReturnTime = gpGlobals->time + m_flWait;
	pev->speed = m_initialSpeed;
	m_targetSpeed = m_initialSpeed;

	if (IsBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TARGET))
	{
		m_hTarget = m_hPlayer;
	}
	else
	{
		m_hTarget = GetNextTarget();
	}

	// Nothing to look at!
	if (m_hTarget == nullptr)
	{
		return;
	}

	if (IsBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL))
	{
		player->EnableControl(false);
	}

	if (!IsStringNull(m_sPath))
	{
		m_hEntPath = UTIL_FindEntityByTargetname(nullptr, STRING(m_sPath));
	}
	else
	{
		m_hEntPath = nullptr;
	}

	m_flStopTime = gpGlobals->time;
	if (auto path = m_hEntPath.Get(); path)
	{
		if (path->pev->speed != 0)
			m_targetSpeed = path->pev->speed;

		m_flStopTime += path->GetDelay();
	}

	// copy over player information
	if (IsBitSet(pev->spawnflags, SF_CAMERA_PLAYER_POSITION))
	{
		SetAbsOrigin(pActivator->GetAbsOrigin() + pActivator->pev->view_ofs);
		pev->angles.x = -pActivator->pev->angles.x;
		pev->angles.y = pActivator->pev->angles.y;
		pev->angles.z = 0;
		pev->velocity = pActivator->pev->velocity;
	}
	else
	{
		pev->velocity = vec3_origin;
	}

	SET_VIEW(pActivator->edict(), edict());

	player->m_hViewEntity = this;

	SetModel(STRING(pActivator->pev->model));

	// follow the player down
	SetThink(&CTriggerCamera::FollowTarget);
	pev->nextthink = gpGlobals->time;

	m_moveDistance = 0;
	Move();
}

void CTriggerCamera::FollowTarget()
{
	auto player = m_hPlayer.Get();

	if (!player)
		return;

	if (m_hTarget == nullptr || m_flReturnTime < gpGlobals->time)
	{
		if (player->IsAlive())
		{
			SET_VIEW(player->edict(), player->edict());
			player->EnableControl(true);
		}

		player->m_hViewEntity = nullptr;
		player->m_bResetViewEntity = false;

		SUB_UseTargets(this, UseType::Toggle, 0);
		pev->avelocity = vec3_origin;
		m_state = false;
		return;
	}

	Vector vecGoal = VectorAngles(m_hTarget->GetAbsOrigin() - GetAbsOrigin());
	vecGoal.x = -vecGoal.x;

	if (pev->angles.y > 360)
		pev->angles.y -= 360;

	if (pev->angles.y < 0)
		pev->angles.y += 360;

	float dx = vecGoal.x - pev->angles.x;
	float dy = vecGoal.y - pev->angles.y;

	if (dx < -180)
		dx += 360;
	if (dx > 180)
		dx = dx - 360;

	if (dy < -180)
		dy += 360;
	if (dy > 180)
		dy = dy - 360;

	pev->avelocity.x = dx * 40 * 0.01;
	pev->avelocity.y = dy * 40 * 0.01;

	if (!(IsBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL)))
	{
		pev->velocity = pev->velocity * 0.8;
		if (pev->velocity.Length() < 10.0)
			pev->velocity = vec3_origin;
	}

	pev->nextthink = gpGlobals->time;

	Move();
}

void CTriggerCamera::Move()
{
	auto path = m_hEntPath.Get();
	// Not moving on a path, return
	if (!path)
		return;

	// Subtract movement from the previous frame
	m_moveDistance -= pev->speed * gpGlobals->frametime;

	// Have we moved enough to reach the target?
	if (m_moveDistance <= 0)
	{
		// Fire the passtarget if there is one
		if (!IsStringNull(path->pev->message))
		{
			FireTargets(STRING(path->pev->message), this, this, UseType::Toggle, 0);
			if (IsBitSet(path->pev->spawnflags, SF_CORNER_FIREONCE))
				path->pev->message = iStringNull;
		}
		// Time to go to the next target
		path = m_hEntPath = path->GetNextTarget();

		// Set up next corner
		if (!path)
		{
			pev->velocity = vec3_origin;
		}
		else
		{
			if (path->pev->speed != 0)
				m_targetSpeed = path->pev->speed;

			const Vector delta = path->GetAbsOrigin() - GetAbsOrigin();
			m_moveDistance = delta.Length();
			pev->movedir = delta.Normalize();
			m_flStopTime = gpGlobals->time + path->GetDelay();
		}
	}

	if (m_flStopTime > gpGlobals->time)
		pev->speed = UTIL_Approach(0, pev->speed, m_deceleration * gpGlobals->frametime);
	else
		pev->speed = UTIL_Approach(m_targetSpeed, pev->speed, m_acceleration * gpGlobals->frametime);

	const float fraction = 2 * gpGlobals->frametime;
	pev->velocity = ((pev->movedir * pev->speed) * fraction) + (pev->velocity * (1 - fraction));
}
