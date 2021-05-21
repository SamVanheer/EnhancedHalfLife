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

#include "CTalkMonster.hpp"
#include "CScientist.generated.hpp"

constexpr int SCIENTIST_BODYGROUP_HEAD = 1;
constexpr int SCIENTIST_BODYGROUP_NEEDLE = 2;

constexpr int SCIENTIST_HEAD_GLASSES = 0;
constexpr int SCIENTIST_HEAD_EINSTEIN = 1;
constexpr int SCIENTIST_HEAD_LUTHER = 2;
constexpr int SCIENTIST_HEAD_SLICK = 3;

constexpr int SCIENTIST_NEEDLE_OFF = 0;
constexpr int SCIENTIST_NEEDLE_ON = 1;

constexpr int NUM_SCIENTIST_HEADS = 4; // four heads available for scientist model

enum
{
	SCHED_HIDE = LAST_TALKMONSTER_SCHEDULE + 1,
	SCHED_FEAR,
	SCHED_PANIC,
	SCHED_STARTLE,
	SCHED_TARGET_CHASE_SCARED,
	SCHED_TARGET_FACE_SCARED,
};

enum
{
	TASK_SAY_HEAL = LAST_TALKMONSTER_TASK + 1,
	TASK_HEAL,
	TASK_SAY_FEAR,
	TASK_RUN_PATH_SCARED,
	TASK_SCREAM,
	TASK_RANDOM_SCREAM,
	TASK_MOVE_TO_TARGET_RANGE_SCARED,
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int SCIENTIST_AE_HEAL = 1;
constexpr int SCIENTIST_AE_NEEDLEON = 2;
constexpr int SCIENTIST_AE_NEEDLEOFF = 3;

/**
*	@brief human scientist (passive lab worker)
*/
class EHL_CLASS() CScientist : public CTalkMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;

	void SetYawSpeed() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;
	void RunTask(Task_t* pTask) override;
	void StartTask(Task_t* pTask) override;
	int	ObjectCaps() override { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	bool TakeDamage(const TakeDamageInfo& info) override;
	int FriendNumber(int arrayNumber) override;
	void SetActivity(Activity newActivity) override;
	Activity GetStoppedActivity() override;
	int SoundMask() override;
	void DeclineFollowing() override;

	float	CoverRadius() override { return 1200; }		// Need more room for cover because scientists want to get far away!
	bool	DisregardEnemy(CBaseEntity* pEnemy) { return !pEnemy->IsAlive() || (gpGlobals->time - m_fearTime) > 15; }

	bool	CanHeal();
	void	Heal();
	void	Scream();

	// Override these to set behavior
	Schedule_t* GetScheduleOfType(int Type) override;
	Schedule_t* GetSchedule() override;
	NPCState GetIdealState() override;

	void DeathSound() override;
	void PainSound() override;

	void TalkInit();

	void Killed(const KilledInfo& info) override;

	CUSTOM_SCHEDULES;

private:
	EHL_FIELD(Persisted, Type=Time)
	float m_painTime = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_healTime = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_fearTime = 0;
};
