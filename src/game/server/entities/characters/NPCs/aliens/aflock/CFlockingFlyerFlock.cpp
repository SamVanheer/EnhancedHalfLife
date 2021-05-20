/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#include "CFlockingFlyer.hpp"
#include "CFlockingFlyerFlock.hpp"

TYPEDESCRIPTION	CFlockingFlyerFlock::m_SaveData[] =
{
	DEFINE_FIELD(CFlockingFlyerFlock, m_cFlockSize, FIELD_INTEGER),
	DEFINE_FIELD(CFlockingFlyerFlock, m_flFlockRadius, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CFlockingFlyerFlock, CBaseMonster);

LINK_ENTITY_TO_CLASS(monster_flyer_flock, CFlockingFlyerFlock);

void CFlockingFlyerFlock::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "iFlockSize"))
	{
		m_cFlockSize = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "flFlockRadius"))
	{
		m_flFlockRadius = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
}

void CFlockingFlyerFlock::Spawn()
{
	Precache();
	SpawnFlock();

	UTIL_RemoveNow(this);		// dump the spawn ent
}

void CFlockingFlyerFlock::Precache()
{
	//PRECACHE_MODEL("models/aflock.mdl");		
	PRECACHE_MODEL("models/boid.mdl");

	PrecacheFlockSounds();
}

void CFlockingFlyerFlock::PrecacheFlockSounds()
{
	PRECACHE_SOUND("boid/boid_alert1.wav");
	PRECACHE_SOUND("boid/boid_alert2.wav");

	PRECACHE_SOUND("boid/boid_idle1.wav");
	PRECACHE_SOUND("boid/boid_idle2.wav");
}

void CFlockingFlyerFlock::SpawnFlock()
{
	const float R = m_flFlockRadius;

	CFlockingFlyer* pLeader = nullptr;

	for (int iCount = 0; iCount < m_cFlockSize; iCount++)
	{
		CFlockingFlyer* pBoid = GetClassPtr((CFlockingFlyer*)nullptr);

		if (!pLeader)
		{
			// make this guy the leader.
			pLeader = pBoid;

			pLeader->m_hSquadLeader = pLeader;
			pLeader->m_hSquadNext = nullptr;
		}

		const Vector vecSpot{GetAbsOrigin() + Vector{RANDOM_FLOAT(-R, R), RANDOM_FLOAT(-R, R), RANDOM_FLOAT(0, 16)}};

		pBoid->SetAbsOrigin(vecSpot);
		pBoid->SetMovetype(Movetype::Fly);
		pBoid->SpawnCommonCode();
		pBoid->pev->flags &= ~FL_ONGROUND;
		pBoid->SetAbsVelocity(vec3_origin);
		pBoid->SetAbsAngles(GetAbsAngles());

		pBoid->pev->frame = 0;
		pBoid->pev->nextthink = gpGlobals->time + 0.2;
		pBoid->SetThink(&CFlockingFlyer::IdleThink);

		if (pBoid != pLeader)
		{
			pLeader->SquadAdd(pBoid);
		}
	}
}


