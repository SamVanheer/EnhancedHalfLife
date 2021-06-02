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

#include "CSpiral.hpp"

void StreakSplash(const Vector& origin, const Vector& direction, int color, int count, int speed, int velocityRange)
{
	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, origin);
	WRITE_BYTE(TE_STREAK_SPLASH);
	WRITE_COORD(origin.x);		// origin
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_COORD(direction.x);	// direction
	WRITE_COORD(direction.y);
	WRITE_COORD(direction.z);
	WRITE_BYTE(color);	// Streak color 6
	WRITE_SHORT(count);	// count
	WRITE_SHORT(speed);
	WRITE_SHORT(velocityRange);	// Random velocity modifier
	MESSAGE_END();
}

void CSpiral::Spawn()
{
	SetMovetype(Movetype::None);
	pev->nextthink = gpGlobals->time;
	SetSolidType(Solid::Not);
	SetSize(vec3_origin, vec3_origin);
	pev->effects |= EF_NODRAW;
	SetAbsAngles(vec3_origin);
}

CSpiral* CSpiral::Create(const Vector& origin, float height, float radius, float duration)
{
	if (duration <= 0)
		return nullptr;

	CSpiral* pSpiral = static_cast<CSpiral*>(g_EntityList.Create("streak_spiral"));
	pSpiral->Spawn();
	pSpiral->pev->dmgtime = pSpiral->pev->nextthink;
	pSpiral->SetAbsOrigin(origin);
	pSpiral->pev->scale = radius;
	pSpiral->pev->dmg = height;
	pSpiral->pev->speed = duration;
	pSpiral->pev->health = 0;
	pSpiral->SetAbsAngles(vec3_origin);

	return pSpiral;
}

constexpr float SPIRAL_INTERVAL = 0.1; //025

void CSpiral::Think()
{
	float time = gpGlobals->time - pev->dmgtime;

	while (time > SPIRAL_INTERVAL)
	{
		Vector position = GetAbsOrigin();

		const float fraction = 1.0 / pev->speed;

		const float radius = (pev->scale * pev->health) * fraction;

		position.z += (pev->health * pev->dmg) * fraction;

		Vector angles = GetAbsAngles();
		angles.y = (pev->health * 360 * 8) * fraction;
		SetAbsAngles(angles);

		UTIL_MakeVectors(angles);
		position = position + gpGlobals->v_forward * radius;
		const Vector direction = (vec3_up + gpGlobals->v_forward).Normalize();

		StreakSplash(position, vec3_up, RANDOM_LONG(8, 11), 20, RANDOM_LONG(50, 150), 400);

		// Jeez, how many counters should this take ? :)
		pev->dmgtime += SPIRAL_INTERVAL;
		pev->health += SPIRAL_INTERVAL;
		time -= SPIRAL_INTERVAL;
	}

	pev->nextthink = gpGlobals->time;

	if (pev->health >= pev->speed)
		UTIL_Remove(this);
}
