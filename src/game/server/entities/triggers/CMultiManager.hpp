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

#include "CBaseToggle.hpp"
#include "CMultiSource.hpp"
#include "CMultiManager.generated.hpp"

constexpr int SF_MULTIMAN_CLONE = 0x80000000;
constexpr int SF_MULTIMAN_THREAD = 0x00000001;

/**
*	@brief The Multimanager Entity - when fired, will fire up to MAX_MULTI_TARGETS targets at specified times.
*	@details FLAG: THREAD (create clones when triggered)
*	FLAG: CLONE (this is a clone for a threaded execution)
*	@see MAX_MULTI_TARGETS
*/
class EHL_CLASS() CMultiManager : public CBaseToggle
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData * pkvd) override;
	void Spawn() override;
	void EXPORT ManagerThink();
	void EXPORT ManagerUse(const UseInfo & info);

#if _DEBUG
	void EXPORT ManagerReport();
#endif

	bool		HasTarget(string_t targetname) override;

	int ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	EHL_FIELD(Persisted)
	int m_cTargets = 0;								//!< the total number of targets in this manager's fire list.

	EHL_FIELD(Persisted)
	int m_index = 0;								//!< Current target

	EHL_FIELD(Persisted, Type=Time)
	float m_startTime = 0;							//!< Time we started firing

	EHL_FIELD(Persisted)
	string_t m_iTargetName[MAX_MULTI_TARGETS]{};	//!< list if indexes into global string array

	EHL_FIELD(Persisted)
	float m_flTargetDelay[MAX_MULTI_TARGETS]{};		//!< delay (in seconds) from time of manager fire to target fire
private:
	inline bool IsClone() { return (pev->spawnflags & SF_MULTIMAN_CLONE) != 0; }
	inline bool ShouldClone()
	{
		if (IsClone())
			return false;

		return (pev->spawnflags & SF_MULTIMAN_THREAD) != 0;
	}

	CMultiManager* Clone();
};
