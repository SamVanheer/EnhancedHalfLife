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

	bool m_active = false;
	string_t m_iszStartEntity = iStringNull;
	string_t m_iszEndEntity = iStringNull;
	float m_life = 0;
	int m_boltWidth = 0;
	int m_noiseAmplitude = 0;
	int m_brightness = 0;
	int m_speed = 0;
	float m_restrike = 0;
	int m_spriteTexture = 0;
	string_t m_iszSpriteName = iStringNull;
	int m_frameStart = 0;

	float m_radius = 0;
};
