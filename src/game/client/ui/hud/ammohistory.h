/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

/**
*	@brief this is the max number of items in each bucket
*/
constexpr int MAX_WEAPON_POSITIONS = MAX_WEAPON_SLOTS;

class WeaponsResource
{
private:
	// Information about weapons & ammo
	WEAPON		rgWeapons[MAX_WEAPONS];	//!< Weapons Array

	// counts of weapons * ammo
	/**
	*	@brief The slots currently in use by weapons. The value is a pointer to the weapon; if it's nullptr, no weapon is there
	*/
	WEAPON* rgSlots[MAX_WEAPON_SLOTS + 1][MAX_WEAPON_POSITIONS + 1];

	/**
	*	@brief count of each ammo type
	*/
	int			riAmmo[MAX_AMMO_TYPES];

public:
	void Init()
	{
		memset(rgWeapons, 0, sizeof rgWeapons);
		Reset();
	}

	void Reset()
	{
		iOldWeaponBits = 0;
		memset(rgSlots, 0, sizeof rgSlots);
		memset(riAmmo, 0, sizeof riAmmo);
	}

	int			iOldWeaponBits;

	WEAPON* GetWeapon(int iId) { return &rgWeapons[iId]; }
	void AddWeapon(WEAPON* wp)
	{
		rgWeapons[wp->iId] = *wp;
		LoadWeaponSprites(&rgWeapons[wp->iId]);
	}

	void PickupWeapon(WEAPON* wp)
	{
		rgSlots[wp->iSlot][wp->iSlotPos] = wp;
	}

	void DropWeapon(WEAPON* wp)
	{
		rgSlots[wp->iSlot][wp->iSlotPos] = nullptr;
	}

	void DropAllWeapons()
	{
		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			if (rgWeapons[i].iId)
				DropWeapon(&rgWeapons[i]);
		}
	}

	WEAPON* GetWeaponSlot(int slot, int pos) { return rgSlots[slot][pos]; }

	void LoadWeaponSprites(WEAPON* wp);
	void LoadAllWeaponSprites();

	/**
	*	@brief Returns the first weapon for a given slot.
	*/
	WEAPON* GetFirstPos(int iSlot);

	WEAPON* GetNextActivePos(int iSlot, int iSlotPos);

	bool HasAmmo(WEAPON* p);

	AMMO GetAmmo(int iId) { return iId; }

	void SetAmmo(int iId, int iCount) { riAmmo[iId] = iCount; }

	int CountAmmo(int iId);

	/**
	*	@brief Helper function to return a Ammo pointer from id
	*/
	HSPRITE* GetAmmoPicFromWeapon(int iAmmoId, wrect_t& rect);
};

extern WeaponsResource gWR;


constexpr int MAX_HISTORY = 12;
enum {
	HISTSLOT_EMPTY,
	HISTSLOT_AMMO,
	HISTSLOT_WEAP,
	HISTSLOT_ITEM,
};

class HistoryResource
{
private:
	struct HIST_ITEM {
		int type;
		float DisplayTime;  // the time at which this item should be removed from the history
		int iCount;
		int iId;
	};

	HIST_ITEM rgAmmoHistory[MAX_HISTORY];

public:

	void Init()
	{
		Reset();
	}

	void Reset()
	{
		memset(rgAmmoHistory, 0, sizeof rgAmmoHistory);
	}

	int iHistoryGap;
	int iCurrentHistorySlot;

	void AddToHistory(int iType, int iId, int iCount = 0);
	void AddToHistory(int iType, const char* szName, int iCount = 0);

	void CheckClearHistory();

	/**
	*	@brief Draw Ammo pickup history
	*/
	bool DrawAmmoHistory(float flTime);
};

extern HistoryResource gHR;
