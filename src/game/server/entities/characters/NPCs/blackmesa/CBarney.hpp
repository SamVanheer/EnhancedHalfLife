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
#include "CBarney.generated.hpp"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
// first flag is barney dying for scripted sequences?
constexpr int BARNEY_AE_DRAW = 2;
constexpr int BARNEY_AE_SHOOT = 3;
constexpr int BARNEY_AE_HOLSTER = 4;

constexpr int BARNEY_BODYGROUP_GUN = 1;

constexpr int BARNEY_BODY_GUNHOLSTERED = 0;
constexpr int BARNEY_BODY_GUNDRAWN = 1;
constexpr int BARNEY_BODY_GUNGONE = 2;

class EHL_CLASS() CBarney : public CTalkMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  SoundMask() override;

	/**
	*	@brief shoots one round from the pistol at the enemy barney is facing.
	*/
	void BarneyFirePistol();

	/**
	*	@brief barney says "Freeze!"
	*/
	void AlertSound() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;

	void RunTask(Task_t* pTask) override;
	void StartTask(Task_t* pTask) override;
	int	ObjectCaps() override { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	bool TakeDamage(const TakeDamageInfo& info) override;
	bool CheckRangeAttack1(float flDot, float flDist) override;

	void DeclineFollowing() override;

	// Override these to set behavior
	Schedule_t* GetScheduleOfType(int Type) override;
	Schedule_t* GetSchedule() override;
	NPCState GetIdealState() override;

	void DeathSound() override;
	void PainSound() override;

	void TalkInit();

	void TraceAttack(const TraceAttackInfo& info) override;
	void Killed(const KilledInfo& info) override;

	EHL_FIELD(Persisted)
	bool m_fGunDrawn = false;

	EHL_FIELD(Persisted, Type=Time)
	float m_painTime = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_checkAttackTime = 0;

	EHL_FIELD(Persisted)
	bool m_lastAttackCheck = false;

	// UNDONE: What is this for?  It isn't used?
	EHL_FIELD(Persisted)
	float m_flPlayerDamage = 0;// how much pain has the player inflicted on me?

	CUSTOM_SCHEDULES;
};
