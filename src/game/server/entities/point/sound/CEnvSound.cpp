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

#include "CEnvSound.hpp"

LINK_ENTITY_TO_CLASS(env_sound, CEnvSound);

void CEnvSound::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	if (AreStringsEqual(pkvd->szKeyName, "roomtype"))
	{
		m_flRoomtype = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
}

/**
*	@brief returns true if the given sound entity (pev) is in range and can see the given player entity (pevTarget)
*/
bool IsEnvSoundInRange(CEnvSound* pSound, CBaseEntity* pTarget, float& flRange)
{
	const Vector vecSpot1 = pSound->GetAbsOrigin() + pSound->pev->view_ofs;
	const Vector vecSpot2 = pTarget->GetAbsOrigin() + pTarget->pev->view_ofs;
	TraceResult tr;

	UTIL_TraceLine(vecSpot1, vecSpot2, IgnoreMonsters::Yes, pSound, &tr);

	// check if line of sight crosses water boundary, or is blocked

	if ((tr.fInOpen && tr.fInWater) || tr.flFraction != 1)
		return false;

	// calc range from sound entity to player

	const Vector vecRange = tr.vecEndPos - vecSpot1;
	const float flCandidateRange = vecRange.Length();

	if (pSound->m_flRadius < flCandidateRange)
		return false;

	flRange = flCandidateRange;

	return true;
}

// CONSIDER: if player in water state, autoset roomtype to 14,15 or 16. 

void CEnvSound::Think()
{
	const float SlowThinkInterval = 0.75f;
	const float FastThinkInterval = 0.25f;

	// get pointer to client if visible; UTIL_FindClientInPVS will
	// cycle through visible clients on consecutive calls.

	auto pPlayer = static_cast<CBasePlayer*>(UTIL_FindClientInPVS(this));

	if (IsNullEnt(pPlayer))
	{
		// no player in pvs of sound entity, slow it down
		pev->nextthink = gpGlobals->time + SlowThinkInterval;
		return;
	}

	float flRange;

	// check to see if this is the sound entity that is 
	// currently affecting this player

	if (CBaseEntity* lastSound = pPlayer->m_hSndLast; lastSound && lastSound == this)
	{
		// this is the entity currently affecting player, check
		// for validity

		if (pPlayer->m_flSndRoomtype != 0 && pPlayer->m_flSndRange != 0) {

			// we're looking at a valid sound entity affecting
			// player, make sure it's still valid, update range

			if (IsEnvSoundInRange(this, pPlayer, flRange)) {
				pPlayer->m_flSndRange = flRange;
				pev->nextthink = gpGlobals->time + FastThinkInterval;
				return;
			}
			else {

				// current sound entity affecting player is no longer valid,
				// flag this state by clearing room_type and range.
				// NOTE: we do not actually change the player's room_type
				// NOTE: until we have a new valid room_type to change it to.

				pPlayer->m_flSndRange = 0;
				pPlayer->m_flSndRoomtype = 0;
			}
		}
		else {
			// entity is affecting player but is out of range,
			// wait passively for another entity to usurp it...
		}

		pev->nextthink = gpGlobals->time + SlowThinkInterval;
		return;
	}

	// if we got this far, we're looking at an entity that is contending
	// for current player sound. the closest entity to player wins.

	if (IsEnvSoundInRange(this, pPlayer, flRange))
	{
		if (flRange < pPlayer->m_flSndRange || pPlayer->m_flSndRange == 0)
		{
			// new entity is closer to player, so it wins.
			pPlayer->m_hSndLast = this;
			pPlayer->m_flSndRoomtype = m_flRoomtype;
			pPlayer->m_flSndRange = flRange;

			// send room_type command to player's server.
			// this should be a rare event - once per change of room_type
			// only!

			//CLIENT_COMMAND(pentPlayer, "room_type %f", m_flRoomtype);

			MESSAGE_BEGIN(MessageDest::One, SVC_ROOMTYPE, pPlayer);		// use the magic #1 for "one client"
			WRITE_SHORT((short)m_flRoomtype);					// sequence number
			MESSAGE_END();

			// crank up nextthink rate for new active sound entity
			// by falling through to think_fast...
		}
		// player is not closer to the contending sound entity,
		// just fall through to think_fast. this effectively
		// cranks up the think_rate of entities near the player.
	}

	// player is in pvs of sound entity, but either not visible or
	// not in range. do nothing, fall through to think_fast...

	pev->nextthink = gpGlobals->time + FastThinkInterval;
}

void CEnvSound::Spawn()
{
	// spread think times
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.0, 0.5);
}
