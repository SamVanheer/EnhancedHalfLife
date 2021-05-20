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

#include "CLadder.hpp"

LINK_ENTITY_TO_CLASS(func_ladder, CLadder);

void CLadder::KeyValue(KeyValueData* pkvd)
{
	CBaseTrigger::KeyValue(pkvd);
}

void CLadder::Precache()
{
	// Do all of this in here because we need to 'convert' old saved games
	SetSolidType(Solid::Not);
	pev->skin = static_cast<int>(Contents::Ladder);
	if (CVAR_GET_FLOAT("showtriggers") == 0)
	{
		SetRenderMode(RenderMode::TransTexture);
		SetRenderAmount(0);
	}
	pev->effects &= ~EF_NODRAW;
}

void CLadder::Spawn()
{
	Precache();

	SetModel(STRING(pev->model));    // set size and link into world
	SetMovetype(Movetype::Push);
}
