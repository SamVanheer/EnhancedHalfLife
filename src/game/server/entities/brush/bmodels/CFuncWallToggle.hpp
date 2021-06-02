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

#include "CFuncWall.hpp"
#include "CFuncWallToggle.generated.hpp"

constexpr int SF_WALL_START_OFF = 0x0001;

class EHL_CLASS(EntityName=func_wall_toggle) CFuncWallToggle : public CFuncWall
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override;
	void	TurnOff();
	void	TurnOn();
	bool	IsOn();
};
