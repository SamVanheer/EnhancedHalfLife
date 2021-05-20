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

constexpr int SF_GLOBAL_SET = 1;			// Set global state to initial state on spawn

enum class GlobalTriggerMode
{
	Off,
	On,
	Dead,
	Toggle
};

class EHL_CLASS() CEnvGlobal : public CPointEntity
{
public:
	void	Spawn() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	Use(const UseInfo& info) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	string_t m_globalstate = iStringNull;
	GlobalTriggerMode m_triggermode = GlobalTriggerMode::Off;
	GlobalEntState m_initialstate = GlobalEntState::Off;
};
