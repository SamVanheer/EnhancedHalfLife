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
#include "CGameTeamSet.generated.hpp"

constexpr int SF_TEAMSET_FIREONCE = 0x0001;
constexpr int SF_TEAMSET_CLEARTEAM = 0x0002;

/**
*	@brief Changes the team of the entity it targets to the activator's team
*	@details Flag: Fire once
*	Flag: Clear team -- Sets the team to "NONE" instead of activator
*/
class EHL_CLASS("EntityName": "game_team_set") CGameTeamSet : public CRulePointEntity
{
	EHL_GENERATED_BODY()

public:
	void		Use(const UseInfo& info) override;
	inline bool RemoveOnFire() { return (pev->spawnflags & SF_TEAMSET_FIREONCE) != 0; }
	inline bool ShouldClearTeam() { return (pev->spawnflags & SF_TEAMSET_CLEARTEAM) != 0; }
};
