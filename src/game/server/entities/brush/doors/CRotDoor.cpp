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

#include "CRotDoor.hpp"

LINK_ENTITY_TO_CLASS(func_door_rotating, CRotDoor);

void CRotDoor::Spawn()
{
	Precache();
	// set the axis of rotation
	CBaseToggle::AxisDir(this);

	// check for clockwise rotation
	if (IsBitSet(pev->spawnflags, SF_DOOR_ROTATE_BACKWARDS))
		pev->movedir = pev->movedir * -1;

	//m_flWait			= 2; who the hell did this? (sjb)
	m_vecAngle1 = GetAbsAngles();
	m_vecAngle2 = GetAbsAngles() + pev->movedir * m_flMoveDistance;

	ASSERTSZ(m_vecAngle1 != m_vecAngle2, "rotating door start/end positions are equal");

	if (IsBitSet(pev->spawnflags, SF_DOOR_PASSABLE))
		SetSolidType(Solid::Not);
	else
		SetSolidType(Solid::BSP);

	SetMovetype(Movetype::Push);
	SetAbsOrigin(GetAbsOrigin());
	SetModel(STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 100;

	// DOOR_START_OPEN is to allow an entity to be lighted in the closed position
	// but spawn in the open position
	if (IsBitSet(pev->spawnflags, SF_DOOR_START_OPEN))
	{	// swap pos1 and pos2, put door at pos2, invert movement direction
		//TODO: swapping incorrectly?
		SetAbsAngles(m_vecAngle2);
		const Vector vecSav = m_vecAngle1;
		m_vecAngle2 = m_vecAngle1;
		m_vecAngle1 = vecSav;
		pev->movedir = pev->movedir * -1;
	}

	m_toggle_state = ToggleState::AtBottom;

	if (IsBitSet(pev->spawnflags, SF_DOOR_USE_ONLY))
	{
		SetTouch(nullptr);
	}
	else // touchable button
		SetTouch(&CRotDoor::DoorTouch);
}
