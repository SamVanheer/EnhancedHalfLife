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

constexpr int SF_SHAKE_EVERYONE = 0x0001;	//!< Don't check radius
// UNDONE: These don't work yet
constexpr int SF_SHAKE_DISRUPT = 0x0002;	//!< Disrupt controls
constexpr int SF_SHAKE_INAIR = 0x0004;		//!< Shake players in air

/**
*	@brief Screen shake
*	@details pev->scale is amplitude
*	pev->dmg_save is frequency
*	pev->dmg_take is duration
*	pev->dmg is radius
*	radius of 0 means all players
*	NOTE: UTIL_ScreenShake() will only shake players who are on the ground
*/
class EHL_CLASS() CShake : public CPointEntity
{
public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override;
	void	KeyValue(KeyValueData* pkvd) override;

	inline	float	Amplitude() { return pev->scale; }
	inline	float	Frequency() { return pev->dmg_save; }
	inline	float	Duration() { return pev->dmg_take; }
	inline	float	Radius() { return pev->dmg; }

	inline	void	SetAmplitude(float amplitude) { pev->scale = amplitude; }
	inline	void	SetFrequency(float frequency) { pev->dmg_save = frequency; }
	inline	void	SetDuration(float duration) { pev->dmg_take = duration; }
	inline	void	SetRadius(float radius) { pev->dmg = radius; }
private:
};
