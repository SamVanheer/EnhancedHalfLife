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
	ItemApplyResult Apply(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			return ItemApplyResult::NotUsed;

		if (pev->spawnflags & SF_SUIT_SHORTLOGON)
			EMIT_SOUND_SUIT(pPlayer, "!HEV_A0");		// short version of suit logon,
		else
			EMIT_SOUND_SUIT(pPlayer, "!HEV_AAx");	// long version of suit logon

		pPlayer->pev->weapons |= (1 << WEAPON_SUIT);
		return ItemApplyResult::Used;
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
	ItemApplyResult Apply(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->deadflag != DeadFlag::No)
		{
			return ItemApplyResult::NotUsed;
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
			return ItemApplyResult::Used;
		}
		return ItemApplyResult::NotUsed;
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
	ItemApplyResult Apply(CBasePlayer* pPlayer) override
	{
		pPlayer->SetSuitUpdate("!HEV_DET4", SuitSoundType::Sentence, SUIT_NEXT_IN_1MIN);

		pPlayer->m_rgItems[ITEM_ANTIDOTE] += 1;
		return ItemApplyResult::Used;
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
	ItemApplyResult Apply(CBasePlayer* pPlayer) override
	{
		if (pPlayer->m_fLongJump)
		{
			return ItemApplyResult::NotUsed;
		}

		if ((pPlayer->pev->weapons & (1 << WEAPON_SUIT)))
		{
			pPlayer->m_fLongJump = true;// player now has longjump module

			g_engfuncs.pfnSetPhysicsKeyValue(pPlayer->edict(), "slj", "1");

			MESSAGE_BEGIN(MessageDest::One, gmsgItemPickup, pPlayer);
			WRITE_STRING(GetClassname());
			MESSAGE_END();

			EMIT_SOUND_SUIT(pPlayer, "!HEV_A1");	// Play the longjump sound UNDONE: Kelly? correct sound?
			return ItemApplyResult::Used;
		}
		return ItemApplyResult::NotUsed;
	}
};

LINK_ENTITY_TO_CLASS(item_longjump, CItemLongJump);
