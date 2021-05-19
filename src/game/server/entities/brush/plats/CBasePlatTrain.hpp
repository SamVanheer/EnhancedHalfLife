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

constexpr int SF_PLAT_TOGGLE = 0x0001;

class CBasePlatTrain : public CBaseToggle
{
public:
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void KeyValue(KeyValueData* pkvd) override;
	void Precache() override;

	// This is done to fix spawn flag collisions between this class and a derived class
	virtual bool IsTogglePlat() { return (pev->spawnflags & SF_PLAT_TOGGLE) != 0; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	byte m_bMoveSnd = 0; // sound a plat makes while moving
	byte m_bStopSnd = 0; // sound a plat makes when it stops
	float m_volume = 0; // Sound volume
};

//TODO: get rid of these
#define noiseMoving noise
#define noiseArrived noise1
#define noiseMovement noise
#define noiseStopMoving noise1
