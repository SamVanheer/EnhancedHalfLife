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

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int ZOMBIE_AE_ATTACK_RIGHT = 0x01;
constexpr int ZOMBIE_AE_ATTACK_LEFT = 0x02;
constexpr int ZOMBIE_AE_ATTACK_BOTH = 0x03;

constexpr int ZOMBIE_FLINCH_DELAY = 2;		//!< at most one flinch every n secs

class EHL_CLASS() CZombie : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;
	int IgnoreConditions() override;

	float m_flNextFlinch = 0;

	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;
	void AttackSound();

	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

	// No range attacks
	bool CheckRangeAttack1(float flDot, float flDist) override { return false; }
	bool CheckRangeAttack2(float flDot, float flDist) override { return false; }
	bool TakeDamage(const TakeDamageInfo& info) override;
};
