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

#include "triggers/CBaseTrigger.hpp"
#include "CChangeLevel.generated.hpp"

constexpr int SF_CHANGELEVEL_USEONLY = 0x0002;

/**
*	@brief When the player touches this, he gets sent to the map listed in the "map" variable.
*/
class EHL_CLASS(EntityName=trigger_changelevel) CChangeLevel : public CBaseTrigger
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void KeyValue(KeyValueData * pkvd) override;

	/**
	*	@brief allows level transitions to be triggered by buttons, etc.
	*/
	void EXPORT UseChangeLevel(const UseInfo & info);
	void EXPORT TouchChangeLevel(CBaseEntity * pOther);
	void ChangeLevelNow(CBaseEntity * pActivator);

	static CBaseEntity* FindLandmark(const char* pLandmarkName);

	/**
	*	@brief This builds the list of all transitions on this level and which entities are in their PVS's and can / should be moved across.
	*/
	static int BuildChangeList(LEVELLIST * pLevelList, int maxList);

	/**
	*	@brief Add a transition to the list, but ignore duplicates (a designer may have placed multiple trigger_changelevels with the same landmark)
	*/
	static bool AddTransitionToList(LEVELLIST * pLevelList, int listCount, const char* pMapName, const char* pLandmarkName, CBaseEntity * pentLandmark);
	static bool InTransitionVolume(CBaseEntity * pEntity, char* pVolumeName);

	EHL_FIELD(Persisted)
	char m_szMapName[MAX_MAPNAME_LENGTH]{};			//!< next map

	EHL_FIELD(Persisted)
	char m_szLandmarkName[MAX_MAPNAME_LENGTH]{};	//!< landmark on next map

	EHL_FIELD(Persisted)
	string_t m_changeTarget = iStringNull;

	EHL_FIELD(Persisted)
	float m_changeTargetDelay = 0;
};
