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

#include "CXenHair.hpp"

void CXenHair::Spawn()
{
	Precache();
	SetModel("models/hair.mdl");
	SetSize(Vector(-4, -4, 0), Vector(4, 4, 32));
	pev->sequence = 0;

	if (!(pev->spawnflags & SF_HAIR_SYNC))
	{
		pev->frame = RANDOM_FLOAT(0, 255);
		pev->framerate = RANDOM_FLOAT(0.7, 1.4);
	}
	ResetSequenceInfo();

	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.1, 0.4);	// Load balance these a bit
}

void CXenHair::Think()
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.5;
}

void CXenHair::Precache()
{
	PRECACHE_MODEL("models/hair.mdl");
}
