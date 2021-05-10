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

#include "CBaseAnimating.hpp"

enum class ItemType
{
	PickupItem = 0,	//!< A pickup item like health, battery, etc
	Ammo,			//!< An ammo item like a Glock magazine, AR grenades, etc
	Weapon			//!< A weapon like the Glock, MP5, etc
};

enum class ItemRespawnMode
{
	Default = 0,
	Always,
	Never
};

enum class ItemRespawnPositionMode
{
	Current,
	Original
};

enum class ItemFallMode
{
	/**
	*	@brief Place on ground with DROP_TO_FLOOR. Entity will be removed if it falls outside the level (can happen if item is clipping into BSP geometry)
	*/
	PlaceOnGround = 0,
	Fall,
	Float,	//!< Movetype::Fly with no falling logic
};

enum class ItemClatterMode
{
	Default,
	Always,
	Never
};

enum class ItemApplyAction
{
	NotUsed = 0,		//!< Item was not used by player and remains in the world
	Used,				//!< Item was used and needs to be checked for respawn/removal
	AttachedToPlayer	//!< Item was attached to player and needs to be checked for respawn
};

namespace ItemFlag
{
constexpr int AlwaysRemoveItem = 1 << 0;
constexpr int PlayerGotItemHandled = 1 << 1;
}

struct ItemApplyResult
{
	const ItemApplyAction Action;
	const int Flags = 0;
};

constexpr float ITEM_DEFAULT_RESPAWN_DELAY = -1;

/**
*	@brief Base class for all items
*	@details Handles item setup in the world, pickup on touch, respawning
*/
class CBaseItem : public CBaseAnimating
{
public:
	/**
	*	@brief Gets the type of item that this is
	*/
	virtual ItemType GetType() const = 0;

	void KeyValue(KeyValueData* pkvd) override;
	void Precache() override;
	void Spawn() override;

	/**
	*	@brief Items that have just spawned run this think to catch them when they hit the ground.
	*	Once we're sure that the object is grounded,
	*	we change its solid type to trigger and set it in a large box that helps the player get it.
	*/
	void EXPORT FallThink();

	void EXPORT ItemTouch(CBaseEntity* pOther);

	CBaseEntity* Respawn() override;

	/**
	*	@brief make an item visible and tangible
	*/
	void EXPORT Materialize();

	/**
	*	@brief the item is trying to rematerialize, should it do so now or wait longer?
	*	the weapon desires to become visible and tangible, if the game rules allow for it
	*/
	void EXPORT AttemptToMaterialize();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

protected:
	void SetupItem(const Vector& mins, const Vector& maxs);

	virtual ItemApplyResult Apply(CBasePlayer* pPlayer) = 0;

	/**
	*	@brief Returns the item that should be respawned after this one has been picked up
	*	Can be this entity if it does not need to be cloned
	*/
	virtual CBaseItem* GetItemToRespawn(const Vector& respawnPoint);

public:
	Vector m_OriginalPosition;
	ItemRespawnMode m_RespawnMode = ItemRespawnMode::Default;
	float m_flRespawnDelay = ITEM_DEFAULT_RESPAWN_DELAY;
	ItemRespawnPositionMode m_RespawnPositionMode = ItemRespawnPositionMode::Current;

protected:
	ItemFallMode m_FallMode = ItemFallMode::PlaceOnGround;

	/**
	*	@brief Can this item be picked up while it's falling?
	*	Only affects initial fall when spawning
	*/
	bool m_bCanPickUpWhileFalling = true;

	/**
	*	@brief Make a clatter sound when falling on the ground
	*/
	ItemClatterMode m_ClatterMode = ItemClatterMode::Default;

	bool m_bStayVisibleDuringRespawn = false;
	bool m_bIsRespawning = false;

	bool m_bFlashOnRespawn = true;

	//Default sounds are precached in W_Precache
	string_t m_iszClatterSound = MAKE_STRING("items/weapondrop1.wav");

	string_t m_iszRespawnSound = MAKE_STRING("items/suitchargeok1.wav");

	/**
	*	@brief Target to trigger when this entity materializes (spawns/respawns)
	*/
	string_t m_iszTriggerOnMaterialize = iStringNull;

	/**
	*	@brief Target to trigger when this entity dematerializes (waiting to respawn, being removed)
	*/
	string_t m_iszTriggerOnDematerialize = iStringNull;
};
