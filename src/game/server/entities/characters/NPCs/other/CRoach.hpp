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

enum class RoachState
{
	Idle = 0,
	Bored,
	ScaredByEntity,
	ScaredByLight,
	SmellFood,
	Eat,
};

/**
*	@brief cockroach
*/
class CRoach : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	void EXPORT MonsterThink() override;
	void Move(float flInterval) override;

	/**
	*	@brief Picks a new spot for roach to run to.
	*/
	void PickNewDest(RoachState iCondition);
	void EXPORT Touch(CBaseEntity* pOther) override;
	void Killed(const KilledInfo& info) override;

	float	m_flLastLightLevel = 0;
	float	m_flNextSmellTime = 0;
	int		Classify() override;

	/**
	*	@brief overriden for the roach, which can virtually see 360 degrees.
	*/
	void	Look(int iDistance) override;
	int		SoundMask() override;

	// UNDONE: These don't necessarily need to be save/restored, but if we add more data, it may
	bool m_fLightHacked = false;
	RoachState m_iMode = RoachState::Idle;
};
