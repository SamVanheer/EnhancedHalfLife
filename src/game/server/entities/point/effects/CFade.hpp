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

#include "CPointEntity.hpp"
#include "CFade.generated.hpp"

constexpr int SF_FADE_IN = 0x0001;			//!< Fade in, not out
constexpr int SF_FADE_MODULATE = 0x0002;	//!< Modulate, don't blend
constexpr int SF_FADE_ONLYONE = 0x0004;

/**
*	@details pev->dmg_take is duration
*	pev->dmg_save is hold duration
*/
class EHL_CLASS() CFade : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override;
	void	KeyValue(KeyValueData* pkvd) override;

	inline	float	Duration() { return pev->dmg_take; }
	inline	float	HoldTime() { return pev->dmg_save; }

	inline	void	SetDuration(float duration) { pev->dmg_take = duration; }
	inline	void	SetHoldTime(float hold) { pev->dmg_save = hold; }
private:
};
