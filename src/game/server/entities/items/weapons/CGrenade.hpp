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

#include "CBaseMonster.hpp"
#include "CGrenade.generated.hpp"

/**
*	@brief Contact Grenade / Timed grenade / Satchel Charge
*/
class EHL_CLASS() CGrenade : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;

	static CGrenade* ShootTimed(CBaseEntity * pOwner, const Vector & vecStart, const Vector & vecVelocity, float time);
	static CGrenade* ShootContact(CBaseEntity * pOwner, const Vector & vecStart, const Vector & vecVelocity);

	void Explode(TraceResult * pTrace, int bitsDamageType);
	void EXPORT Smoke();

	void EXPORT BounceTouch(CBaseEntity * pOther);
	void EXPORT SlideTouch(CBaseEntity * pOther);

	/**
	*	@brief Contact grenade, explode when it touches something
	*/
	void EXPORT ExplodeTouch(CBaseEntity * pOther);
	void EXPORT DangerSoundThink();

	/**
	*	@brief Timed grenade, this think is called when time runs out.
	*/
	void EXPORT PreDetonate();
	void EXPORT Detonate();
	void EXPORT DetonateUse(const UseInfo & info);
	void EXPORT TumbleThink();

	virtual void BounceSound();
	int	BloodColor() override { return DONT_BLEED; }
	void Killed(const KilledInfo & info) override;

	bool m_fRegisteredSound = false;//!< whether or not this grenade has issued its DANGER sound to the world sound list yet.
};
