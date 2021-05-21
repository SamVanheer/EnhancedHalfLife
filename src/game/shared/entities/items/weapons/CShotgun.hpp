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
#include "CShotgun.generated.hpp"

enum shotgun_e
{
	SHOTGUN_IDLE = 0,
	SHOTGUN_FIRE,
	SHOTGUN_FIRE2,
	SHOTGUN_RELOAD,
	SHOTGUN_PUMP,
	SHOTGUN_START_RELOAD,
	SHOTGUN_DRAW,
	SHOTGUN_HOLSTER,
	SHOTGUN_IDLE4,
	SHOTGUN_IDLE_DEEP
};

class EHL_CLASS() CShotgun : public CBaseWeapon
{
	EHL_GENERATED_BODY()

public:
	enum class ReloadState
	{
		NotReloading = 0,
		PlayAnimation = 1,	//!< Play the shell load animation
		AddToClip = 2		//!< Update the clip value (and Hud as a result)
	};

	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_SHOTGUN;
		m_iDefaultAmmo = SHOTGUN_DEFAULT_GIVE;
		SetWorldModelName("models/w_shotgun.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 3; }
	bool GetWeaponInfo(WeaponInfo & p) override;
	bool AddToPlayer(CBasePlayer * pPlayer) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	void WeaponPostFrame() override;

	EHL_FIELD(Persisted, Type=Time)
	float m_flNextReload = 0;

	int m_iShell = 0;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

	void GetWeaponData(weapon_data_t & data) override;
	void SetWeaponData(const weapon_data_t & data) override;

	EHL_FIELD(Persisted)
	ReloadState m_fInSpecialReload = ReloadState::NotReloading; //!< Are we in the middle of a reload

private:
	unsigned short m_usDoubleFire = 0;
	unsigned short m_usSingleFire = 0;
};
