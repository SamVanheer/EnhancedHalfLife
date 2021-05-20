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

constexpr int DEADHEV_BODYGROUP_HEAD = 1;
constexpr int DEADHEV_HEAD_HELMETED = 1;

/**
*	@brief Dead HEV suit prop
*/
class CDeadHEV : public CBaseMonster
{
public:
	void Spawn() override;
	int	Classify() override { return CLASS_HUMAN_MILITARY; }

	void KeyValue(KeyValueData* pkvd) override;

	int	m_iPose = 0;// which sequence to display	-- temporary, don't need to save
	static constexpr const char* m_szPoses[4] = {"deadback", "deadsitting", "deadstomach", "deadtable"};
};