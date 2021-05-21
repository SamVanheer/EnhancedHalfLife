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

#include "CLightning.hpp"
#include "customentity.h"

LINK_ENTITY_TO_CLASS(env_lightning, CLightning);
LINK_ENTITY_TO_CLASS(env_beam, CLightning);

void CLightning::Spawn()
{
	if (IsStringNull(m_iszSpriteName))
	{
		SetThink(&CLightning::SUB_Remove);
		return;
	}
	SetSolidType(Solid::Not);							// Remove model & collisions
	Precache();

	pev->dmgtime = gpGlobals->time;

	if (ServerSide())
	{
		SetThink(nullptr);
		if (pev->dmg > 0)
		{
			SetThink(&CLightning::DamageThink);
			pev->nextthink = gpGlobals->time + 0.1;
		}
		if (!IsStringNull(pev->targetname))
		{
			if (!(pev->spawnflags & SF_BEAM_STARTON))
			{
				pev->effects = EF_NODRAW;
				m_active = false;
				pev->nextthink = 0;
			}
			else
				m_active = true;

			SetUse(&CLightning::ToggleUse);
		}
	}
	else
	{
		m_active = false;
		if (!IsStringNull(pev->targetname))
		{
			SetUse(&CLightning::StrikeUse);
		}
		if (IsStringNull(pev->targetname) || IsBitSet(pev->spawnflags, SF_BEAM_STARTON))
		{
			SetThink(&CLightning::StrikeThink);
			pev->nextthink = gpGlobals->time + 1.0;
		}
	}
}

void CLightning::Precache()
{
	m_spriteTexture = PRECACHE_MODEL(STRING(m_iszSpriteName));
	CBeam::Precache();
}

void CLightning::Activate()
{
	if (ServerSide())
		BeamUpdateVars();
}

void CLightning::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "LightningStart"))
	{
		m_iszStartEntity = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "LightningEnd"))
	{
		m_iszEndEntity = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "life"))
	{
		m_life = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "BoltWidth"))
	{
		m_boltWidth = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "NoiseAmplitude"))
	{
		m_noiseAmplitude = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "TextureScroll"))
	{
		m_speed = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "StrikeTime"))
	{
		m_restrike = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "texture"))
	{
		m_iszSpriteName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "framestart"))
	{
		m_frameStart = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "Radius"))
	{
		m_radius = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBeam::KeyValue(pkvd);
}

void CLightning::ToggleUse(const UseInfo& info)
{
	if (!ShouldToggle(info.GetUseType(), m_active))
		return;
	if (m_active)
	{
		m_active = false;
		pev->effects |= EF_NODRAW;
		pev->nextthink = 0;
	}
	else
	{
		m_active = true;
		pev->effects &= ~EF_NODRAW;
		DoSparks(GetStartPos(), GetEndPos());
		if (pev->dmg > 0)
		{
			pev->nextthink = gpGlobals->time;
			pev->dmgtime = gpGlobals->time;
		}
	}
}

void CLightning::StrikeUse(const UseInfo& info)
{
	if (!ShouldToggle(info.GetUseType(), m_active))
		return;

	if (m_active)
	{
		m_active = false;
		SetThink(nullptr);
	}
	else
	{
		SetThink(&CLightning::StrikeThink);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	if (!IsBitSet(pev->spawnflags, SF_BEAM_TOGGLE))
		SetUse(nullptr);
}

bool IsPointEntity(CBaseEntity* pEnt)
{
	if (!pEnt)
	{
		return false;
	}

	if (!pEnt->pev->modelindex)
		return true;
	if (pEnt->ClassnameIs("info_target") || pEnt->ClassnameIs("info_landmark") || pEnt->ClassnameIs("path_corner"))
		return true;

	return false;
}

void CLightning::StrikeThink()
{
	if (m_life != 0)
	{
		if (pev->spawnflags & SF_BEAM_RANDOM)
			pev->nextthink = gpGlobals->time + m_life + RANDOM_FLOAT(0, m_restrike);
		else
			pev->nextthink = gpGlobals->time + m_life + m_restrike;
	}
	m_active = true;

	if (IsStringNull(m_iszEndEntity))
	{
		if (IsStringNull(m_iszStartEntity))
		{
			RandomArea();
		}
		else
		{
			CBaseEntity* pStart = RandomTargetname(STRING(m_iszStartEntity));
			if (pStart != nullptr)
				RandomPoint(pStart->GetAbsOrigin());
			else
				ALERT(at_console, "env_beam: unknown entity \"%s\"\n", STRING(m_iszStartEntity));
		}
		return;
	}

	CBaseEntity* pStart = RandomTargetname(STRING(m_iszStartEntity));
	CBaseEntity* pEnd = RandomTargetname(STRING(m_iszEndEntity));

	if (pStart != nullptr && pEnd != nullptr)
	{
		if (IsPointEntity(pStart) || IsPointEntity(pEnd))
		{
			if (pev->spawnflags & SF_BEAM_RING)
			{
				// don't work
				return;
			}
		}

		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		if (IsPointEntity(pStart) || IsPointEntity(pEnd))
		{
			if (!IsPointEntity(pEnd))	// One point entity must be in pEnd
			{
				CBaseEntity* pTemp;
				pTemp = pStart;
				pStart = pEnd;
				pEnd = pTemp;
			}
			if (!IsPointEntity(pStart))	// One sided
			{
				WRITE_BYTE(TE_BEAMENTPOINT);
				WRITE_SHORT(pStart->entindex());
				WRITE_COORD(pEnd->GetAbsOrigin().x);
				WRITE_COORD(pEnd->GetAbsOrigin().y);
				WRITE_COORD(pEnd->GetAbsOrigin().z);
			}
			else
			{
				WRITE_BYTE(TE_BEAMPOINTS);
				WRITE_COORD(pStart->GetAbsOrigin().x);
				WRITE_COORD(pStart->GetAbsOrigin().y);
				WRITE_COORD(pStart->GetAbsOrigin().z);
				WRITE_COORD(pEnd->GetAbsOrigin().x);
				WRITE_COORD(pEnd->GetAbsOrigin().y);
				WRITE_COORD(pEnd->GetAbsOrigin().z);
			}
		}
		else
		{
			if (pev->spawnflags & SF_BEAM_RING)
				WRITE_BYTE(TE_BEAMRING);
			else
				WRITE_BYTE(TE_BEAMENTS);
			WRITE_SHORT(pStart->entindex());
			WRITE_SHORT(pEnd->entindex());
		}

		WRITE_SHORT(m_spriteTexture);
		WRITE_BYTE(m_frameStart); // framestart
		WRITE_BYTE((int)pev->framerate); // framerate
		WRITE_BYTE((int)(m_life * 10.0)); // life
		WRITE_BYTE(m_boltWidth);  // width
		WRITE_BYTE(m_noiseAmplitude);   // noise
		WRITE_BYTE((int)GetRenderColor().x);   // r, g, b
		WRITE_BYTE((int)GetRenderColor().y);   // r, g, b
		WRITE_BYTE((int)GetRenderColor().z);   // r, g, b
		WRITE_BYTE(GetRenderAmount());	// brightness
		WRITE_BYTE(m_speed);		// speed
		MESSAGE_END();
		DoSparks(pStart->GetAbsOrigin(), pEnd->GetAbsOrigin());
		if (pev->dmg > 0)
		{
			TraceResult tr;
			UTIL_TraceLine(pStart->GetAbsOrigin(), pEnd->GetAbsOrigin(), IgnoreMonsters::No, nullptr, &tr);
			BeamDamageInstant(&tr, pev->dmg);
		}
	}
}

void CBeam::BeamDamage(TraceResult* ptr)
{
	RelinkBeam();
	if (ptr->flFraction != 1.0 && ptr->pHit != nullptr)
	{
		CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);
		if (pHit)
		{
			ClearMultiDamage();
			pHit->TraceAttack({this, pev->dmg * (gpGlobals->time - pev->dmgtime), (ptr->vecEndPos - GetAbsOrigin()).Normalize(), *ptr, DMG_ENERGYBEAM});
			ApplyMultiDamage(this, this);
			if (pev->spawnflags & SF_BEAM_DECALS)
			{
				if (pHit->IsBSPModel())
					UTIL_DecalTrace(ptr, DECAL_BIGSHOT1 + RANDOM_LONG(0, 4));
			}
		}
	}
	pev->dmgtime = gpGlobals->time;
}

void CLightning::DamageThink()
{
	pev->nextthink = gpGlobals->time + 0.1;
	TraceResult tr;
	UTIL_TraceLine(GetStartPos(), GetEndPos(), IgnoreMonsters::No, nullptr, &tr);
	BeamDamage(&tr);
}

void CLightning::Zap(const Vector& vecSrc, const Vector& vecDest)
{
#if 1
	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(vecSrc.x);
	WRITE_COORD(vecSrc.y);
	WRITE_COORD(vecSrc.z);
	WRITE_COORD(vecDest.x);
	WRITE_COORD(vecDest.y);
	WRITE_COORD(vecDest.z);
	WRITE_SHORT(m_spriteTexture);
	WRITE_BYTE(m_frameStart); // framestart
	WRITE_BYTE((int)pev->framerate); // framerate
	WRITE_BYTE((int)(m_life * 10.0)); // life
	WRITE_BYTE(m_boltWidth);  // width
	WRITE_BYTE(m_noiseAmplitude);   // noise
	WRITE_BYTE((int)GetRenderColor().x);   // r, g, b
	WRITE_BYTE((int)GetRenderColor().y);   // r, g, b
	WRITE_BYTE((int)GetRenderColor().z);   // r, g, b
	WRITE_BYTE(GetRenderAmount());	// brightness
	WRITE_BYTE(m_speed);		// speed
	MESSAGE_END();
#else
	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_LIGHTNING);
	WRITE_COORD(vecSrc.x);
	WRITE_COORD(vecSrc.y);
	WRITE_COORD(vecSrc.z);
	WRITE_COORD(vecDest.x);
	WRITE_COORD(vecDest.y);
	WRITE_COORD(vecDest.z);
	WRITE_BYTE(10);
	WRITE_BYTE(50);
	WRITE_BYTE(40);
	WRITE_SHORT(m_spriteTexture);
	MESSAGE_END();
#endif
	DoSparks(vecSrc, vecDest);
}

void CLightning::RandomArea()
{
	for (int iLoops = 0; iLoops < 10; iLoops++)
	{
		const Vector vecSrc = GetAbsOrigin();

		Vector vecDir1 = Vector(RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0));
		vecDir1 = vecDir1.Normalize();
		TraceResult		tr1;
		UTIL_TraceLine(vecSrc, vecSrc + vecDir1 * m_radius, IgnoreMonsters::Yes, this, &tr1);

		if (tr1.flFraction == 1.0)
			continue;

		Vector vecDir2;
		do {
			vecDir2 = Vector(RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0));
		}
		while (DotProduct(vecDir1, vecDir2) > 0);
		vecDir2 = vecDir2.Normalize();
		TraceResult		tr2;
		UTIL_TraceLine(vecSrc, vecSrc + vecDir2 * m_radius, IgnoreMonsters::Yes, this, &tr2);

		if (tr2.flFraction == 1.0)
			continue;

		if ((tr1.vecEndPos - tr2.vecEndPos).Length() < m_radius * 0.1)
			continue;

		UTIL_TraceLine(tr1.vecEndPos, tr2.vecEndPos, IgnoreMonsters::Yes, this, &tr2);

		if (tr2.flFraction != 1.0)
			continue;

		Zap(tr1.vecEndPos, tr2.vecEndPos);

		break;
	}
}

void CLightning::RandomPoint(const Vector& vecSrc)
{
	for (int iLoops = 0; iLoops < 10; iLoops++)
	{
		Vector vecDir1 = Vector(RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0));
		vecDir1 = vecDir1.Normalize();
		TraceResult		tr1;
		UTIL_TraceLine(vecSrc, vecSrc + vecDir1 * m_radius, IgnoreMonsters::Yes, this, &tr1);

		if ((tr1.vecEndPos - vecSrc).Length() < m_radius * 0.1)
			continue;

		if (tr1.flFraction == 1.0)
			continue;

		Zap(vecSrc, tr1.vecEndPos);
		break;
	}
}

void CLightning::BeamUpdateVars()
{
	CBaseEntity* pStart = UTIL_FindEntityByTargetname(nullptr, STRING(m_iszStartEntity));
	CBaseEntity* pEnd = UTIL_FindEntityByTargetname(nullptr, STRING(m_iszEndEntity));

	//Use the world if the entity could not be found
	if (!pStart)
	{
		pStart = UTIL_GetWorld();
	}

	if (!pEnd)
	{
		pEnd = UTIL_GetWorld();
	}

	bool pointStart = IsPointEntity(pStart);
	bool pointEnd = IsPointEntity(pEnd);

	pev->skin = 0;
	pev->sequence = 0;
	SetRenderMode(RenderMode::Normal);
	pev->flags |= FL_CUSTOMENTITY;
	pev->model = m_iszSpriteName;
	SetTexture(m_spriteTexture);

	int beamType = BEAM_ENTS;
	if (pointStart || pointEnd)
	{
		if (!pointStart)	// One point entity must be in pStart
		{
			// Swap start & end
			std::swap(pStart, pEnd);
			std::swap(pointStart, pointEnd);
		}
		if (!pointEnd)
			beamType = BEAM_ENTPOINT;
		else
			beamType = BEAM_POINTS;
	}

	SetType(beamType);
	if (beamType == BEAM_POINTS || beamType == BEAM_ENTPOINT || beamType == BEAM_HOSE)
	{
		SetStartPos(pStart->GetAbsOrigin());
		if (beamType == BEAM_POINTS || beamType == BEAM_HOSE)
			SetEndPos(pEnd->GetAbsOrigin());
		else
			SetEndEntity(pEnd->entindex());
	}
	else
	{
		SetStartEntity(pStart->entindex());
		SetEndEntity(pEnd->entindex());
	}

	RelinkBeam();

	SetWidth(m_boltWidth);
	SetNoise(m_noiseAmplitude);
	SetFrame(m_frameStart);
	SetScrollRate(m_speed);
	if (pev->spawnflags & SF_BEAM_SHADEIN)
		SetFlags(BEAM_FSHADEIN);
	else if (pev->spawnflags & SF_BEAM_SHADEOUT)
		SetFlags(BEAM_FSHADEOUT);
}
