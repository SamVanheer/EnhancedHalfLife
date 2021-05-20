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

#include "CFrictionModifier.hpp"

LINK_ENTITY_TO_CLASS(func_friction, CFrictionModifier);

void CFrictionModifier::Spawn()
{
	SetSolidType(Solid::Trigger);
	SetModel(STRING(pev->model));    // set size and link into world
	SetMovetype(Movetype::None);
	SetTouch(&CFrictionModifier::ChangeFriction);
}

void CFrictionModifier::ChangeFriction(CBaseEntity* pOther)
{
	if (pOther->GetMovetype() != Movetype::BounceMissile && pOther->GetMovetype() != Movetype::Bounce)
		pOther->pev->friction = m_frictionFraction;
}

void CFrictionModifier::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "modifier"))
	{
		m_frictionFraction = atof(pkvd->szValue) / 100.0;
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}
