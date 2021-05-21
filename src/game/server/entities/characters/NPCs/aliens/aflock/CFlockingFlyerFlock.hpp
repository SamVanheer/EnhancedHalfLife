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

#include "CBaseMonster.hpp"
#include "CFlockingFlyerFlock.generated.hpp"

class EHL_CLASS() CFlockingFlyerFlock : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;
	void SpawnFlock();

	// Sounds are shared by the flock
	static  void PrecacheFlockSounds();

	EHL_FIELD(Persisted)
	int m_cFlockSize = 0;

	EHL_FIELD(Persisted)
	float m_flFlockRadius = 0;
};
