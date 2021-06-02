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
#include "CHoundeye.generated.hpp"

// houndeye does 20 points of damage spread over a sphere 384 units in diameter, and each additional 
// squad member increases the BASE damage by 110%, per the spec.
constexpr int HOUNDEYE_MAX_SQUAD_SIZE = 4;
constexpr int HOUNDEYE_MAX_ATTACK_RADIUS = 384;
constexpr float HOUNDEYE_SQUAD_BONUS = 1.1;

constexpr int HOUNDEYE_EYE_FRAMES = 4; // how many different switchable maps for the eye

constexpr int HOUNDEYE_SOUND_STARTLE_VOLUME = 128; // how loud a sound has to be to badly scare a sleeping houndeye

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_HOUND_CLOSE_EYE = LAST_COMMON_TASK + 1,
	TASK_HOUND_OPEN_EYE,
	TASK_HOUND_THREAT_DISPLAY,
	TASK_HOUND_FALL_ASLEEP,
	TASK_HOUND_WAKE_UP,
	TASK_HOUND_HOP_BACK
};

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_HOUND_AGITATED = LAST_COMMON_SCHEDULE + 1,
	SCHED_HOUND_HOP_RETREAT,
	SCHED_HOUND_FAIL,
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int HOUND_AE_WARN = 1;
constexpr int HOUND_AE_STARTATTACK = 2;
constexpr int HOUND_AE_THUMP = 3;
constexpr int HOUND_AE_ANGERSOUND1 = 4;
constexpr int HOUND_AE_ANGERSOUND2 = 5;
constexpr int HOUND_AE_HOPBACK = 6;
constexpr int HOUND_AE_CLOSE_EYE = 7;

/**
*	@brief spooky sonic dog.
*/
class EHL_CLASS(EntityName=monster_houndeye) CHoundeye : public CSquadMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;
	void SetYawSpeed() override;
	void WarmUpSound();
	void AlertSound() override;
	void DeathSound() override;
	void WarnSound();
	void PainSound() override;
	void IdleSound() override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	void SonicAttack();
	void PrescheduleThink() override;
	void SetActivity(Activity NewActivity) override;

	/**
	*	@brief writes a color vector to the network based on the size of the group.
	*/
	void WriteBeamColor();

	/**
	*	@brief overridden for houndeyes so that they try to get within half of their max attack radius before attacking,
	*	so as to increase their chances of doing damage.
	*/
	bool CheckRangeAttack1(float flDot, float flDist) override;
	bool ValidateHintType(short sHint) override;
	bool CanActiveIdle() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	Schedule_t* GetSchedule() override;

	CUSTOM_SCHEDULES;

	//TODO: shouldn't be saved
	EHL_FIELD(Persisted)
	int m_iSpriteTexture = 0;

	EHL_FIELD(Persisted)
	bool m_fAsleep = false;// some houndeyes sleep in idle mode if this is set, the houndeye is lying down

	EHL_FIELD(Persisted)
	bool m_fDontBlink = false;// don't try to open/close eye if this bit is set!

	EHL_FIELD(Persisted, Type=Position)
	Vector m_vecPackCenter; // the center of the pack. The leader maintains this by averaging the origins of all pack members.
};
