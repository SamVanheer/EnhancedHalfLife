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

#include "CSatchelCharge.hpp"

void CSatchelCharge::Deactivate()
{
	SetSolidType(Solid::Not);
	UTIL_Remove(this);
}

void CSatchelCharge::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Bounce);
	SetSolidType(Solid::BBox);

	SetModel("models/w_satchel.mdl");
	//SetSize( Vector( -16, -16, -4), Vector(16, 16, 32));	// Old box -- size of headcrab monsters/players get blocked by this
	SetSize(Vector(-4, -4, -4), Vector(4, 4, 4));	// Uses point-sized, and can be stepped over
	SetAbsOrigin(GetAbsOrigin());

	SetTouch(&CSatchelCharge::SatchelSlide);
	SetUse(&CSatchelCharge::DetonateUse);
	SetThink(&CSatchelCharge::SatchelThink);
	pev->nextthink = gpGlobals->time + 0.1;

	pev->gravity = 0.5;
	pev->friction = 0.8;

	pev->dmg = gSkillData.plrDmgSatchel;
	// ResetSequenceInfo( );
	pev->sequence = 1;
}

void CSatchelCharge::SatchelSlide(CBaseEntity* pOther)
{
	// don't hit the guy that launched this grenade
	if (pOther == GetOwner())
		return;

	// pev->avelocity = Vector (300, 300, 300);
	pev->gravity = 1;// normal gravity now

	// HACKHACK - On ground isn't always set, so look for ground underneath
	TraceResult tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 10), IgnoreMonsters::Yes, this, &tr);

	if (tr.flFraction < 1.0)
	{
		// add a bit of static friction
		SetAbsVelocity(GetAbsVelocity() * 0.95);
		pev->avelocity = pev->avelocity * 0.9;
		// play sliding sound, volume based on velocity
	}
	if (!(pev->flags & FL_ONGROUND) && GetAbsVelocity().Length2D() > 10)
	{
		BounceSound();
	}
	StudioFrameAdvance();
}

void CSatchelCharge::SatchelThink()
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if (pev->waterlevel == WaterLevel::Head)
	{
		SetMovetype(Movetype::Fly);

		Vector velocity = GetAbsVelocity() * 0.8;
		velocity.z += 8;
		SetAbsVelocity(velocity);

		pev->avelocity = pev->avelocity * 0.9;
	}
	else if (pev->waterlevel == WaterLevel::Dry)
	{
		SetMovetype(Movetype::Bounce);
	}
	else
	{
		SetAbsVelocity(GetAbsVelocity() - Vector{0, 0, 8});
	}
}

void CSatchelCharge::Precache()
{
	PRECACHE_MODEL("models/grenade.mdl");
	PRECACHE_SOUND("weapons/g_bounce1.wav");
	PRECACHE_SOUND("weapons/g_bounce2.wav");
	PRECACHE_SOUND("weapons/g_bounce3.wav");
}

void CSatchelCharge::BounceSound()
{
	switch (RANDOM_LONG(0, 2))
	{
	case 0:	EmitSound(SoundChannel::Voice, "weapons/g_bounce1.wav"); break;
	case 1:	EmitSound(SoundChannel::Voice, "weapons/g_bounce2.wav"); break;
	case 2:	EmitSound(SoundChannel::Voice, "weapons/g_bounce3.wav"); break;
	}
}

void DeactivateSatchels(CBasePlayer* pOwner)
{
	CBaseEntity* pFind = nullptr;

	while ((pFind = UTIL_FindEntityByClassname(pFind, "monster_satchel")) != nullptr)
	{
		CSatchelCharge* pSatchel = (CSatchelCharge*)pFind;

		if (pSatchel->GetOwner() == pOwner)
		{
			pSatchel->Deactivate();
		}
	}
}
