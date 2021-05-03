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

#include "items.h"
#include "UserMessages.h"

void CItem::Spawn()
{
	SetMovetype(Movetype::Toss);
	SetSolidType(Solid::Trigger);
	SetAbsOrigin(GetAbsOrigin());
	SetSize(Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch(&CItem::ItemTouch);

	if (DROP_TO_FLOOR(edict()) == 0)
	{
		ALERT(at_error, "Item %s fell out of level at %f,%f,%f", GetClassname(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
		UTIL_Remove(this);
		return;
	}
}

extern int gEvilImpulse101;

void CItem::ItemTouch(CBaseEntity* pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
	{
		return;
	}

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;

	// ok, a player is touching this item, but can he have it?
	if (!g_pGameRules->CanHaveItem(pPlayer, this))
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch(pPlayer))
	{
		SUB_UseTargets(pOther, UseType::Toggle, 0);
		SetTouch(nullptr);

		// player grabbed the item. 
		g_pGameRules->PlayerGotItem(pPlayer, this);
		if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_YES)
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);
		}
	}
	else if (gEvilImpulse101)
	{
		UTIL_Remove(this);
	}
}

CBaseEntity* CItem::Respawn()
{
	SetTouch(nullptr);
	pev->effects |= EF_NODRAW;

	SetAbsOrigin(g_pGameRules->ItemRespawnSpot(this));// blip to whereever you should respawn.

	SetThink(&CItem::Materialize);
	pev->nextthink = g_pGameRules->ItemRespawnTime(this);
	return this;
}

void CItem::Materialize()
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		EmitSound(SoundChannel::Weapon, "items/suitchargeok1.wav", VOL_NORM, ATTN_NORM, 150);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch(&CItem::ItemTouch);
}

constexpr int SF_SUIT_SHORTLOGON = 0x0001;

class CItemSuit : public CItem
{
	void Spawn() override
	{
		Precache();
		SetModel("models/w_suit.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_suit.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			return false;

		if (pev->spawnflags & SF_SUIT_SHORTLOGON)
			EMIT_SOUND_SUIT(pPlayer, "!HEV_A0");		// short version of suit logon,
		else
			EMIT_SOUND_SUIT(pPlayer, "!HEV_AAx");	// long version of suit logon

		pPlayer->pev->weapons |= (1 << WEAPON_SUIT);
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit);

class CItemBattery : public CItem
{
	void Spawn() override
	{
		Precache();
		SetModel("models/w_battery.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_battery.mdl");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->deadflag != DeadFlag::No)
		{
			return false;
		}

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			(pPlayer->pev->weapons & (1 << WEAPON_SUIT)))
		{
			pPlayer->pev->armorvalue += gSkillData.batteryCapacity;
			pPlayer->pev->armorvalue = std::min(pPlayer->pev->armorvalue, static_cast<float>(MAX_NORMAL_BATTERY));

			pPlayer->EmitSound(SoundChannel::Item, "items/gunpickup2.wav");

			MESSAGE_BEGIN(MessageDest::One, gmsgItemPickup, pPlayer);
			WRITE_STRING(GetClassname());
			MESSAGE_END();


			// Suit reports new power level
			// For some reason this wasn't working in release build -- round it.
			int pct = (int)((float)(pPlayer->pev->armorvalue * 100.0) * (1.0 / MAX_NORMAL_BATTERY) + 0.5);
			pct = (pct / 5);
			if (pct > 0)
				pct--;

			char szcharge[64];
			snprintf(szcharge, sizeof(szcharge), "!HEV_%1dP", pct);

			//EMIT_SOUND_SUIT(ENT(pev), szcharge);
			pPlayer->SetSuitUpdate(szcharge, SuitSoundType::Sentence, SUIT_NEXT_IN_30SEC);
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);

class CItemAntidote : public CItem
{
	void Spawn() override
	{
		Precache();
		SetModel("models/w_antidote.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_antidote.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		pPlayer->SetSuitUpdate("!HEV_DET4", SuitSoundType::Sentence, SUIT_NEXT_IN_1MIN);

		pPlayer->m_rgItems[ITEM_ANTIDOTE] += 1;
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_antidote, CItemAntidote);

class CItemLongJump : public CItem
{
	void Spawn() override
	{
		Precache();
		SetModel("models/w_longjump.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_longjump.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->m_fLongJump)
		{
			return false;
		}

		if ((pPlayer->pev->weapons & (1 << WEAPON_SUIT)))
		{
			pPlayer->m_fLongJump = true;// player now has longjump module

			g_engfuncs.pfnSetPhysicsKeyValue(pPlayer->edict(), "slj", "1");

			MESSAGE_BEGIN(MessageDest::One, gmsgItemPickup, pPlayer);
			WRITE_STRING(GetClassname());
			MESSAGE_END();

			EMIT_SOUND_SUIT(pPlayer, "!HEV_A1");	// Play the longjump sound UNDONE: Kelly? correct sound?
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_longjump, CItemLongJump);
