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

#include "CXenTreeTrigger.hpp"

CXenTreeTrigger* CXenTreeTrigger::TriggerCreate(CBaseEntity* pOwner, const Vector& position)
{
	CXenTreeTrigger* pTrigger = static_cast<CXenTreeTrigger*>(g_EntityList.Create("xen_ttrigger"));
	pTrigger->SetAbsOrigin(position);
	pTrigger->SetSolidType(Solid::Trigger);
	pTrigger->SetMovetype(Movetype::None);
	pTrigger->SetOwner(pOwner);

	return pTrigger;
}

void CXenTreeTrigger::Touch(CBaseEntity* pOther)
{
	if (auto entity = GetOwner(); entity)
	{
		entity->Touch(pOther);
	}
}
