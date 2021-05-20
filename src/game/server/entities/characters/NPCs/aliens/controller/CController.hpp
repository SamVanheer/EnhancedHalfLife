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

#include "CSquadMonster.hpp"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int CONTROLLER_AE_HEAD_OPEN = 1;
constexpr int CONTROLLER_AE_BALL_SHOOT = 2;
constexpr int CONTROLLER_AE_SMALL_SHOOT = 3;
constexpr int CONTROLLER_AE_POWERUP_FULL = 4;
constexpr int CONTROLLER_AE_POWERUP_HALF = 5;

constexpr int CONTROLLER_FLINCH_DELAY = 2;		// at most one flinch every n secs

class EHL_CLASS() CController : public CSquadMonster
{
public:
	void OnRemove() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;

	void RunAI() override;
	bool CheckRangeAttack1(float flDot, float flDist) override;	//!< shoot a bigass energy ball out of their head
	bool CheckRangeAttack2(float flDot, float flDist) override;	//!< head
	bool CheckMeleeAttack1(float flDot, float flDist) override;	//!< block, throw
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	CUSTOM_SCHEDULES;

	void Stop() override;
	void Move(float flInterval) override;
	LocalMoveResult CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, float* pflDist) override;
	void MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval) override;
	void SetActivity(Activity NewActivity) override;
	bool ShouldAdvanceRoute(float flWaypointDist) override;
	int LookupFloat();

	float m_flNextFlinch = 0;

	float m_flShootTime = 0;
	float m_flShootEnd = 0;

	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;
	void AttackSound();
	void DeathSound() override;

	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];

	bool TakeDamage(const TakeDamageInfo& info) override;
	void Killed(const KilledInfo& info) override;
	void GibMonster() override;

	EHandle<CSprite> m_hBall[2];	// hand balls
	int m_iBall[2]{};				// how bright it should be
	float m_iBallTime[2]{};			// when it should be that color
	int m_iBallCurrent[2]{};		// current brightness

	Vector m_vecEstVelocity;

	Vector m_velocity;
	bool m_fInCombat = false;
};
