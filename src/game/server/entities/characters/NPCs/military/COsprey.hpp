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

constexpr int SF_WAITFORTRIGGER = 0x40;

constexpr int MAX_CARRY = 24;

class EHL_CLASS() COsprey : public CBaseMonster
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
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

	EHandle<CBaseEntity> m_hGoalEnt;
	Vector m_vel1;
	Vector m_vel2;
	Vector m_pos1;
	Vector m_pos2;
	Vector m_ang1;
	Vector m_ang2;
	float m_startTime = 0;
	float m_dTime = 0;

	Vector m_velocity;

	float m_flIdealtilt = 0;
	float m_flRotortilt = 0;

	float m_flRightHealth = 0;
	float m_flLeftHealth = 0;

	int	m_iUnits = 0;
	EHANDLE m_hGrunt[MAX_CARRY];
	Vector m_vecOrigin[MAX_CARRY];
	EHANDLE m_hRepel[4];

	int m_iSoundState = 0;
	int m_iSpriteTexture = 0;

	int m_iPitch = 0;

	int m_iExplode = 0;
	int	m_iTailGibs = 0;
	int	m_iBodyGibs = 0;
	int	m_iEngineGibs = 0;

	int m_iDoLeftSmokePuff = 0;
	int m_iDoRightSmokePuff = 0;
};
