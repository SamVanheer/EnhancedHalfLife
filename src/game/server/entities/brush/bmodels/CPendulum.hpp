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

constexpr int SF_PENDULUM_SWING = 2;	// spawnflag that makes a pendulum a rope swing.

constexpr int SF_PENDULUM_AUTO_RETURN = 16;
constexpr int SF_PENDULUM_PASSABLE = 32;

class EHL_CLASS() CPendulum : public CBaseEntity
{
public:
	void	Spawn() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	EXPORT Swing();
	void	EXPORT PendulumUse(const UseInfo& info);
	void	EXPORT Stop();
	void	Touch(CBaseEntity* pOther) override;
	void	EXPORT RopeTouch(CBaseEntity* pOther);// this touch func makes the pendulum a rope
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	void	Blocked(CBaseEntity* pOther) override;

	static	TYPEDESCRIPTION m_SaveData[];

	float m_accel = 0;			// Acceleration
	float m_distance = 0;			// 
	float m_time = 0;
	float m_damp = 0;
	float m_maxSpeed = 0;
	float m_dampSpeed = 0;
	Vector m_center;
	Vector m_start;

	EHandle<CBaseEntity> m_hRopeUser;
};
