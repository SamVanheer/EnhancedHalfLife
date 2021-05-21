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

#include "CFuncMortarField.hpp"

LINK_ENTITY_TO_CLASS(func_mortar_field, CFuncMortarField);

void CFuncMortarField::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "m_iszXController"))
	{
		m_iszXController = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_iszYController"))
	{
		m_iszYController = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_flSpread"))
	{
		m_flSpread = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_fControl"))
	{
		//TODO: validate input
		m_fControl = static_cast<MortarControlType>(atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_iCount"))
	{
		m_iCount = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
}

void CFuncMortarField::Spawn()
{
	SetSolidType(Solid::Not);
	SetModel(STRING(pev->model));    // set size and link into world
	SetMovetype(Movetype::None);
	SetBits(pev->effects, EF_NODRAW);
	SetUse(&CFuncMortarField::FieldUse);
	Precache();
}

void CFuncMortarField::Precache()
{
	PRECACHE_SOUND("weapons/mortar.wav");
	PRECACHE_SOUND("weapons/mortarhit.wav");
	PRECACHE_MODEL("sprites/lgtning.spr");
}

void CFuncMortarField::FieldUse(const UseInfo& info)
{
	Vector vecStart
	{
		RANDOM_FLOAT(pev->mins.x, pev->maxs.x),
		RANDOM_FLOAT(pev->mins.y, pev->maxs.y),
		pev->maxs.z
	};

	switch (m_fControl)
	{
	case MortarControlType::Random:	// random
		break;
	case MortarControlType::Activator: // Trigger Activator
		if (auto pActivator = info.GetActivator(); pActivator != nullptr)
		{
			vecStart.x = pActivator->GetAbsOrigin().x;
			vecStart.y = pActivator->GetAbsOrigin().y;
		}
		break;
	case MortarControlType::Table: // table
	{
		if (!IsStringNull(m_iszXController))
		{
			CBaseEntity* pController = UTIL_FindEntityByTargetname(nullptr, STRING(m_iszXController));
			if (pController != nullptr)
			{
				vecStart.x = pev->mins.x + pController->pev->ideal_yaw * (pev->size.x);
			}
		}
		if (!IsStringNull(m_iszYController))
		{
			CBaseEntity* pController = UTIL_FindEntityByTargetname(nullptr, STRING(m_iszYController));
			if (pController != nullptr)
			{
				vecStart.y = pev->mins.y + pController->pev->ideal_yaw * (pev->size.y);
			}
		}
	}
	break;
	}

	const int pitch = RANDOM_LONG(95, 124);

	EmitSound(SoundChannel::Voice, "weapons/mortar.wav", VOL_NORM, ATTN_NONE, pitch);

	float t = 2.5;
	for (int i = 0; i < m_iCount; i++)
	{
		Vector vecSpot = vecStart;
		vecSpot.x += RANDOM_FLOAT(-m_flSpread, m_flSpread);
		vecSpot.y += RANDOM_FLOAT(-m_flSpread, m_flSpread);

		TraceResult tr;
		UTIL_TraceLine(vecSpot, vecSpot + vec3_down * WORLD_BOUNDARY, IgnoreMonsters::Yes, this, &tr);

		CBaseEntity* pMortar = Create("monster_mortar", tr.vecEndPos, vec3_origin, info.GetActivator());
		pMortar->pev->nextthink = gpGlobals->time + t;
		t += RANDOM_FLOAT(0.2, 0.5);

		if (i == 0)
			CSoundEnt::InsertSound(bits_SOUND_DANGER, tr.vecEndPos, 400, 0.3);
	}
}
