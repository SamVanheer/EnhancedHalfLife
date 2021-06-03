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
#include "CRpg.generated.hpp"

class CLaserSpot;

enum rpg_e
{
	RPG_IDLE = 0,
	RPG_FIDGET,
	RPG_RELOAD,		// to reload
	RPG_FIRE2,		// to empty
	RPG_HOLSTER1,	// loaded
	RPG_DRAW1,		// loaded
	RPG_HOLSTER2,	// unloaded
	RPG_DRAW_UL,	// unloaded
	RPG_IDLE_UL,	// unloaded idle
	RPG_FIDGET_UL,	// unloaded fidget
};

class EHL_CLASS("EntityName": "weapon_rpg") CRpg : public CBaseWeapon
{
	EHL_GENERATED_BODY()

public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_RPG;

		if (bIsMultiplayer())
		{
			// more default ammo in multiplay. 
			m_iDefaultAmmo = RPG_DEFAULT_GIVE * 2;
		}
		else
		{
			m_iDefaultAmmo = RPG_DEFAULT_GIVE;
		}

		SetWorldModelName("models/w_rpg.mdl");
	}

	void OnRemove() override;
	void Spawn() override;
	void Precache() override;
	void Reload() override;
	int WeaponSlot() override { return 4; }
	bool GetWeaponInfo(WeaponInfo & p) override;
	bool AddToPlayer(CBasePlayer * pPlayer) override;

	bool Deploy() override;
	bool CanHolster() override;
	void Holster() override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;

	void UpdateSpot();
	bool ShouldWeaponIdle() override { return true; }

	EHandle<CLaserSpot> m_hSpot;

	EHL_FIELD("Persisted": true)
	bool m_fSpotActive = false;

	EHL_FIELD("Persisted": true)
	int m_cActiveRockets = 0;// how many missiles in flight from this launcher right now?

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usRpg = 0;
};
