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

#include "doors/CBaseDoor.hpp"
#include "CRotButton.hpp"

LINK_ENTITY_TO_CLASS(func_rot_button, CRotButton);

void CRotButton::Spawn()
{
	//----------------------------------------------------
	//determine sounds for buttons
	//a sound of 0 should not make a sound
	//----------------------------------------------------
	const char* pszSound = ButtonSound(m_sounds);
	PRECACHE_SOUND(pszSound);
	pev->noise = ALLOC_STRING(pszSound);

	// set the axis of rotation
	CBaseToggle::AxisDir(this);

	// check for clockwise rotation
	if (IsBitSet(pev->spawnflags, SF_DOOR_ROTATE_BACKWARDS))
		pev->movedir = pev->movedir * -1;

	SetMovetype(Movetype::Push);

	if (pev->spawnflags & SF_ROTBUTTON_NOTSOLID)
		SetSolidType(Solid::Not);
	else
		SetSolidType(Solid::BSP);

	SetModel(STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 40;

	if (m_flWait == 0)
		m_flWait = 1;

	if (pev->health > 0)
	{
		SetDamageMode(DamageMode::Yes);
	}

	m_toggle_state = ToggleState::AtBottom;
	m_vecAngle1 = GetAbsAngles();
	m_vecAngle2 = GetAbsAngles() + pev->movedir * m_flMoveDistance;
	ASSERTSZ(m_vecAngle1 != m_vecAngle2, "rotating button start/end positions are equal");

	m_fStayPushed = m_flWait == -1;
	m_fRotating = true;

	// if the button is flagged for USE button activation only, take away it's touch function and add a use function
	if (!IsBitSet(pev->spawnflags, SF_BUTTON_TOUCH_ONLY))
	{
		SetTouch(nullptr);
		SetUse(&CRotButton::ButtonUse);
	}
	else // touchable button
		SetTouch(&CRotButton::ButtonTouch);

	//SetTouch( ButtonTouch );
}
