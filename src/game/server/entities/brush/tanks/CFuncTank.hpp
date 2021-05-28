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

#include "CBaseEntity.hpp"
#include "CFuncTank.generated.hpp"

constexpr int SF_TANK_ACTIVE = 0x0001;
constexpr int SF_TANK_PLAYER = 0x0002;
constexpr int SF_TANK_HUMANS = 0x0004;
constexpr int SF_TANK_ALIENS = 0x0008;
constexpr int SF_TANK_LINEOFSIGHT = 0x0010;
constexpr int SF_TANK_CANCONTROL = 0x0020;
constexpr int SF_TANK_SOUNDON = 0x8000;

constexpr Vector gTankSpread[] =
{
	Vector(0, 0, 0),		// perfect
	Vector(0.025, 0.025, 0.025),	// small cone
	Vector(0.05, 0.05, 0.05),  // medium cone
	Vector(0.1, 0.1, 0.1),	// large cone
	Vector(0.25, 0.25, 0.25),	// extra-large cone
};

constexpr int MAX_FIRING_SPREADS = ArraySize(gTankSpread);

enum class TankBullet
{
	None = 0,
	Cal9mm = 1,
	MP5 = 2,
	Cal12mm = 3,
};

/**
*	@details Custom damage
*	env_laser (duration is 0.5 rate of fire)
*	rockets
*	explosion?
*/
class EHL_CLASS() CFuncTank : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	Precache() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	Use(const UseInfo& info) override;
	void	Think() override;
	void	TrackTarget();

	/**
	*	@brief Fire targets and spawn sprites
	*/
	virtual void Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker);
	virtual Vector UpdateTargetPosition(CBaseEntity* pTarget)
	{
		return pTarget->BodyTarget(GetAbsOrigin());
	}

	void	StartRotSound();
	void	StopRotSound();

	/**
	*	@brief Bmodels don't go across transitions
	*/
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	inline bool IsActive() { return (pev->spawnflags & SF_TANK_ACTIVE) != 0; }

	inline void TankActivate()
	{
		pev->spawnflags |= SF_TANK_ACTIVE;
		pev->nextthink = pev->ltime + 0.1;
		m_fireLast = 0;
	}

	inline void TankDeactivate()
	{
		pev->spawnflags &= ~SF_TANK_ACTIVE;
		m_fireLast = 0;
		StopRotSound();
	}

	inline bool CanFire() { return (gpGlobals->time - m_lastSightTime) < m_persist; }
	bool		InRange(float range);

	/**
	*	@brief Acquire a target.  pPlayer is a player in the PVS
	*/
	CBaseEntity* FindTarget(CBaseEntity* pPlayer);

	void		TankTrace(const Vector& vecStart, const Vector& vecForward, const Vector& vecSpread, TraceResult& tr);

	Vector		BarrelPosition()
	{
		Vector forward, right, up;
		AngleVectors(GetAbsAngles(), forward, right, up);
		return GetAbsOrigin() + (forward * m_barrelPos.x) + (right * m_barrelPos.y) + (up * m_barrelPos.z);
	}

	/**
	*	@brief If barrel is offset, add in additional rotation
	*/
	void		AdjustAnglesForBarrel(Vector& angles, float distance);

	/**
	*	@brief TANK CONTROLLING
	*/
	bool OnControls(CBaseEntity* pTest) override;
	bool StartControl(CBasePlayer* pController);
	void StopControl();

	/**
	*	@brief Called each frame by the player's ItemPostFrame
	*/
	void ControllerPostFrame();


protected:
	EHL_FIELD(Persisted)
	EHandle<CBasePlayer> m_hController;

	EHL_FIELD(Persisted, Type=Time)
	float m_flNextAttack = 0;

	//TODO: should be position
	EHL_FIELD(Persisted)
	Vector m_vecControllerUsePos;

	EHL_FIELD(Persisted)
	float m_yawCenter = 0;		// "Center" yaw

	EHL_FIELD(Persisted)
	float m_yawRate = 0;		// Max turn rate to track targets

	EHL_FIELD(Persisted)
	float m_yawRange = 0;		// Range of turning motion (one-sided: 30 is +/- 30 degress from center)
								// Zero is full rotation

	EHL_FIELD(Persisted)
	float m_yawTolerance = 0;	// Tolerance angle

	EHL_FIELD(Persisted)
	float m_pitchCenter = 0;	// "Center" pitch

	EHL_FIELD(Persisted)
	float m_pitchRate = 0;		// Max turn rate on pitch

	EHL_FIELD(Persisted)
	float m_pitchRange = 0;		// Range of pitch motion as above

	EHL_FIELD(Persisted)
	float m_pitchTolerance = 0;	// Tolerance angle

	EHL_FIELD(Persisted, Type=Time)
	float m_fireLast = 0;		// Last time I fired

	EHL_FIELD(Persisted)
	float m_fireRate = 0;		// How many rounds/second

	EHL_FIELD(Persisted, Type=Time)
	float m_lastSightTime = 0;	// Last time I saw target

	EHL_FIELD(Persisted, Type=SoundName)
	string_t m_iszRotateSound = iStringNull;

	EHL_FIELD(Persisted)
	float m_persist = 0;		// Persistence of firing (how long do I shoot when I can't see)

	EHL_FIELD(Persisted)
	float m_minRange = 0;		// Minimum range to aim/track

	EHL_FIELD(Persisted)
	float m_maxRange = 0;		// Max range to aim/track

	EHL_FIELD(Persisted)
	Vector m_barrelPos;			// Length of the freakin barrel

	EHL_FIELD(Persisted)
	float m_spriteScale = 0;	// Scale of any sprites we shoot

	//TODO: should probably both be ModelName
	EHL_FIELD(Persisted)
	string_t m_iszSpriteSmoke = iStringNull;

	EHL_FIELD(Persisted)
	string_t m_iszSpriteFlash = iStringNull;

	EHL_FIELD(Persisted)
	TankBullet m_bulletType = TankBullet::None;	// Bullet type

	EHL_FIELD(Persisted)
	int m_iBulletDamage = 0;	// 0 means use Bullet type's default damage

	//TODO: should be position
	EHL_FIELD(Persisted)
	Vector m_sightOrigin;		// Last sight of target

	EHL_FIELD(Persisted)
	int m_spread = 0;			// firing spread

	EHL_FIELD(Persisted)
	string_t m_iszMaster = iStringNull;	// Master entity (game_team_master or multisource)
};
