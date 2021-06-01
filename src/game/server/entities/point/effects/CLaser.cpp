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

#include "CLaser.hpp"
#include "CSprite.hpp"

LINK_ENTITY_TO_CLASS(env_laser, CLaser);

void CLaser::OnRemove()
{
	m_hSprite.Remove();
	CBeam::OnRemove();
}

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

	if (HasTargetname() && !(pev->spawnflags & SF_BEAM_STARTON))
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
