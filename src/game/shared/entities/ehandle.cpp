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

#include "extdll.hpp"
#include "util.hpp"
#include "cbase.hpp"
#include "ehandle.hpp"

CBaseEntity* BaseHandle::operator=(CBaseEntity* entity)
{
	Set(entity);
	return entity;
}

CBaseEntity* BaseHandle::Get() const
{
	if (!m_pent || m_pent->serialnumber != m_serialnumber)
	{
		return nullptr;
	}

	return CBaseEntity::InstanceOrNull(m_pent);
}

void BaseHandle::Set(CBaseEntity* entity)
{
	if (entity)
	{
		m_pent = entity->edict();
		if (m_pent)
			m_serialnumber = m_pent->serialnumber;
	}
	else
	{
		m_pent = nullptr;
		m_serialnumber = 0;
	}
}
