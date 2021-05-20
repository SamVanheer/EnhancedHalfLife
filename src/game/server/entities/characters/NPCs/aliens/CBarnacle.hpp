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

#include "CBaseMonster.hpp"

constexpr int BARNACLE_BODY_HEIGHT = 44; // how 'tall' the barnacle's model is.
constexpr int BARNACLE_PULL_SPEED = 8;
constexpr int BARNACLE_KILL_VICTIM_DELAY = 5; // how many seconds after pulling prey in to gib them. 

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int BARNACLE_AE_PUKEGIB = 2;

/**
*	@brief stationary ceiling mounted 'fishing' monster
*/
class EHL_CLASS() CBarnacle : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;

	/**
	*	@brief does a trace along the barnacle's tongue to see if any entity is touching it.
	*	Also stores the length of the trace in the int pointer provided.
	*/
	CBaseEntity* TongueTouchEnt(float* pflLength);
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;
	void EXPORT BarnacleThink();
	void EXPORT WaitTillDead();
	void Killed(const KilledInfo& info) override;
	bool TakeDamage(const TakeDamageInfo& info) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	float m_flAltitude = 0;
	float m_flKillVictimTime = 0;
	int m_cGibs = 0;// barnacle loads up on gibs each time it kills something.
	bool m_fTongueExtended = false;
	bool m_fLiftingPrey = false;
	float m_flTongueAdj = 0;
};
