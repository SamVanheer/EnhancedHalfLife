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

#include "CWorld.generated.hpp"

/**
*	@brief This spawns first when each level begins.
*
*	this moved here from world.cpp, to allow classes to be derived from it
*/
class EHL_CLASS(EntityName=worldspawn) CWorld : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	~CWorld();

	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;

	int DamageDecal(int bitsDamageType) override;
};
