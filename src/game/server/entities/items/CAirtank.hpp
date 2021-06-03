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

#include "CGrenade.hpp"
#include "CAirtank.generated.hpp"

class EHL_CLASS("EntityName": "item_airtank") CAirtank : public CGrenade
{
	EHL_GENERATED_BODY()

	void Spawn() override;
	void Precache() override;
	void EXPORT TankThink();
	void EXPORT TankTouch(CBaseEntity* pOther);
	int	 BloodColor() override { return DONT_BLEED; }
	void Killed(const KilledInfo& info) override;

	EHL_FIELD("Persisted": true)
	bool m_state = false;
};
