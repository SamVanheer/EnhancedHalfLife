/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

#include "CEnvExplosion.hpp"

TYPEDESCRIPTION	CEnvExplosion::m_SaveData[] =
{
	DEFINE_FIELD(CEnvExplosion, m_iMagnitude, FIELD_INTEGER),
	DEFINE_FIELD(CEnvExplosion, m_spriteScale, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEnvExplosion, CBaseMonster);
LINK_ENTITY_TO_CLASS(env_explosion, CEnvExplosion);

void CEnvExplosion::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "iMagnitude"))
	{
		m_iMagnitude = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CEnvExplosion::Spawn()
{
	SetSolidType(Solid::Not);
	pev->effects = EF_NODRAW;

	SetMovetype(Movetype::None);
	/*
	if ( m_iMagnitude > 250 )
	{
		m_iMagnitude = 250;
	}
	*/

	float flSpriteScale = (m_iMagnitude - 50) * 0.6;

	/*
	if ( flSpriteScale > 50 )
	{
		flSpriteScale = 50;
	}
	*/
	if (flSpriteScale < 10)
	{
		flSpriteScale = 10;
	}

	m_spriteScale = (int)flSpriteScale;
}

void CEnvExplosion::Use(const UseInfo& info)
{
	pev->model = iStringNull;//invisible
	SetSolidType(Solid::Not);// intangible

	const Vector vecSpot = GetAbsOrigin() + Vector(0, 0, 8); // trace starts here!
	TraceResult tr;
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -40), IgnoreMonsters::Yes, this, &tr);

	// Pull out of the wall a bit
	if (tr.flFraction != 1.0)
	{
		SetAbsOrigin(tr.vecEndPos + (tr.vecPlaneNormal * (m_iMagnitude - 24) * 0.6));
	}
	else
	{
		SetAbsOrigin(GetAbsOrigin());
	}

	// draw decal
	if (!(pev->spawnflags & SF_ENVEXPLOSION_NODECAL))
	{
		if (RANDOM_FLOAT(0, 1) < 0.5)
		{
			UTIL_DecalTrace(&tr, DECAL_SCORCH1);
		}
		else
		{
			UTIL_DecalTrace(&tr, DECAL_SCORCH2);
		}
	}

	// draw fireball
	if (!(pev->spawnflags & SF_ENVEXPLOSION_NOFIREBALL))
	{
		MESSAGE_BEGIN(MessageDest::PAS, SVC_TEMPENTITY, GetAbsOrigin());
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(GetAbsOrigin().x);
		WRITE_COORD(GetAbsOrigin().y);
		WRITE_COORD(GetAbsOrigin().z);
		WRITE_SHORT(g_sModelIndexFireball);
		WRITE_BYTE((byte)m_spriteScale); // scale * 10
		WRITE_BYTE(15); // framerate
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();
	}
	else
	{
		MESSAGE_BEGIN(MessageDest::PAS, SVC_TEMPENTITY, GetAbsOrigin());
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(GetAbsOrigin().x);
		WRITE_COORD(GetAbsOrigin().y);
		WRITE_COORD(GetAbsOrigin().z);
		WRITE_SHORT(g_sModelIndexFireball);
		WRITE_BYTE(0); // no sprite
		WRITE_BYTE(15); // framerate
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();
	}

	// do damage
	if (!(pev->spawnflags & SF_ENVEXPLOSION_NODAMAGE))
	{
		RadiusDamage(this, this, m_iMagnitude, CLASS_NONE, DMG_BLAST);
	}

	SetThink(&CEnvExplosion::Smoke);
	pev->nextthink = gpGlobals->time + 0.3;

	// draw sparks
	if (!(pev->spawnflags & SF_ENVEXPLOSION_NOSPARKS))
	{
		const int sparkCount = RANDOM_LONG(0, 3);

		for (int i = 0; i < sparkCount; i++)
		{
			Create("spark_shower", GetAbsOrigin(), tr.vecPlaneNormal, nullptr);
		}
	}
}

void CEnvExplosion::Smoke()
{
	if (!(pev->spawnflags & SF_ENVEXPLOSION_NOSMOKE))
	{
		MESSAGE_BEGIN(MessageDest::PAS, SVC_TEMPENTITY, GetAbsOrigin());
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(GetAbsOrigin().x);
		WRITE_COORD(GetAbsOrigin().y);
		WRITE_COORD(GetAbsOrigin().z);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE((byte)m_spriteScale); // scale * 10
		WRITE_BYTE(12); // framerate
		MESSAGE_END();
	}

	if (!(pev->spawnflags & SF_ENVEXPLOSION_REPEATABLE))
	{
		UTIL_Remove(this);
	}
}

void UTIL_CreateExplosion(Vector center, const Vector& angles, CBaseEntity* owner, int magnitude, bool doDamage, float randomRange, float delay)
{
	if (randomRange != 0)
	{
		center.x += RANDOM_FLOAT(-randomRange, randomRange);
		center.y += RANDOM_FLOAT(-randomRange, randomRange);
	}

	CBaseEntity* pExplosion = CBaseEntity::Create("env_explosion", center, angles, owner);

	char buf[128];
	snprintf(buf, sizeof(buf), "%3d", magnitude);

	KeyValueData kvd{};
	kvd.szKeyName = "iMagnitude";
	kvd.szValue = buf;
	pExplosion->KeyValue(&kvd);

	if (doDamage)
	{
		pExplosion->pev->spawnflags |= SF_ENVEXPLOSION_NODAMAGE;
	}

	pExplosion->Spawn();

	if (delay > 0)
	{
		pExplosion->SetThink(&CBaseEntity::SUB_CallUseToggle);
		pExplosion->pev->nextthink = gpGlobals->time + delay;
	}
	else
	{
		pExplosion->Use({nullptr, nullptr, UseType::Toggle});
	}
}
