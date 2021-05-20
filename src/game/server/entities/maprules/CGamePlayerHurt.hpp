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

#include "CRulePointEntity.hpp"

constexpr int SF_PKILL_FIREONCE = 0x0001;

/**
*	@brief Damages the player who fires it
*	@details Flag: Fire once
*/
class CGamePlayerHurt : public CRulePointEntity
{
public:
	void		Use(const UseInfo& info) override;
	inline bool RemoveOnFire() { return (pev->spawnflags & SF_PKILL_FIREONCE) != 0; }
};
