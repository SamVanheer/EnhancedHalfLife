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
#include "CPlayerSetHealth.generated.hpp"

constexpr int PLAYERSETHEALTH_ALLPLAYERS = 1 << 0;
constexpr int PLAYERSETHEALTH_SETHEALTH = 1 << 1;
constexpr int PLAYERSETHEALTH_SETARMOR = 1 << 2;

/**
*	@brief Sets the player's health and/or armor to a mapper-specified value
*/
class EHL_CLASS("EntityName": "player_sethealth") CPlayerSetHealth : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData* pkvd) override;

	void Use(const UseInfo& info) override;

private:
	void ApplyToTarget(CBasePlayer* player);

private:
	EHL_FIELD("Persisted": true)
	int m_Flags = 0;

	EHL_FIELD("Persisted": true)
	int m_Health = 100;

	EHL_FIELD("Persisted": true)
	int m_Armor = 0;
};
