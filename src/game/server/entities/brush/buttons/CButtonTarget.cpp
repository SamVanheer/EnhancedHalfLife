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

#include "CButtonTarget.hpp"

LINK_ENTITY_TO_CLASS(button_target, CButtonTarget);

void CButtonTarget::Spawn()
{
	SetMovetype(Movetype::Push);
	SetSolidType(Solid::BSP);
	SetModel(STRING(pev->model));
	SetDamageMode(DamageMode::Yes);

	if (IsBitSet(pev->spawnflags, SF_BTARGET_ON))
		pev->frame = 1;
}

void CButtonTarget::Use(const UseInfo& info)
{
	if (!ShouldToggle(info.GetUseType(), (int)pev->frame))
		return;
	pev->frame = 1 - pev->frame;
	if (pev->frame)
		SUB_UseTargets(info.GetActivator(), UseType::On, 0);
	else
		SUB_UseTargets(info.GetActivator(), UseType::Off, 0);
}

int	CButtonTarget::ObjectCaps()
{
	const int caps = CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;

	if (IsBitSet(pev->spawnflags, SF_BTARGET_USE))
		return caps | FCAP_IMPULSE_USE;
	else
		return caps;
}

bool CButtonTarget::TakeDamage(const TakeDamageInfo& info)
{
	Use({info.GetAttacker(), this, UseType::Toggle});

	return true;
}
