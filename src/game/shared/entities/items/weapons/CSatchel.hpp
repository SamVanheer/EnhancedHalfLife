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
#include "CSatchel.generated.hpp"

enum satchel_e
{
	SATCHEL_IDLE1 = 0,
	SATCHEL_FIDGET1,
	SATCHEL_DRAW,
	SATCHEL_DROP
};

enum satchel_radio_e
{
	SATCHEL_RADIO_IDLE1 = 0,
	SATCHEL_RADIO_FIDGET1,
	SATCHEL_RADIO_DRAW,
	SATCHEL_RADIO_FIRE,
	SATCHEL_RADIO_HOLSTER
};

class EHL_CLASS() CSatchel : public CBaseWeapon
{
	EHL_GENERATED_BODY()

public:
	enum class ChargeState
	{
		NoSatchelsDeployed = 0,
		SatchelsDeployed,
		Reloading,
	};

	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_SATCHEL;
		m_iDefaultAmmo = SATCHEL_DEFAULT_GIVE;
		SetWorldModelName("models/w_satchel.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 5; }
	bool GetWeaponInfo(WeaponInfo & p) override;
	bool AddToPlayer(CBasePlayer * pPlayer) override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool AddDuplicate(CBaseWeapon * pOriginal) override;
	bool CanDeploy() override;
	bool Deploy() override;
	bool IsUseable() override;

	void Holster() override;
	void WeaponIdle() override;
	void Throw();

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
		ChargeState m_chargeReady = ChargeState::NoSatchelsDeployed;
};
