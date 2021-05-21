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
#include "CPendulum.generated.hpp"

constexpr int SF_PENDULUM_SWING = 2;	// spawnflag that makes a pendulum a rope swing.

constexpr int SF_PENDULUM_AUTO_RETURN = 16;
constexpr int SF_PENDULUM_PASSABLE = 32;

class EHL_CLASS() CPendulum : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	EXPORT Swing();
	void	EXPORT PendulumUse(const UseInfo& info);
	void	EXPORT Stop();
	void	Touch(CBaseEntity* pOther) override;
	void	EXPORT RopeTouch(CBaseEntity* pOther);// this touch func makes the pendulum a rope
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void	Blocked(CBaseEntity* pOther) override;

	EHL_FIELD(Persisted)
	float m_accel = 0;			// Acceleration

	EHL_FIELD(Persisted)
	float m_distance = 0;			// 

	EHL_FIELD(Persisted, Type=Time)
	float m_time = 0;

	EHL_FIELD(Persisted)
	float m_damp = 0;

	EHL_FIELD(Persisted)
	float m_maxSpeed = 0;

	EHL_FIELD(Persisted)
	float m_dampSpeed = 0;

	EHL_FIELD(Persisted)
	Vector m_center;

	EHL_FIELD(Persisted)
	Vector m_start;

	EHL_FIELD(Persisted)
	EHandle<CBaseEntity> m_hRopeUser;
};
