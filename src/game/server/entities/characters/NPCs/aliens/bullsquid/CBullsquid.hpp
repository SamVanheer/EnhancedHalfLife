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
#include "CBaseMonster.monsters.hpp"
#include "CBullsquid.generated.hpp"

constexpr int SQUID_SPRINT_DIST = 256; // how close the squid has to get before starting to sprint and refusing to swerve

inline int			   iSquidSpitSprite;

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_SQUID_HURTHOP = LAST_COMMON_SCHEDULE + 1,
	SCHED_SQUID_SMELLFOOD,
	SCHED_SQUID_SEECRAB,
	SCHED_SQUID_EAT,
	SCHED_SQUID_SNIFF_AND_EAT,
	SCHED_SQUID_WALLOW,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_SQUID_HOPTURN = LAST_COMMON_TASK + 1,
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int BSQUID_AE_SPIT = 1;
constexpr int BSQUID_AE_BITE = 2;
constexpr int BSQUID_AE_BLINK = 3;
constexpr int BSQUID_AE_TAILWHIP = 4;
constexpr int BSQUID_AE_HOP = 5;
constexpr int BSQUID_AE_THROW = 6;

/**
*	@brief big, spotty tentacle-mouthed meanie.
*/
class EHL_CLASS(EntityName=monster_bullchicken) CBullsquid : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  SoundMask() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;
	void IdleSound() override;
	void PainSound() override;
	void DeathSound() override;
	void AlertSound() override;
	void AttackSound();

	/**
	*	@brief OVERRIDDEN for bullsquid because it needs to know explicitly when the last attempt to chase the enemy failed,
	*	since that impacts its attack choices.
	*/
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;

	/**
	*	@brief bullsquid is a big guy, so has a longer melee range than most monsters.
	*	This is the tailwhip attack
	*/
	bool CheckMeleeAttack1(float flDot, float flDist) override;

	/**
	*	@brief bullsquid is a big guy, so has a longer melee range than most monsters.
	*	This is the bite attack.
	*	@details this attack will not be performed if the tailwhip attack is valid.
	*/
	bool CheckMeleeAttack2(float flDot, float flDist) override;
	bool CheckRangeAttack1(float flDot, float flDist) override;

	/**
	*	@brief overridden for bullsquid because there are things that need to be checked every think.
	*/
	void RunAI() override;
	bool ValidateHintType(short sHint) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;

	/**
	*	@brief overridden for bullsquid so we can keep track of how much time has passed since it was last injured
	*/
	bool TakeDamage(const TakeDamageInfo& info) override;

	/**
	*	@brief overridden for bullsquid so that it can be made to ignore its love of headcrabs for a while.
	*/
	Relationship GetRelationship(CBaseEntity* pTarget) override;
	int IgnoreConditions() override;

	/**
	*	@brief Overridden for Bullsquid to deal with the feature that makes it lose interest in headcrabs for a while if something injures it.
	*/
	NPCState GetIdealState() override;

	CUSTOM_SCHEDULES;

	EHL_FIELD(Persisted)
	bool m_fCanThreatDisplay = false;// this is so the squid only does the "I see a headcrab!" dance one time. 

	EHL_FIELD(Persisted, Type=Time)
	float m_flLastHurtTime = 0;// we keep track of this, because if something hurts a squid, it will forget about its love of headcrabs for a while.

	EHL_FIELD(Persisted, Type=Time)
	float m_flNextSpitTime = 0;// last time the bullsquid used the spit attack.
};
