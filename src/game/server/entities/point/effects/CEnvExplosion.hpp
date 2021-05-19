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

/**
*	@file
*
*	Explosion-related code
*/

#include "CBaseEntity.hpp"
#include "basemonster.h"

#include "util.h" //TODO: get rid of DLL_GLOBAL so we can remove this header include

class CBaseEntity;

constexpr int SF_ENVEXPLOSION_NODAMAGE = 1 << 0;	// when set, ENV_EXPLOSION will not actually inflict damage
constexpr int SF_ENVEXPLOSION_REPEATABLE = 1 << 1;	// can this entity be refired?
constexpr int SF_ENVEXPLOSION_NOFIREBALL = 1 << 2;	// don't draw the fireball
constexpr int SF_ENVEXPLOSION_NOSMOKE = 1 << 3;		// don't draw the smoke
constexpr int SF_ENVEXPLOSION_NODECAL = 1 << 4;		// don't make a scorch mark
constexpr int SF_ENVEXPLOSION_NOSPARKS = 1 << 5;	// don't make a scorch mark

class CEnvExplosion : public CBaseMonster
{
public:
	void Spawn() override;
	void EXPORT Smoke();
	void KeyValue(KeyValueData* pkvd) override;
	void Use(const UseInfo& info) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	int m_iMagnitude = 0;// how large is the fireball? how much damage?
	int m_spriteScale = 0; // what's the exact fireball sprite scale? 
};

extern DLL_GLOBAL	short	g_sModelIndexFireball;
extern DLL_GLOBAL	short	g_sModelIndexSmoke;

void UTIL_CreateExplosion(Vector center, const Vector& angles, CBaseEntity* owner, int magnitude, bool doDamage = true, float randomRange = 0, float delay = 0);
