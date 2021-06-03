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

#include "weapons.hpp"
#include "CWeaponBox.generated.hpp"

/**
*	@brief a single entity that can store weapons and ammo.
*/
class EHL_CLASS("EntityName": "weaponbox") CWeaponBox : public CBaseEntity
{
	EHL_GENERATED_BODY()

	void Precache() override;
	void Spawn() override;

	/**
	*	@brief Touch: try to add my contents to the toucher if the toucher is a player.
	*/
	void Touch(CBaseEntity* pOther) override;
	void KeyValue(KeyValueData* pkvd) override;

	/**
	*	@brief is there anything in this box?
	*/
	bool IsEmpty();
	int  GiveAmmo(int iCount, const char* szName, int iMax, int* pIndex = nullptr);
	void SetObjectCollisionBox() override;

public:
	/**
	*	@brief the think function that removes the box from the world.
	*/
	void EXPORT Kill();

	/**
	*	@brief is a weapon of this type already packed in this box?
	*/
	bool HasWeapon(CBaseWeapon* pCheckWeapon);

	/**
	*	@brief PackWeapon: Add this weapon to the box
	*/
	bool PackWeapon(CBaseWeapon* pWeapon);

	bool PackAmmo(string_t iszName, int iCount);

	EHL_FIELD("Persisted": true)
	EHandle<CBaseWeapon> m_hPlayerWeapons[MAX_WEAPON_TYPES];// one slot for each 

	EHL_FIELD("Persisted": true)
	string_t m_rgiszAmmo[MAX_AMMO_TYPES]{};// ammo names

	EHL_FIELD("Persisted": true)
	int	m_rgAmmo[MAX_AMMO_TYPES]{};// ammo quantities

	EHL_FIELD("Persisted": true)
	int m_cAmmoTypes = 0;// how many ammo types packed into this box (if packed by a level designer)
};
