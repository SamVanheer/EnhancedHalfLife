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

#include "CRuleEntity.hpp"

void CRuleEntity::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->effects = EF_NODRAW;
}

bool CRuleEntity::CanFireForActivator(CBaseEntity* pActivator)
{
	if (!IsStringNull(m_iszMaster))
	{
		return UTIL_IsMasterTriggered(m_iszMaster, pActivator);
	}

	return true;
}
