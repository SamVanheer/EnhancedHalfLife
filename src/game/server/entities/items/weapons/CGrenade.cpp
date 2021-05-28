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

#include "CGrenade.hpp"

LINK_ENTITY_TO_CLASS(grenade, CGrenade);

// Grenades flagged with this will be triggered when the owner calls detonateSatchelCharges
constexpr int SF_DETONATE = 0x0001;

// UNDONE: temporary scorching for PreAlpha - find a less sleazy permenant solution.
void CGrenade::Explode(TraceResult* pTrace, int bitsDamageType)
{
	pev->model = iStringNull;//invisible
	SetSolidType(Solid::Not);// intangible

	SetDamageMode(DamageMode::No);

	// Pull out of the wall a bit
	if (pTrace->flFraction != 1.0)
	{
		SetAbsOrigin(pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24) * 0.6));
	}

	const Contents iContents = UTIL_PointContents(GetAbsOrigin());

	MESSAGE_BEGIN(MessageDest::PAS, SVC_TEMPENTITY, GetAbsOrigin());
	WRITE_BYTE(TE_EXPLOSION);		// This makes a dynamic light and the explosion sprites/sound
	WRITE_COORD(GetAbsOrigin().x);	// Send to PAS because of the sound
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	if (iContents != Contents::Water)
	{
		WRITE_SHORT(g_sModelIndexFireball);
	}
	else
	{
		WRITE_SHORT(g_sModelIndexWExplosion);
	}
	WRITE_BYTE((pev->dmg - 50) * .60); // scale * 10
	WRITE_BYTE(15); // framerate
	WRITE_BYTE(TE_EXPLFLAG_NONE);
	MESSAGE_END();

	CSoundEnt::InsertSound(bits_SOUND_COMBAT, GetAbsOrigin(), NORMAL_EXPLOSION_VOLUME, 3.0);

	auto oldOwner = GetOwner();

	SetOwner(nullptr); // can't traceline attack owner if this is set

	::RadiusDamage(GetAbsOrigin(), this, oldOwner, pev->dmg, pev->dmg * RadiusDamageMagnitudeMultiplier, CLASS_NONE, bitsDamageType);

	if (RANDOM_FLOAT(0, 1) < 0.5)
	{
		UTIL_DecalTrace(pTrace, DECAL_SCORCH1);
	}
	else
	{
		UTIL_DecalTrace(pTrace, DECAL_SCORCH2);
	}

	const float flRndSound = RANDOM_FLOAT(0, 1); // sound randomizer

	switch (RANDOM_LONG(0, 2))
	{
	case 0:	EmitSound(SoundChannel::Voice, "weapons/debris1.wav", 0.55);	break;
	case 1:	EmitSound(SoundChannel::Voice, "weapons/debris2.wav", 0.55);	break;
	case 2:	EmitSound(SoundChannel::Voice, "weapons/debris3.wav", 0.55);	break;
	}

	pev->effects |= EF_NODRAW;
	SetThink(&CGrenade::Smoke);
	SetAbsVelocity(vec3_origin);
	pev->nextthink = gpGlobals->time + 0.3;

	if (iContents != Contents::Water)
	{
		const int sparkCount = RANDOM_LONG(0, 3);
		for (int i = 0; i < sparkCount; i++)
			Create("spark_shower", GetAbsOrigin(), pTrace->vecPlaneNormal, nullptr);
	}
}

void CGrenade::Smoke()
{
	if (UTIL_PointContents(GetAbsOrigin()) == Contents::Water)
	{
		UTIL_Bubbles(GetAbsOrigin() - Vector(64, 64, 64), GetAbsOrigin() + Vector(64, 64, 64), 100);
	}
	else
	{
		MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, GetAbsOrigin());
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(GetAbsOrigin().x);
		WRITE_COORD(GetAbsOrigin().y);
		WRITE_COORD(GetAbsOrigin().z);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE((pev->dmg - 50) * 0.80); // scale * 10
		WRITE_BYTE(12); // framerate
		MESSAGE_END();
	}
	UTIL_Remove(this);
}

void CGrenade::Killed(const KilledInfo& info)
{
	Detonate();
}

void CGrenade::DetonateUse(const UseInfo& info)
{
	SetThink(&CGrenade::Detonate);
	pev->nextthink = gpGlobals->time;
}

void CGrenade::PreDetonate()
{
	CSoundEnt::InsertSound(bits_SOUND_DANGER, GetAbsOrigin(), 400, 0.3);

	SetThink(&CGrenade::Detonate);
	pev->nextthink = gpGlobals->time + 1;
}

void CGrenade::Detonate()
{
	const Vector vecSpot = GetAbsOrigin() + Vector(0, 0, 8); // trace starts here!
	TraceResult tr;
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), IgnoreMonsters::Yes, this, &tr);

	Explode(&tr, DMG_BLAST);
}

void CGrenade::ExplodeTouch(CBaseEntity* pOther)
{
	const Vector vecSpot = GetAbsOrigin() - GetAbsVelocity().Normalize() * 32; // trace starts here!
	TraceResult tr;
	UTIL_TraceLine(vecSpot, vecSpot + GetAbsVelocity().Normalize() * 64, IgnoreMonsters::Yes, this, &tr);

	Explode(&tr, DMG_BLAST);
}

void CGrenade::DangerSoundThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	CSoundEnt::InsertSound(bits_SOUND_DANGER, GetAbsOrigin() + GetAbsVelocity() * 0.5, GetAbsVelocity().Length(), 0.2);
	pev->nextthink = gpGlobals->time + 0.2;

	if (pev->waterlevel != WaterLevel::Dry)
	{
		SetAbsVelocity(GetAbsVelocity() * 0.5);
	}
}

void CGrenade::BounceTouch(CBaseEntity* pOther)
{
	// don't hit the guy that launched this grenade
	if (pOther == GetOwner())
		return;

	// only do damage if we're moving fairly fast
	if (m_flNextAttack < gpGlobals->time && GetAbsVelocity().Length() > 100)
	{
		if (auto pOwner = GetOwner(); pOwner)
		{
			TraceResult tr = UTIL_GetGlobalTrace();
			ClearMultiDamage();
			pOther->TraceAttack({pOwner, 1, gpGlobals->v_forward, tr, DMG_CLUB});
			ApplyMultiDamage(this, pOwner);
		}
		m_flNextAttack = gpGlobals->time + 1.0; // debounce
	}

	// pev->avelocity = Vector (300, 300, 300);

	// this is my heuristic for modulating the grenade velocity because grenades dropped purely vertical
	// or thrown very far tend to slow down too quickly for me to always catch just by testing velocity. 
	// trimming the Z velocity a bit seems to help quite a bit.
	Vector vecTestVelocity = GetAbsVelocity();
	vecTestVelocity.z *= 0.45;

	if (!m_fRegisteredSound && vecTestVelocity.Length() <= 60)
	{
		//ALERT( at_console, "Grenade Registered!: %f\n", vecTestVelocity.Length() );

		// grenade is moving really slow. It's probably very close to where it will ultimately stop moving. 
		// go ahead and emit the danger sound.

		// register a radius louder than the explosion, so we make sure everyone gets out of the way
		CSoundEnt::InsertSound(bits_SOUND_DANGER, GetAbsOrigin(), pev->dmg / 0.4, 0.3);
		m_fRegisteredSound = true;
	}

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		SetAbsVelocity(GetAbsVelocity() * 0.8);

		pev->sequence = RANDOM_LONG(1, 1);
	}
	else
	{
		// play bounce sound
		BounceSound();
	}
	pev->framerate = GetAbsVelocity().Length() / 200.0;
	if (pev->framerate > 1.0)
		pev->framerate = 1;
	else if (pev->framerate < 0.5)
		pev->framerate = 0;
}

void CGrenade::SlideTouch(CBaseEntity* pOther)
{
	// don't hit the guy that launched this grenade
	if (pOther == GetOwner())
		return;

	// pev->avelocity = Vector (300, 300, 300);

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		SetAbsVelocity(GetAbsVelocity() * 0.95);

		if (GetAbsVelocity().x != 0 || GetAbsVelocity().y != 0)
		{
			// maintain sliding sound
		}
	}
	else
	{
		BounceSound();
	}
}

void CGrenade::BounceSound()
{
	switch (RANDOM_LONG(0, 2))
	{
	case 0:	EmitSound(SoundChannel::Voice, "weapons/grenade_hit1.wav", 0.25); break;
	case 1:	EmitSound(SoundChannel::Voice, "weapons/grenade_hit2.wav", 0.25); break;
	case 2:	EmitSound(SoundChannel::Voice, "weapons/grenade_hit3.wav", 0.25); break;
	}
}

void CGrenade::TumbleThink()
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->dmgtime - 1 < gpGlobals->time)
	{
		CSoundEnt::InsertSound(bits_SOUND_DANGER, GetAbsOrigin() + GetAbsVelocity() * (pev->dmgtime - gpGlobals->time), 400, 0.1);
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		SetThink(&CGrenade::Detonate);
	}
	if (pev->waterlevel != WaterLevel::Dry)
	{
		SetAbsVelocity(GetAbsVelocity() * 0.5);
		pev->framerate = 0.2;
	}
}

void CGrenade::Spawn()
{
	SetMovetype(Movetype::Bounce);
	SetClassname("grenade");

	SetSolidType(Solid::BBox);

	SetModel("models/grenade.mdl");
	SetSize(vec3_origin, vec3_origin);

	pev->dmg = 100;
	m_fRegisteredSound = false;
}

CGrenade* CGrenade::ShootContact(CBaseEntity* pOwner, const Vector& vecStart, const Vector& vecVelocity)
{
	CGrenade* pGrenade = GetClassPtr((CGrenade*)nullptr);
	pGrenade->Spawn();
	// contact grenades arc lower
	pGrenade->pev->gravity = 0.5;// lower gravity since grenade is aerodynamic and engine doesn't know it.
	pGrenade->SetAbsOrigin(vecStart);
	pGrenade->SetAbsVelocity(vecVelocity);
	pGrenade->SetAbsAngles(VectorAngles(pGrenade->GetAbsVelocity()));
	pGrenade->SetOwner(pOwner);

	// make monsters afaid of it while in the air
	pGrenade->SetThink(&CGrenade::DangerSoundThink);
	pGrenade->pev->nextthink = gpGlobals->time;

	// Tumble in air
	pGrenade->pev->avelocity.x = RANDOM_FLOAT(-100, -500);

	// Explode on contact
	pGrenade->SetTouch(&CGrenade::ExplodeTouch);

	pGrenade->pev->dmg = gSkillData.plrDmgM203Grenade;

	return pGrenade;
}

CGrenade* CGrenade::ShootTimed(CBaseEntity* pOwner, const Vector& vecStart, const Vector& vecVelocity, float time)
{
	CGrenade* pGrenade = GetClassPtr((CGrenade*)nullptr);
	pGrenade->Spawn();
	pGrenade->SetAbsOrigin(vecStart);
	pGrenade->SetAbsVelocity(vecVelocity);
	pGrenade->SetAbsAngles(VectorAngles(pGrenade->GetAbsVelocity()));
	pGrenade->SetOwner(pOwner);

	pGrenade->SetTouch(&CGrenade::BounceTouch);	// Bounce if touched

	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink(&CGrenade::TumbleThink);
	pGrenade->pev->nextthink = gpGlobals->time + 0.1;
	if (time < 0.1)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->SetAbsVelocity(vec3_origin);
	}

	pGrenade->pev->sequence = RANDOM_LONG(3, 6);
	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pGrenade->pev->gravity = 0.5;
	pGrenade->pev->friction = 0.8;

	pGrenade->SetModel("models/w_grenade.mdl");
	pGrenade->pev->dmg = 100;

	return pGrenade;
}
