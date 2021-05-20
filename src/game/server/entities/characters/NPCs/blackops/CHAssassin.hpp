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
#include "CBaseMonster.monsters.hpp"

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_ASSASSIN_EXPOSED = LAST_COMMON_SCHEDULE + 1,// cover was blown.
	SCHED_ASSASSIN_JUMP,	// fly through the air
	SCHED_ASSASSIN_JUMP_ATTACK,	// fly through the air and shoot
	SCHED_ASSASSIN_JUMP_LAND, // hit and run away
};

//=========================================================
// monster-specific tasks
//=========================================================

enum
{
	TASK_ASSASSIN_FALL_TO_GROUND = LAST_COMMON_TASK + 1, // falling and waiting to hit ground
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int ASSASSIN_AE_SHOOT1 = 1;
constexpr int ASSASSIN_AE_TOSS1 = 2;
constexpr int ASSASSIN_AE_JUMP = 3;

constexpr int bits_MEMORY_BADJUMP = bits_MEMORY_CUSTOM1;

/**
*	@brief Human assassin, fast and stealthy
*/
class CHAssassin : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify() override;
	int  SoundMask() override;
	void Shoot();
	void HandleAnimEvent(AnimationEvent& event) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	bool CheckMeleeAttack1(float flDot, float flDist) override;	//!< jump like crazy if the enemy gets too close. 
	bool CheckRangeAttack1(float flDot, float flDist) override;	//!< drop a cap in their ass
	bool CheckRangeAttack2(float flDot, float flDist) override;	//!< toss grenade is enemy gets in the way and is too close.
	void StartTask(Task_t* pTask) override;
	void RunAI() override;
	void RunTask(Task_t* pTask) override;
	void DeathSound() override;
	void IdleSound() override;
	CUSTOM_SCHEDULES;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	float m_flLastShot = 0;
	float m_flDiviation = 0;

	float m_flNextJump = 0;
	Vector m_vecJumpVelocity;

	float m_flNextGrenadeCheck = 0;
	Vector m_vecTossVelocity;
	bool m_fThrowGrenade = false;

	int m_iTargetRanderamt = 0;

	int m_iFrustration = 0;

	int m_iShell = 0;
};
