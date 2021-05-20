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

#include "CSmoker.hpp"

LINK_ENTITY_TO_CLASS(env_smoker, CSmoker);

void CSmoker::Spawn()
{
	SetMovetype(Movetype::None);
	pev->nextthink = gpGlobals->time;
	SetSolidType(Solid::Not);
	SetSize(vec3_origin, vec3_origin);
	pev->effects |= EF_NODRAW;
	SetAbsAngles(vec3_origin);
}

void CSmoker::Think()
{
	// lots of smoke
	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, GetAbsOrigin());
	WRITE_BYTE(TE_SMOKE);
	WRITE_COORD(GetAbsOrigin().x + RANDOM_FLOAT(-pev->dmg, pev->dmg));
	WRITE_COORD(GetAbsOrigin().y + RANDOM_FLOAT(-pev->dmg, pev->dmg));
	WRITE_COORD(GetAbsOrigin().z);
	WRITE_SHORT(g_sModelIndexSmoke);
	WRITE_BYTE(RANDOM_LONG(pev->scale, pev->scale * 1.1));
	WRITE_BYTE(RANDOM_LONG(8, 14)); // framerate
	MESSAGE_END();

	pev->health--;
	if (pev->health > 0)
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.1, 0.2);
	else
		UTIL_Remove(this);
}
