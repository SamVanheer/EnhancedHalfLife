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

#include "CMultiSource.hpp"

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

	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->nextthink = gpGlobals->time + 0.1;
	pev->spawnflags |= SF_MULTI_INIT;	// Until it's initialized
	SetThink(&CMultiSource::Register);
}

void CMultiSource::Use(const UseInfo& info)
{
	//No known sources so do nothing. Prevents out of bounds access.
	if (m_iTotal <= 0)
	{
		return;
	}

	int i = 0;

	// Find the entity in our list
	while (i < m_iTotal)
		if (m_rgEntities[i++] == info.GetCaller())
			break;

	// if we didn't find it, report error and leave
	if (i > m_iTotal)
	{
		ALERT(at_console, "MultiSrc:Used by non member %s.\n", info.GetCaller()->GetClassname());
		return;
	}

	// CONSIDER: a Use input to the multisource always toggles.  Could check useType for ON/OFF/TOGGLE

	m_rgTriggered[i - 1] = !m_rgTriggered[i - 1];

	// 
	if (IsTriggered(info.GetActivator()))
	{
		ALERT(at_aiconsole, "Multisource %s enabled (%d inputs)\n", GetTargetname(), m_iTotal);
		UseType useType = UseType::Toggle;
		if (!IsStringNull(m_globalstate))
			useType = UseType::On;
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

	CBaseEntity* pTarget = nullptr;

	while ((pTarget = UTIL_FindEntityByTarget(pTarget, GetTargetname())) != nullptr && (m_iTotal < MS_MAX_TARGETS))
	{
		m_rgEntities[m_iTotal++] = pTarget;
	}

	pTarget = nullptr;

	while ((pTarget = UTIL_FindEntityByClassname(pTarget, "multi_manager")) != nullptr && (m_iTotal < MS_MAX_TARGETS))
	{
		if (pTarget->HasTarget(MAKE_STRING(GetTargetname())))
		{
			m_rgEntities[m_iTotal++] = pTarget;
		}
	}

	pev->spawnflags &= ~SF_MULTI_INIT;
}
