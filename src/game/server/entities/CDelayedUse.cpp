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

#include "CDelayedUse.hpp"

LINK_ENTITY_TO_CLASS(DelayedUse, CDelayedUse);

void CDelayedUse::DelayThink()
{
	CBaseEntity* pActivator = nullptr;

	if (auto owner = GetOwner(); owner)		// A player activated this on delay
	{
		pActivator = owner;
	}
	// The use type is cached (and stashed) in pev->button
	SUB_UseTargets(pActivator, (UseType)pev->button, 0);
	UTIL_RemoveNow(this);
}
