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

class CBaseEntity;

enum class GlobalEntState
{
	Off = 0,
	On = 1,
	Dead = 2
};

struct globalentity_t
{
	char			name[64]{};
	char			levelName[32]{};
	GlobalEntState	state = GlobalEntState::Off;
	globalentity_t* pNext = nullptr;
};

class CGlobalState
{
public:
	CGlobalState()
	{
		Reset();
	}

	void Reset()
	{
		m_pList = nullptr;
		m_listCount = 0;
	}

	void			ClearStates();
	void			EntityAdd(string_t globalname, string_t mapName, GlobalEntState state);
	void			EntitySetState(string_t globalname, GlobalEntState state);
	void			EntityUpdate(string_t globalname, string_t mapname);
	const globalentity_t* EntityFromTable(string_t globalname);
	GlobalEntState	EntityGetState(string_t globalname);
	bool			EntityInTable(string_t globalname) { return Find(globalname) != nullptr; }
	bool Save(CSave& save);
	bool Restore(CRestore& restore);
	static TYPEDESCRIPTION m_SaveData[];

	//#ifdef _DEBUG
	/**
	*	@brief This is available all the time now on impulse 104, remove later
	*/
	void			DumpGlobals();
	//#endif

private:
	globalentity_t* Find(string_t globalname);
	globalentity_t* m_pList = nullptr;
	int				m_listCount = 0;
};

inline CGlobalState gGlobalState;

/**
*	@brief Find the matching global entity.
*
*	Spit out an error if the designer made entities of different classes with the same global name
*/
CBaseEntity* FindGlobalEntity(string_t classname, string_t globalname);
