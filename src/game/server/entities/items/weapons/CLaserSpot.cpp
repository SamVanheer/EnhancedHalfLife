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

#include "CLaserSpot.hpp"

LINK_ENTITY_TO_CLASS(laser_spot, CLaserSpot);

CLaserSpot* CLaserSpot::CreateSpot()
{
	CLaserSpot* pSpot = GetClassPtr((CLaserSpot*)nullptr);
	pSpot->Spawn();

	pSpot->SetClassname("laser_spot");

	return pSpot;
}

void CLaserSpot::Spawn()
{
	Precache();
	SetMovetype(Movetype::None);
	SetSolidType(Solid::Not);

	SetRenderMode(RenderMode::Glow);
	SetRenderFX(RenderFX::NoDissipation);
	SetRenderAmount(255);

	SetModel("sprites/laserdot.spr");
	SetAbsOrigin(GetAbsOrigin());
}

void CLaserSpot::Suspend(float flSuspendTime)
{
	pev->effects |= EF_NODRAW;

	SetThink(&CLaserSpot::Revive);
	pev->nextthink = gpGlobals->time + flSuspendTime;
}

void CLaserSpot::Revive()
{
	pev->effects &= ~EF_NODRAW;

	SetThink(nullptr);
}

void CLaserSpot::Precache()
{
	PRECACHE_MODEL("sprites/laserdot.spr");
}
