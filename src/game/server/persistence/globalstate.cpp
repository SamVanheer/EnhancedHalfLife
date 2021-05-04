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

globalentity_t* CGlobalState::Find(string_t globalname)
{
	if (IsStringNull(globalname))
		return nullptr;

	const char* pEntityName = STRING(globalname);

	for (globalentity_t* pTest = m_pList; pTest; pTest = pTest->pNext)
	{
		if (AreStringsEqual(pEntityName, pTest->name))
			return pTest;
	}

	return nullptr;
}

//#ifdef _DEBUG
void CGlobalState::DumpGlobals()
{
	static constexpr const char* estates[] = {"Off", "On", "Dead"};

	ALERT(at_console, "-- Globals --\n");

	for (globalentity_t* pTest = m_pList; pTest; pTest = pTest->pNext)
	{
		ALERT(at_console, "%s: %s (%s)\n", pTest->name, pTest->levelName, estates[static_cast<std::size_t>(pTest->state)]);
	}
}
//#endif

void CGlobalState::EntityAdd(string_t globalname, string_t mapName, GlobalEntState state)
{
	ASSERT(!Find(globalname));

	globalentity_t* pNewEntity = new globalentity_t{};
	ASSERT(pNewEntity != nullptr);
	pNewEntity->pNext = m_pList;
	m_pList = pNewEntity;
	safe_strcpy(pNewEntity->name, STRING(globalname));
	safe_strcpy(pNewEntity->levelName, STRING(mapName));
	pNewEntity->state = state;
	m_listCount++;
}

void CGlobalState::EntitySetState(string_t globalname, GlobalEntState state)
{
	if (globalentity_t* pEnt = Find(globalname); pEnt)
		pEnt->state = state;
}

const globalentity_t* CGlobalState::EntityFromTable(string_t globalname)
{
	globalentity_t* pEnt = Find(globalname);
	return pEnt;
}

GlobalEntState CGlobalState::EntityGetState(string_t globalname)
{
	if (globalentity_t* pEnt = Find(globalname); pEnt)
		return pEnt->state;

	return GlobalEntState::Off;
}

// Global Savedata for Delay
TYPEDESCRIPTION	CGlobalState::m_SaveData[] =
{
	DEFINE_FIELD(CGlobalState, m_listCount, FIELD_INTEGER),
};

// Global Savedata for Delay
TYPEDESCRIPTION	gGlobalEntitySaveData[] =
{
	DEFINE_ARRAY(globalentity_t, name, FIELD_CHARACTER, 64),
	DEFINE_ARRAY(globalentity_t, levelName, FIELD_CHARACTER, 32),
	DEFINE_FIELD(globalentity_t, state, FIELD_INTEGER),
};

bool CGlobalState::Save(CSave& save)
{
	if (!save.WriteFields("GLOBAL", this, m_SaveData, ArraySize(m_SaveData)))
		return false;

	globalentity_t* pEntity = m_pList;
	for (int i = 0; i < m_listCount && pEntity; ++i)
	{
		if (!save.WriteFields("GENT", pEntity, gGlobalEntitySaveData, ArraySize(gGlobalEntitySaveData)))
			return false;

		pEntity = pEntity->pNext;
	}

	return true;
}

bool CGlobalState::Restore(CRestore& restore)
{
	ClearStates();
	if (!restore.ReadFields("GLOBAL", this, m_SaveData, ArraySize(m_SaveData)))
		return false;

	const int listCount = m_listCount; // Get new list count
	m_listCount = 0; // Clear loaded data

	globalentity_t tmpEntity;

	for (int i = 0; i < listCount; i++)
	{
		if (!restore.ReadFields("GENT", &tmpEntity, gGlobalEntitySaveData, ArraySize(gGlobalEntitySaveData)))
			return false;
		EntityAdd(MAKE_STRING(tmpEntity.name), MAKE_STRING(tmpEntity.levelName), tmpEntity.state);
	}
	return true;
}

void CGlobalState::EntityUpdate(string_t globalname, string_t mapname)
{
	if (globalentity_t* pEnt = Find(globalname); pEnt)
		safe_strcpy(pEnt->levelName, STRING(mapname));
}

void CGlobalState::ClearStates()
{
	for (globalentity_t* pNext = nullptr, *pFree = m_pList; pFree; pFree = pNext)
	{
		pNext = pFree->pNext;
		delete pFree;
	}
	Reset();
}

void SaveGlobalState(SAVERESTOREDATA* pSaveData)
{
	CSave saveHelper(pSaveData);
	gGlobalState.Save(saveHelper);
}

void RestoreGlobalState(SAVERESTOREDATA* pSaveData)
{
	CRestore restoreHelper(pSaveData);
	gGlobalState.Restore(restoreHelper);
}

void ResetGlobalState()
{
	gGlobalState.ClearStates();
	gInitHUD = true;	// Init the HUD on a new game / load game
}

CBaseEntity* FindGlobalEntity(string_t classname, string_t globalname)
{
	CBaseEntity* pReturn = UTIL_FindEntityByString(nullptr, "globalname", STRING(globalname));

	if (pReturn)
	{
		if (!pReturn->ClassnameIs(STRING(classname)))
		{
			ALERT(at_console, "Global entity found %s, wrong class %s\n", STRING(globalname), pReturn->GetClassname());
			pReturn = nullptr;
		}
	}

	return pReturn;
}
