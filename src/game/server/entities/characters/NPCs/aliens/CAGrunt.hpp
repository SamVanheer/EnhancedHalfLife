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

#include "CBaseMonster.monsters.hpp"
#include "CSquadMonster.hpp"

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_AGRUNT_SUPPRESS = LAST_COMMON_SCHEDULE + 1,
	SCHED_AGRUNT_THREAT_DISPLAY,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_AGRUNT_SETUP_HIDE_ATTACK = LAST_COMMON_TASK + 1,
	TASK_AGRUNT_GET_PATH_TO_ENEMY_CORPSE,
};

int iAgruntMuzzleFlash;

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int AGRUNT_AE_HORNET1 = 1;
constexpr int AGRUNT_AE_HORNET2 = 2;
constexpr int AGRUNT_AE_HORNET3 = 3;
constexpr int AGRUNT_AE_HORNET4 = 4;
constexpr int AGRUNT_AE_HORNET5 = 5;
// some events are set up in the QC file that aren't recognized by the code yet.
constexpr int AGRUNT_AE_PUNCH = 6;
constexpr int AGRUNT_AE_BITE = 7;

constexpr int AGRUNT_AE_LEFT_FOOT = 10;
constexpr int AGRUNT_AE_RIGHT_FOOT = 11;

constexpr int AGRUNT_AE_LEFT_PUNCH = 12;
constexpr int AGRUNT_AE_RIGHT_PUNCH = 13;

constexpr int AGRUNT_MELEE_DIST = 100;

/**
*	@brief Dominant, warlike alien grunt monster
*/
class EHL_CLASS() CAGrunt : public CSquadMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify() override;
	int  SoundMask() override;
	void HandleAnimEvent(AnimationEvent& event) override;
	void SetObjectCollisionBox() override
	{
		pev->absmin = GetAbsOrigin() + Vector(-32, -32, 0);
		pev->absmax = GetAbsOrigin() + Vector(32, 32, 85);
	}

	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;

	/**
	*	@brief this is overridden for alien grunts because they can use their smart weapons against unseen enemies.
	*	Base class doesn't attack anyone it can't see.
	*/
	bool CanCheckAttacks() override;

	/**
	*	@brief alien grunts zap the crap out of any enemy that gets too close.
	*/
	bool CheckMeleeAttack1(float flDot, float flDist) override;

	/**
	*	@brief !!!LATER - we may want to load balance this.
	*	Several tracelines are done, so we may not want to do this every server frame. Definitely not while firing.
	*/
	bool CheckRangeAttack1(float flDot, float flDist) override;
	void StartTask(Task_t* pTask) override;
	void AlertSound() override;
	void DeathSound() override;
	void PainSound() override;
	void AttackSound();
	void PrescheduleThink() override;
	void TraceAttack(const TraceAttackInfo& info) override;

	/**
	*	@brief overridden because Human Grunts are Alien Grunt's nemesis.
	*/
	Relationship GetRelationship(CBaseEntity* pTarget) override;

	/**
	*	@brief won't speak again for 10-20 seconds.
	*/
	void StopTalking();

	/**
	*	@brief Should this agrunt be talking?
	*/
	bool ShouldSpeak();
	CUSTOM_SCHEDULES;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pAttackSounds[];
	static const char* pDieSounds[];
	static const char* pPainSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];

	bool m_fCanHornetAttack = false;
	float m_flNextHornetAttackCheck = 0;

	float m_flNextPainTime = 0;

	// three hacky fields for speech stuff. These don't really need to be saved.
	float m_flNextSpeakTime = 0;
	float m_flNextWordTime = 0;
	int m_iLastWord = 0;
};
