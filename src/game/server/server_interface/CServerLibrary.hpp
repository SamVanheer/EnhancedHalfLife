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

#pragma once

#include <unordered_set>

struct SAVERESTOREDATA;
struct server_t;

/**
*	@brief Handles core server actions
*/
class CServerLibrary final
{
public:
	CServerLibrary() = default;
	~CServerLibrary() = default;

	CServerLibrary(const CServerLibrary&) = delete;
	CServerLibrary& operator=(const CServerLibrary&) = delete;

	void MapIsEnding()
	{
		m_isStartingNewMap = true;
	}

	bool CheckForNewMapStart(bool loadGame)
	{
		if (!m_isLoadingSave)
		{
			m_isLoadingSave = loadGame;
			m_landmarksCheckedForRestore.clear();
		}

		if (m_isStartingNewMap)
		{
			m_isStartingNewMap = false;
			NewMapStarted(loadGame);
			return true;
		}

		return false;
	}

	/**
	*	@brief Checks if the entity list needs restoring
	*	@details To override the engine's entity creation code we need to do two things:
	*	1. Tell the engine all entity classnames are "custom". This is the function normally used for unrecognized entities during entity data parsing.\n
	*		This also provides a catch-all for any engine-level calls that might still try to create entities (there shouldn't be any)
	*	2. When an entity is being restored, check if we need to create all entities in the table that the entity came from.\n
	*		This will be the case for the main save file's entities when loading a save game\n
	*		and when changing maps with changelevel2 (landmark-based changelevel).\n
	*		This will also be the case for the entity lists from adjacent maps.\n
	*		The first entity from each list that is restored will trigger creation of all existing entities,\n
	*		just like the engine would have done.
	*
	*	This is necessary because the restore code has to be able to initialize ehandles, and some PostRestore code accesses fully formed entities.
	*/
	void CheckEntityListNeedsRestoring();

	void MapActivate();

private:
	void NewMapStarted(bool loadGame);

	void ParseEntityData();

	void CreateRestoredEntities(SAVERESTOREDATA* saveData);

private:
	bool m_isStartingNewMap = true;
	bool m_isLoadingSave = false;

	server_t* m_engineServer = nullptr;

	std::unordered_set<std::size_t> m_landmarksCheckedForRestore;
};

inline CServerLibrary g_Server;
