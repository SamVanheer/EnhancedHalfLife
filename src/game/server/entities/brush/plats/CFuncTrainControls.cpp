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

#include "CFuncTrackTrain.hpp"
#include "CFuncTrainControls.hpp"

LINK_ENTITY_TO_CLASS(func_traincontrols, CFuncTrainControls);

void CFuncTrainControls::Find()
{
	CBaseEntity* pTarget = nullptr;

	do
	{
		pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target));
	}
	while (!IsNullEnt(pTarget) && !pTarget->ClassnameIs("func_tracktrain"));

	if (IsNullEnt(pTarget))
	{
		ALERT(at_console, "No train %s\n", STRING(pev->target));
		return;
	}

	CFuncTrackTrain* ptrain = CFuncTrackTrain::Instance(pTarget);
	ptrain->SetControls(this);
	UTIL_Remove(this);
}

void CFuncTrainControls::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	SetModel(STRING(pev->model));

	SetSize(pev->mins, pev->maxs);
	SetAbsOrigin(GetAbsOrigin());

	SetThink(&CFuncTrainControls::Find);
	pev->nextthink = gpGlobals->time;
}
