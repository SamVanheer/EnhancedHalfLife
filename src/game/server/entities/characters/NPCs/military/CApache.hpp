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
#include "CApache.generated.hpp"

class CBeam;

constexpr int SF_APACHE_WAITFORTRIGGER = 0x04 | 0x40; // UNDONE: Fix!
constexpr int SF_NOWRECKAGE = 0x08;

class EHL_CLASS(EntityName=monster_apache) CApache : public CBaseMonster
{
	EHL_GENERATED_BODY()

	void Spawn() override;
	void Precache() override;
	int  Classify() override { return CLASS_HUMAN_MILITARY; }
	int  BloodColor() override { return DONT_BLEED; }
	void Killed(const KilledInfo& info) override;
	void GibMonster() override;

	void SetObjectCollisionBox() override
	{
		pev->absmin = GetAbsOrigin() + Vector(-300, -300, -172);
		pev->absmax = GetAbsOrigin() + Vector(300, 300, 8);
	}

	void EXPORT HuntThink();
	void EXPORT FlyTouch(CBaseEntity* pOther);
	void EXPORT CrashTouch(CBaseEntity* pOther);
	void EXPORT DyingThink();
	void EXPORT StartupUse(const UseInfo& info);
	void EXPORT NullThink();

	void ShowDamage();
	void Flight();
	void FireRocket();
	bool FireGun();

	bool TakeDamage(const TakeDamageInfo& info) override;
	void TraceAttack(const TraceAttackInfo& info) override;

	EHL_FIELD(Persisted)
	int m_iRockets = 0;

	EHL_FIELD(Persisted)
	float m_flForce = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flNextRocket = 0;

	EHL_FIELD(Persisted)
	Vector m_vecTarget;

	EHL_FIELD(Persisted, Type=Position)
	Vector m_posTarget;

	EHL_FIELD(Persisted)
	Vector m_vecDesired;

	EHL_FIELD(Persisted, Type=Position)
	Vector m_posDesired;

	EHL_FIELD(Persisted)
	Vector m_vecGoal;

	EHL_FIELD(Persisted)
	Vector m_angGun;

	EHL_FIELD(Persisted, Type=Time)
	float m_flLastSeen = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flPrevSeen = 0;

	//Don't save, precached
	int m_iSoundState = 0; // don't save this

	int m_iSpriteTexture = 0;
	int m_iExplode = 0;
	int m_iBodyGibs = 0;

	EHL_FIELD(Persisted)
	float m_flGoalSpeed = 0;

	EHL_FIELD(Persisted)
	int m_iDoSmokePuff = 0;

	EHL_FIELD(Persisted)
	EHandle<CBeam> m_hBeam;
};
