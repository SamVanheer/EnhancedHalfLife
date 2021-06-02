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
#include "CFurniture.generated.hpp"

/**
*	@brief this is the cool comment I cut-and-pasted
*/
class EHL_CLASS(EntityName=monster_furniture) CFurniture : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	/**
	*	@brief This used to have something to do with bees flying, but now it only initializes moving furniture in scripted sequences
	*/
	void Spawn() override;

	/**
	*	@brief ID's Furniture as neutral (noone will attack it)
	*/
	int	 Classify() override;
	int	ObjectCaps() override { return (CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
};
