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

#include "CTriggerPush.hpp"

LINK_ENTITY_TO_CLASS(trigger_push, CTriggerPush);

void CTriggerPush::KeyValue(KeyValueData* pkvd)
{
	CBaseTrigger::KeyValue(pkvd);
}

void CTriggerPush::Spawn()
{
	if (Vector myAngles = GetAbsAngles(); myAngles == vec3_origin)
	{
		myAngles.y = 360;
		SetAbsAngles(myAngles);
	}
	InitTrigger();

	if (pev->speed == 0)
		pev->speed = 100;

	if (IsBitSet(pev->spawnflags, SF_TRIGGER_PUSH_START_OFF))// if flagged to Start Turned Off, make trigger nonsolid.
		SetSolidType(Solid::Not);

	SetUse(&CTriggerPush::ToggleUse);

	SetAbsOrigin(GetAbsOrigin());		// Link into the list
}

void CTriggerPush::Touch(CBaseEntity* pOther)
{
	// UNDONE: Is there a better way than health to detect things that have physics? (clients/monsters)
	switch (pOther->GetMovetype())
	{
	case Movetype::None:
	case Movetype::Push:
	case Movetype::Noclip:
	case Movetype::Follow:
		return;
	}

	if (pOther->GetSolidType() != Solid::Not && pOther->GetSolidType() != Solid::BSP)
	{
		// Instant trigger, just transfer velocity and remove
		if (IsBitSet(pev->spawnflags, SF_TRIGGER_PUSH_ONCE))
		{
			pOther->SetAbsVelocity(pOther->GetAbsVelocity() + (pev->speed * pev->movedir));
			if (pOther->GetAbsVelocity().z > 0)
				pOther->pev->flags &= ~FL_ONGROUND;
			UTIL_Remove(this);
		}
		else
		{	// Push field, transfer to base velocity
			Vector vecPush = (pev->speed * pev->movedir);
			if (pOther->pev->flags & FL_BASEVELOCITY)
				vecPush = vecPush + pOther->pev->basevelocity;

			pOther->pev->basevelocity = vecPush;

			pOther->pev->flags |= FL_BASEVELOCITY;
			//			ALERT( at_console, "Vel %f, base %f\n", pOther->GetAbsVelocity().z, pOther->pev->basevelocity.z );
		}
	}
}
