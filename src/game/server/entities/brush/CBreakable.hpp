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

#include "CBaseDelay.hpp"
#include "CBreakable.generated.hpp"

enum class Explosions
{
	Random,
	Directed
};

enum class Materials
{
	Glass = 0,
	Wood,
	Metal,
	Flesh,
	CinderBlock,
	CeilingTile,
	Computer,
	UnbreakableGlass,
	Rocks,
	None,
	LastMaterial
};

constexpr int NUM_SHARDS = 6; //!< this many shards spawned when breakable objects break

constexpr int SF_BREAK_TRIGGER_ONLY = 1;	//!< may only be broken by trigger
constexpr int SF_BREAK_TOUCH = 2;			//!< can be 'crashed through' by running player (plate glass)
constexpr int SF_BREAK_PRESSURE = 4;		//!< can be broken by a player standing on it
constexpr int SF_BREAK_CROWBAR = 256;		//!< instant break if hit with crowbar

/**
*	@brief bmodel that breaks into pieces after taking damage
*/
class EHL_CLASS() CBreakable : public CBaseDelay
{
	EHL_GENERATED_BODY()

public:
	// basic functions
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;
	void EXPORT BreakTouch(CBaseEntity* pOther);

	/**
	*	@brief Break when triggered
	*/
	void Use(const UseInfo& info) override;

	/**
	*	@brief play shard sound when func_breakable takes damage.
	*	the more damage, the louder the shard sound.
	*/
	void DamageSound();

	/**
	*	@brief Special takedamage for func_breakable.
	*	Allows us to make exceptions that are breakable-specific
	*/
	bool TakeDamage(const TakeDamageInfo& info) override;
	/**
	*	@brief To spark when hit
	*/
	void TraceAttack(const TraceAttackInfo& info) override;

	bool IsBreakable();

	int	 DamageDecal(int bitsDamageType) override;

	void EXPORT		Die();
	int		ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	inline bool		Explodable() { return ExplosionMagnitude() > 0; }
	inline int		ExplosionMagnitude() { return pev->impulse; }
	inline void		ExplosionSetMagnitude(int magnitude) { pev->impulse = magnitude; }

	static void MaterialSoundPrecache(Materials precacheMaterial);
	static void MaterialSoundRandom(CBaseEntity* entity, Materials soundMaterial, float volume);
	static const char** MaterialSoundList(Materials precacheMaterial, int& soundCount);

	static const char* pSoundsWood[];
	static const char* pSoundsFlesh[];
	static const char* pSoundsGlass[];
	static const char* pSoundsMetal[];
	static const char* pSoundsConcrete[];
	static const char* pSpawnObjects[];

	EHL_FIELD(Persisted)
	Materials m_Material = Materials::Glass;

	EHL_FIELD(Persisted)
	Explosions m_Explosion = Explosions::Random;

	// Don't need to save/restore these because we precache after restore
	int m_idShard = 0;

	EHL_FIELD(Persisted)
	float m_angle = 0;

	EHL_FIELD(Persisted)
	string_t m_iszGibModel = iStringNull;

	EHL_FIELD(Persisted)
	string_t m_iszSpawnObject = iStringNull;

	// Explosion magnitude is stored in pev->impulse
};
