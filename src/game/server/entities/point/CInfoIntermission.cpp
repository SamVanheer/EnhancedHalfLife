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

#include "CInfoIntermission.hpp"

LINK_ENTITY_TO_CLASS(info_intermission, CInfoIntermission);

void CInfoIntermission::Spawn()
{
	SetAbsOrigin(GetAbsOrigin());
	SetSolidType(Solid::Not);
	pev->effects = EF_NODRAW;
	pev->v_angle = vec3_origin;

	pev->nextthink = gpGlobals->time + 2;// let targets spawn!
}

void CInfoIntermission::Think()
{
	// find my target
	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));

	if (!IsNullEnt(pTarget))
	{
		pev->v_angle = VectorAngles((pTarget->GetAbsOrigin() - GetAbsOrigin()).Normalize());
		pev->v_angle.x = -pev->v_angle.x;
	}
}
