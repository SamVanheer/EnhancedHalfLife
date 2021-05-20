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

#include "CItem.hpp"

constexpr float HEALTHKIT_DEFAULT_CAPACITY = -1;

class CHealthKit : public CItem
{
	void OnConstruct() override
	{
		CItem::OnConstruct();
		SetModelName("models/w_medkit.mdl");
	}

	void KeyValue(KeyValueData* pkvd) override;
	void Precache() override;
	ItemApplyResult Apply(CBasePlayer* pPlayer) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	float m_flCustomCapacity = HEALTHKIT_DEFAULT_CAPACITY;
};