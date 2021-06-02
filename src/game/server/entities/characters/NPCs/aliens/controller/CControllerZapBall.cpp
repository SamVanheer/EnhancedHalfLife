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

#include "CControllerZapBall.hpp"

void CControllerZapBall::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetModel("sprites/xspark4.spr");
	SetRenderMode(RenderMode::TransAdd);
	SetRenderColor({255, 255, 255});
	SetRenderAmount(255);
	pev->scale = 0.5;

	SetSize(vec3_origin, vec3_origin);
	SetAbsOrigin(GetAbsOrigin());

	SetThink(&CControllerZapBall::AnimateThink);
	SetTouch(&CControllerZapBall::ExplodeTouch);

	m_hOwner = InstanceOrWorld(pev->owner);
	pev->dmgtime = gpGlobals->time; // keep track of when ball spawned
	pev->nextthink = gpGlobals->time + 0.1;
}

void CControllerZapBall::Precache()
{
	PRECACHE_MODEL("sprites/xspark4.spr");
	// PRECACHE_SOUND("debris/zap4.wav");
	// PRECACHE_SOUND("weapons/electro4.wav");
}

void CControllerZapBall::AnimateThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	pev->frame = ((int)pev->frame + 1) % 11;

	if (gpGlobals->time - pev->dmgtime > 5 || GetAbsVelocity().Length() < 10)
	{
		SetTouch(nullptr);
		UTIL_Remove(this);
	}
}

void CControllerZapBall::ExplodeTouch(CBaseEntity* pOther)
{
	if (pOther->pev->takedamage)
	{
		TraceResult tr = UTIL_GetGlobalTrace();

		CBaseEntity* pOwner = m_hOwner ? m_hOwner : this;

		ClearMultiDamage();
		pOther->TraceAttack({pOwner, gSkillData.controllerDmgBall, GetAbsVelocity().Normalize(), tr, DMG_ENERGYBEAM});
		ApplyMultiDamage(pOwner, pOwner);

		UTIL_EmitAmbientSound(this, tr.vecEndPos, "weapons/electro4.wav", 0.3, ATTN_NORM, 0, RANDOM_LONG(90, 99));
	}

	UTIL_Remove(this);
}
