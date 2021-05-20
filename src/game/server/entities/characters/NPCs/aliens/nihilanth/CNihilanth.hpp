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

class CNihilanthHVR;
class CSprite;

constexpr int N_SCALE = 15;
constexpr int N_SPHERES = 20;

/**
*	@brief Nihilanth, final Boss monster
*/
class EHL_CLASS() CNihilanth : public CBaseMonster
{
public:
	void OnRemove() override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;
	void Precache() override;
	int  Classify() override { return CLASS_ALIEN_MILITARY; }
	int  BloodColor() override { return BLOOD_COLOR_YELLOW; }
	void Killed(const KilledInfo& info) override;
	void GibMonster() override;

	void SetObjectCollisionBox() override
	{
		pev->absmin = GetAbsOrigin() + Vector(-16 * N_SCALE, -16 * N_SCALE, -48 * N_SCALE);
		pev->absmax = GetAbsOrigin() + Vector(16 * N_SCALE, 16 * N_SCALE, 28 * N_SCALE);
	}

	void HandleAnimEvent(AnimationEvent& event) override;

	void EXPORT StartupThink();
	void EXPORT HuntThink();
	void EXPORT CrashTouch(CBaseEntity* pOther);
	void EXPORT DyingThink();
	void EXPORT StartupUse(const UseInfo& info);
	void EXPORT NullThink();
	void EXPORT CommandUse(const UseInfo& info);

	void FloatSequence();
	void NextActivity();

	void Flight();

	bool AbsorbSphere();
	bool EmitSphere();
	void TargetSphere(UseType useType, float value);
	CBaseEntity* RandomTargetname(const char* szName);
	void ShootBalls();
	void MakeFriend(Vector vecPos);

	bool TakeDamage(const TakeDamageInfo& info) override;
	void TraceAttack(const TraceAttackInfo& info) override;

	void PainSound() override;
	void DeathSound() override;

	static const char* pAttackSounds[];	// vocalization: play sometimes when he launches an attack
	static const char* pBallSounds[];	// the sound of the lightening ball launch
	static const char* pShootSounds[];	// grunting vocalization: play sometimes when he launches an attack
	static const char* pRechargeSounds[];	// vocalization: play when he recharges
	static const char* pLaughSounds[];	// vocalization: play sometimes when hit and still has lots of health
	static const char* pPainSounds[];	// vocalization: play sometimes when hit and has much less health and no more chargers
	static const char* pDeathSounds[];	// vocalization: play as he dies

	// x_teleattack1.wav	the looping sound of the teleport attack ball.

	float m_flForce = 0;

	float m_flNextPainSound = 0;

	Vector m_velocity;
	Vector m_avelocity;

	Vector m_vecTarget;
	Vector m_posTarget;

	Vector m_vecDesired;
	Vector m_posDesired;

	float  m_flMinZ = 0;
	float  m_flMaxZ = 0;

	Vector m_vecGoal;

	float m_flLastSeen = 0;
	float m_flPrevSeen = 0;

	int m_irritation = 0;

	int m_iLevel = 0;
	int m_iTeleport = 0;

	EHANDLE m_hRecharger;

	EHandle<CNihilanthHVR> m_hSphere[N_SPHERES];
	int	m_iActiveSpheres = 0;

	float m_flAdj = 0;

	EHandle<CSprite> m_hBall;

	char m_szRechargerTarget[64]{};
	char m_szDrawUse[64]{};
	char m_szTeleportUse[64]{};
	char m_szTeleportTouch[64]{};
	char m_szDeadUse[64]{};
	char m_szDeadTouch[64]{};

	float m_flShootEnd = 0;
	float m_flShootTime = 0;

	EHANDLE m_hFriend[3];
};
