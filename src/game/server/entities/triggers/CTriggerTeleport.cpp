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

#include "CTriggerTeleport.hpp"

void CTriggerTeleport::Spawn()
{
	InitTrigger();

	SetTouch(&CTriggerTeleport::TeleportTouch);
}

void CTriggerTeleport::TeleportTouch(CBaseEntity* pOther)
{
	// Only teleport monsters or clients
	if (!IsBitSet(pOther->pev->flags, FL_CLIENT | FL_MONSTER))
		return;

	if (!UTIL_IsMasterTriggered(m_iszMaster, pOther))
		return;

	if (!(pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS))
	{// no monsters allowed!
		if (IsBitSet(pOther->pev->flags, FL_MONSTER))
		{
			return;
		}
	}

	if ((pev->spawnflags & SF_TRIGGER_NOCLIENTS))
	{// no clients allowed
		if (pOther->IsPlayer())
		{
			return;
		}
	}

	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, GetTarget());
	if (IsNullEnt(pTarget))
		return;

	Vector tmp = pTarget->GetAbsOrigin();

	if (pOther->IsPlayer())
	{
		tmp.z -= pOther->pev->mins.z;// make origin adjustments in case the teleportee is a player. (origin in center, not at feet)
	}

	tmp.z++;

	pOther->pev->flags &= ~FL_ONGROUND;

	pOther->SetAbsOrigin(tmp);

	pOther->SetAbsAngles(pTarget->GetAbsAngles());

	if (pOther->IsPlayer())
	{
		pOther->pev->v_angle = pTarget->GetAbsAngles();
	}

	pOther->pev->fixangle = FixAngleMode::Absolute;
	pOther->pev->basevelocity = vec3_origin;
	pOther->SetAbsVelocity(vec3_origin);
}
