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

#include "CTriggerCamera.generated.hpp"

class CBasePlayer;

constexpr int SF_CAMERA_PLAYER_POSITION = 1;
constexpr int SF_CAMERA_PLAYER_TARGET = 2;
constexpr int SF_CAMERA_PLAYER_TAKECONTROL = 4;

class EHL_CLASS("EntityName": "trigger_camera") CTriggerCamera : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void KeyValue(KeyValueData * pkvd) override;
	void Use(const UseInfo & info) override;
	void EXPORT FollowTarget();
	void Move();

	int	ObjectCaps() override { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	EHL_FIELD("Persisted": true)
	EHandle<CBasePlayer> m_hPlayer;

	EHL_FIELD("Persisted": true)
	EHANDLE m_hTarget;

	EHL_FIELD("Persisted": true)
	EHandle<CBaseEntity> m_hEntPath;

	EHL_FIELD("Persisted": true)
	string_t m_sPath;

	EHL_FIELD("Persisted": true)
	float m_flWait = 0;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_flReturnTime = 0;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_flStopTime = 0;

	EHL_FIELD("Persisted": true)
	float m_moveDistance = 0;

	EHL_FIELD("Persisted": true)
	float m_targetSpeed = 0;

	EHL_FIELD("Persisted": true)
	float m_initialSpeed = 0;

	EHL_FIELD("Persisted": true)
	float m_acceleration = 0;

	EHL_FIELD("Persisted": true)
	float m_deceleration = 0;

	EHL_FIELD("Persisted": true)
	bool m_state = false;
};

