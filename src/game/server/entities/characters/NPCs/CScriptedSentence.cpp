/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#include "CScriptedSentence.hpp"

LINK_ENTITY_TO_CLASS(scripted_sentence, CScriptedSentence);

void CScriptedSentence::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "sentence"))
	{
		m_iszSentence = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "entity"))
	{
		m_iszEntity = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "duration"))
	{
		m_flDuration = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "refire"))
	{
		m_flRepeat = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "attenuation"))
	{
		pev->impulse = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "volume"))
	{
		m_flVolume = atof(pkvd->szValue) * 0.1;
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "listener"))
	{
		m_iszListener = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		BaseClass::KeyValue(pkvd);
}

void CScriptedSentence::Use(const UseInfo& info)
{
	if (!m_active)
		return;
	//	ALERT( at_console, "Firing sentence: %s\n", STRING(m_iszSentence) );
	SetThink(&CScriptedSentence::FindThink);
	pev->nextthink = gpGlobals->time;
}

void CScriptedSentence::Spawn()
{
	SetSolidType(Solid::Not);

	m_active = true;
	// if no targetname, start now
	if (!HasTargetname())
	{
		SetThink(&CScriptedSentence::FindThink);
		pev->nextthink = gpGlobals->time + 1.0;
	}

	switch (static_cast<SentenceAttenuation>(pev->impulse))
	{
	case SentenceAttenuation::MediumRadius: // Medium radius
		m_flAttenuation = ATTN_STATIC;
		break;

	case SentenceAttenuation::LargeRadius:	// Large radius
		m_flAttenuation = ATTN_NORM;
		break;

	case SentenceAttenuation::PlayEverywhere:	//EVERYWHERE
		m_flAttenuation = ATTN_NONE;
		break;

	default:
	case SentenceAttenuation::SmallRadius: // Small radius
		m_flAttenuation = ATTN_IDLE;
		break;
	}
	pev->impulse = 0;

	// No volume, use normal
	if (m_flVolume <= 0)
		m_flVolume = 1.0;
}

void CScriptedSentence::FindThink()
{
	if (CBaseMonster* pMonster = FindEntity(); pMonster)
	{
		StartSentence(pMonster);
		if (pev->spawnflags & SF_SENTENCE_ONCE)
			UTIL_Remove(this);
		SetThink(&CScriptedSentence::DelayThink);
		pev->nextthink = gpGlobals->time + m_flDuration + m_flRepeat;
		m_active = false;
		//		ALERT( at_console, "%s: found monster %s\n", STRING(m_iszSentence), STRING(m_iszEntity) );
	}
	else
	{
		//		ALERT( at_console, "%s: can't find monster %s\n", STRING(m_iszSentence), STRING(m_iszEntity) );
		pev->nextthink = gpGlobals->time + m_flRepeat + 0.5;
	}
}

void CScriptedSentence::DelayThink()
{
	m_active = true;
	if (!HasTargetname())
		pev->nextthink = gpGlobals->time + 0.1;
	SetThink(&CScriptedSentence::FindThink);
}

bool CScriptedSentence::AcceptableSpeaker(CBaseMonster* pMonster)
{
	if (pMonster)
	{
		if (pev->spawnflags & SF_SENTENCE_FOLLOWERS)
		{
			if (pMonster->m_hTargetEnt == nullptr || !pMonster->m_hTargetEnt->IsPlayer())
				return false;
		}
		const bool override = (pev->spawnflags & SF_SENTENCE_INTERRUPT) != 0;

		if (pMonster->CanPlaySentence(override))
			return true;
	}
	return false;
}

CBaseMonster* CScriptedSentence::FindEntity()
{
	CBaseEntity* pTarget = nullptr;

	while ((pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(m_iszEntity))) != nullptr)
	{
		if (CBaseMonster* pMonster = pTarget->MyMonsterPointer(); pMonster != nullptr)
		{
			if (AcceptableSpeaker(pMonster))
				return pMonster;
			//			ALERT( at_console, "%s (%s), not acceptable\n", pMonster->GetClassname(), pMonster->GetTargetname() );
		}
	}

	pTarget = nullptr;
	while ((pTarget = UTIL_FindEntityInSphere(pTarget, GetAbsOrigin(), m_flRadius)) != nullptr)
	{
		if (pTarget->ClassnameIs(STRING(m_iszEntity)))
		{
			if (IsBitSet(pTarget->pev->flags, FL_MONSTER))
			{
				if (CBaseMonster* pMonster = pTarget->MyMonsterPointer(); AcceptableSpeaker(pMonster))
					return pMonster;
			}
		}
	}

	return nullptr;
}

bool CScriptedSentence::StartSentence(CBaseMonster* pTarget)
{
	if (!pTarget)
	{
		ALERT(at_aiconsole, "Not Playing sentence %s\n", STRING(m_iszSentence));
		return false;
	}

	CBaseEntity* pListener = nullptr;
	if (!IsStringNull(m_iszListener))
	{
		float radius = m_flRadius;

		if (AreStringsEqual(STRING(m_iszListener), "player"))
			radius = WORLD_BOUNDARY;	// Always find the player

		pListener = UTIL_FindEntityGeneric(STRING(m_iszListener), pTarget->GetAbsOrigin(), radius);
	}

	const bool bConcurrent = !(pev->spawnflags & SF_SENTENCE_CONCURRENT);

	pTarget->PlayScriptedSentence(STRING(m_iszSentence), m_flDuration, m_flVolume, m_flAttenuation, bConcurrent, pListener);
	ALERT(at_aiconsole, "Playing sentence %s (%.1f)\n", STRING(m_iszSentence), m_flDuration);
	SUB_UseTargets(nullptr, UseType::Toggle, 0);
	return true;
}
