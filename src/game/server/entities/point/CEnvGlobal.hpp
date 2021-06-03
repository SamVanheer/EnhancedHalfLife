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

#include "CPointEntity.hpp"
#include "CEnvGlobal.generated.hpp"

constexpr int SF_GLOBAL_SET = 1;			// Set global state to initial state on spawn

enum class GlobalTriggerMode
{
	Off,
	On,
	Dead,
	Toggle
};

class EHL_CLASS("EntityName": "env_global") CEnvGlobal : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	Use(const UseInfo& info) override;

	EHL_FIELD("Persisted": true)
	string_t m_globalstate = iStringNull;

	EHL_FIELD("Persisted": true)
	GlobalTriggerMode m_triggermode = GlobalTriggerMode::Off;

	EHL_FIELD("Persisted": true)
	GlobalEntState m_initialstate = GlobalEntState::Off;
};
