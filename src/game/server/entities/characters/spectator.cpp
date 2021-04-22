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

// YWB:  UNDONE

/**
*	@file
*
*	Spectator functions
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "spectator.h"

void CBaseSpectator::SpectatorConnect()
{
	pev->flags = FL_SPECTATOR;
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;

	m_pGoalEnt = nullptr;
}

void CBaseSpectator::SpectatorDisconnect()
{
}

void CBaseSpectator::SpectatorImpulseCommand()
{
	static edict_t* pGoal = nullptr;
	edict_t* pPreviousGoal;
	edict_t* pCurrentGoal;
	bool			bFound;

	switch (pev->impulse)
	{
	case 1:
		// teleport the spectator to the next spawn point
		// note that if the spectator is tracking, this doesn't do
		// much
		//TODO: dangerous!
		pPreviousGoal = pGoal;
		pCurrentGoal = pGoal;
		// Start at the current goal, skip the world, and stop if we looped
		//  back around

		bFound = false;
		while (true)
		{
			pCurrentGoal = FIND_ENTITY_BY_CLASSNAME(pCurrentGoal, "info_player_deathmatch");
			// Looped around, failure
			if (pCurrentGoal == pPreviousGoal)
			{
				ALERT(at_console, "Could not find a spawn spot.\n");
				break;
			}
			// Found a non-world entity, set success, otherwise, look for the next one.
			if (!FNullEnt(pCurrentGoal))
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)  // Didn't find a good spot.
			break;

		pGoal = pCurrentGoal;
		UTIL_SetOrigin(pev, pGoal->v.origin);
		pev->angles = pGoal->v.angles;
		pev->fixangle = FIXANGLE_NONE;
		break;
	default:
		ALERT(at_console, "Unknown spectator impulse\n");
		break;
	}

	pev->impulse = 0;
}

void  CBaseSpectator::SpectatorThink()
{
	if (!(pev->flags & FL_SPECTATOR))
	{
		pev->flags = FL_SPECTATOR;
	}

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;

	if (pev->impulse)
		SpectatorImpulseCommand();
}

void CBaseSpectator::Spawn()
{
	pev->flags = FL_SPECTATOR;
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;

	m_pGoalEnt = nullptr;
}
