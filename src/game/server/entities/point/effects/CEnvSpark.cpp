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

#include "CEnvSpark.hpp"

void DoSpark(CBaseEntity* entity, const Vector& location)
{
	const Vector tmp = location + entity->pev->size * 0.5;
	UTIL_Sparks(tmp);

	const float flVolume = RANDOM_FLOAT(0.25, 0.75) * 0.4;//random volume range
	switch ((int)(RANDOM_FLOAT(0, 1) * 6))
	{
	case 0: entity->EmitSound(SoundChannel::Voice, "buttons/spark1.wav", flVolume); break;
	case 1: entity->EmitSound(SoundChannel::Voice, "buttons/spark2.wav", flVolume); break;
	case 2: entity->EmitSound(SoundChannel::Voice, "buttons/spark3.wav", flVolume); break;
	case 3: entity->EmitSound(SoundChannel::Voice, "buttons/spark4.wav", flVolume); break;
	case 4: entity->EmitSound(SoundChannel::Voice, "buttons/spark5.wav", flVolume); break;
	case 5: entity->EmitSound(SoundChannel::Voice, "buttons/spark6.wav", flVolume); break;
	}
}

LINK_ENTITY_TO_CLASS(env_spark, CEnvSpark);
LINK_ENTITY_TO_CLASS(env_debris, CEnvSpark);

void CEnvSpark::Spawn()
{
	SetThink(nullptr);
	SetUse(nullptr);

	if (IsBitSet(pev->spawnflags, SF_SPARK_TOGGLE)) // Use for on/off
	{
		if (IsBitSet(pev->spawnflags, SF_SPARK_START_ON)) // Start on
		{
			SetThink(&CEnvSpark::SparkThink);	// start sparking
			SetUse(&CEnvSpark::SparkStop);		// set up +USE to stop sparking
		}
		else
			SetUse(&CEnvSpark::SparkStart);
	}
	else
		SetThink(&CEnvSpark::SparkThink);

	pev->nextthink = gpGlobals->time + (0.1 + RANDOM_FLOAT(0, 1.5));

	if (m_flDelay <= 0)
		m_flDelay = 1.5;

	Precache();
}

void CEnvSpark::Precache()
{
	PRECACHE_SOUND("buttons/spark1.wav");
	PRECACHE_SOUND("buttons/spark2.wav");
	PRECACHE_SOUND("buttons/spark3.wav");
	PRECACHE_SOUND("buttons/spark4.wav");
	PRECACHE_SOUND("buttons/spark5.wav");
	PRECACHE_SOUND("buttons/spark6.wav");
}

void CEnvSpark::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "MaxDelay"))
	{
		m_flDelay = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "style") ||
		AreStringsEqual(pkvd->szKeyName, "height") ||
		AreStringsEqual(pkvd->szKeyName, "killtarget") ||
		AreStringsEqual(pkvd->szKeyName, "value1") ||
		AreStringsEqual(pkvd->szKeyName, "value2") ||
		AreStringsEqual(pkvd->szKeyName, "value3"))
		pkvd->fHandled = true;
	else
		CBaseEntity::KeyValue(pkvd);
}

void EXPORT CEnvSpark::SparkThink()
{
	pev->nextthink = gpGlobals->time + 0.1 + RANDOM_FLOAT(0, m_flDelay);
	DoSpark(this, GetAbsOrigin());
}

void EXPORT CEnvSpark::SparkStart(const UseInfo& info)
{
	SetUse(&CEnvSpark::SparkStop);
	SetThink(&CEnvSpark::SparkThink);
	pev->nextthink = gpGlobals->time + (0.1 + RANDOM_FLOAT(0, m_flDelay));
}

void EXPORT CEnvSpark::SparkStop(const UseInfo& info)
{
	SetUse(&CEnvSpark::SparkStart);
	SetThink(nullptr);
}
