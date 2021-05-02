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
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"

class CAirtank : public CGrenade
{
	void Spawn() override;
	void Precache() override;
	void EXPORT TankThink();
	void EXPORT TankTouch(CBaseEntity* pOther);
	int	 BloodColor() override { return DONT_BLEED; }
	void Killed(const KilledInfo& info) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	bool m_state = false;
};

LINK_ENTITY_TO_CLASS(item_airtank, CAirtank);

TYPEDESCRIPTION	CAirtank::m_SaveData[] =
{
	DEFINE_FIELD(CAirtank, m_state, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CAirtank, CGrenade);

void CAirtank::Spawn()
{
	Precache();
	// motor
	pev->movetype = Movetype::Fly;
	pev->solid = Solid::BBox;

	SET_MODEL(ENT(pev), "models/w_oxygen.mdl");
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 36));
	UTIL_SetOrigin(pev, pev->origin);

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
	pev->owner = EdictOrNull(info.GetAttacker());

	// UNDONE: this should make a big bubble cloud, not an explosion

	Explode(pev->origin, vec3_down);
}

void CAirtank::TankThink()
{
	// Fire trigger
	m_state = true;
	SUB_UseTargets(this, USE_TOGGLE, 0);
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
	SUB_UseTargets(this, USE_TOGGLE, 1);
}
