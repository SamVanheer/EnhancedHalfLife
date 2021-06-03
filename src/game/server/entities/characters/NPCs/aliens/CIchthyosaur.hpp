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
#include "CFlyingMonster.hpp"
#include "CIchthyosaur.generated.hpp"

class CBeam;

constexpr int SEARCH_RETRY = 16;

constexpr int ICHTHYOSAUR_SPEED = 150;

constexpr int EYE_MAD = 0;
constexpr int EYE_BASE = 1;
constexpr int EYE_CLOSED = 2;
constexpr int EYE_BACK = 3;
constexpr int EYE_LOOK = 4;

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int ICHTHYOSAUR_AE_SHAKE_RIGHT = 1;
constexpr int ICHTHYOSAUR_AE_SHAKE_LEFT = 2;

/**
*	@brief evil, satan fish monster
*/
class EHL_CLASS("EntityName": "monster_ichthyosaur") CIchthyosaur : public CFlyingMonster
{
	EHL_GENERATED_BODY()

public:
	void  Spawn() override;
	void  Precache() override;
	void  SetYawSpeed() override;
	int   Classify() override;
	void  HandleAnimEvent(AnimationEvent& event) override;
	CUSTOM_SCHEDULES;

	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;

	void Killed(const KilledInfo& info) override;
	void BecomeDead() override;

	void EXPORT CombatUse(const UseInfo& info);
	void EXPORT BiteTouch(CBaseEntity* pOther);

	void  StartTask(Task_t* pTask) override;
	void  RunTask(Task_t* pTask) override;

	bool  CheckMeleeAttack1(float flDot, float flDist) override;

	/**
	*	@brief swim in for a chomp
	*/
	bool  CheckRangeAttack1(float flDot, float flDist) override;

	float ChangeYaw(int speed) override;
	Activity GetStoppedActivity() override;

	void  Move(float flInterval) override;
	void  MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval) override;
	void  MonsterThink() override;
	void  Stop() override;
	void  Swim();
	Vector DoProbe(const Vector& Probe);

	float VectorToPitch(const Vector& vec);
	float PitchDiff();
	float ChangePitch(int speed);

	EHL_FIELD("Persisted": true)
	Vector m_SaveVelocity;

	EHL_FIELD("Persisted": true)
	float m_idealDist = 0;

	EHL_FIELD("Persisted": true)
	float m_flBlink = 0;

	//TODO: should be time
	EHL_FIELD("Persisted": true)
	float m_flEnemyTouched = 0;

	EHL_FIELD("Persisted": true)
	bool  m_bOnAttack = false;

	EHL_FIELD("Persisted": true)
	float m_flMaxSpeed = 0;

	EHL_FIELD("Persisted": true)
	float m_flMinSpeed = 0;

	EHL_FIELD("Persisted": true)
	float m_flMaxDist = 0;

	EHandle<CBeam> m_hBeam;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_flNextAlert = 0;

	float m_flLastPitchTime = 0;

	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pAttackSounds[];
	static const char* pBiteSounds[];
	static const char* pDieSounds[];
	static const char* pPainSounds[];

	void IdleSound() override;
	void AlertSound() override;
	void AttackSound();
	void BiteSound();
	void DeathSound() override;
	void PainSound() override;

	template<std::size_t Size>
	void EMIT_ICKY_SOUND(SoundChannel chan, const char* (&array)[Size])
	{
		EmitSound(chan, array[RANDOM_LONG(0, ArraySize(array) - 1)], VOL_NORM, 0.6, RANDOM_LONG(95, 105));
	}
};
