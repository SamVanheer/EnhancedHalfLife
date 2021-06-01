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

#include "CFuncPlat.hpp"
#include "CPlatTrigger.hpp"

LINK_ENTITY_TO_CLASS(func_plat_trigger, CPlatTrigger);

void CPlatTrigger::SpawnInsideTrigger(CFuncPlat* pPlatform)
{
	m_hPlatform = pPlatform;
	// Create trigger entity, "point" it at the owning platform, give it a touch method
	SetSolidType(Solid::Trigger);
	SetMovetype(Movetype::None);
	SetAbsOrigin(pPlatform->GetAbsOrigin());

	// Establish the trigger field's size
	Vector vecTMin = pPlatform->pev->mins + Vector(25, 25, 0);
	Vector vecTMax = pPlatform->pev->maxs + Vector(25, 25, 8);
	vecTMin.z = vecTMax.z - (pPlatform->m_vecPosition1.z - pPlatform->m_vecPosition2.z + 8);
	if (pPlatform->pev->size.x <= 50)
	{
		vecTMin.x = (pPlatform->pev->mins.x + pPlatform->pev->maxs.x) / 2;
		vecTMax.x = vecTMin.x + 1;
	}
	if (pPlatform->pev->size.y <= 50)
	{
		vecTMin.y = (pPlatform->pev->mins.y + pPlatform->pev->maxs.y) / 2;
		vecTMax.y = vecTMin.y + 1;
	}
	SetSize(vecTMin, vecTMax);
}

void CPlatTrigger::Touch(CBaseEntity* pOther)
{
	//Platform was removed, remove trigger
	auto platform = m_hPlatform.Get();

	if (!platform || !platform->pev)
	{
		UTIL_Remove(this);
		return;
	}

	// Ignore touches by non-players
	if (!pOther->IsPlayer())
		return;

	// Ignore touches by corpses
	if (!pOther->IsAlive())
		return;

	// Make linked platform go up/down.
	if (platform->m_toggle_state == ToggleState::AtBottom)
		platform->GoUp();
	else if (platform->m_toggle_state == ToggleState::AtTop)
		platform->pev->nextthink = platform->pev->ltime + 1;// delay going down
}
