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

enum crossbow_e
{
	CROSSBOW_IDLE1 = 0,	// full
	CROSSBOW_IDLE2,		// empty
	CROSSBOW_FIDGET1,	// full
	CROSSBOW_FIDGET2,	// empty
	CROSSBOW_FIRE1,		// full
	CROSSBOW_FIRE2,		// reload
	CROSSBOW_FIRE3,		// empty
	CROSSBOW_RELOAD,	// from empty
	CROSSBOW_DRAW1,		// full
	CROSSBOW_DRAW2,		// empty
	CROSSBOW_HOLSTER1,	// full
	CROSSBOW_HOLSTER2,	// empty
};

class EHL_CLASS() CCrossbow : public CBaseWeapon
{
public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_CROSSBOW;
		m_iDefaultAmmo = CROSSBOW_DEFAULT_GIVE;
		SetWorldModelName("models/w_crossbow.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 3; }
	bool GetWeaponInfo(WeaponInfo & p) override;

	void FireBolt();
	void FireSniperBolt();
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool AddToPlayer(CBasePlayer * pPlayer) override;
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
	unsigned short m_usCrossbow = 0;
	unsigned short m_usCrossbow2 = 0;
};
