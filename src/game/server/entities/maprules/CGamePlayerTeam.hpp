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
#include "CGamePlayerTeam.generated.hpp"

constexpr int SF_PTEAM_FIREONCE = 0x0001;
constexpr int SF_PTEAM_KILL = 0x0002;
constexpr int SF_PTEAM_GIB = 0x0004;

/**
*	@brief Changes the team of the player who fired it
*	@details Flag: Fire once
*	Flag: Kill Player
*	Flag: Gib Player
*/
class EHL_CLASS(EntityName=game_player_team) CGamePlayerTeam : public CRulePointEntity
{
	EHL_GENERATED_BODY()

public:
	void		Use(const UseInfo& info) override;

private:

	inline bool RemoveOnFire() { return (pev->spawnflags & SF_PTEAM_FIREONCE) != 0; }
	inline bool ShouldKillPlayer() { return (pev->spawnflags & SF_PTEAM_KILL) != 0; }
	inline bool ShouldGibPlayer() { return (pev->spawnflags & SF_PTEAM_GIB) != 0; }

	const char* TargetTeamName(const char* pszTargetName);
};
