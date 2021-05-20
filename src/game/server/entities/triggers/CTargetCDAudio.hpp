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

/**
*	@brief This plays a CD track when fired or when the player enters it's radius
*/
class EHL_CLASS() CTargetCDAudio : public CPointEntity
{
public:
	void			Spawn() override;
	void			KeyValue(KeyValueData * pkvd) override;

	void	Use(const UseInfo & info) override;
	void			Think() override;
	void			Play();
};
