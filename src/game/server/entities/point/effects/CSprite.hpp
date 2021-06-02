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

#pragma once

#include "CPointEntity.hpp"
#include "CSprite.generated.hpp"

constexpr int SF_SPRITE_STARTON = 0x0001;
constexpr int SF_SPRITE_ONCE = 0x0002;
constexpr int SF_SPRITE_TEMPORARY = 0x8000;

class EHL_CLASS(EntityName=env_sprite) CSprite : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;

	int		ObjectCaps() override
	{
		int flags = 0;
		if (pev->spawnflags & SF_SPRITE_TEMPORARY)
			flags = FCAP_DONT_SAVE;
		return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | flags;
	}
	void EXPORT AnimateThink();
	void EXPORT ExpandThink();
	void Use(const UseInfo & info) override;
	void Animate(float frames);
	void Expand(float scaleSpeed, float fadeSpeed);
	void SpriteInit(const char* pSpriteName, const Vector & origin);

	inline void SetAttachment(CBaseEntity * pEntity, int attachment)
	{
		if (pEntity)
		{
			pev->skin = pEntity->entindex();
			pev->body = attachment;
			SetAimEntity(pEntity);
			SetMovetype(Movetype::Follow);
		}
	}
	void TurnOff();
	void TurnOn();
	inline float Frames() { return m_maxFrame; }
	inline void SetTransparency(RenderMode rendermode, const Vector & rendercolor, int a, RenderFX fx)
	{
		SetRenderMode(rendermode);
		SetRenderColor(rendercolor);
		SetRenderAmount(a);
		SetRenderFX(fx);
	}
	inline void SetTexture(int spriteIndex) { pev->modelindex = spriteIndex; }
	inline void SetScale(float scale) { pev->scale = scale; }
	inline void SetColor(int r, int g, int b) { SetRenderColor({static_cast<float>(r), static_cast<float>(g), static_cast<float>(b)}); }
	inline void SetBrightness(int brightness) { SetRenderAmount(brightness); }

	inline void AnimateAndDie(float framerate)
	{
		SetThink(&CSprite::AnimateUntilDead);
		pev->framerate = framerate;
		pev->dmgtime = gpGlobals->time + (m_maxFrame / framerate);
		pev->nextthink = gpGlobals->time;
	}

	void EXPORT AnimateUntilDead();

	static CSprite* SpriteCreate(const char* pSpriteName, const Vector & origin, bool animate);

private:
	EHL_FIELD(Persisted, Type=Time)
	float m_lastTime = 0;

	EHL_FIELD(Persisted)
	float m_maxFrame = 0;
};
