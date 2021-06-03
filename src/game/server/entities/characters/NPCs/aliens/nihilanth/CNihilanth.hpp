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
#include "CNihilanth.generated.hpp"

class CNihilanthHVR;
class CSprite;

constexpr int N_SCALE = 15;
constexpr int N_SPHERES = 20;

/**
*	@brief Nihilanth, final Boss monster
*/
class EHL_CLASS("EntityName": "monster_nihilanth") CNihilanth : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void OnRemove() override;

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

	EHL_FIELD("Persisted": true)
	float m_flForce = 0;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_flNextPainSound = 0;

	EHL_FIELD("Persisted": true)
	Vector m_velocity;

	EHL_FIELD("Persisted": true)
	Vector m_avelocity;

	EHL_FIELD("Persisted": true)
	Vector m_vecTarget;

	EHL_FIELD("Persisted": true, "Type": "Position")
	Vector m_posTarget;

	EHL_FIELD("Persisted": true)
	Vector m_vecDesired;

	EHL_FIELD("Persisted": true, "Type": "Position")
	Vector m_posDesired;

	EHL_FIELD("Persisted": true)
	float  m_flMinZ = 0;

	EHL_FIELD("Persisted": true)
	float  m_flMaxZ = 0;

	EHL_FIELD("Persisted": true)
	Vector m_vecGoal;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_flLastSeen = 0;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_flPrevSeen = 0;

	EHL_FIELD("Persisted": true)
	int m_irritation = 0;

	EHL_FIELD("Persisted": true)
	int m_iLevel = 0;

	EHL_FIELD("Persisted": true)
	int m_iTeleport = 0;

	EHL_FIELD("Persisted": true)
	EHANDLE m_hRecharger;

	EHL_FIELD("Persisted": true)
	EHandle<CNihilanthHVR> m_hSphere[N_SPHERES];

	EHL_FIELD("Persisted": true)
	int	m_iActiveSpheres = 0;

	EHL_FIELD("Persisted": true)
	float m_flAdj = 0;

	EHL_FIELD("Persisted": true)
	EHandle<CSprite> m_hBall;

	EHL_FIELD("Persisted": true)
	char m_szRechargerTarget[64]{};

	EHL_FIELD("Persisted": true)
	char m_szDrawUse[64]{};

	EHL_FIELD("Persisted": true)
	char m_szTeleportUse[64]{};

	EHL_FIELD("Persisted": true)
	char m_szTeleportTouch[64]{};

	EHL_FIELD("Persisted": true)
	char m_szDeadUse[64]{};

	EHL_FIELD("Persisted": true)
	char m_szDeadTouch[64]{};

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_flShootEnd = 0;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_flShootTime = 0;

	EHL_FIELD("Persisted": true)
	EHANDLE m_hFriend[3];
};
