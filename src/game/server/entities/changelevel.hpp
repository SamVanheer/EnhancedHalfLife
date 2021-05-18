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

#include "triggers.hpp"

/**
*	@brief Define space that travels across a level transition
*	Derive from point entity so this doesn't move across levels
*/
class EHL_CLASS() CTriggerVolume : public CPointEntity
{
public:
	void		Spawn() override;
};

/**
*	@brief Fires a target after level transition and then dies
*/
class EHL_CLASS() CFireAndDie : public CBaseDelay
{
public:
	void Spawn() override;
	void Precache() override;
	void Think() override;
	int ObjectCaps() override { return CBaseDelay::ObjectCaps() | FCAP_FORCE_TRANSITION; }	// Always go across transitions
};

constexpr int SF_CHANGELEVEL_USEONLY = 0x0002;

/**
*	@brief When the player touches this, he gets sent to the map listed in the "map" variable.
*/
class EHL_CLASS() CChangeLevel : public CBaseTrigger
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;

	/**
	*	@brief allows level transitions to be triggered by buttons, etc.
	*/
	void EXPORT UseChangeLevel(const UseInfo& info);
	void EXPORT TouchChangeLevel(CBaseEntity* pOther);
	void ChangeLevelNow(CBaseEntity* pActivator);

	static CBaseEntity* FindLandmark(const char* pLandmarkName);

	/**
	*	@brief This builds the list of all transitions on this level and which entities are in their PVS's and can / should be moved across.
	*/
	static int BuildChangeList(LEVELLIST* pLevelList, int maxList);

	/**
	*	@brief Add a transition to the list, but ignore duplicates (a designer may have placed multiple trigger_changelevels with the same landmark)
	*/
	static bool AddTransitionToList(LEVELLIST* pLevelList, int listCount, const char* pMapName, const char* pLandmarkName, CBaseEntity* pentLandmark);
	static bool InTransitionVolume(CBaseEntity* pEntity, char* pVolumeName);

	EHL_FIELD(Persisted)
	char m_szMapName[MAX_MAPNAME_LENGTH]{};			//!< next map

	EHL_FIELD(Persisted)
	char m_szLandmarkName[MAX_MAPNAME_LENGTH]{};	//!< landmark on next map

	EHL_FIELD(Persisted)
	string_t m_changeTarget = iStringNull;

	EHL_FIELD(Persisted)
	float m_changeTargetDelay = 0;
};

constexpr int SF_ENDSECTION_USEONLY = 0x0001;

class EHL_CLASS() CTriggerEndSection : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT EndSectionTouch(CBaseEntity* pOther);
	void KeyValue(KeyValueData* pkvd) override;
	void EXPORT EndSectionUse(const UseInfo& info);
};

class EHL_CLASS() CTriggerSave : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT SaveTouch(CBaseEntity* pOther);
};

class EHL_CLASS() CRevertSaved : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void	Use(const UseInfo& info) override;
	void	EXPORT MessageThink();
	void	EXPORT LoadThink();
	void	KeyValue(KeyValueData* pkvd) override;

	inline	float	Duration() { return pev->dmg_take; }
	inline	float	HoldTime() { return pev->dmg_save; }
	inline	float	MessageTime() { return m_messageTime; }
	inline	float	LoadTime() { return m_loadTime; }

	inline	void	SetDuration(float duration) { pev->dmg_take = duration; }
	inline	void	SetHoldTime(float hold) { pev->dmg_save = hold; }
	inline	void	SetMessageTime(float time) { m_messageTime = time; }
	inline	void	SetLoadTime(float time) { m_loadTime = time; }

private:
	EHL_FIELD(Persisted)
	float m_messageTime = 0; //!< These are not actual times, but durations, so save as floats

	EHL_FIELD(Persisted)
	float m_loadTime = 0;
};
