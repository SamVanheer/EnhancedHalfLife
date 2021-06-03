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

#include "CBaseEntity.hpp"
#include "CCrossbowBolt.generated.hpp"

constexpr int BOLT_AIR_VELOCITY = 2000;
constexpr int BOLT_WATER_VELOCITY = 1000;

// UNDONE: Save/restore this?
// 
// OVERLOADS SOME ENTVARS:
//
// speed - the ideal magnitude of my velocity
class EHL_CLASS("EntityName": "crossbow_bolt") CCrossbowBolt : public CBaseEntity
{
	EHL_GENERATED_BODY()

	void Spawn() override;
	void Precache() override;
	int  Classify() override;
	void EXPORT BubbleThink();
	void EXPORT BoltTouch(CBaseEntity* pOther);
	void EXPORT ExplodeThink();

	int m_iTrail = 0;

public:
	static CCrossbowBolt* BoltCreate();
};
