/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#pragma once

#include "CBaseEntity.hpp"
#include "brush/CBreakable.hpp"
#include "CGib.generated.hpp"

/**
*	@brief A gib is a chunk of a body, or a piece of wood/metal/rocks/etc.
*/
class EHL_CLASS() CGib : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	/**
	*	@brief Throw a chunk
	*/
	void Spawn(const char* szGibModel);

	/**
	*	@brief Gib bounces on the ground or wall, sponges some blood down, too!
	*/
	void EXPORT BounceGibTouch(CBaseEntity * pOther);

	/**
	*	@brief Sticky gib puts blood on the wall and stays put.
	*/
	void EXPORT StickyGibTouch(CBaseEntity * pOther);

	/**
	*	@brief in order to emit their meaty scent from the proper location,
	*	gibs should wait until they stop bouncing to emit their scent. That's what this function does.
	*/
	void EXPORT WaitTillLand();
	void		LimitVelocity();

	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	static	void SpawnHeadGib(CBaseEntity * pVictim);
	static	void SpawnRandomGibs(CBaseEntity * pVictim, int cGibs, bool human);
	static  void SpawnStickyGibs(CBaseEntity * pVictim, const Vector & vecOrigin, int cGibs);

	int m_bloodColor = 0;
	int m_cBloodDecals = 0;
	Materials m_material = Materials::Glass;
	float m_lifeTime = 0;
};
