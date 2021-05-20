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

#include "CTriggerMonsterJump.hpp"

LINK_ENTITY_TO_CLASS(trigger_monsterjump, CTriggerMonsterJump);

void CTriggerMonsterJump::Spawn()
{
	SetMovedir(this);

	InitTrigger();

	pev->nextthink = 0;
	pev->speed = 200;
	m_flHeight = 150;

	if (!IsStringNull(pev->targetname))
	{// if targetted, spawn turned off
		SetSolidType(Solid::Not);
		SetAbsOrigin(GetAbsOrigin()); // Unlink from trigger list
		SetUse(&CTriggerMonsterJump::ToggleUse);
	}
}

void CTriggerMonsterJump::Think()
{
	SetSolidType(Solid::Not);// kill the trigger for now !!!UNDONE
	SetAbsOrigin(GetAbsOrigin()); // Unlink from trigger list
	SetThink(nullptr);
}

void CTriggerMonsterJump::Touch(CBaseEntity* pOther)
{
	if (!IsBitSet(pOther->pev->flags, FL_MONSTER))
	{// touched by a non-monster.
		return;
	}

	pOther->SetAbsOrigin(pOther->GetAbsOrigin() + vec3_up);

	if (IsBitSet(pOther->pev->flags, FL_ONGROUND))
	{// clear the onground so physics don't bitch
		pOther->pev->flags &= ~FL_ONGROUND;
	}

	// toss the monster!
	Vector velocity = pev->movedir * pev->speed;
	velocity.z += m_flHeight;
	pOther->SetAbsVelocity(velocity);
	pev->nextthink = gpGlobals->time;
}
