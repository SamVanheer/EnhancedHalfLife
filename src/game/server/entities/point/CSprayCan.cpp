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

#include "CSprayCan.hpp"

LINK_ENTITY_TO_CLASS(spray_can, CSprayCan);

void CSprayCan::Spawn(CBaseEntity* pOwner)
{
	SetAbsOrigin(pOwner->GetAbsOrigin() + Vector(0, 0, 32));
	SetAbsAngles(pOwner->pev->v_angle);
	SetOwner(pOwner);
	pev->frame = 0;

	pev->nextthink = gpGlobals->time + 0.1;
	EmitSound(SoundChannel::Voice, "player/sprayer.wav");
}

void CSprayCan::Think()
{
	auto pPlayer = (CBasePlayer*)GetOwner();

	const int nFrames = pPlayer ? pPlayer->GetCustomDecalFrames() : -1;

	const int playernum = pPlayer ? pPlayer->entindex() : 0;

	// ALERT(at_console, "Spray by player %i, %i of %i\n", playernum, (int)(pev->frame + 1), nFrames);

	TraceResult	tr;
	UTIL_MakeVectors(GetAbsAngles());
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + gpGlobals->v_forward * 128, IgnoreMonsters::Yes, pPlayer, &tr);

	// No customization present.
	if (nFrames == -1)
	{
		UTIL_DecalTrace(&tr, DECAL_LAMBDA6);
		UTIL_Remove(this);
	}
	else
	{
		UTIL_PlayerDecalTrace(&tr, playernum, pev->frame, true);
		// Just painted last custom frame.
		if (pev->frame++ >= (nFrames - 1))
			UTIL_Remove(this);
	}

	pev->nextthink = gpGlobals->time + 0.1;
}
