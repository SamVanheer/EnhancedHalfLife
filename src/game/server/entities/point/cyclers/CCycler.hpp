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
#include "CCycler.generated.hpp"

/**
*	@brief we should get rid of all the other cyclers and replace them with this.
*/
class EHL_CLASS("EntityName": "cycler") CCycler : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE); }

	/**
	*	@brief changes sequences when shot
	*/
	bool TakeDamage(const TakeDamageInfo& info) override;
	void Spawn() override;
	void Think() override;
	//void Pain( float flDamage );

	/**
	*	@brief starts a rotation trend
	*/
	void Use(const UseInfo& info) override;

	// Don't treat as a live target
	bool IsAlive() override { return false; }

	EHL_FIELD("Persisted": true)
	bool m_animate = false;
};
