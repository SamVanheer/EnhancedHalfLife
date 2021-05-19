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

#include "CFuncWall.hpp"

LINK_ENTITY_TO_CLASS(func_wall, CFuncWall);

void CFuncWall::Spawn()
{
	SetAbsAngles(vec3_origin);
	SetMovetype(Movetype::Push);  // so it doesn't get pushed by anything
	SetSolidType(Solid::BSP);
	SetModel(STRING(pev->model));

	// If it can't move/go away, it's really part of the world
	pev->flags |= FL_WORLDBRUSH;
}

void CFuncWall::Use(const UseInfo& info)
{
	if (ShouldToggle(info.GetUseType(), (int)(pev->frame)))
		pev->frame = 1 - pev->frame;
}
