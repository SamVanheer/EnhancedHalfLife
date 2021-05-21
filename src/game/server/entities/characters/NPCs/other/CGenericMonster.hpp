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
#include "CGenericMonster.generated.hpp"

// For holograms, make them not solid so the player can walk through them
constexpr int	SF_GENERICMONSTER_NOTSOLID = 4;

/**
*	@brief purely for scripted sequence work.
*/
class EHL_CLASS() CGenericMonster : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;

	/**
	*	@brief generic monster can't hear.
	*/
	int SoundMask() override;
};
