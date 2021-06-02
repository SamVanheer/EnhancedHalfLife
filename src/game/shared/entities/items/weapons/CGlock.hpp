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
#include "CGlock.generated.hpp"

enum glock_e
{
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER
};

class EHL_CLASS(EntityName=weapon_9mmhandgun, EntityNameAliases=[weapon_glock]) CGlock : public CBaseWeapon
{
	EHL_GENERATED_BODY()

public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_GLOCK;
		m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;
		SetWorldModelName("models/w_9mmhandgun.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 2; }
	bool GetWeaponInfo(WeaponInfo & p) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void GlockFire(float flSpread, float flCycleTime, bool fUseAutoAim);
	bool Deploy() override;
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
	int m_iShell = 0;

	unsigned short m_usFireGlock1 = 0;
	unsigned short m_usFireGlock2 = 0;
};
