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

constexpr int SF_GAMECOUNT_FIREONCE = 0x0001;
constexpr int SF_GAMECOUNT_RESET = 0x0002;

/**
*	@brief Counts events and fires target
*	@details Flag: Fire once
*	Flag: Reset on Fire
*/
class EHL_CLASS() CGameCounter : public CRulePointEntity
{
public:
	void		Spawn() override;
	void		Use(const UseInfo& info) override;
	inline bool RemoveOnFire() { return (pev->spawnflags & SF_GAMECOUNT_FIREONCE) != 0; }
	inline bool ResetOnFire() { return (pev->spawnflags & SF_GAMECOUNT_RESET) != 0; }

	inline void CountUp() { pev->frags++; }
	inline void CountDown() { pev->frags--; }
	inline void ResetCount() { pev->frags = pev->dmg; }
	inline int  CountValue() { return pev->frags; }
	inline int	LimitValue() { return pev->health; }

	inline bool HitLimit() { return CountValue() == LimitValue(); }

private:

	inline void SetCountValue(int value) { pev->frags = value; }
	inline void SetInitialValue(int value) { pev->dmg = value; }
};
