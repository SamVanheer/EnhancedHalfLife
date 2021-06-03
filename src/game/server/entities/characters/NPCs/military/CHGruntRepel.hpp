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
#include "CHGruntRepel.generated.hpp"

/**
*	@brief when triggered, spawns a monster_human_grunt repelling down a line.
*/
class EHL_CLASS("EntityName": "monster_grunt_repel") CHGruntRepel : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void EXPORT RepelUse(const UseInfo& info);
	int m_iSpriteTexture = 0;	// Don't save, precache
};
