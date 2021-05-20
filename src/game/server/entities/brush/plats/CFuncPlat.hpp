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
#include "CBasePlatTrain.hpp"

/**
*	@details Plats are always drawn in the extended position, so they will light correctly.
*	speed	default 150
*	If the plat has a targetname, it will start out disabled in the extended position until it is triggered,
*	when it will lower and become a normal plat.
*	If the "height" key is set, that will determine the amount the plat moves,
*	instead ofbeing implicitly determined by the model's height.
*/
class EHL_CLASS() CFuncPlat : public CBasePlatTrain
{
public:
	void Spawn() override;
	void Precache() override;
	void Setup();

	void Blocked(CBaseEntity* pOther) override;

	/**
	*	@brief Start bringing platform down.
	*/
	void EXPORT PlatUse(const UseInfo& info);

	void	EXPORT CallGoDown() { GoDown(); }
	void	EXPORT CallHitTop() { HitTop(); }
	void	EXPORT CallHitBottom() { HitBottom(); }

	/**
	*	@brief Platform is at bottom, now starts moving up
	*/
	virtual void GoUp();

	/**
	*	@brief Platform is at top, now starts moving down.
	*/
	virtual void GoDown();

	/**
	*	@brief Platform has hit top. Pauses, then starts back down again.
	*/
	virtual void HitTop();

	/**
	*	@brief Platform has hit bottom. Stops and waits forever.
	*/
	virtual void HitBottom();
};
