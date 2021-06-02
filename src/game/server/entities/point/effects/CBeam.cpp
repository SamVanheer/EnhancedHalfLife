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

#include "CBeam.hpp"
#include "customentity.hpp"

void CBeam::Spawn()
{
	SetSolidType(Solid::Not);							// Remove model & collisions
	Precache();
}

void CBeam::Precache()
{
	if (auto owner = GetOwner(); owner)
		SetStartEntity(owner->entindex());
	if (auto aiment = GetAimEntity(); aiment)
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
	SetAimEntity(UTIL_EntityByIndex(entityIndex));
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
		return GetAbsAngles();
	}

	if (CBaseEntity* pEntity = UTIL_EntityByIndex(GetEndEntity()); pEntity)
		return pEntity->GetAbsOrigin();
	return GetAbsAngles();
}

CBeam* CBeam::BeamCreate(const char* pSpriteName, int width)
{
	// Create a new entity with CBeam private data
	CBeam* pBeam = static_cast<CBeam*>(g_EntityList.Create("beam"));

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
