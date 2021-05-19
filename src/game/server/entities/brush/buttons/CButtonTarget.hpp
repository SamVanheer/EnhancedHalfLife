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

#include "CBaseEntity.hpp"

constexpr int SF_BTARGET_USE = 0x0001;
constexpr int SF_BTARGET_ON = 0x0002;

class CButtonTarget : public CBaseEntity
{
public:
	void Spawn() override;
	void Use(const UseInfo& info) override;
	bool TakeDamage(const TakeDamageInfo& info) override;
	int	ObjectCaps() override;
};
