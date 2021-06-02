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

#include "CAirtank.hpp"

void CAirtank::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetModel("models/w_oxygen.mdl");
	SetSize(Vector(-16, -16, 0), Vector(16, 16, 36));
	SetAbsOrigin(GetAbsOrigin());

	SetTouch(&CAirtank::TankTouch);
	SetThink(&CAirtank::TankThink);

	pev->flags |= FL_MONSTER;
	SetDamageMode(DamageMode::Yes);
	pev->health = 20;
	pev->dmg = 50;
	m_state = true;
}

void CAirtank::Precache()
{
	PRECACHE_MODEL("models/w_oxygen.mdl");
	PRECACHE_SOUND("doors/aliendoor3.wav");
}

void CAirtank::Killed(const KilledInfo& info)
{
	SetOwner(info.GetAttacker());

	// UNDONE: this should make a big bubble cloud, not an explosion

	TraceResult tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -32), IgnoreMonsters::Yes, this, &tr);

	Explode(&tr, DMG_BLAST);
}

void CAirtank::TankThink()
{
	// Fire trigger
	m_state = true;
	SUB_UseTargets(this, UseType::Toggle, 0);
}

void CAirtank::TankTouch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
		return;

	if (!m_state)
	{
		// "no oxygen" sound
		EmitSound(SoundChannel::Body, "player/pl_swim2.wav");
		return;
	}

	// give player 12 more seconds of air
	pOther->pev->air_finished = gpGlobals->time + PLAYER_AIRTIME;

	// suit recharge sound
	EmitSound(SoundChannel::Voice, "doors/aliendoor3.wav");

	// recharge airtank in 30 seconds
	pev->nextthink = gpGlobals->time + 30;
	m_state = false;
	SUB_UseTargets(this, UseType::Toggle, 1);
}
