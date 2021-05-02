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

/**
*	@brief halflife specific ai code
*/

#include "game.h"

constexpr int NUM_LATERAL_CHECKS = 13;  //!< how many checks are made on each side of a monster looking for lateral cover
constexpr int NUM_LATERAL_LOS_CHECKS = 6;  //!< how many checks are made on each side of a monster looking for lateral cover

//=========================================================
// 
// AI UTILITY FUNCTIONS
//
// !!!UNDONE - move CBaseMonster functions to monsters.cpp
//=========================================================

bool IsBoxVisible(CBaseEntity* pLooker, CBaseEntity* pTarget, Vector& vecTargetOrigin, float flSize)
{
	// don't look through water
	if ((pLooker->pev->waterlevel != WaterLevel::Head && pTarget->pev->waterlevel == WaterLevel::Head)
		|| (pLooker->pev->waterlevel == WaterLevel::Head && pTarget->pev->waterlevel == WaterLevel::Dry))
		return false;

	TraceResult tr;
	const Vector vecLookerOrigin = pLooker->pev->origin + pLooker->pev->view_ofs;//look through the monster's 'eyes'
	for (int i = 0; i < 5; i++)
	{
		Vector vecTarget = pTarget->pev->origin;
		vecTarget.x += RANDOM_FLOAT(pTarget->pev->mins.x + flSize, pTarget->pev->maxs.x - flSize);
		vecTarget.y += RANDOM_FLOAT(pTarget->pev->mins.y + flSize, pTarget->pev->maxs.y - flSize);
		vecTarget.z += RANDOM_FLOAT(pTarget->pev->mins.z + flSize, pTarget->pev->maxs.z - flSize);

		UTIL_TraceLine(vecLookerOrigin, vecTarget, IgnoreMonsters::Yes, IgnoreGlass::Yes, pLooker, &tr);

		if (tr.flFraction == 1.0)
		{
			vecTargetOrigin = vecTarget;
			return true;// line of sight is valid.
		}
	}
	return false;// Line of sight is not established
}

Vector CheckToss(CBaseEntity* pEntity, const Vector& vecSpot1, Vector vecSpot2, float flGravityAdj)
{
	if (vecSpot2.z - vecSpot1.z > 500)
	{
		// to high, fail
		return vec3_origin;
	}

	const float flGravity = g_psv_gravity->value * flGravityAdj;

	UTIL_MakeVectors(pEntity->pev->angles);

	// toss a little bit to the left or right, not right down on the enemy's bean (head). 
	vecSpot2 = vecSpot2 + gpGlobals->v_right * (RANDOM_FLOAT(-8, 8) + RANDOM_FLOAT(-16, 16));
	vecSpot2 = vecSpot2 + gpGlobals->v_forward * (RANDOM_FLOAT(-8, 8) + RANDOM_FLOAT(-16, 16));

	// calculate the midpoint and apex of the 'triangle'
	// UNDONE: normalize any Z position differences between spot1 and spot2 so that triangle is always RIGHT

	// How much time does it take to get there?

	// get a rough idea of how high it can be thrown
	// halfway point between Spot1 and Spot2
	Vector vecMidPoint = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5;
	TraceResult tr;
	UTIL_TraceLine(vecMidPoint, vecMidPoint + Vector(0, 0, 500), IgnoreMonsters::Yes, pEntity, &tr);
	vecMidPoint = tr.vecEndPos;
	// (subtract 15 so the grenade doesn't hit the ceiling)
	vecMidPoint.z -= 15;

	if (vecMidPoint.z < vecSpot1.z || vecMidPoint.z < vecSpot2.z)
	{
		// to not enough space, fail
		return vec3_origin;
	}

	// How high should the grenade travel to reach the apex
	const float distance1 = (vecMidPoint.z - vecSpot1.z);
	const float distance2 = (vecMidPoint.z - vecSpot2.z);

	// How long will it take for the grenade to travel this distance
	const float time1 = sqrt(distance1 / (0.5 * flGravity));
	const float time2 = sqrt(distance2 / (0.5 * flGravity));

	if (time1 < 0.1)
	{
		// too close
		return vec3_origin;
	}

	// how hard to throw sideways to get there in time.
	Vector vecGrenadeVel = (vecSpot2 - vecSpot1) / (time1 + time2);
	// how hard upwards to reach the apex at the right time.
	vecGrenadeVel.z = flGravity * time1;

	// find the highest point
	Vector vecApex = vecSpot1 + vecGrenadeVel * time1;
	vecApex.z = vecMidPoint.z;

	UTIL_TraceLine(vecSpot1, vecApex, IgnoreMonsters::No, pEntity, &tr);
	if (tr.flFraction != 1.0)
	{
		// fail!
		return vec3_origin;
	}

	// UNDONE: either ignore monsters or change it to not care if we hit our enemy
	UTIL_TraceLine(vecSpot2, vecApex, IgnoreMonsters::Yes, pEntity, &tr);
	if (tr.flFraction != 1.0)
	{
		// fail!
		return vec3_origin;
	}

	return vecGrenadeVel;
}

Vector CheckThrow(CBaseEntity* pEntity, const Vector& vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj)
{
	const float flGravity = g_psv_gravity->value * flGravityAdj;

	Vector vecGrenadeVel = vecSpot2 - vecSpot1;

	// throw at a constant time
	const float time = vecGrenadeVel.Length() / flSpeed;
	vecGrenadeVel = vecGrenadeVel * (1.0 / time);

	// adjust upward toss to compensate for gravity loss
	vecGrenadeVel.z += flGravity * time * 0.5;

	Vector vecApex = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5;
	vecApex.z += 0.5 * flGravity * (time * 0.5) * (time * 0.5);

	TraceResult tr;
	UTIL_TraceLine(vecSpot1, vecApex, IgnoreMonsters::No, pEntity, &tr);
	if (tr.flFraction != 1.0)
	{
		// fail!
		return vec3_origin;
	}

	UTIL_TraceLine(vecSpot2, vecApex, IgnoreMonsters::Yes, pEntity, &tr);
	if (tr.flFraction != 1.0)
	{
		// fail!
		return vec3_origin;
	}

	return vecGrenadeVel;
}
