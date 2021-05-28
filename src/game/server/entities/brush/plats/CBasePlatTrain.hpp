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
#include "CBasePlatTrain.generated.hpp"

constexpr int SF_PLAT_TOGGLE = 0x0001;

class EHL_CLASS() CBasePlatTrain : public CBaseToggle
{
	EHL_GENERATED_BODY()

public:
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void KeyValue(KeyValueData* pkvd) override;
	void Precache() override;

	// This is done to fix spawn flag collisions between this class and a derived class
	virtual bool IsTogglePlat() { return (pev->spawnflags & SF_PLAT_TOGGLE) != 0; }

	EHL_FIELD(Persisted)
	byte m_bMoveSnd = 0; // sound a plat makes while moving

	EHL_FIELD(Persisted)
	byte m_bStopSnd = 0; // sound a plat makes when it stops

	EHL_FIELD(Persisted)
	float m_volume = 0; // Sound volume

	EHL_FIELD(Persisted, Type=SoundName)
	string_t m_iszMovingSound = iStringNull;

	EHL_FIELD(Persisted, Type=SoundName)
	string_t m_iszArrivedSound = iStringNull;
};
