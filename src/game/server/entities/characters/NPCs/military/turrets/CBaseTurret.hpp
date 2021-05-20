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

class CSprite;

constexpr int TURRET_SHOTS = 2;
constexpr int TURRET_RANGE = 100 * 12;
constexpr Vector TURRET_SPREAD(0, 0, 0);
constexpr int TURRET_TURNRATE = 30;				//angles per 0.1 second
constexpr int TURRET_MAXWAIT = 15;				// seconds turret will stay active w/o a target
constexpr int TURRET_MAXSPIN = 5;				// seconds turret barrel will spin w/o a target
constexpr float TURRET_MACHINE_VOLUME = 0.5;

enum class TurretAnim
{
	None = 0,
	Fire,
	Spin,
	Deploy,
	Retire,
	Die,
};

enum class TurretOrientation
{
	Floor = 0,
	Ceiling
};

class EHL_CLASS() CBaseTurret : public CBaseMonster
{
public:
	void OnRemove() override;
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;
	void EXPORT TurretUse(const UseInfo& info);

	void TraceAttack(const TraceAttackInfo& info) override;
	bool TakeDamage(const TakeDamageInfo& info) override;

	/**
	*	@brief ID as a machine
	*/
	int	 Classify() override;

	int BloodColor() override { return DONT_BLEED; }
	void GibMonster() override {}	// UNDONE: Throw turret gibs?

	// Think functions

	void EXPORT ActiveThink();

	/**
	*	@brief This search function will sit with the turret deployed and look for a new target.
	*	After a set amount of time, the barrel will spin down. After m_flMaxWait, the turret will retract.
	*/
	void EXPORT SearchThink();

	/**
	*	@brief This think function will deploy the turret when something comes into range.
	*	This is for automatically activated turrets.
	*/
	void EXPORT AutoSearchThink();
	void EXPORT TurretDeath();

	virtual void EXPORT SpinDownCall() { m_iSpin = false; }
	virtual void EXPORT SpinUpCall() { m_iSpin = true; }

	void EXPORT Deploy();
	void EXPORT Retire();

	void EXPORT Initialize();

	virtual void Ping();
	virtual void EyeOn();
	virtual void EyeOff();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	// other functions
	void SetTurretAnim(TurretAnim anim);
	bool MoveTurret();
	virtual void Shoot(const Vector& vecSrc, const Vector& vecDirToEnemy) {}

	float m_flMaxSpin = 0;		// Max time to spin the barrel w/o a target
	bool m_iSpin = false;

	EHandle<CSprite> m_hEyeGlow;
	int m_eyeBrightness = 0;

	int	m_iDeployHeight = 0;
	int	m_iRetractHeight = 0;
	int m_iMinPitch = 0;

	int m_iBaseTurnRate = 0;	// angles per second
	float m_fTurnRate = 0;		// actual turn rate
	TurretOrientation m_iOrientation = TurretOrientation::Floor;
	bool m_iOn = false;
	bool m_fBeserk = false;		// Sometimes this bitch will just freak out
	bool m_iAutoStart = false;	// true if the turret auto deploys when a target
								// enters its range

	Vector m_vecLastSight;
	float m_flLastSight = 0;	// Last time we saw a target
	float m_flMaxWait = 0;		// Max time to seach w/o a target
	int m_iSearchSpeed = 0;		// Not Used!

	// movement
	float m_flStartYaw = 0;
	Vector m_vecCurAngles;
	Vector m_vecGoalAngles;

	float m_flPingTime = 0;		// Time until the next ping, used when searching
	float m_flSpinUpTime = 0;	// Amount of time until the barrel should spin down when searching
};
