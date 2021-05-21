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

#include "CCyclerSprite.hpp"

LINK_ENTITY_TO_CLASS(cycler_sprite, CCyclerSprite);

void CCyclerSprite::Spawn()
{
	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::None);
	SetDamageMode(DamageMode::Yes);
	pev->effects = 0;

	pev->frame = 0;
	pev->nextthink = gpGlobals->time + 0.1;
	m_animate = true;
	m_lastTime = gpGlobals->time;

	PRECACHE_MODEL(STRING(pev->model));
	SetModel(STRING(pev->model));

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
}

void CCyclerSprite::Think()
{
	if (ShouldAnimate())
		Animate(pev->framerate * (gpGlobals->time - m_lastTime));

	pev->nextthink = gpGlobals->time + 0.1;
	m_lastTime = gpGlobals->time;
}

void CCyclerSprite::Use(const UseInfo& info)
{
	m_animate = !m_animate;
	ALERT(at_console, "Sprite: %s\n", STRING(pev->model));
}

bool	CCyclerSprite::TakeDamage(const TakeDamageInfo& info)
{
	if (m_maxFrame > 1.0)
	{
		Animate(1.0);
	}
	return true;
}

void CCyclerSprite::Animate(float frames)
{
	pev->frame += frames;
	if (m_maxFrame > 0)
		pev->frame = fmod(pev->frame, m_maxFrame);
}
