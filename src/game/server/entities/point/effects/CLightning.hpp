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

#include "CBeam.hpp"
#include "CLightning.generated.hpp"

class EHL_CLASS(EntityName=env_beam, EntityNameAliases=[env_lightning]) CLightning : public CBeam
{
	EHL_GENERATED_BODY()

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

	void	BeamUpdateVars();

	EHL_FIELD(Persisted)
	bool m_active = false;

	EHL_FIELD(Persisted)
	string_t m_iszStartEntity = iStringNull;

	EHL_FIELD(Persisted)
	string_t m_iszEndEntity = iStringNull;

	EHL_FIELD(Persisted)
	float m_life = 0;

	EHL_FIELD(Persisted)
	int m_boltWidth = 0;

	EHL_FIELD(Persisted)
	int m_noiseAmplitude = 0;

	EHL_FIELD(Persisted)
	int m_brightness = 0;

	EHL_FIELD(Persisted)
	int m_speed = 0;

	EHL_FIELD(Persisted)
	float m_restrike = 0;

	//TODO: shouldn't be saved
	EHL_FIELD(Persisted)
	int m_spriteTexture = 0;

	EHL_FIELD(Persisted)
	string_t m_iszSpriteName = iStringNull;

	EHL_FIELD(Persisted)
	int m_frameStart = 0;

	EHL_FIELD(Persisted)
	float m_radius = 0;
};
