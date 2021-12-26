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

#include "CBaseItem.hpp"
#include "CBaseAmmo.generated.hpp"

class EHL_CLASS() CBaseAmmo : public CBaseItem
{
	EHL_GENERATED_BODY()

public:
	void OnConstruct() override
	{
		CBaseItem::OnConstruct();
		m_FallMode = ItemFallMode::Fall;
		m_bCanPickUpWhileFalling = true;
	}

	ItemType GetType() const override final { return ItemType::Ammo; }

	void KeyValue(KeyValueData * pkvd) override;

	void Precache() override;

protected:
	ItemApplyResult DefaultGiveAmmo(CBasePlayer * player, int amount, const char* ammoName, bool playSound);

	ItemApplyResult Apply(CBasePlayer * player) override;

protected:
	EHL_FIELD("Persisted": true)
		int m_iAmount = 0;

	EHL_FIELD("Persisted": true)
		string_t m_iszAmmoName;

	EHL_FIELD("Persisted": true, "Type": "SoundName")
		string_t m_iszPickupSound = MAKE_STRING("items/9mmclip1.wav");
};
