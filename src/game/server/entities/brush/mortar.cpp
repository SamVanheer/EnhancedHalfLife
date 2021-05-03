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

enum class MortarControlType
{
	Random,
	Activator,
	Table
};

/**
*	@brief the "LaBuznik" mortar device
*	Drop bombs from above
*/
class CFuncMortarField : public CBaseToggle
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;

	// Bmodels don't go across transitions
	int	ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	/**
	*	@brief If connected to a table, then use the table controllers, else hit where the trigger is.
	*/
	void EXPORT FieldUse(const UseInfo& info);

	string_t m_iszXController;
	string_t m_iszYController;
	float m_flSpread;
	float m_flDelay;
	int m_iCount;
	MortarControlType m_fControl;
};

LINK_ENTITY_TO_CLASS(func_mortar_field, CFuncMortarField);

TYPEDESCRIPTION	CFuncMortarField::m_SaveData[] =
{
	DEFINE_FIELD(CFuncMortarField, m_iszXController, FIELD_STRING),
	DEFINE_FIELD(CFuncMortarField, m_iszYController, FIELD_STRING),
	DEFINE_FIELD(CFuncMortarField, m_flSpread, FIELD_FLOAT),
	DEFINE_FIELD(CFuncMortarField, m_flDelay, FIELD_FLOAT),
	DEFINE_FIELD(CFuncMortarField, m_iCount, FIELD_INTEGER),
	DEFINE_FIELD(CFuncMortarField, m_fControl, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CFuncMortarField, CBaseToggle);

void CFuncMortarField::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "m_iszXController"))
	{
		m_iszXController = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_iszYController"))
	{
		m_iszYController = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_flSpread"))
	{
		m_flSpread = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_fControl"))
	{
		//TODO: validate input
		m_fControl = static_cast<MortarControlType>(atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_iCount"))
	{
		m_iCount = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
}

void CFuncMortarField::Spawn()
{
	SetSolidType(Solid::Not);
	SetModel(STRING(pev->model));    // set size and link into world
	SetMovetype(Movetype::None);
	SetBits(pev->effects, EF_NODRAW);
	SetUse(&CFuncMortarField::FieldUse);
	Precache();
}

void CFuncMortarField::Precache()
{
	PRECACHE_SOUND("weapons/mortar.wav");
	PRECACHE_SOUND("weapons/mortarhit.wav");
	PRECACHE_MODEL("sprites/lgtning.spr");
}

void CFuncMortarField::FieldUse(const UseInfo& info)
{
	Vector vecStart
	{
		RANDOM_FLOAT(pev->mins.x, pev->maxs.x),
		RANDOM_FLOAT(pev->mins.y, pev->maxs.y),
		pev->maxs.z
	};

	switch (m_fControl)
	{
	case MortarControlType::Random:	// random
		break;
	case MortarControlType::Activator: // Trigger Activator
		if (auto pActivator = info.GetActivator(); pActivator != nullptr)
		{
			vecStart.x = pActivator->GetAbsOrigin().x;
			vecStart.y = pActivator->GetAbsOrigin().y;
		}
		break;
	case MortarControlType::Table: // table
	{
		if (!IsStringNull(m_iszXController))
		{
			CBaseEntity* pController = UTIL_FindEntityByTargetname(nullptr, STRING(m_iszXController));
			if (pController != nullptr)
			{
				vecStart.x = pev->mins.x + pController->pev->ideal_yaw * (pev->size.x);
			}
		}
		if (!IsStringNull(m_iszYController))
		{
			CBaseEntity* pController = UTIL_FindEntityByTargetname(nullptr, STRING(m_iszYController));
			if (pController != nullptr)
			{
				vecStart.y = pev->mins.y + pController->pev->ideal_yaw * (pev->size.y);
			}
		}
	}
	break;
	}

	const int pitch = RANDOM_LONG(95, 124);

	EmitSound(SoundChannel::Voice, "weapons/mortar.wav", VOL_NORM, ATTN_NONE, pitch);

	float t = 2.5;
	for (int i = 0; i < m_iCount; i++)
	{
		Vector vecSpot = vecStart;
		vecSpot.x += RANDOM_FLOAT(-m_flSpread, m_flSpread);
		vecSpot.y += RANDOM_FLOAT(-m_flSpread, m_flSpread);

		TraceResult tr;
		UTIL_TraceLine(vecSpot, vecSpot + vec3_down * WORLD_BOUNDARY, IgnoreMonsters::Yes, this, &tr);

		CBaseEntity* pMortar = Create("monster_mortar", tr.vecEndPos, vec3_origin, info.GetActivator());
		pMortar->pev->nextthink = gpGlobals->time + t;
		t += RANDOM_FLOAT(0.2, 0.5);

		if (i == 0)
			CSoundEnt::InsertSound(bits_SOUND_DANGER, tr.vecEndPos, 400, 0.3);
	}
}

class CMortar : public CGrenade
{
public:
	void Spawn() override;
	void Precache() override;

	void EXPORT MortarExplode();

	int m_spriteTexture;
};

LINK_ENTITY_TO_CLASS(monster_mortar, CMortar);

void CMortar::Spawn()
{
	SetMovetype(Movetype::None);
	SetSolidType(Solid::Not);

	pev->dmg = 200;

	SetThink(&CMortar::MortarExplode);
	pev->nextthink = 0;

	Precache();
}

void CMortar::Precache()
{
	m_spriteTexture = PRECACHE_MODEL("sprites/lgtning.spr");
}

void CMortar::MortarExplode()
{
#if 1
	// mortar beam
	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(GetAbsOrigin().x);
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	WRITE_COORD(GetAbsOrigin().x);
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z + 1024);
	WRITE_SHORT(m_spriteTexture);
	WRITE_BYTE(0); // framerate
	WRITE_BYTE(0); // framerate
	WRITE_BYTE(1); // life
	WRITE_BYTE(40);  // width
	WRITE_BYTE(0);   // noise
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(160);   // r, g, b
	WRITE_BYTE(100);   // r, g, b
	WRITE_BYTE(128);	// brightness
	WRITE_BYTE(0);		// speed
	MESSAGE_END();
#endif

#if 0
	// blast circle
	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMTORUS);
	WRITE_COORD(GetAbsOrigin().x);
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z + 32);
	WRITE_COORD(GetAbsOrigin().x);
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z + 32 + pev->dmg * 2 / .2); // reach damage radius over .3 seconds
	WRITE_SHORT(m_spriteTexture);
	WRITE_BYTE(0); // startframe
	WRITE_BYTE(0); // framerate
	WRITE_BYTE(2); // life
	WRITE_BYTE(12);  // width
	WRITE_BYTE(0);   // noise
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(160);   // r, g, b
	WRITE_BYTE(100);   // r, g, b
	WRITE_BYTE(255);	// brightness
	WRITE_BYTE(0);		// speed
	MESSAGE_END();
#endif

	TraceResult tr;
	UTIL_TraceLine(GetAbsOrigin() + Vector(0, 0, 1024), GetAbsOrigin() - Vector(0, 0, 1024), IgnoreMonsters::No, this, &tr);

	Explode(&tr, DMG_BLAST | DMG_MORTAR);
	UTIL_ScreenShake(tr.vecEndPos, 25.0, 150.0, 1.0, 750);

#if 0
	const int pitch = RANDOM_LONG(95, 124);
	EmitSound(SoundChannel::Voice, "weapons/mortarhit.wav", VOL_NORM, 0.55, pitch);

	// ForceSound( SNDRADIUS_MP5, bits_SOUND_COMBAT );

	// ExplodeModel( GetAbsOrigin(), 400, g_sModelIndexShrapnel, 30 );

	RadiusDamage(this, GetOwner(), pev->dmg, CLASS_NONE, DMG_BLAST);

	/*
	if ( RANDOM_FLOAT ( 0 , 1 ) < 0.5 )
	{
		UTIL_DecalTrace( pTrace, DECAL_SCORCH1 );
	}
	else
	{
		UTIL_DecalTrace( pTrace, DECAL_SCORCH2 );
	}
	*/

	SetThink(&CMortar::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
#endif

}

#if 0
void CMortar::ShootTimed(EVARS* pevOwner, Vector vecStart, float time)
{
	CMortar* pMortar = GetClassPtr((CMortar*)nullptr);
	pMortar->Spawn();

	TraceResult tr;
	UTIL_TraceLine(vecStart, vecStart + vec3_down * WORLD_BOUNDARY, ignore_monsters, ENT(pMortar->pev), &tr);

	pMortar->pev->nextthink = gpGlobals->time + time;

	pMortar->SetAbsOrigin(tr.vecEndPos);
}
#endif
