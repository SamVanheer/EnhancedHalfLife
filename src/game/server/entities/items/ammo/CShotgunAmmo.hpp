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
#include "CBaseWeapon.hpp"
#include "CShotgunAmmo.generated.hpp"

class EHL_CLASS() CShotgunAmmo : public CBaseAmmo
{
	EHL_GENERATED_BODY()

public:
	void OnConstruct() override
	{
		CBaseAmmo::OnConstruct();
		SetModelName("models/w_shotbox.mdl");
		m_iAmount = AMMO_BUCKSHOTBOX_GIVE;
		m_iszAmmoName = MAKE_STRING("buckshot");
	}
};
