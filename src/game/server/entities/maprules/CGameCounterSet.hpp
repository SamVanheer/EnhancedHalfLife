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

#include "CRulePointEntity.hpp"
#include "CGameCounterSet.generated.hpp"

constexpr int SF_GAMECOUNTSET_FIREONCE = 0x0001;

/**
*	@brief Sets the counter's value
*	@details Flag: Fire once
*/
class EHL_CLASS(EntityName=game_counter_set) CGameCounterSet : public CRulePointEntity
{
	EHL_GENERATED_BODY()

public:
	void		Use(const UseInfo& info) override;
	inline bool RemoveOnFire() { return (pev->spawnflags & SF_GAMECOUNTSET_FIREONCE) != 0; }
};
