/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#pragma once

#include "CBaseMonster.hpp"
#include "CDeadBarney.generated.hpp"

/**
*	@brief Designer selects a pose in worldcraft, 0 through ArraySize(m_szPoses)-1
*/
class EHL_CLASS() CDeadBarney : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	int	Classify() override { return	CLASS_PLAYER_ALLY; }

	void KeyValue(KeyValueData* pkvd) override;

	int	m_iPose = 0;// which sequence to display	-- temporary, don't need to save
	static const char* m_szPoses[3];
};