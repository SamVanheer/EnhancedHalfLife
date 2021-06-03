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

#include "CPointEntity.hpp"
#include "CInfoBM.generated.hpp"

constexpr int SF_INFOBM_RUN = 0x0001;
constexpr int SF_INFOBM_WAIT = 0x0002;

// AI Nodes for Big Momma
class EHL_CLASS("EntityName": "info_bigmomma") CInfoBM : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;

	// name in pev->targetname
	// next in pev->target
	// radius in pev->scale
	// health in pev->health
	// Reach target in pev->message
	// Reach delay in pev->speed
	// Reach sequence in pev->netname

	EHL_FIELD("Persisted": true)
	string_t m_preSequence = iStringNull;
};
