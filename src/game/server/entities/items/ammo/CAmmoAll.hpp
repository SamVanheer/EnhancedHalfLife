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

#include "CBaseAmmo.hpp"
#include "CAmmoAll.generated.hpp"

constexpr int AMMOALL_REFILLAMMO = -1;

/**
*	@brief Gives the player all ammo types
*/
class EHL_CLASS("EntityName": "ammo_all") CAmmoAll : public CBaseAmmo
{
	EHL_GENERATED_BODY()

public:
	void KeyValue(KeyValueData * pkvd) override;

protected:
	ItemApplyResult Apply(CBasePlayer * player) override;
};
