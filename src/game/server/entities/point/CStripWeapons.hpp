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

constexpr int WEAPONSTRIP_REMOVEWEAPONS = 1 << 0;
constexpr int WEAPONSTRIP_REMOVESUIT = 1 << 1;
constexpr int WEAPONSTRIP_REMOVELONGJUMP = 1 << 2;

class CStripWeapons : public CPointEntity
{
public:
	void KeyValue(KeyValueData* pkvd) override;

	void Use(const UseInfo& info) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_Flags = WEAPONSTRIP_REMOVEWEAPONS;
};
