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

#include "CSprite.hpp"

void CSprite::Spawn()
{
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);
	pev->effects = 0;
	pev->frame = 0;

	Precache();
	SetModel(STRING(pev->model));

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
	if (HasTargetname() && !(pev->spawnflags & SF_SPRITE_STARTON))
		TurnOff();
	else
		TurnOn();

	// Worldcraft only sets y rotation, copy to Z
	Vector myAngles = GetAbsAngles();
	if (myAngles.y != 0 && myAngles.z == 0)
	{
		myAngles.z = myAngles.y;
		myAngles.y = 0;
		SetAbsAngles(myAngles);
	}
}

void CSprite::Precache()
{
	PRECACHE_MODEL(STRING(pev->model));

	// Reset attachment after save/restore
	if (auto aiment = GetAimEntity(); aiment)
		SetAttachment(aiment, pev->body);
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
	CSprite* pSprite = static_cast<CSprite*>(g_EntityList.Create("env_sprite"));
	pSprite->SpriteInit(pSpriteName, origin);
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
