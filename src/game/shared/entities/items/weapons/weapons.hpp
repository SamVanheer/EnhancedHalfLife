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
*	functions governing the selection/use of weapons for players
*/

#include "effects/CBeam.hpp"
#include "effects/CSprite.hpp"

/**
*	@brief called by worldspawn
*/
void W_Precache();

// constant items
constexpr int ITEM_ANTIDOTE = 0;

constexpr int MAX_NORMAL_BATTERY = 100;

// bullet types
enum Bullet
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM, // glock
	BULLET_PLAYER_MP5, // mp5
	BULLET_PLAYER_357, // python
	BULLET_PLAYER_BUCKSHOT, // shotgun
	BULLET_PLAYER_CROWBAR, // crowbar swipe

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,
};

extern DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
extern DLL_GLOBAL	const char* g_pModelNameLaser;

extern DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
extern DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
extern DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
extern DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for blood drops
extern DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for blood spray (bigger)

/**
*	@brief resets the global multi damage accumulator
*/
void ClearMultiDamage();

/**
*	@brief inflicts contents of global multi damage register on gMultiDamage.pEntity
*/
void ApplyMultiDamage(CBaseEntity* pInflictor, CBaseEntity* pAttacker);

/**
*	@brief Collects multiple small damages into a single damage
*/
void AddMultiDamage(CBaseEntity* pInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType);

void DecalGunshot(TraceResult* pTrace, int iBulletType);
void SpawnBlood(const Vector& vecSpot, int bloodColor, float flDamage);

/**
*	@brief this entity is exploding, or otherwise needs to inflict damage upon entities within a certain range.
*	@details only damages ents that can clearly be seen by the explosion!
*/
void RadiusDamage(Vector vecSrc, CBaseEntity* pInflictor, CBaseEntity* pAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType);

struct MULTIDAMAGE
{
	CBaseEntity* pEntity;
	float			amount;
	int				type;
};

extern MULTIDAMAGE gMultiDamage;

constexpr int LOUD_GUN_VOLUME = 1000;
constexpr int NORMAL_GUN_VOLUME = 600;
constexpr int QUIET_GUN_VOLUME = 200;

constexpr int BRIGHT_GUN_FLASH = 512;
constexpr int NORMAL_GUN_FLASH = 256;
constexpr int DIM_GUN_FLASH = 128;

constexpr int BIG_EXPLOSION_VOLUME = 2048;
constexpr int NORMAL_EXPLOSION_VOLUME = 1024;
constexpr int SMALL_EXPLOSION_VOLUME = 512;

constexpr int WEAPON_ACTIVITY_VOLUME = 64;

constexpr Vector VECTOR_CONE_1DEGREES(0.00873, 0.00873, 0.00873);
constexpr Vector VECTOR_CONE_2DEGREES(0.01745, 0.01745, 0.01745);
constexpr Vector VECTOR_CONE_3DEGREES(0.02618, 0.02618, 0.02618);
constexpr Vector VECTOR_CONE_4DEGREES(0.03490, 0.03490, 0.03490);
constexpr Vector VECTOR_CONE_5DEGREES(0.04362, 0.04362, 0.04362);
constexpr Vector VECTOR_CONE_6DEGREES(0.05234, 0.05234, 0.05234);
constexpr Vector VECTOR_CONE_7DEGREES(0.06105, 0.06105, 0.06105);
constexpr Vector VECTOR_CONE_8DEGREES(0.06976, 0.06976, 0.06976);
constexpr Vector VECTOR_CONE_9DEGREES(0.07846, 0.07846, 0.07846);
constexpr Vector VECTOR_CONE_10DEGREES(0.08716, 0.08716, 0.08716);
constexpr Vector VECTOR_CONE_15DEGREES(0.13053, 0.13053, 0.13053);
constexpr Vector VECTOR_CONE_20DEGREES(0.17365, 0.17365, 0.17365);

/**
*	@brief Returns if it's multiplayer.
*	Mostly used by the client side weapons.
*/
bool bIsMultiplayer();

/**
*	@brief pass in a name and this function will tell you the maximum amount of that type of ammunition that a player can carry.
*/
int MaxAmmoCarry(string_t iszName);

#ifdef CLIENT_DLL
/**
*	@brief Just loads a v_ model.
*/
void LoadVModel(const char* szViewModel, CBasePlayer* m_pPlayer);
#endif
