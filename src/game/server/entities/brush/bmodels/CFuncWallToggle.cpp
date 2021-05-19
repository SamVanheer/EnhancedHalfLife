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

#include "CFuncWallToggle.hpp"

LINK_ENTITY_TO_CLASS(func_wall_toggle, CFuncWallToggle);

void CFuncWallToggle::Spawn()
{
	CFuncWall::Spawn();
	if (pev->spawnflags & SF_WALL_START_OFF)
		TurnOff();
}

void CFuncWallToggle::TurnOff()
{
	SetSolidType(Solid::Not);
	pev->effects |= EF_NODRAW;
	SetAbsOrigin(GetAbsOrigin());
}

void CFuncWallToggle::TurnOn()
{
	SetSolidType(Solid::BSP);
	pev->effects &= ~EF_NODRAW;
	SetAbsOrigin(GetAbsOrigin());
}

bool CFuncWallToggle::IsOn()
{
	return GetSolidType() != Solid::Not;
}

void CFuncWallToggle::Use(const UseInfo& info)
{
	const bool status = IsOn();

	if (ShouldToggle(info.GetUseType(), status))
	{
		if (status)
			TurnOff();
		else
			TurnOn();
	}
}
