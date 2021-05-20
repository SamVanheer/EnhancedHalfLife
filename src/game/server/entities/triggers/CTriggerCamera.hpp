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

#include "CBaseDelay.hpp"

#include "CTriggerCamera.generated.hpp"

class CBasePlayer;

constexpr int SF_CAMERA_PLAYER_POSITION = 1;
constexpr int SF_CAMERA_PLAYER_TARGET = 2;
constexpr int SF_CAMERA_PLAYER_TAKECONTROL = 4;

class EHL_CLASS() CTriggerCamera : public CBaseDelay
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void KeyValue(KeyValueData * pkvd) override;
	void Use(const UseInfo & info) override;
	void EXPORT FollowTarget();
	void Move();

	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	EHL_FIELD(Persisted)
	EHandle<CBasePlayer> m_hPlayer;

	EHL_FIELD(Persisted)
	EHANDLE m_hTarget;

	EHL_FIELD(Persisted)
	EHandle<CBaseEntity> m_hEntPath;

	EHL_FIELD(Persisted)
	string_t m_sPath = iStringNull;

	EHL_FIELD(Persisted)
	float m_flWait = 0;

	EHL_FIELD(Persisted, Type = Time)
	float m_flReturnTime = 0;

	EHL_FIELD(Persisted, Type = Time)
	float m_flStopTime = 0;

	EHL_FIELD(Persisted)
	float m_moveDistance = 0;

	EHL_FIELD(Persisted)
	float m_targetSpeed = 0;

	EHL_FIELD(Persisted)
	float m_initialSpeed = 0;

	EHL_FIELD(Persisted)
	float m_acceleration = 0;

	EHL_FIELD(Persisted)
	float m_deceleration = 0;

	EHL_FIELD(Persisted)
	bool m_state = false;
};

