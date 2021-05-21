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

#include "cbase.hpp"
#include "CBasePlayer.hpp"
#include "weapons.hpp"
#include "gamerules.hpp"
#include "CBaseItem.hpp"

void CBaseItem::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "respawn_mode"))
	{
		if (AreStringsEqual(pkvd->szValue, "Default"))
		{
			m_RespawnMode = ItemRespawnMode::Default;
		}
		else if (AreStringsEqual(pkvd->szValue, "Always"))
		{
			m_RespawnMode = ItemRespawnMode::Always;
		}
		else if (AreStringsEqual(pkvd->szValue, "Never"))
		{
			m_RespawnMode = ItemRespawnMode::Never;
		}
		else
		{
			ALERT(at_warning, "Invalid respawn_mode value \"%s\" for \"%s\" (entity index %d)\n", pkvd->szValue, GetClassname(), entindex());
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "respawn_delay"))
	{
		m_flRespawnDelay = std::max(0.0, atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "respawn_position_mode"))
	{
		if (AreStringsEqual(pkvd->szValue, "Current"))
		{
			m_RespawnPositionMode = ItemRespawnPositionMode::Current;
		}
		else if (AreStringsEqual(pkvd->szValue, "Original"))
		{
			m_RespawnPositionMode = ItemRespawnPositionMode::Original;
		}
		else
		{
			ALERT(at_warning, "Invalid respawn_position_mode value \"%s\" for \"%s\" (entity index %d)\n", pkvd->szValue, GetClassname(), entindex());
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "stay_visible_during_respawn"))
	{
		if (AreStringsEqual(pkvd->szValue, "Default"))
		{
			//Use default value
		}
		else if (AreStringsEqual(pkvd->szValue, "No"))
		{
			m_bStayVisibleDuringRespawn = false;
		}
		else if (AreStringsEqual(pkvd->szValue, "Yes"))
		{
			m_bStayVisibleDuringRespawn = true;
		}
		else
		{
			ALERT(at_warning, "Invalid stay_visible_during_respawn value \"%s\" for \"%s\" (entity index %d)\n", pkvd->szValue, GetClassname(), entindex());
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "flash_on_respawn"))
	{
		if (AreStringsEqual(pkvd->szValue, "Default"))
		{
			//Use default value
		}
		else if (AreStringsEqual(pkvd->szValue, "No"))
		{
			m_bFlashOnRespawn = false;
		}
		else if (AreStringsEqual(pkvd->szValue, "Yes"))
		{
			m_bFlashOnRespawn = true;
		}
		else
		{
			ALERT(at_warning, "Invalid flash_on_respawn value \"%s\" for \"%s\" (entity index %d)\n", pkvd->szValue, GetClassname(), entindex());
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "fall_mode"))
	{
		if (AreStringsEqual(pkvd->szValue, "PlaceOnGround"))
		{
			m_FallMode = ItemFallMode::PlaceOnGround;
		}
		else if (AreStringsEqual(pkvd->szValue, "Fall"))
		{
			m_FallMode = ItemFallMode::Fall;
		}
		else if (AreStringsEqual(pkvd->szValue, "Float"))
		{
			m_FallMode = ItemFallMode::Float;
		}
		else
		{
			ALERT(at_warning, "Invalid fall_mode value \"%s\" for \"%s\" (entity index %d)\n", pkvd->szValue, GetClassname(), entindex());
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "can_pick_up_while_falling"))
	{
		if (AreStringsEqual(pkvd->szValue, "Default"))
		{
			//Use default value
		}
		else if (AreStringsEqual(pkvd->szValue, "No"))
		{
			m_bCanPickUpWhileFalling = false;
		}
		else if (AreStringsEqual(pkvd->szValue, "Yes"))
		{
			m_bCanPickUpWhileFalling = true;
		}
		else
		{
			ALERT(at_warning, "Invalid can_pick_up_while_falling value \"%s\" for \"%s\" (entity index %d)\n", pkvd->szValue, GetClassname(), entindex());
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "clatter_mode"))
	{
		if (AreStringsEqual(pkvd->szValue, "Default"))
		{
			m_ClatterMode = ItemClatterMode::Default;
		}
		else if (AreStringsEqual(pkvd->szValue, "Never"))
		{
			m_ClatterMode = ItemClatterMode::Never;
		}
		else if (AreStringsEqual(pkvd->szValue, "Always"))
		{
			m_ClatterMode = ItemClatterMode::Always;
		}
		else
		{
			ALERT(at_warning, "Invalid clatter_mode value \"%s\" for \"%s\" (entity index %d)\n", pkvd->szValue, GetClassname(), entindex());
		}

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "clatter_sound"))
	{
		m_iszClatterSound = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "respawn_sound"))
	{
		m_iszRespawnSound = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "trigger_on_materialize"))
	{
		m_iszTriggerOnMaterialize = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "trigger_on_dematerialize"))
	{
		m_iszTriggerOnDematerialize = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
	{
		CBaseAnimating::KeyValue(pkvd);
	}
}

void CBaseItem::SetupItem(const Vector& mins, const Vector& maxs)
{
	m_OriginalPosition = GetAbsOrigin();

	if (m_FallMode == ItemFallMode::Float)
	{
		SetMovetype(Movetype::Fly);
	}
	else
	{
		SetMovetype(Movetype::Toss);
	}

	if (m_bCanPickUpWhileFalling)
	{
		SetSolidType(Solid::Trigger);
	}
	else
	{
		SetSolidType(Solid::BBox);
	}

	SetSize(mins, maxs);
	SetAbsOrigin(GetAbsOrigin());

	SetTouch(&CBaseItem::ItemTouch);

	if (m_FallMode == ItemFallMode::PlaceOnGround && DROP_TO_FLOOR(edict()) == 0)
	{
		ALERT(at_error, "Item %s fell out of level at %f,%f,%f", GetClassname(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
		UTIL_Remove(this);
		return;
	}

	//Floating items immediately materialize
	if (m_FallMode == ItemFallMode::Float)
	{
		SetThink(&CBaseItem::Materialize);
	}
	else
	{
		SetThink(&CBaseItem::FallThink);
	}

	pev->nextthink = gpGlobals->time + 0.1;

	//All items are animated by default
	pev->framerate = 1;
}

void CBaseItem::Precache()
{
	if (auto modelName = GetModelName(); *modelName)
	{
		PRECACHE_MODEL(modelName);
	}

	if (auto sound = STRING(m_iszClatterSound); *sound)
	{
		PRECACHE_SOUND(sound);
	}

	if (auto sound = STRING(m_iszRespawnSound); *sound)
	{
		PRECACHE_SOUND(STRING(m_iszRespawnSound));
	}
}

void CBaseItem::Spawn()
{
#ifndef CLIENT_DLL
	Precache();
	SetModel(GetModelName());
	SetupItem({-16, -16, 0}, {16, 16, 16});
#endif
}

void CBaseItem::FallThink()
{
#ifndef CLIENT_DLL
	//Float mode never uses this method
	ASSERT(m_FallMode != ItemFallMode::Float);

	//If we're not using fall mode just do this immediately
	if (m_FallMode != ItemFallMode::Fall || pev->flags & FL_ONGROUND)
	{
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		// Also clatter if the mapper wants it to
		if (m_ClatterMode == ItemClatterMode::Always
			|| (m_ClatterMode == ItemClatterMode::Default && !IsNullEnt(GetOwner())))
		{
			if (auto sound = STRING(m_iszClatterSound); *sound)
			{
				int pitch = 95 + RANDOM_LONG(0, 29);
				EmitSound(SoundChannel::Voice, sound, VOL_NORM, ATTN_NORM, pitch);
			}
		}

		// lie flat
		SetAbsAngles({0, GetAbsAngles().y, 0});

		Materialize();
	}
	else
	{
		pev->nextthink = gpGlobals->time + 0.1;
	}
#endif
}

extern int gEvilImpulse101;

void CBaseItem::ItemTouch(CBaseEntity* pOther)
{
#ifndef CLIENT_DLL
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
	{
		return;
	}

	auto pPlayer = static_cast<CBasePlayer*>(pOther);

	bool shouldRemoveEntity = false;
	bool attachedToPlayer = false;

	// ok, a player is touching this item, but can he have it?
	if (g_pGameRules->CanHaveItem(*pPlayer, *this))
	{
		const ItemApplyResult result = Apply(pPlayer);

		if (result.Action != ItemApplyAction::NotUsed)
		{
			// player grabbed the item. 
			if (!(result.Flags & ItemFlag::PlayerGotItemHandled))
			{
				g_pGameRules->PlayerGotItem(*pPlayer, *this);
			}

			//Do this after telling gamerules so it can set stuff up
			SUB_UseTargets(pOther, UseType::Toggle, 0);

			if (g_pGameRules->ItemShouldRespawn(*this))
			{
				Respawn();
			}
			else
			{
				//We're not respawning so call this here
				FireTargets(STRING(m_iszTriggerOnDematerialize), this, this, UseType::Off, 0);
				shouldRemoveEntity = result.Action == ItemApplyAction::Used;
			}
		}

		if (result.Flags & ItemFlag::AlwaysRemoveItem)
		{
			shouldRemoveEntity = true;
		}

		attachedToPlayer = result.Action == ItemApplyAction::AttachedToPlayer;
	}

	//Items should only be removed if:
	//1. they were used and they did not attach to the player and they did not respawn
	//2. they were spawned using Impulse 101 and not attached to the player
	if (shouldRemoveEntity || (!attachedToPlayer && gEvilImpulse101))
	{
		SetTouch(nullptr);
		UTIL_Remove(this);
	}
#endif
}

CBaseItem* CBaseItem::GetItemToRespawn(const Vector& respawnPoint)
{
#ifndef CLIENT_DLL
	SetAbsOrigin(respawnPoint);// move to wherever I'm supposed to respawn.
#endif

	return this;
}

CBaseEntity* CBaseItem::Respawn()
{
#ifndef CLIENT_DLL
	if (auto newItem = GetItemToRespawn(g_pGameRules->ItemRespawnSpot(*this)); newItem)
	{
		// not a typo! We want to know when the item the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.
		const float respawnTime = g_pGameRules->ItemRespawnTime(*this);

		if (respawnTime > gpGlobals->time && !m_bStayVisibleDuringRespawn)
		{
			newItem->pev->effects |= EF_NODRAW;
		}

		newItem->m_bIsRespawning = true;

		newItem->SetTouch(nullptr);
		newItem->SetThink(&CBaseItem::AttemptToMaterialize);

		//Drop to the floor right away if:
		//1. we're not floating
		//2. we respawn at the current position (should already be on the ground)
		//3. we respawn at the original spawn point and our fall mode is to be placed on the ground (map obstacles may be toggling on/off)
		if (m_FallMode != ItemFallMode::Float
			&& (m_RespawnPositionMode == ItemRespawnPositionMode::Current
				|| m_FallMode == ItemFallMode::PlaceOnGround))
		{
			DROP_TO_FLOOR(newItem->edict());
		}

		newItem->pev->nextthink = respawnTime;

		//Fire this on the new entity so anything that operates on the activator or caller works properly
		FireTargets(STRING(m_iszTriggerOnDematerialize), newItem, newItem, UseType::Off, 0);

		return newItem;
	}

	ALERT(at_console, "Item respawn failed to create %s!\n", GetClassname());
#endif

	return nullptr;
}

void CBaseItem::Materialize()
{
#ifndef CLIENT_DLL
	if (m_bIsRespawning)
	{
		// changing from invisible state to visible.
		if (auto sound = STRING(m_iszRespawnSound); *sound)
		{
			EmitSound(SoundChannel::Weapon, sound, VOL_NORM, ATTN_NORM, 150);
		}

		pev->effects &= ~EF_NODRAW;

		if (m_bFlashOnRespawn)
		{
			pev->effects |= EF_MUZZLEFLASH;
		}

		m_bIsRespawning = false;
	}

	SetSolidType(Solid::Trigger);
	SetAbsOrigin(GetAbsOrigin());// link into world.
	SetTouch(&CBaseItem::ItemTouch);
	SetThink(nullptr);

	FireTargets(STRING(m_iszTriggerOnMaterialize), this, this, UseType::On, 0);
#endif
}

void CBaseItem::AttemptToMaterialize()
{
#ifndef CLIENT_DLL
	const float time = g_pGameRules->ItemTryRespawn(*this);

	if (time == 0)
	{
		switch (m_FallMode)
		{
		case ItemFallMode::PlaceOnGround:
			[[fallthrough]];
		case ItemFallMode::Float:
			Materialize();
			break;

			//Fall first, then materialize (plays clatter sound)
		case ItemFallMode::Fall:
			pev->flags &= ~FL_ONGROUND;
			SetThink(&CBaseItem::FallThink);
			pev->nextthink = gpGlobals->time + 0.1;
			break;

		default: ASSERT(!"Invalid fall mode in CBaseItem::AttemptToMaterialize");
		}
		return;
	}

	pev->nextthink = time;
#endif
}
