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

#include "CTriggerVolume.hpp"

LINK_ENTITY_TO_CLASS(trigger_transition, CTriggerVolume);

void CTriggerVolume::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	SetModel(STRING(pev->model));    // set size and link into world
	pev->model = iStringNull;
	pev->modelindex = 0;
}
