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

#include "CGameCounterSet.hpp"

LINK_ENTITY_TO_CLASS(game_counter_set, CGameCounterSet);

void CGameCounterSet::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	SUB_UseTargets(info.GetActivator(), UseType::Set, pev->frags);

	if (RemoveOnFire())
	{
		UTIL_Remove(this);
	}
}