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
#include "CBloater.generated.hpp"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int BLOATER_AE_ATTACK_MELEE1 = 0x01;

class EHL_CLASS() CBloater : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;

	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;
	void AttackSnd();

	// No range attacks
	bool CheckRangeAttack1(float flDot, float flDist) override { return false; }
	bool CheckRangeAttack2(float flDot, float flDist) override { return false; }
	bool TakeDamage(const TakeDamageInfo& info) override;
};
