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
#include "CGameScore.generated.hpp"

constexpr int SF_SCORE_NEGATIVE = 0x0001;
constexpr int SF_SCORE_TEAM = 0x0002;

/**
*	@brief award points to player / team
*	@details Points +/- total
*	Flag: Allow negative scores					SF_SCORE_NEGATIVE
*	Flag: Award points to team in teamplay		SF_SCORE_TEAM
*/
class EHL_CLASS("EntityName": "game_score") CGameScore : public CRulePointEntity
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override;
	void	KeyValue(KeyValueData* pkvd) override;

	inline	int		Points() { return pev->frags; }
	inline	bool	AllowNegativeScore() { return (pev->spawnflags & SF_SCORE_NEGATIVE) != 0; }
	inline	bool	AwardToTeam() { return (pev->spawnflags & SF_SCORE_TEAM) != 0; }

	inline	void	SetPoints(int points) { pev->frags = points; }

private:
};