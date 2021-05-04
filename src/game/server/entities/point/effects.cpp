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

#include "customentity.h"
#include "func_break.h"
#include "shake.h"

constexpr int SF_GIBSHOOTER_REPEATABLE = 1; //!< allows a gibshooter to be refired

constexpr int SF_FUNNEL_REVERSE = 1; //!< funnel effect repels particles instead of attracting them.

/**
*	@brief Lightning target, just alias landmark
*/
LINK_ENTITY_TO_CLASS(info_target, CPointEntity);

class CBubbling : public CBaseEntity
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	KeyValue(KeyValueData* pkvd) override;

	void	EXPORT FizzThink();
	void	Use(const UseInfo& info) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	int		ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	static	TYPEDESCRIPTION m_SaveData[];

	int		m_density;
	int		m_frequency;
	int		m_bubbleModel;
	bool	m_state;
};

LINK_ENTITY_TO_CLASS(env_bubbles, CBubbling);

TYPEDESCRIPTION	CBubbling::m_SaveData[] =
{
	DEFINE_FIELD(CBubbling, m_density, FIELD_INTEGER),
	DEFINE_FIELD(CBubbling, m_frequency, FIELD_INTEGER),
	DEFINE_FIELD(CBubbling, m_state, FIELD_BOOLEAN),
	// Let spawn restore this!
	//	DEFINE_FIELD( CBubbling, m_bubbleModel, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE(CBubbling, CBaseEntity);

constexpr int SF_BUBBLES_STARTOFF = 0x0001;

void CBubbling::Spawn()
{
	Precache();
	SetModel(STRING(pev->model));		// Set size

	SetSolidType(Solid::Not);							// Remove model & collisions
	SetRenderAmount(0);								// The engine won't draw this model if this is set to 0 and blending is on
	SetRenderMode(RenderMode::TransTexture);
	const int speed = pev->speed > 0 ? pev->speed : -pev->speed;

	// HACKHACK!!! - Speed in rendercolor
	SetRenderColor({static_cast<float>(speed >> 8), static_cast<float>(speed & 0xFF), static_cast<float>(pev->speed < 0 ? 1 : 0)});

	if (!(pev->spawnflags & SF_BUBBLES_STARTOFF))
	{
		SetThink(&CBubbling::FizzThink);
		pev->nextthink = gpGlobals->time + 2.0;
		m_state = true;
	}
	else
		m_state = false;
}

void CBubbling::Precache()
{
	m_bubbleModel = PRECACHE_MODEL("sprites/bubble.spr");			// Precache bubble sprite
}

void CBubbling::Use(const UseInfo& info)
{
	if (ShouldToggle(info.GetUseType(), m_state))
		m_state = !m_state;

	if (m_state)
	{
		SetThink(&CBubbling::FizzThink);
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		SetThink(nullptr);
		pev->nextthink = 0;
	}
}

void CBubbling::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "density"))
	{
		m_density = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "frequency"))
	{
		m_frequency = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "current"))
	{
		pev->speed = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CBubbling::FizzThink()
{
	MESSAGE_BEGIN(MessageDest::PAS, SVC_TEMPENTITY, GetBrushModelOrigin(this));
	WRITE_BYTE(TE_FIZZ);
	WRITE_SHORT((short)entindex());
	WRITE_SHORT((short)m_bubbleModel);
	WRITE_BYTE(m_density);
	MESSAGE_END();

	if (m_frequency > 19)
		pev->nextthink = gpGlobals->time + 0.5;
	else
		pev->nextthink = gpGlobals->time + 2.5 - (0.1 * m_frequency);
}

LINK_ENTITY_TO_CLASS(beam, CBeam);

void CBeam::Spawn()
{
	SetSolidType(Solid::Not);							// Remove model & collisions
	Precache();
}

void CBeam::Precache()
{
	if (auto owner = GetOwner(); owner)
		SetStartEntity(owner->entindex());
	if (auto aiment = InstanceOrNull(pev->aiment); aiment)
		SetEndEntity(aiment->entindex());
}

void CBeam::SetStartEntity(int entityIndex)
{
	pev->sequence = (entityIndex & 0x0FFF) | ((pev->sequence & 0xF000) << 12);
	SetOwner(UTIL_EntityByIndex(entityIndex));
}

void CBeam::SetEndEntity(int entityIndex)
{
	pev->skin = (entityIndex & 0x0FFF) | ((pev->skin & 0xF000) << 12);
	pev->aiment = g_engfuncs.pfnPEntityOfEntIndex(entityIndex);
}

const Vector& CBeam::GetStartPos()
{
	if (GetType() == BEAM_ENTS)
	{
		auto pEntity = UTIL_EntityByIndex(GetStartEntity());
		return pEntity->GetAbsOrigin();
	}
	return GetAbsOrigin();
}

const Vector& CBeam::GetEndPos()
{
	const int type = GetType();
	if (type == BEAM_POINTS || type == BEAM_HOSE)
	{
		return pev->angles;
	}

	if (CBaseEntity* pEntity = UTIL_EntityByIndex(GetEndEntity()); pEntity)
		return pEntity->GetAbsOrigin();
	return pev->angles;
}

CBeam* CBeam::BeamCreate(const char* pSpriteName, int width)
{
	// Create a new entity with CBeam private data
	CBeam* pBeam = GetClassPtr((CBeam*)nullptr);
	pBeam->SetClassname("beam");

	pBeam->BeamInit(pSpriteName, width);

	return pBeam;
}

void CBeam::BeamInit(const char* pSpriteName, int width)
{
	pev->flags |= FL_CUSTOMENTITY;
	SetColor(255, 255, 255);
	SetBrightness(255);
	SetNoise(0);
	SetFrame(0);
	SetScrollRate(0);
	pev->model = MAKE_STRING(pSpriteName);
	SetTexture(PRECACHE_MODEL(pSpriteName));
	SetWidth(width);
	pev->skin = 0;
	pev->sequence = 0;
	SetRenderMode(RenderMode::Normal);
}

void CBeam::PointsInit(const Vector& start, const Vector& end)
{
	SetType(BEAM_POINTS);
	SetStartPos(start);
	SetEndPos(end);
	SetStartAttachment(0);
	SetEndAttachment(0);
	RelinkBeam();
}

void CBeam::HoseInit(const Vector& start, const Vector& direction)
{
	SetType(BEAM_HOSE);
	SetStartPos(start);
	SetEndPos(direction);
	SetStartAttachment(0);
	SetEndAttachment(0);
	RelinkBeam();
}

void CBeam::PointEntInit(const Vector& start, int endIndex)
{
	SetType(BEAM_ENTPOINT);
	SetStartPos(start);
	SetEndEntity(endIndex);
	SetStartAttachment(0);
	SetEndAttachment(0);
	RelinkBeam();
}

void CBeam::EntsInit(int startIndex, int endIndex)
{
	SetType(BEAM_ENTS);
	SetStartEntity(startIndex);
	SetEndEntity(endIndex);
	SetStartAttachment(0);
	SetEndAttachment(0);
	RelinkBeam();
}

void CBeam::RelinkBeam()
{
	const Vector& startPos = GetStartPos(), & endPos = GetEndPos();

	pev->mins.x = std::min(startPos.x, endPos.x);
	pev->mins.y = std::min(startPos.y, endPos.y);
	pev->mins.z = std::min(startPos.z, endPos.z);
	pev->maxs.x = std::max(startPos.x, endPos.x);
	pev->maxs.y = std::max(startPos.y, endPos.y);
	pev->maxs.z = std::max(startPos.z, endPos.z);
	pev->mins = pev->mins - GetAbsOrigin();
	pev->maxs = pev->maxs - GetAbsOrigin();

	SetSize(pev->mins, pev->maxs);
	SetAbsOrigin(GetAbsOrigin());
}

#if 0
void CBeam::SetObjectCollisionBox()
{
	const Vector& startPos = GetStartPos(), & endPos = GetEndPos();

	pev->absmin.x = std::min(startPos.x, endPos.x);
	pev->absmin.y = std::min(startPos.y, endPos.y);
	pev->absmin.z = std::min(startPos.z, endPos.z);
	pev->absmax.x = std::max(startPos.x, endPos.x);
	pev->absmax.y = std::max(startPos.y, endPos.y);
	pev->absmax.z = std::max(startPos.z, endPos.z);
}
#endif

void CBeam::TriggerTouch(CBaseEntity* pOther)
{
	if (pOther->pev->flags & (FL_CLIENT | FL_MONSTER))
	{
		if (auto owner = GetOwner(); owner)
		{
			owner->Use({pOther, this, UseType::Toggle});
		}
		ALERT(at_console, "Firing targets!!!\n");
	}
}

CBaseEntity* CBeam::RandomTargetname(const char* szName)
{
	int total = 0;

	CBaseEntity* pEntity = nullptr;
	CBaseEntity* pNewEntity = nullptr;
	while ((pNewEntity = UTIL_FindEntityByTargetname(pNewEntity, szName)) != nullptr)
	{
		total++;
		if (RANDOM_LONG(0, total - 1) < 1)
			pEntity = pNewEntity;
	}
	return pEntity;
}

void CBeam::DoSparks(const Vector& start, const Vector& end)
{
	if (pev->spawnflags & (SF_BEAM_SPARKSTART | SF_BEAM_SPARKEND))
	{
		if (pev->spawnflags & SF_BEAM_SPARKSTART)
		{
			UTIL_Sparks(start);
		}
		if (pev->spawnflags & SF_BEAM_SPARKEND)
		{
			UTIL_Sparks(end);
		}
	}
}

class CLightning : public CBeam
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	Activate() override;

	void	EXPORT StrikeThink();
	void	EXPORT DamageThink();
	void	RandomArea();
	void	RandomPoint(const Vector& vecSrc);
	void	Zap(const Vector& vecSrc, const Vector& vecDest);
	void	EXPORT StrikeUse(const UseInfo& info);
	void	EXPORT ToggleUse(const UseInfo& info);

	inline bool ServerSide()
	{
		return m_life == 0 && !(pev->spawnflags & SF_BEAM_RING);
	}

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	void	BeamUpdateVars();

	bool m_active;
	string_t m_iszStartEntity;
	string_t m_iszEndEntity;
	float	m_life;
	int		m_boltWidth;
	int		m_noiseAmplitude;
	int		m_brightness;
	int		m_speed;
	float	m_restrike;
	int		m_spriteTexture;
	string_t m_iszSpriteName;
	int		m_frameStart;

	float	m_radius;
};

LINK_ENTITY_TO_CLASS(env_lightning, CLightning);
LINK_ENTITY_TO_CLASS(env_beam, CLightning);

// UNDONE: Jay -- This is only a test
#if _DEBUG
class CTripBeam : public CLightning
{
	void Spawn() override;
};
LINK_ENTITY_TO_CLASS(trip_beam, CTripBeam);

void CTripBeam::Spawn()
{
	CLightning::Spawn();
	SetTouch(&CTripBeam::TriggerTouch);
	SetSolidType(Solid::Trigger);
	RelinkBeam();
}
#endif

TYPEDESCRIPTION	CLightning::m_SaveData[] =
{
	DEFINE_FIELD(CLightning, m_active, FIELD_BOOLEAN),
	DEFINE_FIELD(CLightning, m_iszStartEntity, FIELD_STRING),
	DEFINE_FIELD(CLightning, m_iszEndEntity, FIELD_STRING),
	DEFINE_FIELD(CLightning, m_life, FIELD_FLOAT),
	DEFINE_FIELD(CLightning, m_boltWidth, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_noiseAmplitude, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_brightness, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_speed, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_restrike, FIELD_FLOAT),
	DEFINE_FIELD(CLightning, m_spriteTexture, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_iszSpriteName, FIELD_STRING),
	DEFINE_FIELD(CLightning, m_frameStart, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_radius, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CLightning, CBeam);

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

LINK_ENTITY_TO_CLASS(env_laser, CLaser);

TYPEDESCRIPTION	CLaser::m_SaveData[] =
{
	DEFINE_FIELD(CLaser, m_hSprite, FIELD_EHANDLE),
	DEFINE_FIELD(CLaser, m_iszSpriteName, FIELD_STRING),
	DEFINE_FIELD(CLaser, m_firePosition, FIELD_POSITION_VECTOR),
};

IMPLEMENT_SAVERESTORE(CLaser, CBeam);

void CLaser::Spawn()
{
	if (IsStringNull(pev->model))
	{
		SetThink(&CLaser::SUB_Remove);
		return;
	}
	SetSolidType(Solid::Not);							// Remove model & collisions
	Precache();

	SetThink(&CLaser::StrikeThink);
	pev->flags |= FL_CUSTOMENTITY;

	PointsInit(GetAbsOrigin(), GetAbsOrigin());

	if (!m_hSprite && !IsStringNull(m_iszSpriteName))
		m_hSprite = CSprite::SpriteCreate(STRING(m_iszSpriteName), GetAbsOrigin(), true);
	else
		m_hSprite = nullptr;

	if (auto sprite = m_hSprite.Get(); sprite)
		sprite->SetTransparency(RenderMode::Glow, GetRenderColor(), GetRenderAmount(), GetRenderFX());

	if (!IsStringNull(pev->targetname) && !(pev->spawnflags & SF_BEAM_STARTON))
		TurnOff();
	else
		TurnOn();
}

void CLaser::Precache()
{
	pev->modelindex = PRECACHE_MODEL(STRING(pev->model));
	if (!IsStringNull(m_iszSpriteName))
		PRECACHE_MODEL(STRING(m_iszSpriteName));
}

void CLaser::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "LaserTarget"))
	{
		pev->message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "width"))
	{
		SetWidth((int)atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "NoiseAmplitude"))
	{
		SetNoise(atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "TextureScroll"))
	{
		SetScrollRate(atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "texture"))
	{
		pev->model = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "EndSprite"))
	{
		m_iszSpriteName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "framestart"))
	{
		pev->frame = atoi(pkvd->szValue);
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

bool CLaser::IsOn()
{
	if (pev->effects & EF_NODRAW)
		return false;
	return true;
}

void CLaser::TurnOff()
{
	pev->effects |= EF_NODRAW;
	pev->nextthink = 0;
	if (auto sprite = m_hSprite.Get(); sprite)
		sprite->TurnOff();
}

void CLaser::TurnOn()
{
	pev->effects &= ~EF_NODRAW;
	if (auto sprite = m_hSprite.Get(); sprite)
		sprite->TurnOn();
	pev->dmgtime = gpGlobals->time;
	pev->nextthink = gpGlobals->time;
}

void CLaser::Use(const UseInfo& info)
{
	const bool active = IsOn();

	if (!ShouldToggle(info.GetUseType(), active))
		return;
	if (active)
	{
		TurnOff();
	}
	else
	{
		TurnOn();
	}
}

void CLaser::FireAtPoint(TraceResult& tr)
{
	SetEndPos(tr.vecEndPos);
	if (auto sprite = m_hSprite.Get(); sprite)
		sprite->SetAbsOrigin(tr.vecEndPos);

	BeamDamage(&tr);
	DoSparks(GetStartPos(), tr.vecEndPos);
}

void CLaser::StrikeThink()
{
	if (CBaseEntity* pEnd = RandomTargetname(STRING(pev->message)); pEnd)
		m_firePosition = pEnd->GetAbsOrigin();

	TraceResult tr;

	UTIL_TraceLine(GetAbsOrigin(), m_firePosition, IgnoreMonsters::No, nullptr, &tr);
	FireAtPoint(tr);
	pev->nextthink = gpGlobals->time + 0.1;
}

class CGlow : public CPointEntity
{
public:
	void Spawn() override;
	void Think() override;
	void Animate(float frames);
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	float		m_lastTime;
	float		m_maxFrame;
};

LINK_ENTITY_TO_CLASS(env_glow, CGlow);

TYPEDESCRIPTION	CGlow::m_SaveData[] =
{
	DEFINE_FIELD(CGlow, m_lastTime, FIELD_TIME),
	DEFINE_FIELD(CGlow, m_maxFrame, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CGlow, CPointEntity);

void CGlow::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->effects = 0;
	pev->frame = 0;

	PRECACHE_MODEL(STRING(pev->model));
	SetModel(STRING(pev->model));

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
	if (m_maxFrame > 1.0 && pev->framerate != 0)
		pev->nextthink = gpGlobals->time + 0.1;

	m_lastTime = gpGlobals->time;
}

void CGlow::Think()
{
	Animate(pev->framerate * (gpGlobals->time - m_lastTime));

	pev->nextthink = gpGlobals->time + 0.1;
	m_lastTime = gpGlobals->time;
}

void CGlow::Animate(float frames)
{
	if (m_maxFrame > 0)
		pev->frame = fmod(pev->frame + frames, m_maxFrame);
}

LINK_ENTITY_TO_CLASS(env_sprite, CSprite);

TYPEDESCRIPTION	CSprite::m_SaveData[] =
{
	DEFINE_FIELD(CSprite, m_lastTime, FIELD_TIME),
	DEFINE_FIELD(CSprite, m_maxFrame, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CSprite, CPointEntity);

void CSprite::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->effects = 0;
	pev->frame = 0;

	Precache();
	SetModel(STRING(pev->model));

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
	if (!IsStringNull(pev->targetname) && !(pev->spawnflags & SF_SPRITE_STARTON))
		TurnOff();
	else
		TurnOn();

	// Worldcraft only sets y rotation, copy to Z
	if (pev->angles.y != 0 && pev->angles.z == 0)
	{
		pev->angles.z = pev->angles.y;
		pev->angles.y = 0;
	}
}

void CSprite::Precache()
{
	PRECACHE_MODEL(STRING(pev->model));

	// Reset attachment after save/restore
	if (pev->aiment)
		SetAttachment(InstanceOrNull(pev->aiment), pev->body);
	else
	{
		// Clear attachment
		pev->skin = 0;
		pev->body = 0;
	}
}

void CSprite::SpriteInit(const char* pSpriteName, const Vector& origin)
{
	pev->model = MAKE_STRING(pSpriteName);
	SetAbsOrigin(origin);
	Spawn();
}

CSprite* CSprite::SpriteCreate(const char* pSpriteName, const Vector& origin, bool animate)
{
	CSprite* pSprite = GetClassPtr((CSprite*)nullptr);
	pSprite->SpriteInit(pSpriteName, origin);
	pSprite->SetClassname("env_sprite");
	pSprite->SetSolidType(Solid::Not);
	pSprite->SetMovetype(Movetype::Noclip);
	if (animate)
		pSprite->TurnOn();

	return pSprite;
}

void CSprite::AnimateThink()
{
	Animate(pev->framerate * (gpGlobals->time - m_lastTime));

	pev->nextthink = gpGlobals->time + 0.1;
	m_lastTime = gpGlobals->time;
}

void CSprite::AnimateUntilDead()
{
	if (gpGlobals->time > pev->dmgtime)
		UTIL_Remove(this);
	else
	{
		AnimateThink();
		pev->nextthink = gpGlobals->time;
	}
}

void CSprite::Expand(float scaleSpeed, float fadeSpeed)
{
	pev->speed = scaleSpeed;
	pev->health = fadeSpeed;
	SetThink(&CSprite::ExpandThink);

	pev->nextthink = gpGlobals->time;
	m_lastTime = gpGlobals->time;
}

void CSprite::ExpandThink()
{
	const float frametime = gpGlobals->time - m_lastTime;
	pev->scale += pev->speed * frametime;
	SetRenderAmount(GetRenderAmount() - (pev->health * frametime));
	if (GetRenderAmount() <= 0)
	{
		SetRenderAmount(0);
		UTIL_Remove(this);
	}
	else
	{
		pev->nextthink = gpGlobals->time + 0.1;
		m_lastTime = gpGlobals->time;
	}
}

void CSprite::Animate(float frames)
{
	pev->frame += frames;
	if (pev->frame > m_maxFrame)
	{
		if (pev->spawnflags & SF_SPRITE_ONCE)
		{
			TurnOff();
		}
		else
		{
			if (m_maxFrame > 0)
				pev->frame = fmod(pev->frame, m_maxFrame);
		}
	}
}

void CSprite::TurnOff()
{
	pev->effects = EF_NODRAW;
	pev->nextthink = 0;
}

void CSprite::TurnOn()
{
	pev->effects = 0;
	if ((pev->framerate && m_maxFrame > 1.0) || (pev->spawnflags & SF_SPRITE_ONCE))
	{
		SetThink(&CSprite::AnimateThink);
		pev->nextthink = gpGlobals->time;
		m_lastTime = gpGlobals->time;
	}
	pev->frame = 0;
}

void CSprite::Use(const UseInfo& info)
{
	const bool on = pev->effects != EF_NODRAW;
	if (ShouldToggle(info.GetUseType(), on))
	{
		if (on)
		{
			TurnOff();
		}
		else
		{
			TurnOn();
		}
	}
}

class CGibShooter : public CBaseDelay
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void EXPORT ShootThink();
	void Use(const UseInfo& info) override;

	virtual CGib* CreateGib();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	int	m_iGibs;
	int m_iGibCapacity;
	Materials m_iGibMaterial;
	int m_iGibModelIndex;
	float m_flGibVelocity;
	float m_flVariance;
	float m_flGibLife;
};

TYPEDESCRIPTION CGibShooter::m_SaveData[] =
{
	DEFINE_FIELD(CGibShooter, m_iGibs, FIELD_INTEGER),
	DEFINE_FIELD(CGibShooter, m_iGibCapacity, FIELD_INTEGER),
	DEFINE_FIELD(CGibShooter, m_iGibMaterial, FIELD_INTEGER),
	DEFINE_FIELD(CGibShooter, m_iGibModelIndex, FIELD_INTEGER),
	DEFINE_FIELD(CGibShooter, m_flGibVelocity, FIELD_FLOAT),
	DEFINE_FIELD(CGibShooter, m_flVariance, FIELD_FLOAT),
	DEFINE_FIELD(CGibShooter, m_flGibLife, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CGibShooter, CBaseDelay);
LINK_ENTITY_TO_CLASS(gibshooter, CGibShooter);

void CGibShooter::Precache()
{
	if (g_Language == LANGUAGE_GERMAN)
	{
		m_iGibModelIndex = PRECACHE_MODEL("models/germanygibs.mdl");
	}
	else
	{
		m_iGibModelIndex = PRECACHE_MODEL("models/hgibs.mdl");
	}
}

void CGibShooter::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "m_iGibs"))
	{
		m_iGibs = m_iGibCapacity = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_flVelocity"))
	{
		m_flGibVelocity = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_flVariance"))
	{
		m_flVariance = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_flGibLife"))
	{
		m_flGibLife = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
	{
		CBaseDelay::KeyValue(pkvd);
	}
}

void CGibShooter::Use(const UseInfo& info)
{
	SetThink(&CGibShooter::ShootThink);
	pev->nextthink = gpGlobals->time;
}

void CGibShooter::Spawn()
{
	Precache();

	SetSolidType(Solid::Not);
	pev->effects = EF_NODRAW;

	if (m_flDelay == 0)
	{
		m_flDelay = 0.1;
	}

	if (m_flGibLife == 0)
	{
		m_flGibLife = 25;
	}

	SetMovedir(this);
	pev->body = MODEL_FRAMES(m_iGibModelIndex);
}

CGib* CGibShooter::CreateGib()
{
	if (CVAR_GET_FLOAT("violence_hgibs") == 0)
		return nullptr;

	CGib* pGib = GetClassPtr((CGib*)nullptr);
	pGib->Spawn("models/hgibs.mdl");
	pGib->m_bloodColor = BLOOD_COLOR_RED;

	if (pev->body <= 1)
	{
		ALERT(at_aiconsole, "GibShooter Body is <= 1!\n");
	}

	pGib->pev->body = RANDOM_LONG(1, pev->body - 1);// avoid throwing random amounts of the 0th gib. (skull).

	return pGib;
}

void CGibShooter::ShootThink()
{
	pev->nextthink = gpGlobals->time + m_flDelay;

	Vector vecShootDir = pev->movedir;

	vecShootDir = vecShootDir + gpGlobals->v_right * RANDOM_FLOAT(-1, 1) * m_flVariance;
	vecShootDir = vecShootDir + gpGlobals->v_forward * RANDOM_FLOAT(-1, 1) * m_flVariance;
	vecShootDir = vecShootDir + gpGlobals->v_up * RANDOM_FLOAT(-1, 1) * m_flVariance;

	vecShootDir = vecShootDir.Normalize();
	CGib* pGib = CreateGib();

	if (pGib)
	{
		pGib->SetAbsOrigin(GetAbsOrigin());
		pGib->SetAbsVelocity(vecShootDir * m_flGibVelocity);

		pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
		pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);

		const float thinkTime = pGib->pev->nextthink - gpGlobals->time;

		pGib->m_lifeTime = (m_flGibLife * RANDOM_FLOAT(0.95, 1.05));	// +/- 5%
		if (pGib->m_lifeTime < thinkTime)
		{
			pGib->pev->nextthink = gpGlobals->time + pGib->m_lifeTime;
			pGib->m_lifeTime = 0;
		}
	}

	if (--m_iGibs <= 0)
	{
		if (pev->spawnflags & SF_GIBSHOOTER_REPEATABLE)
		{
			m_iGibs = m_iGibCapacity;
			SetThink(nullptr);
			pev->nextthink = gpGlobals->time;
		}
		else
		{
			SetThink(&CGibShooter::SUB_Remove);
			pev->nextthink = gpGlobals->time;
		}
	}
}

class CEnvShooter : public CGibShooter
{
	void		Precache() override;
	void		KeyValue(KeyValueData* pkvd) override;

	CGib* CreateGib() override;
};

LINK_ENTITY_TO_CLASS(env_shooter, CEnvShooter);

void CEnvShooter::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "shootmodel"))
	{
		pev->model = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "shootsounds"))
	{
		const int iNoise = atoi(pkvd->szValue);
		pkvd->fHandled = true;
		switch (iNoise)
		{
		case 0:
			m_iGibMaterial = Materials::Glass;
			break;
		case 1:
			m_iGibMaterial = Materials::Wood;
			break;
		case 2:
			m_iGibMaterial = Materials::Metal;
			break;
		case 3:
			m_iGibMaterial = Materials::Flesh;
			break;
		case 4:
			m_iGibMaterial = Materials::Rocks;
			break;

		default:
		case -1:
			m_iGibMaterial = Materials::None;
			break;
		}
	}
	else
	{
		CGibShooter::KeyValue(pkvd);
	}
}

void CEnvShooter::Precache()
{
	m_iGibModelIndex = PRECACHE_MODEL(STRING(pev->model));
	CBreakable::MaterialSoundPrecache((Materials)m_iGibMaterial);
}

CGib* CEnvShooter::CreateGib()
{
	CGib* pGib = GetClassPtr((CGib*)nullptr);

	pGib->Spawn(STRING(pev->model));

	int bodyPart = 0;

	if (pev->body > 1)
		bodyPart = RANDOM_LONG(0, pev->body - 1);

	pGib->pev->body = bodyPart;
	pGib->m_bloodColor = DONT_BLEED;
	pGib->m_material = m_iGibMaterial;

	pGib->SetRenderMode(GetRenderMode());
	pGib->SetRenderAmount(GetRenderAmount());
	pGib->SetRenderColor(GetRenderColor());
	pGib->SetRenderFX(GetRenderFX());
	pGib->pev->scale = pev->scale;
	pGib->pev->skin = pev->skin;

	return pGib;
}

class CTestEffect : public CBaseDelay
{
public:
	void	Spawn() override;
	void	Precache() override;
	// void	KeyValue( KeyValueData *pkvd ) override;
	void EXPORT TestThink();
	void Use(const UseInfo& info) override;

	int		m_iBeam;
	CBeam* m_pBeam[24];
	float	m_flBeamTime[24];
	float	m_flStartTime;
};

LINK_ENTITY_TO_CLASS(test_effect, CTestEffect);

void CTestEffect::Spawn()
{
	Precache();
}

void CTestEffect::Precache()
{
	PRECACHE_MODEL("sprites/lgtning.spr");
}

void CTestEffect::TestThink()
{
	if (m_iBeam < 24)
	{
		CBeam* pbeam = CBeam::BeamCreate("sprites/lgtning.spr", 100);

		TraceResult		tr;

		Vector vecSrc = GetAbsOrigin();
		Vector vecDir = Vector(RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0));
		vecDir = vecDir.Normalize();
		UTIL_TraceLine(vecSrc, vecSrc + vecDir * 128, IgnoreMonsters::Yes, this, &tr);

		pbeam->PointsInit(vecSrc, tr.vecEndPos);
		// pbeam->SetColor( 80, 100, 255 );
		pbeam->SetColor(255, 180, 100);
		pbeam->SetWidth(100);
		pbeam->SetScrollRate(12);

		m_flBeamTime[m_iBeam] = gpGlobals->time;
		m_pBeam[m_iBeam] = pbeam;
		m_iBeam++;

#if 0
		Vector vecMid = (vecSrc + tr.vecEndPos) * 0.5;
		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(vecMid.x);	// X
		WRITE_COORD(vecMid.y);	// Y
		WRITE_COORD(vecMid.z);	// Z
		WRITE_BYTE(20);		// radius * 0.1
		WRITE_BYTE(255);		// r
		WRITE_BYTE(180);		// g
		WRITE_BYTE(100);		// b
		WRITE_BYTE(20);		// time * 10
		WRITE_BYTE(0);		// decay * 0.1
		MESSAGE_END();
#endif
	}

	float t = gpGlobals->time - m_flStartTime;

	if (t < 3.0)
	{
		for (int i = 0; i < m_iBeam; i++)
		{
			t = (gpGlobals->time - m_flBeamTime[i]) / (3 + m_flStartTime - m_flBeamTime[i]);
			m_pBeam[i]->SetBrightness(255 * t);
			// m_pBeam[i]->SetScrollRate( 20 * t );
		}
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		for (int i = 0; i < m_iBeam; i++)
		{
			UTIL_Remove(m_pBeam[i]);
		}
		m_flStartTime = gpGlobals->time;
		m_iBeam = 0;
		// pev->nextthink = gpGlobals->time;
		SetThink(nullptr);
	}
}

void CTestEffect::Use(const UseInfo& info)
{
	SetThink(&CTestEffect::TestThink);
	pev->nextthink = gpGlobals->time + 0.1;
	m_flStartTime = gpGlobals->time;
}

/**
*	@brief Blood effects
*/
class CBlood : public CPointEntity
{
public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override;
	void	KeyValue(KeyValueData* pkvd) override;

	inline	int		Color() { return pev->impulse; }
	inline	float 	BloodAmount() { return pev->dmg; }

	inline	void SetColor(int color) { pev->impulse = color; }
	inline	void SetBloodAmount(float amount) { pev->dmg = amount; }

	Vector	Direction();
	Vector	BloodPosition(CBaseEntity* pActivator);

private:
};

LINK_ENTITY_TO_CLASS(env_blood, CBlood);

constexpr int SF_BLOOD_RANDOM = 0x0001;
constexpr int SF_BLOOD_STREAM = 0x0002;
constexpr int SF_BLOOD_PLAYER = 0x0004;
constexpr int SF_BLOOD_DECAL = 0x0008;

void CBlood::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->effects = 0;
	pev->frame = 0;
	SetMovedir(this);
}

void CBlood::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "color"))
	{
		const int color = atoi(pkvd->szValue);
		switch (color)
		{
		case 1:
			SetColor(BLOOD_COLOR_YELLOW);
			break;
		default:
			SetColor(BLOOD_COLOR_RED);
			break;
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "amount"))
	{
		SetBloodAmount(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

Vector CBlood::Direction()
{
	if (pev->spawnflags & SF_BLOOD_RANDOM)
		return UTIL_RandomBloodVector();

	return pev->movedir;
}

Vector CBlood::BloodPosition(CBaseEntity* pActivator)
{
	if (pev->spawnflags & SF_BLOOD_PLAYER)
	{
		CBaseEntity* pPlayer;

		if (pActivator && pActivator->IsPlayer())
		{
			pPlayer = pActivator;
		}
		else
			pPlayer = UTIL_EntityByIndex(1);
		if (pPlayer)
			return (pPlayer->GetAbsOrigin() + pPlayer->pev->view_ofs) + Vector(RANDOM_FLOAT(-10, 10), RANDOM_FLOAT(-10, 10), RANDOM_FLOAT(-10, 10));
	}

	return GetAbsOrigin();
}

void CBlood::Use(const UseInfo& info)
{
	if (pev->spawnflags & SF_BLOOD_STREAM)
		UTIL_BloodStream(BloodPosition(info.GetActivator()), Direction(), (Color() == BLOOD_COLOR_RED) ? 70 : Color(), BloodAmount());
	else
		UTIL_BloodDrips(BloodPosition(info.GetActivator()), Direction(), Color(), BloodAmount());

	if (pev->spawnflags & SF_BLOOD_DECAL)
	{
		const Vector forward = Direction();
		const Vector start = BloodPosition(info.GetActivator());
		TraceResult tr;

		UTIL_TraceLine(start, start + forward * BloodAmount() * 2, IgnoreMonsters::Yes, nullptr, &tr);
		if (tr.flFraction != 1.0)
			UTIL_BloodDecalTrace(&tr, Color());
	}
}

/**
*	@brief Screen shake
*	@details pev->scale is amplitude
*	pev->dmg_save is frequency
*	pev->dmg_take is duration
*	pev->dmg is radius
*	radius of 0 means all players
*	NOTE: UTIL_ScreenShake() will only shake players who are on the ground
*/
class CShake : public CPointEntity
{
public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override;
	void	KeyValue(KeyValueData* pkvd) override;

	inline	float	Amplitude() { return pev->scale; }
	inline	float	Frequency() { return pev->dmg_save; }
	inline	float	Duration() { return pev->dmg_take; }
	inline	float	Radius() { return pev->dmg; }

	inline	void	SetAmplitude(float amplitude) { pev->scale = amplitude; }
	inline	void	SetFrequency(float frequency) { pev->dmg_save = frequency; }
	inline	void	SetDuration(float duration) { pev->dmg_take = duration; }
	inline	void	SetRadius(float radius) { pev->dmg = radius; }
private:
};

LINK_ENTITY_TO_CLASS(env_shake, CShake);

constexpr int SF_SHAKE_EVERYONE = 0x0001;	//!< Don't check radius
// UNDONE: These don't work yet
constexpr int SF_SHAKE_DISRUPT = 0x0002;	//!< Disrupt controls
constexpr int SF_SHAKE_INAIR = 0x0004;		//!< Shake players in air

void CShake::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->effects = 0;
	pev->frame = 0;

	if (pev->spawnflags & SF_SHAKE_EVERYONE)
		pev->dmg = 0;
}

void CShake::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "amplitude"))
	{
		SetAmplitude(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "frequency"))
	{
		SetFrequency(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "duration"))
	{
		SetDuration(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "radius"))
	{
		SetRadius(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CShake::Use(const UseInfo& info)
{
	UTIL_ScreenShake(GetAbsOrigin(), Amplitude(), Frequency(), Duration(), Radius());
}

/**
*	@details pev->dmg_take is duration
*	pev->dmg_save is hold duration
*/
class CFade : public CPointEntity
{
public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override;
	void	KeyValue(KeyValueData* pkvd) override;

	inline	float	Duration() { return pev->dmg_take; }
	inline	float	HoldTime() { return pev->dmg_save; }

	inline	void	SetDuration(float duration) { pev->dmg_take = duration; }
	inline	void	SetHoldTime(float hold) { pev->dmg_save = hold; }
private:
};

LINK_ENTITY_TO_CLASS(env_fade, CFade);

constexpr int SF_FADE_IN = 0x0001;			//!< Fade in, not out
constexpr int SF_FADE_MODULATE = 0x0002;	//!< Modulate, don't blend
constexpr int SF_FADE_ONLYONE = 0x0004;

void CFade::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->effects = 0;
	pev->frame = 0;
}

void CFade::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "duration"))
	{
		SetDuration(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "holdtime"))
	{
		SetHoldTime(atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CFade::Use(const UseInfo& info)
{
	int fadeFlags = 0;

	if (!(pev->spawnflags & SF_FADE_IN))
		fadeFlags |= FFADE_OUT;

	if (pev->spawnflags & SF_FADE_MODULATE)
		fadeFlags |= FFADE_MODULATE;

	if (pev->spawnflags & SF_FADE_ONLYONE)
	{
		if (info.GetActivator()->IsNetClient())
		{
			UTIL_ScreenFade(static_cast<CBasePlayer*>(info.GetActivator()), GetRenderColor(), Duration(), HoldTime(), GetRenderAmount(), fadeFlags);
		}
	}
	else
	{
		UTIL_ScreenFadeAll(GetRenderColor(), Duration(), HoldTime(), GetRenderAmount(), fadeFlags);
	}
	SUB_UseTargets(this, UseType::Toggle, 0);
}

class CMessage : public CPointEntity
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	Use(const UseInfo& info) override;
	void	KeyValue(KeyValueData* pkvd) override;
private:
};

LINK_ENTITY_TO_CLASS(env_message, CMessage);

void CMessage::Spawn()
{
	Precache();

	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);

	switch (pev->impulse)
	{
	case 1: // Medium radius
		pev->speed = ATTN_STATIC;
		break;

	case 2:	// Large radius
		pev->speed = ATTN_NORM;
		break;

	case 3:	//EVERYWHERE
		pev->speed = ATTN_NONE;
		break;

	default:
	case 0: // Small radius
		pev->speed = ATTN_IDLE;
		break;
	}
	pev->impulse = 0;

	// No volume, use normal
	if (pev->scale <= 0)
		pev->scale = 1.0;
}

void CMessage::Precache()
{
	if (!IsStringNull(pev->noise))
		PRECACHE_SOUND(STRING(pev->noise));
}

void CMessage::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "messagesound"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "messagevolume"))
	{
		pev->scale = atof(pkvd->szValue) * 0.1;
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "messageattenuation"))
	{
		pev->impulse = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CMessage::Use(const UseInfo& info)
{
	if (pev->spawnflags & SF_MESSAGE_ALL)
		UTIL_ShowMessageAll(STRING(pev->message));
	else
	{
		CBaseEntity* pPlayer;

		if (auto activator = info.GetActivator(); activator && activator->IsPlayer())
			pPlayer = activator;
		else
		{
			//TODO: make it so that this only happens in singleplayer
			pPlayer = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));
		}
		if (pPlayer)
			UTIL_ShowMessage(STRING(pev->message), static_cast<CBasePlayer*>(pPlayer));
	}
	if (!IsStringNull(pev->noise))
	{
		EmitSound(SoundChannel::Body, STRING(pev->noise), pev->scale, pev->speed);
	}
	if (pev->spawnflags & SF_MESSAGE_ONCE)
		UTIL_Remove(this);

	SUB_UseTargets(this, UseType::Toggle, 0);
}

/**
*	@brief Funnel Effect
*/
class CEnvFunnel : public CBaseDelay
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	Use(const UseInfo& info) override;

	int		m_iSprite;	// Don't save, precache
};

void CEnvFunnel::Precache()
{
	m_iSprite = PRECACHE_MODEL("sprites/flare6.spr");
}

LINK_ENTITY_TO_CLASS(env_funnel, CEnvFunnel);

void CEnvFunnel::Use(const UseInfo& info)
{
	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_LARGEFUNNEL);
	WRITE_COORD(GetAbsOrigin().x);
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	WRITE_SHORT(m_iSprite);

	if (pev->spawnflags & SF_FUNNEL_REVERSE)// funnel flows in reverse?
	{
		WRITE_SHORT(1);
	}
	else
	{
		WRITE_SHORT(0);
	}


	MESSAGE_END();

	SetThink(&CEnvFunnel::SUB_Remove);
	pev->nextthink = gpGlobals->time;
}

void CEnvFunnel::Spawn()
{
	Precache();
	SetSolidType(Solid::Not);
	pev->effects = EF_NODRAW;
}

/**
*	@brief Beverage Dispenser
*	@details overloaded pev->frags, is now a flag for whether or not a can is stuck in the dispenser.
*	overloaded pev->health, is now how many cans remain in the machine.
*/
class CEnvBeverage : public CBaseDelay
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	Use(const UseInfo& info) override;
};

void CEnvBeverage::Precache()
{
	PRECACHE_MODEL("models/can.mdl");
	PRECACHE_SOUND("weapons/g_bounce3.wav");
}

LINK_ENTITY_TO_CLASS(env_beverage, CEnvBeverage);

void CEnvBeverage::Use(const UseInfo& info)
{
	if (pev->frags != 0 || pev->health <= 0)
	{
		// no more cans while one is waiting in the dispenser, or if I'm out of cans.
		return;
	}

	CBaseEntity* pCan = CBaseEntity::Create("item_sodacan", GetAbsOrigin(), pev->angles, this);

	if (pev->skin == 6)
	{
		// random
		pCan->pev->skin = RANDOM_LONG(0, 5);
	}
	else
	{
		pCan->pev->skin = pev->skin;
	}

	pev->frags = 1;
	pev->health--;

	//SetThink (SUB_Remove);
	//pev->nextthink = gpGlobals->time;
}

void CEnvBeverage::Spawn()
{
	Precache();
	SetSolidType(Solid::Not);
	pev->effects = EF_NODRAW;
	pev->frags = 0;

	if (pev->health == 0)
	{
		pev->health = 10;
	}
}

/**
*	@brief Soda can
*/
class CItemSoda : public CBaseEntity
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	EXPORT CanThink();
	void	EXPORT CanTouch(CBaseEntity* pOther);
};

void CItemSoda::Precache()
{
}

LINK_ENTITY_TO_CLASS(item_sodacan, CItemSoda);

void CItemSoda::Spawn()
{
	Precache();
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::Toss);

	SetModel("models/can.mdl");
	SetSize(vec3_origin, vec3_origin);

	SetThink(&CItemSoda::CanThink);
	pev->nextthink = gpGlobals->time + 0.5;
}

void CItemSoda::CanThink()
{
	EmitSound(SoundChannel::Weapon, "weapons/g_bounce3.wav");

	SetSolidType(Solid::Trigger);
	SetSize(Vector(-8, -8, 0), Vector(8, 8, 8));
	SetThink(nullptr);
	SetTouch(&CItemSoda::CanTouch);
}

void CItemSoda::CanTouch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{
		return;
	}

	// spoit sound here

	pOther->GiveHealth(1, DMG_GENERIC);// a bit of health.

	if (auto owner = GetOwner(); !IsNullEnt(owner))
	{
		// tell the machine the can was taken
		owner->pev->frags = 0;
	}

	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->effects = EF_NODRAW;
	SetTouch(nullptr);
	SetThink(&CItemSoda::SUB_Remove);
	pev->nextthink = gpGlobals->time;
}
