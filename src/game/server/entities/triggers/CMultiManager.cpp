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

#include "CMultiManager.hpp"

void CMultiManager::KeyValue(KeyValueData* pkvd)
{
	// UNDONE: Maybe this should do something like this:
	//BaseClass::KeyValue( pkvd );
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
	CMultiManager* pMulti = static_cast<CMultiManager*>(g_EntityList.Create("multi_manager"));

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
