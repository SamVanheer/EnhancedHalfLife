/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#include "CTentacleMaw.hpp"

LINK_ENTITY_TO_CLASS(monster_tentaclemaw, CTentacleMaw);

void CTentacleMaw::Spawn()
{
	Precache();
	SetModel("models/maw.mdl");
	SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));

	SetSolidType(Solid::Not);
	SetMovetype(Movetype::Step);
	pev->effects = 0;
	pev->health = 75;
	pev->yaw_speed = 8;
	pev->sequence = 0;

	SetAbsAngles({90, GetAbsAngles().y, GetAbsAngles().z});

	// ResetSequenceInfo( );
}

void CTentacleMaw::Precache()
{
	PRECACHE_MODEL("models/maw.mdl");
}
