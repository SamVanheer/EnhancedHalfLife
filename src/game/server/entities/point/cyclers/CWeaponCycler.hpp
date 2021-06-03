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

#include "CBaseWeapon.hpp"
#include "CWeaponCycler.generated.hpp"

class EHL_CLASS("EntityName": "cycler_weapon") CWeaponCycler : public CBaseWeapon
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	int WeaponSlot() override { return 1; }
	bool GetWeaponInfo(WeaponInfo& p) override { return false; }

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Holster() override;
	string_t m_iszModel = iStringNull;
	int m_iModel = 0;
};
