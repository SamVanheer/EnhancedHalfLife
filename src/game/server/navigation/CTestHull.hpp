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

/**
*	@brief TestHull is a modelless clip hull that verifies reachable nodes by walking from every node to each of it's connections
*/
class EHL_CLASS() CTestHull : public CBaseMonster
{
public:
	void Spawn(CBaseEntity* pMasterNode);
	int	ObjectCaps() override { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void EXPORT CallBuildNodeGraph();

	/**
	*	@brief think function called by the empty walk hull that is spawned by the first node to spawn.
	*	This function links all nodes that can see each other, then eliminates all inline links,
	*	then uses a monster-sized hull that walks between each node and each of its links
	*	to ensure that a monster can actually fit through the space
	*/
	void BuildNodeGraph();

	/**
	*	@brief makes a bad node fizzle.
	*	When there's a problem with node graph generation, the test hull will be placed up the bad node's location and will generate particles
	*/
	void EXPORT ShowBadNode();

	/**
	*	@brief spawns TestHull on top of the 0th node and drops it to the ground.
	*/
	void EXPORT DropDelay();

	/**
	*	@brief returns a hardcoded path.
	*/
	void EXPORT PathFind();

	Vector	vecBadNodeOrigin;
};
