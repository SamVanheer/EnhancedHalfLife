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
#include "CGauss.generated.hpp"

constexpr int GAUSS_PRIMARY_CHARGE_VOLUME = 256;	// how loud gauss is while charging
constexpr int GAUSS_PRIMARY_FIRE_VOLUME = 450;		// how loud gauss is when discharged

enum gauss_e
{
	GAUSS_IDLE = 0,
	GAUSS_IDLE2,
	GAUSS_FIDGET,
	GAUSS_SPINUP,
	GAUSS_SPIN,
	GAUSS_FIRE,
	GAUSS_FIRE2,
	GAUSS_HOLSTER,
	GAUSS_DRAW
};

class EHL_CLASS() CGauss : public CBaseWeapon
{
	EHL_GENERATED_BODY()

public:
	enum class AttackState
	{
		NotAttacking = 0,
		SpinUp,
		Charging,
		Aftershock,
	};

	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_GAUSS;
		m_iDefaultAmmo = GAUSS_DEFAULT_GIVE;
		SetWorldModelName("models/w_gauss.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 4; }
	bool GetWeaponInfo(WeaponInfo & p) override;
	bool AddToPlayer(CBasePlayer * pPlayer) override;

	bool Deploy() override;
	void Holster() override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;

	/**
	*	@details since all of this code has to run and then call Fire(),
	*	it was easier at this point to rip it out of weaponidle() and make its own function then to try to merge this into Fire(),
	*	which has some identical variable names
	*/
	void StartFire();
	void Fire(const Vector & vecOrigSrc, Vector vecDirShooting, float flDamage);
	float GetFullChargeTime();
	int m_iBalls = 0;
	int m_iGlow = 0;
	int m_iBeam = 0;
	int m_iSoundState = 0; // don't save this

	// was this weapon just fired primary or secondary?
	// we need to know so we can pick the right set of effects. 
	EHL_FIELD(Persisted)
		bool m_fPrimaryFire = false;

	EHL_FIELD(Persisted)
		AttackState m_fInAttack = AttackState::NotAttacking;

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
	void DecrementTimers() override;

	//TODO: needs save?
	float m_flStartCharge = 0;
	float m_flAmmoStartCharge = 0;
	float m_flPlayAftershock = 0;
	float m_flNextAmmoBurn = 0;//!< while charging, when to absorb another unit of player's ammo?

private:
	unsigned short m_usGaussFire = 0;
	unsigned short m_usGaussSpin = 0;
};
