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
#include "CItemSoda.generated.hpp"

/**
*	@brief Soda can
*/
class EHL_CLASS() CItemSoda : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	Precache() override;
	void	EXPORT CanThink();
	void	EXPORT CanTouch(CBaseEntity* pOther);
};
