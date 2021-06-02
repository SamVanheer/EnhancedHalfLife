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

#include "CItemSoda.hpp"

void CItemSoda::Precache()
{
}

void CItemSoda::Spawn()
{
	Precache();
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::Toss);

	SetModel("models/can.mdl");
	SetSize(vec3_origin, vec3_origin);

	SetThink(&CItemSoda::CanThink);
	pev->nextthink = gpGlobals->time + 0.5;
}

void CItemSoda::CanThink()
{
	EmitSound(SoundChannel::Weapon, "weapons/g_bounce3.wav");

	SetSolidType(Solid::Trigger);
	SetSize(Vector(-8, -8, 0), Vector(8, 8, 8));
	SetThink(nullptr);
	SetTouch(&CItemSoda::CanTouch);
}

void CItemSoda::CanTouch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{
		return;
	}

	// spoit sound here

	pOther->GiveHealth(1, DMG_GENERIC);// a bit of health.

	if (auto owner = GetOwner(); !IsNullEnt(owner))
	{
		// tell the machine the can was taken
		owner->pev->frags = 0;
	}

	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->effects = EF_NODRAW;
	SetTouch(nullptr);
	SetThink(&CItemSoda::SUB_Remove);
	pev->nextthink = gpGlobals->time;
}
