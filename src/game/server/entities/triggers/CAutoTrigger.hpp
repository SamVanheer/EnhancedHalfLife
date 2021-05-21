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

#include "CBaseDelay.hpp"
#include "CAutoTrigger.generated.hpp"

constexpr int SF_AUTO_FIREONCE = 0x0001;

/**
*	@brief This trigger will fire when the level spawns (or respawns if not fire once)
*	It will check a global state before firing. It supports delay and killtargets
*/
class EHL_CLASS() CAutoTrigger : public CBaseDelay
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData * pkvd) override;
	void Spawn() override;
	void Precache() override;
	void Think() override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
private:
	EHL_FIELD(Persisted)
	string_t m_globalstate = iStringNull;

	EHL_FIELD(Persisted)
	UseType	triggerType = UseType::Off;
};
