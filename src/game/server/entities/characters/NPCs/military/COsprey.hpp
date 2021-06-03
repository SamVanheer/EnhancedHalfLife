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
#include "COsprey.generated.hpp"

constexpr int SF_OSPREY_WAITFORTRIGGER = 0x40;

constexpr int MAX_CARRY = 24;

class EHL_CLASS("EntityName": "monster_osprey") COsprey : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	int		ObjectCaps() override { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	void Spawn() override;
	void Precache() override;
	int  Classify() override { return CLASS_MACHINE; }
	int  BloodColor() override { return DONT_BLEED; }
	void Killed(const KilledInfo& info) override;

	void UpdateGoal();
	bool HasDead();
	void EXPORT FlyThink();
	void EXPORT DeployThink();
	void Flight();
	void EXPORT HitTouch(CBaseEntity* pOther);
	void EXPORT FindAllThink();
	void EXPORT HoverThink();
	CBaseMonster* MakeGrunt(const Vector& vecSrc);
	void EXPORT CrashTouch(CBaseEntity* pOther);
	void EXPORT DyingThink();
	void EXPORT CommandUse(const UseInfo& info);

	// bool TakeDamage(const TakeDamageInfo& info) override;
	void TraceAttack(const TraceAttackInfo& info) override;
	void ShowDamage();

	EHL_FIELD("Persisted": true)
	EHandle<CBaseEntity> m_hGoalEnt;

	EHL_FIELD("Persisted": true)
	Vector m_vel1;

	EHL_FIELD("Persisted": true)
	Vector m_vel2;

	EHL_FIELD("Persisted": true, "Type": "Position")
	Vector m_pos1;

	EHL_FIELD("Persisted": true, "Type": "Position")
	Vector m_pos2;

	EHL_FIELD("Persisted": true)
	Vector m_ang1;

	EHL_FIELD("Persisted": true)
	Vector m_ang2;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_startTime = 0;

	EHL_FIELD("Persisted": true)
	float m_dTime = 0;

	EHL_FIELD("Persisted": true)
	Vector m_velocity;

	EHL_FIELD("Persisted": true)
	float m_flIdealtilt = 0;

	EHL_FIELD("Persisted": true)
	float m_flRotortilt = 0;

	EHL_FIELD("Persisted": true)
	float m_flRightHealth = 0;

	EHL_FIELD("Persisted": true)
	float m_flLeftHealth = 0;

	EHL_FIELD("Persisted": true)
	int	m_iUnits = 0;

	EHL_FIELD("Persisted": true)
	EHANDLE m_hGrunt[MAX_CARRY];

	EHL_FIELD("Persisted": true, "Type": "Position")
	Vector m_vecOrigin[MAX_CARRY];

	EHL_FIELD("Persisted": true)
	EHANDLE m_hRepel[4];

	//These 7 not saved
	int m_iSoundState = 0;
	int m_iSpriteTexture = 0;

	int m_iPitch = 0;

	int m_iExplode = 0;
	int	m_iTailGibs = 0;
	int	m_iBodyGibs = 0;
	int	m_iEngineGibs = 0;

	EHL_FIELD("Persisted": true)
	int m_iDoLeftSmokePuff = 0;

	EHL_FIELD("Persisted": true)
	int m_iDoRightSmokePuff = 0;
};
