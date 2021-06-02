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

#include "CFuncConveyor.hpp"

void CFuncConveyor::Spawn()
{
	SetMovedir(this);
	CFuncWall::Spawn();

	if (!(pev->spawnflags & SF_CONVEYOR_VISUAL))
		SetBits(pev->flags, FL_CONVEYOR);

	// HACKHACK - This is to allow for some special effects
	if (pev->spawnflags & SF_CONVEYOR_NOTSOLID)
	{
		SetSolidType(Solid::Not);
		pev->skin = 0;		// Don't want the engine thinking we've got special contents on this brush
	}

	if (pev->speed == 0)
		pev->speed = 100;

	UpdateSpeed(pev->speed);
}

// HACKHACK -- This is ugly, but encode the speed in the rendercolor to avoid adding more data to the network stream
void CFuncConveyor::UpdateSpeed(float speed)
{
	// Encode it as an integer with 4 fractional bits
	const int speedCode = (int)(fabs(speed) * 16.0);

	SetRenderColor({static_cast<float>(speed < 0 ? 1 : 0), static_cast<float>(speedCode >> 8), static_cast<float>(speedCode & 0xFF)});
}

void CFuncConveyor::Use(const UseInfo& info)
{
	pev->speed = -pev->speed;
	UpdateSpeed(pev->speed);
}
