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

#include "CFuncTankControls.hpp"

int	CFuncTankControls::ObjectCaps()
{
	return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_IMPULSE_USE;
}

void CFuncTankControls::Use(const UseInfo& info)
{ // pass the Use command onto the controls
	if (auto tank = m_hTank.Get(); tank)
		tank->Use(info);

	ASSERT(m_hTank != nullptr);	// if this fails,  most likely means save/restore hasn't worked properly
}

void CFuncTankControls::Think()
{
	CBaseEntity* pTarget = nullptr;

	while ((pTarget = UTIL_FindEntityByTargetname(pTarget, GetTarget())) != nullptr)
	{
		if (!strncmp(pTarget->GetClassname(), "func_tank", 9))
		{
			break;
		}
	}

	if (IsNullEnt(pTarget))
	{
		ALERT(at_console, "No tank %s\n", GetTarget());
		return;
	}

	m_hTank = (CFuncTank*)pTarget;
}

void CFuncTankControls::Spawn()
{
	SetSolidType(Solid::Trigger);
	SetMovetype(Movetype::None);
	pev->effects |= EF_NODRAW;
	SetModel(STRING(pev->model));

	SetSize(pev->mins, pev->maxs);
	SetAbsOrigin(GetAbsOrigin());

	//TODO: maybe use Activate() instead?
	pev->nextthink = gpGlobals->time + 0.3;	// After all the func_tank's have spawned

	CBaseEntity::Spawn();
}
