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

class CBeam;

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int ISLAVE_AE_CLAW = 1;
constexpr int ISLAVE_AE_CLAWRAKE = 2;
constexpr int ISLAVE_AE_ZAP_POWERUP = 3;
constexpr int ISLAVE_AE_ZAP_SHOOT = 4;
constexpr int ISLAVE_AE_ZAP_DONE = 5;

constexpr int ISLAVE_MAX_BEAMS = 8;

constexpr int SF_ISLAVE_IS_REVIVED_SLAVE = 1 << 0;

/**
*	@brief Alien slave monster
*/
class CISlave : public CSquadMonster
{
public:
	void OnRemove() override;
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int	 SoundMask() override;
	int  Classify() override;
	Relationship GetRelationship(CBaseEntity* pTarget) override;
	void HandleAnimEvent(AnimationEvent& event) override;

	/**
	*	@brief normal beam attack
	*/
	bool CheckRangeAttack1(float flDot, float flDist) override;

	/**
	*	@brief check bravery and try to resurect dead comrades
	*/
	bool CheckRangeAttack2(float flDot, float flDist) override;
	void CallForHelp(const char* szClassname, float flDist, EHANDLE hEnemy, const Vector& vecLocation);
	void TraceAttack(const TraceAttackInfo& info) override;

	/**
	*	@brief get provoked when injured
	*/
	bool TakeDamage(const TakeDamageInfo& info) override;

	void DeathSound() override;
	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;

	void Killed(const KilledInfo& info) override;

	void StartTask(Task_t* pTask) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	CUSTOM_SCHEDULES;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void ClearBeams();

	/**
	*	@brief small beam from arm to nearby geometry
	*/
	void ArmBeam(int side);

	/**
	*	@brief regenerate dead colleagues
	*/
	void WackBeam(int side, CBaseEntity* pEntity);

	/**
	*	@brief heavy damage directly forward
	*/
	void ZapBeam(int side);

	/**
	*	@brief brighten all beams
	*/
	void BeamGlow();

	int m_iBravery = 0;

	EHandle<CBeam> m_hBeam[ISLAVE_MAX_BEAMS];

	int m_iBeams = 0;
	float m_flNextAttack = 0;

	int	m_voicePitch = 0;

	EHANDLE m_hDead;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
};
