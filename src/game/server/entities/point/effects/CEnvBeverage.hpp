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

#include "CBaseDelay.hpp"

/**
*	@brief Beverage Dispenser
*	@details overloaded pev->frags, is now a flag for whether or not a can is stuck in the dispenser.
*	overloaded pev->health, is now how many cans remain in the machine.
*/
class EHL_CLASS() CEnvBeverage : public CBaseDelay
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	Use(const UseInfo& info) override;
};
