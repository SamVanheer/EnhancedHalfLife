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
#include "CLaserSpot.generated.hpp"

class EHL_CLASS() CLaserSpot : public CBaseEntity
{
	EHL_GENERATED_BODY()

	void Spawn() override;
	void Precache() override;

	int	ObjectCaps() override { return FCAP_DONT_SAVE; }

public:
	/**
	*	@brief make the laser sight invisible.
	*/
	void Suspend(float flSuspendTime);

	/**
	*	@brief bring a suspended laser sight back.
	*/
	void EXPORT Revive();

	static CLaserSpot* CreateSpot();
};
