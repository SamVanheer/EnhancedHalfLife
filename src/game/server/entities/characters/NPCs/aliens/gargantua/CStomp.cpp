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

#include "CStomp.hpp"

LINK_ENTITY_TO_CLASS(garg_stomp, CStomp);

CStomp* CStomp::StompCreate(const Vector& origin, const Vector& end, float speed)
{
	CStomp* pStomp = GetClassPtr((CStomp*)nullptr);

	pStomp->SetAbsOrigin(origin);
	const Vector dir = (end - origin);
	pStomp->pev->scale = dir.Length();
	pStomp->pev->movedir = dir.Normalize();
	pStomp->pev->speed = speed;
	pStomp->Spawn();

	return pStomp;
}

void CStomp::Spawn()
{
	pev->nextthink = gpGlobals->time;
	SetClassname("garg_stomp");
	pev->dmgtime = gpGlobals->time;

	pev->framerate = 30;
	pev->model = MAKE_STRING(GARG_STOMP_SPRITE_NAME.data());
	SetRenderMode(RenderMode::TransTexture);
	SetRenderAmount(0);
	EmitSound(SoundChannel::Body, GARG_STOMP_BUZZ_SOUND.data(), VOL_NORM, ATTN_NORM, PITCH_NORM * 0.55);
}

constexpr float	STOMP_INTERVAL = 0.025;

void CStomp::Think()
{
	if (m_flLastThinkTime == 0)
	{
		m_flLastThinkTime = gpGlobals->time - gpGlobals->frametime;
	}

	//Use 1/4th the delta time to match the original behavior more closely
	const float deltaTime = (gpGlobals->time - m_flLastThinkTime) / 4;

	m_flLastThinkTime = gpGlobals->time;

	TraceResult tr;

	pev->nextthink = gpGlobals->time + 0.1;

	// Do damage for this frame
	Vector vecStart = GetAbsOrigin();
	vecStart.z += 30;
	const Vector vecEnd = vecStart + (pev->movedir * pev->speed * deltaTime);

	UTIL_TraceHull(vecStart, vecEnd, IgnoreMonsters::No, Hull::Head, this, &tr);

	if (auto hit = InstanceOrNull(tr.pHit); hit && hit != GetOwner())
	{
		hit->TakeDamage({this, InstanceOrDefault(pev->owner, this), gSkillData.gargantuaDmgStomp, DMG_SONIC});
	}

	// Accelerate the effect
	pev->speed = pev->speed + deltaTime * pev->framerate;
	pev->framerate = pev->framerate + deltaTime * 1500;

	// Move and spawn trails
	while (gpGlobals->time - pev->dmgtime > STOMP_INTERVAL)
	{
		SetAbsOrigin(GetAbsOrigin() + pev->movedir * pev->speed * STOMP_INTERVAL);
		for (int i = 0; i < 2; i++)
		{
			if (CSprite* pSprite = CSprite::SpriteCreate(GARG_STOMP_SPRITE_NAME.data(), GetAbsOrigin(), true); pSprite)
			{
				UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 500), IgnoreMonsters::Yes, this, &tr);
				pSprite->SetAbsOrigin(tr.vecEndPos);
				pSprite->SetAbsVelocity(Vector(RANDOM_FLOAT(-200, 200), RANDOM_FLOAT(-200, 200), 175));
				// pSprite->AnimateAndDie( RANDOM_FLOAT( 8.0, 12.0 ) );
				pSprite->pev->nextthink = gpGlobals->time + 0.3;
				pSprite->SetThink(&CSprite::SUB_Remove);
				pSprite->SetTransparency(RenderMode::TransAdd, {255, 255, 255}, 255, RenderFX::FadeFast);
			}
		}
		pev->dmgtime += STOMP_INTERVAL;
		// Scale has the "life" of this effect
		pev->scale -= STOMP_INTERVAL * pev->speed;
		if (pev->scale <= 0)
		{
			// Life has run out
			UTIL_Remove(this);
			StopSound(SoundChannel::Body, GARG_STOMP_BUZZ_SOUND.data());
		}
	}
}
