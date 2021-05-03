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

#include "changelevel.hpp"
#include "dll_functions.hpp"
#include "shake.h"

LINK_ENTITY_TO_CLASS(trigger_transition, CTriggerVolume);

void CTriggerVolume::Spawn()
{
	pev->solid = Solid::Not;
	pev->movetype = Movetype::None;
	SetModel(STRING(pev->model));    // set size and link into world
	pev->model = iStringNull;
	pev->modelindex = 0;
}

LINK_ENTITY_TO_CLASS(fireanddie, CFireAndDie);

void CFireAndDie::Spawn()
{
	SetClassname("fireanddie");
	// Don't call Precache() - it should be called on restore
}

void CFireAndDie::Precache()
{
	// This gets called on restore
	pev->nextthink = gpGlobals->time + m_flDelay;
}

void CFireAndDie::Think()
{
	SUB_UseTargets(this, UseType::Toggle, 0);
	UTIL_Remove(this);
}

LINK_ENTITY_TO_CLASS(trigger_changelevel, CChangeLevel);

// Global Savedata for changelevel trigger
TYPEDESCRIPTION	CChangeLevel::m_SaveData[] =
{
	DEFINE_ARRAY(CChangeLevel, m_szMapName, FIELD_CHARACTER, MAX_MAPNAME_LENGTH),
	DEFINE_ARRAY(CChangeLevel, m_szLandmarkName, FIELD_CHARACTER, MAX_MAPNAME_LENGTH),
	DEFINE_FIELD(CChangeLevel, m_changeTarget, FIELD_STRING),
	DEFINE_FIELD(CChangeLevel, m_changeTargetDelay, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CChangeLevel, CBaseTrigger);

void CChangeLevel::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "map"))
	{
		if (strlen(pkvd->szValue) >= MAX_MAPNAME_LENGTH)
			ALERT(at_error, "Map name '%s' too long (32 chars)\n", pkvd->szValue);
		safe_strcpy(m_szMapName, pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "landmark"))
	{
		if (strlen(pkvd->szValue) >= MAX_MAPNAME_LENGTH)
			ALERT(at_error, "Landmark name '%s' too long (32 chars)\n", pkvd->szValue);
		safe_strcpy(m_szLandmarkName, pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "changetarget"))
	{
		m_changeTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "changedelay"))
	{
		m_changeTargetDelay = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseTrigger::KeyValue(pkvd);
}

void CChangeLevel::Spawn()
{
	if (AreStringsEqual(m_szMapName, ""))
		ALERT(at_console, "a trigger_changelevel doesn't have a map");

	if (AreStringsEqual(m_szLandmarkName, ""))
		ALERT(at_console, "trigger_changelevel to %s doesn't have a landmark", m_szMapName);

	if (!IsStringNull(pev->targetname))
	{
		SetUse(&CChangeLevel::UseChangeLevel);
	}
	InitTrigger();
	if (!(pev->spawnflags & SF_CHANGELEVEL_USEONLY))
		SetTouch(&CChangeLevel::TouchChangeLevel);
	//	ALERT( at_console, "TRANSITION: %s (%s)\n", m_szMapName, m_szLandmarkName );
}

static char st_szNextMap[MAX_MAPNAME_LENGTH];
static char st_szNextSpot[MAX_MAPNAME_LENGTH];

CBaseEntity* CChangeLevel::FindLandmark(const char* pLandmarkName)
{
	CBaseEntity* pLandmark = nullptr;

	while ((pLandmark = UTIL_FindEntityByTargetname(pLandmark, pLandmarkName)) != nullptr)
	{
		// Found the landmark
		if (pLandmark->ClassnameIs("info_landmark"))
			return pLandmark;
	}

	ALERT(at_error, "Can't find landmark %s\n", pLandmarkName);
	return nullptr;
}

void CChangeLevel::UseChangeLevel(const UseInfo& info)
{
	ChangeLevelNow(info.GetActivator());
}

void CChangeLevel::ChangeLevelNow(CBaseEntity* pActivator)
{
	ASSERT(!AreStringsEqual(m_szMapName, ""));

	if (!g_pGameRules->AreChangeLevelsAllowed())
		return;

	// Some people are firing these multiple times in a frame, disable
	if (gpGlobals->time == pev->dmgtime)
		return;

	pev->dmgtime = gpGlobals->time;

	CBaseEntity* pPlayer = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));
	if (!InTransitionVolume(pPlayer, m_szLandmarkName))
	{
		ALERT(at_aiconsole, "Player isn't in the transition volume %s, aborting\n", m_szLandmarkName);
		return;
	}

	// Create an entity to fire the changetarget
	if (!IsStringNull(m_changeTarget))
	{
		CFireAndDie* pFireAndDie = GetClassPtr((CFireAndDie*)nullptr);
		if (pFireAndDie)
		{
			// Set target and delay
			pFireAndDie->pev->target = m_changeTarget;
			pFireAndDie->m_flDelay = m_changeTargetDelay;
			pFireAndDie->pev->origin = pPlayer->pev->origin;
			// Call spawn
			DispatchSpawn(pFireAndDie->edict());
		}
	}
	// This object will get removed in the call to CHANGE_LEVEL, copy the params into "safe" memory
	safe_strcpy(st_szNextMap, m_szMapName);

	m_hActivator = pActivator;
	SUB_UseTargets(pActivator, UseType::Toggle, 0);
	st_szNextSpot[0] = 0;	// Init landmark to NULL

	// look for a landmark entity		
	CBaseEntity* pLandmark = FindLandmark(m_szLandmarkName);
	if (!IsNullEnt(pLandmark))
	{
		safe_strcpy(st_szNextSpot, m_szLandmarkName);
		gpGlobals->vecLandmarkOffset = pLandmark->pev->origin;
	}
	//LEVELLIST	levels[16];
	//	ALERT( at_console, "Level touches %d levels\n", ChangeList( levels, 16 ) );
	ALERT(at_console, "CHANGE LEVEL: %s %s\n", st_szNextMap, st_szNextSpot);
	CHANGE_LEVEL(st_szNextMap, st_szNextSpot);
}

void CChangeLevel::TouchChangeLevel(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
		return;

	ChangeLevelNow(pOther);
}

bool CChangeLevel::AddTransitionToList(LEVELLIST* pLevelList, int listCount, const char* pMapName, const char* pLandmarkName, CBaseEntity* pLandmark)
{
	if (!pLevelList || !pMapName || !pLandmarkName || !pLandmark)
		return false;

	for (int i = 0; i < listCount; i++)
	{
		if (pLevelList[i].pentLandmark == pLandmark->edict() && strcmp(pLevelList[i].mapName, pMapName) == 0)
			return false;
	}
	safe_strcpy(pLevelList[listCount].mapName, pMapName);
	safe_strcpy(pLevelList[listCount].landmarkName, pLandmarkName);
	pLevelList[listCount].pentLandmark = pLandmark->edict();
	pLevelList[listCount].vecLandmarkOrigin = pLandmark->pev->origin;

	return true;
}

bool CChangeLevel::InTransitionVolume(CBaseEntity* pEntity, char* pVolumeName)
{
	if (pEntity->ObjectCaps() & FCAP_FORCE_TRANSITION)
		return true;

	// If you're following another entity, follow it through the transition (weapons follow the player)
	if (pEntity->pev->movetype == Movetype::Follow)
	{
		if (pEntity->pev->aiment != nullptr)
			pEntity = CBaseEntity::Instance(pEntity->pev->aiment);
	}

	bool inVolume = true;	// Unless we find a trigger_transition, everything is in the volume

	CBaseEntity* pVolume = nullptr;
	while ((pVolume = UTIL_FindEntityByTargetname(pVolume, pVolumeName)) != nullptr)
	{
		if (pVolume && pVolume->ClassnameIs("trigger_transition"))
		{
			if (pVolume->Intersects(pEntity))	// It touches one, it's in the volume
				return true;
			else
				inVolume = false;	// Found a trigger_transition, but I don't intersect it -- if I don't find another, don't go!
		}
	}

	return inVolume;
}

/**
*	@brief We can only ever move 512 entities across a transition
*/
constexpr int MAX_ENTITY = 512;

// This has grown into a complicated beast
// Can we make this more elegant?
int CChangeLevel::BuildChangeList(LEVELLIST* pLevelList, int maxList)
{
	// Find all of the possible level changes on this BSP
	int count = 0;

	{
		CBaseEntity* pChangelevel = nullptr;

		while ((pChangelevel = UTIL_FindEntityByClassname(pChangelevel, "trigger_changelevel")) != nullptr)
		{
			auto pTrigger = static_cast<CChangeLevel*>(pChangelevel);

			// Find the corresponding landmark
			CBaseEntity* pLandmark = FindLandmark(pTrigger->m_szLandmarkName);
			if (pLandmark)
			{
				// Build a list of unique transitions
				if (AddTransitionToList(pLevelList, count, pTrigger->m_szMapName, pTrigger->m_szLandmarkName, pLandmark))
				{
					count++;
					if (count >= maxList)		// FULL!!
						break;
				}
			}
		}
	}

	if (count > 0 && gpGlobals->pSaveData && ((SAVERESTOREDATA*)gpGlobals->pSaveData)->pTable)
	{
		CSave saveHelper((SAVERESTOREDATA*)gpGlobals->pSaveData);

		for (int i = 0; i < count; i++)
		{
			int entityCount = 0;
			CBaseEntity* pEntList[MAX_ENTITY]{};
			int entityFlags[MAX_ENTITY]{};

			// Follow the linked list of entities in the PVS of the transition landmark
			// Build a list of valid entities in this linked list (we're going to use pEntity->pev->chain again)
			for (CBaseEntity* pEntity = Instance(UTIL_EntitiesInPVS(pLevelList[i].pentLandmark));
				!IsNullEnt(pEntity);
				pEntity = Instance(pEntity->pev->chain))
			{
				//					ALERT( at_console, "Trying %s\n", pEntity->GetClassname() );
				int caps = pEntity->ObjectCaps();
				if (!(caps & FCAP_DONT_SAVE))
				{
					int flags = 0;

					// If this entity can be moved or is global, mark it
					if (caps & FCAP_ACROSS_TRANSITION)
						flags |= FENTTABLE_MOVEABLE;
					if (!IsStringNull(pEntity->pev->globalname) && !pEntity->IsDormant())
						flags |= FENTTABLE_GLOBAL;
					if (flags)
					{
						pEntList[entityCount] = pEntity;
						entityFlags[entityCount] = flags;
						entityCount++;
						if (entityCount > MAX_ENTITY)
							ALERT(at_error, "Too many entities across a transition!");
					}
					//						else
					//							ALERT( at_console, "Failed %s\n", pEntity->GetClassname() );
				}
				//					else
				//						ALERT( at_console, "DON'T SAVE %s\n", pEntity->GetClassname() );
			}

			for (int j = 0; j < entityCount; j++)
			{
				// Check to make sure the entity isn't screened out by a trigger_transition
				if (entityFlags[j] && InTransitionVolume(pEntList[j], pLevelList[i].landmarkName))
				{
					// Mark entity table with 1<<i
					int index = saveHelper.EntityIndex(pEntList[j]);
					// Flag it with the level number
					saveHelper.EntityFlagsSet(index, entityFlags[j] | (1 << i));
				}
				//				else
				//					ALERT( at_console, "Screened out %s\n", pEntList[j]->GetClassname() );

			}
		}
	}

	return count;
}

LINK_ENTITY_TO_CLASS(trigger_endsection, CTriggerEndSection);

void CTriggerEndSection::EndSectionUse(const UseInfo& info)
{
	// Only save on clients
	if (info.GetActivator() && !info.GetActivator()->IsNetClient())
		return;

	SetUse(nullptr);

	if (!IsStringNull(pev->message))
	{
		g_engfuncs.pfnEndSection(STRING(pev->message));
	}
	UTIL_Remove(this);
}

void CTriggerEndSection::Spawn()
{
	if (g_pGameRules->IsMultiplayer())
	{
		UTIL_RemoveNow(this);
		return;
	}

	InitTrigger();

	SetUse(&CTriggerEndSection::EndSectionUse);
	// If it is a "use only" trigger, then don't set the touch function.
	if (!(pev->spawnflags & SF_ENDSECTION_USEONLY))
		SetTouch(&CTriggerEndSection::EndSectionTouch);
}

void CTriggerEndSection::EndSectionTouch(CBaseEntity* pOther)
{
	// Only save on clients
	if (!pOther->IsNetClient())
		return;

	SetTouch(nullptr);

	if (!IsStringNull(pev->message))
	{
		g_engfuncs.pfnEndSection(STRING(pev->message));
	}
	UTIL_Remove(this);
}

void CTriggerEndSection::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "section"))
	{
		//		m_iszSectionName = ALLOC_STRING( pkvd->szValue );
				// Store this in message so we don't have to write save/restore for this ent
		pev->message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseTrigger::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(trigger_autosave, CTriggerSave);

void CTriggerSave::Spawn()
{
	if (!g_pGameRules->AreSaveGamesSupported())
	{
		UTIL_RemoveNow(this);
		return;
	}

	InitTrigger();
	SetTouch(&CTriggerSave::SaveTouch);
}

void CTriggerSave::SaveTouch(CBaseEntity* pOther)
{
	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
		return;

	// Only save on clients
	if (!pOther->IsPlayer())
		return;

	SetTouch(nullptr);
	UTIL_Remove(this);
	SERVER_COMMAND("autosave\n");
}

LINK_ENTITY_TO_CLASS(player_loadsaved, CRevertSaved);

TYPEDESCRIPTION	CRevertSaved::m_SaveData[] =
{
	DEFINE_FIELD(CRevertSaved, m_messageTime, FIELD_FLOAT),	// These are not actual times, but durations, so save as floats
	DEFINE_FIELD(CRevertSaved, m_loadTime, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CRevertSaved, CPointEntity);

void CRevertSaved::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "duration"))
	{
		SetDuration(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "holdtime"))
	{
		SetHoldTime(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "messagetime"))
	{
		SetMessageTime(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "loadtime"))
	{
		SetLoadTime(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CRevertSaved::Use(const UseInfo& info)
{
	UTIL_ScreenFadeAll(pev->rendercolor, Duration(), HoldTime(), pev->renderamt, FFADE_OUT);
	pev->nextthink = gpGlobals->time + MessageTime();
	SetThink(&CRevertSaved::MessageThink);
}

void CRevertSaved::MessageThink()
{
	UTIL_ShowMessageAll(STRING(pev->message));
	const float nextThink = LoadTime() - MessageTime();
	if (nextThink > 0)
	{
		pev->nextthink = gpGlobals->time + nextThink;
		SetThink(&CRevertSaved::LoadThink);
	}
	else
		LoadThink();
}

void CRevertSaved::LoadThink()
{
	if (g_pGameRules->AreSaveGamesSupported())
	{
		SERVER_COMMAND("reload\n");
	}
}
