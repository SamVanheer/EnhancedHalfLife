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
#include "CHandGrenade.generated.hpp"

enum handgrenade_e
{
	HANDGRENADE_IDLE = 0,
	HANDGRENADE_FIDGET,
	HANDGRENADE_PINPULL,
	HANDGRENADE_THROW1,	// toss
	HANDGRENADE_THROW2,	// medium
	HANDGRENADE_THROW3,	// hard
	HANDGRENADE_HOLSTER,
	HANDGRENADE_DRAW
};

class EHL_CLASS(EntityName=weapon_handgrenade) CHandGrenade : public CBaseWeapon
{
	EHL_GENERATED_BODY()

public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_HANDGRENADE;
		m_iDefaultAmmo = HANDGRENADE_DEFAULT_GIVE;
		SetWorldModelName("models/w_grenade.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 5; }
	bool GetWeaponInfo(WeaponInfo & p) override;

	void PrimaryAttack() override;
	bool Deploy() override;
	bool CanHolster() override;
	void Holster() override;
	void WeaponIdle() override;

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

	float m_flStartThrow = 0;
	float m_flReleaseThrow = 0;
};
