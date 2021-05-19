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

#include "CCrossbowBolt.hpp"

LINK_ENTITY_TO_CLASS(crossbow_bolt, CCrossbowBolt);

CCrossbowBolt* CCrossbowBolt::BoltCreate()
{
	// Create a new entity with CCrossbowBolt private data
	CCrossbowBolt* pBolt = GetClassPtr((CCrossbowBolt*)nullptr);
	pBolt->SetClassname("bolt");
	pBolt->Spawn();

	return pBolt;
}

void CCrossbowBolt::Spawn()
{
	Precache();
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	pev->gravity = 0.5;

	SetModel("models/crossbow_bolt.mdl");

	SetAbsOrigin(GetAbsOrigin());
	SetSize(vec3_origin, vec3_origin);

	SetTouch(&CCrossbowBolt::BoltTouch);
	SetThink(&CCrossbowBolt::BubbleThink);
	pev->nextthink = gpGlobals->time + 0.2;
}

void CCrossbowBolt::Precache()
{
	PRECACHE_MODEL("models/crossbow_bolt.mdl");
	PRECACHE_SOUND("weapons/xbow_hitbod1.wav");
	PRECACHE_SOUND("weapons/xbow_hitbod2.wav");
	PRECACHE_SOUND("weapons/xbow_fly1.wav");
	PRECACHE_SOUND("weapons/xbow_hit1.wav");
	PRECACHE_SOUND("fvox/beep.wav");
	m_iTrail = PRECACHE_MODEL("sprites/streak.spr");
}

int	CCrossbowBolt::Classify()
{
	return	CLASS_NONE;
}

void CCrossbowBolt::BoltTouch(CBaseEntity* pOther)
{
	SetTouch(nullptr);
	SetThink(nullptr);

	if (pOther->pev->takedamage)
	{
		TraceResult tr = UTIL_GetGlobalTrace();

		auto owner = GetOwner();

		ClearMultiDamage();

		if (pOther->IsPlayer())
		{
			pOther->TraceAttack({owner, gSkillData.plrDmgCrossbowClient, GetAbsVelocity().Normalize(), tr, DMG_NEVERGIB});
		}
		else
		{
			pOther->TraceAttack({owner, gSkillData.plrDmgCrossbowMonster, GetAbsVelocity().Normalize(), tr, DMG_BULLET | DMG_NEVERGIB});
		}

		ApplyMultiDamage(this, owner);

		SetAbsVelocity(vec3_origin);
		// play body "thwack" sound
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EmitSound(SoundChannel::Body, "weapons/xbow_hitbod1.wav"); break;
		case 1:
			EmitSound(SoundChannel::Body, "weapons/xbow_hitbod2.wav"); break;
		}

		if (!g_pGameRules->IsMultiplayer())
		{
			Killed({this, this, GibType::Never});
		}
	}
	else
	{
		EmitSound(SoundChannel::Body, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 98 + RANDOM_LONG(0, 7));

		SetThink(&CCrossbowBolt::SUB_Remove);
		pev->nextthink = gpGlobals->time;// this will get changed below if the bolt is allowed to stick in what it hit.

		if (pOther->ClassnameIs("worldspawn"))
		{
			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity().Normalize();
			SetAbsOrigin(GetAbsOrigin() - vecDir * 12);
			SetSolidType(Solid::Not);
			SetMovetype(Movetype::Fly);
			SetAbsVelocity(vec3_origin);
			pev->avelocity.z = 0;

			Vector myAngles = VectorAngles(vecDir);
			myAngles.z = RANDOM_LONG(0, 360);
			SetAbsAngles(myAngles);

			pev->nextthink = gpGlobals->time + 10.0;
		}

		if (UTIL_PointContents(GetAbsOrigin()) != Contents::Water)
		{
			UTIL_Sparks(GetAbsOrigin());
		}
	}

	if (g_pGameRules->IsMultiplayer())
	{
		SetThink(&CCrossbowBolt::ExplodeThink);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CCrossbowBolt::BubbleThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->waterlevel == WaterLevel::Dry)
		return;

	UTIL_BubbleTrail(GetAbsOrigin() - GetAbsVelocity() * 0.1, GetAbsOrigin(), 1);
}

void CCrossbowBolt::ExplodeThink()
{
	const Contents iContents = UTIL_PointContents(GetAbsOrigin());

	pev->dmg = 40;

	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, GetAbsOrigin());
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(GetAbsOrigin().x);
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
	WRITE_BYTE(10); // scale * 10
	WRITE_BYTE(15); // framerate
	WRITE_BYTE(TE_EXPLFLAG_NONE);
	MESSAGE_END();

	auto oldOwner = GetOwner();

	SetOwner(nullptr); // can't traceline attack owner if this is set

	::RadiusDamage(GetAbsOrigin(), this, oldOwner, pev->dmg, 128, CLASS_NONE, DMG_BLAST | DMG_ALWAYSGIB);

	UTIL_Remove(this);
}
