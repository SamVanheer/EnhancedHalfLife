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
#include "CPython.generated.hpp"

enum python_e
{
	PYTHON_IDLE1 = 0,
	PYTHON_FIDGET,
	PYTHON_FIRE1,
	PYTHON_RELOAD,
	PYTHON_HOLSTER,
	PYTHON_DRAW,
	PYTHON_IDLE2,
	PYTHON_IDLE3
};

class EHL_CLASS(EntityName=weapon_357, EntityNameAliases=[weapon_python]) CPython : public CBaseWeapon
{
	EHL_GENERATED_BODY()

public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_PYTHON;
		m_iDefaultAmmo = PYTHON_DEFAULT_GIVE;
		SetWorldModelName("models/w_357.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 2; }
	bool GetWeaponInfo(WeaponInfo & p) override;
	bool AddToPlayer(CBasePlayer * pPlayer) override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Holster() override;
	void Reload() override;
	void WeaponIdle() override;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usFirePython = 0;
};
