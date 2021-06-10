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

#include "cbase.hpp"
#include "com_model.hpp"
#include "CServerLibrary.hpp"
#include "EntityParser.hpp"
#include "server_t.hpp"

extern "C" DLLEXPORT void custom(entvars_t * pev)
{
	//Nothing. Must be provided so the engine doesn't free the already created entity
}

/**
*	@details This is provided separately so worldspawn's classname can be just that.
*	It also avoids complications from needing to handle the second pfnKeyValue call.
*/
extern "C" DLLEXPORT void worldspawn(entvars_t * pev)
{
	//Nothing. Must be provided so the engine doesn't free the already created worldspawn
}

void CServerLibrary::NewMapStarted(bool loadGame)
{
	m_engineServer = SV_GetServerGlobal();

	ClearStringPool();

	//Initialize the list to the current engine list
	g_EntityList = CEntityList(m_engineServer->edicts);

	if (!loadGame)
	{
		ParseEntityData();
	}
}

void CServerLibrary::CheckEntityListNeedsRestoring()
{
	if (m_isLoadingSave)
	{
		auto saveData = reinterpret_cast<SAVERESTOREDATA*>(gpGlobals->pSaveData);

		std::size_t hash = 0;
		HashCombine(hash, std::string_view{saveData->szCurrentMapName}, std::string_view{saveData->szLandmarkName});

		if (!m_landmarksCheckedForRestore.contains(hash))
		{
			m_landmarksCheckedForRestore.insert(hash);
			CreateRestoredEntities(saveData);
		}
	}
}

void CServerLibrary::MapActivate()
{
	m_isLoadingSave = false;
}

void CServerLibrary::ParseEntityData()
{
	//We're parsing the entity data string ourselves instead of letting the engine do it
	const std::string_view entityData{m_engineServer->worldmodel->entities};

	EntityParser parser{entityData};

	parser.Parse();

	//Clear the entity data string so the engine won't do anything
	//This must be done by altering the string after the first '{', wherever that may be
	//If there is whitespace in front of it this must still work
	const auto start = entityData.find('{');

	if (start != std::string_view::npos)
	{
		//Must have enough space for "{}\0" at minimum
		if (start + 1 <= entityData.size())
		{
			m_engineServer->worldmodel->entities[start + 1] = '}';
			m_engineServer->worldmodel->entities[start + 2] = '\0';
		}
		else
		{
			//Should never get here, engine only calls into this code if it finds a classname keyvalue, so there must be enough space
			ALERT(at_error, "Entity data string is invalid: not enough space for empty entity block\n");
		}
	}
	else
	{
		//Should never get here, engine validates the first parsed token before we can get to this
		ALERT(at_error, "Entity data string is invalid: does not start with '{'\n");
	}
}

void CServerLibrary::CreateRestoredEntities(SAVERESTOREDATA* saveData)
{
	//Create all entities in the current save data list
	const int savedIndex = saveData->currentIndex;
	saveData->size = saveData->pTable[saveData->currentIndex].location;
	saveData->pCurrentData = saveData->pBaseData + saveData->size;

	entvars_t tmpVars;

	for (int i = 0; i < saveData->tableCount; ++i)
	{
		auto& table = saveData->pTable[i];

		if (table.pent)
		{
			saveData->currentIndex = i;
			saveData->size = table.location;
			saveData->pCurrentData = saveData->pBaseData + saveData->size;

			CRestore tmpRestore(saveData);
			tmpRestore.PrecacheMode(false);
			tmpRestore.ReadEntVars("ENTVARS", &tmpVars);

			auto className = STRING(tmpVars.classname);

			auto entity = g_EntityList.Create(className, table.pent);

			if (!entity)
			{
				ALERT(at_console, "Can't create entity: %s\n", className);
				//Will be skipped during engine restore (unless this is the first entity)
				g_engfuncs.pfnRemoveEntity(table.pent);
				table.pent = nullptr;
			}
		}
	}

	saveData->currentIndex = savedIndex;
	saveData->size = saveData->pTable[saveData->currentIndex].location;
	saveData->pCurrentData = saveData->pBaseData + saveData->size;
}
