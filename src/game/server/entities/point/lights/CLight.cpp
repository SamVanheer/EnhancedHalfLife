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

#include "CLight.hpp"

LINK_ENTITY_TO_CLASS(light, CLight);

// shut up spawn functions for new spotlights
LINK_ENTITY_TO_CLASS(light_spot, CLight);

void CLight::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "style"))
	{
		m_iStyle = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "pitch"))
	{
		SetAbsAngles({static_cast<float>(atof(pkvd->szValue)), GetAbsAngles().y, GetAbsAngles().z});
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "pattern"))
	{
		m_iszPattern = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
	{
		CPointEntity::KeyValue(pkvd);
	}
}

void CLight::Spawn()
{
	if (IsStringNull(pev->targetname))
	{       // inert light
		UTIL_RemoveNow(this);
		return;
	}

	if (m_iStyle >= 32)
	{
		//		CHANGE_METHOD(ENT(pev), em_use, light_use);
		if (IsBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
			LIGHT_STYLE(m_iStyle, "a");
		else if (!IsStringNull(m_iszPattern))
			LIGHT_STYLE(m_iStyle, STRING(m_iszPattern));
		else
			LIGHT_STYLE(m_iStyle, "m");
	}
}

void CLight::Use(const UseInfo& info)
{
	if (m_iStyle >= 32)
	{
		if (!ShouldToggle(info.GetUseType(), !IsBitSet(pev->spawnflags, SF_LIGHT_START_OFF)))
			return;

		if (IsBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
		{
			if (!IsStringNull(m_iszPattern))
				LIGHT_STYLE(m_iStyle, STRING(m_iszPattern));
			else
				LIGHT_STYLE(m_iStyle, "m");
			ClearBits(pev->spawnflags, SF_LIGHT_START_OFF);
		}
		else
		{
			LIGHT_STYLE(m_iStyle, "a");
			SetBits(pev->spawnflags, SF_LIGHT_START_OFF);
		}
	}
}
