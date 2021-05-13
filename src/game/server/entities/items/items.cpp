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

enum class SuitLogonType
{
	NoLogon = 0,
	LongLogon,
	ShortLogon,
	
};

class CItemSuit : public CItem
{
	void OnConstruct() override
	{
		CItem::OnConstruct();
		SetModelName("models/w_suit.mdl");
	}

	void KeyValue(KeyValueData* pkvd) override
	{
		if (AreStringsEqual(pkvd->szKeyName, "logon_type"))
		{
			if (AreStringsEqual(pkvd->szValue, "NoLogon"))
			{
				m_LogonType = SuitLogonType::NoLogon;
			}
			else if (AreStringsEqual(pkvd->szValue, "LongLogon"))
			{
				m_LogonType = SuitLogonType::LongLogon;
			}
			else if (AreStringsEqual(pkvd->szValue, "ShortLogon"))
			{
				m_LogonType = SuitLogonType::ShortLogon;
			}
			else
			{
				ALERT(at_warning, "Invalid logon_type value \"%s\" for \"%s\" (entity index %d)\n", pkvd->szValue, GetClassname(), entindex());
			}

			pkvd->fHandled = true;
		}
		else
		{
			CItem::KeyValue(pkvd);
		}
	}

	ItemApplyResult Apply(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			return {ItemApplyAction::NotUsed};

		switch (m_LogonType)
		{
		case SuitLogonType::NoLogon: break;
		case SuitLogonType::LongLogon:
			EMIT_SOUND_SUIT(pPlayer, "!HEV_AAx");
			break;
		case SuitLogonType::ShortLogon:
			EMIT_SOUND_SUIT(pPlayer, "!HEV_A0");
			break;
		}

		pPlayer->pev->weapons |= (1 << WEAPON_SUIT);
		return {ItemApplyAction::Used};
	}

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

protected:
	SuitLogonType m_LogonType = SuitLogonType::NoLogon;
};

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit);

TYPEDESCRIPTION CItemSuit::m_SaveData[] =
{
	DEFINE_FIELD(CItemSuit, m_LogonType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CItemSuit, CBaseItem);

constexpr float BATTERY_DEFAULT_CAPACITY = -1;

class CItemBattery : public CItem
{
	void OnConstruct() override
	{
		CItem::OnConstruct();
		SetModelName("models/w_battery.mdl");
	}

	void KeyValue(KeyValueData* pkvd)
	{
		if (AreStringsEqual(pkvd->szKeyName, "custom_capacity"))
		{
			m_flCustomCapacity = std::max(0.0, atof(pkvd->szValue));
			pkvd->fHandled = true;
		}
		else
		{
			CItem::KeyValue(pkvd);
		}
	}

	void Precache() override
	{
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	ItemApplyResult Apply(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->deadflag != DeadFlag::No)
		{
			return {ItemApplyAction::NotUsed};
		}

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			(pPlayer->pev->weapons & (1 << WEAPON_SUIT)))
		{
			float capacity = gSkillData.batteryCapacity;

			if (m_flCustomCapacity != BATTERY_DEFAULT_CAPACITY)
			{
				capacity = m_flCustomCapacity;
			}

			pPlayer->pev->armorvalue += capacity;
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
			return {ItemApplyAction::Used};
		}
		return {ItemApplyAction::NotUsed};
	}

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	float m_flCustomCapacity = BATTERY_DEFAULT_CAPACITY;
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);

TYPEDESCRIPTION	CItemBattery::m_SaveData[] =
{
	DEFINE_FIELD(CItemBattery, m_flCustomCapacity, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CItemBattery, CItem);

class CItemAntidote : public CItem
{
	void OnConstruct() override
	{
		CItem::OnConstruct();
		SetModelName("models/w_antidote.mdl");
	}

	ItemApplyResult Apply(CBasePlayer* pPlayer) override
	{
		pPlayer->SetSuitUpdate("!HEV_DET4", SuitSoundType::Sentence, SUIT_NEXT_IN_1MIN);

		pPlayer->m_rgItems[ITEM_ANTIDOTE] += 1;
		return {ItemApplyAction::Used};
	}
};

LINK_ENTITY_TO_CLASS(item_antidote, CItemAntidote);

class CItemLongJump : public CItem
{
	void OnConstruct() override
	{
		CItem::OnConstruct();
		SetModelName("models/w_longjump.mdl");
	}

	ItemApplyResult Apply(CBasePlayer* pPlayer) override
	{
		if (pPlayer->m_fLongJump)
		{
			return {ItemApplyAction::NotUsed};
		}

		if ((pPlayer->pev->weapons & (1 << WEAPON_SUIT)))
		{
			pPlayer->SetHasLongJump(true);// player now has longjump module

			MESSAGE_BEGIN(MessageDest::One, gmsgItemPickup, pPlayer);
			WRITE_STRING(GetClassname());
			MESSAGE_END();

			EMIT_SOUND_SUIT(pPlayer, "!HEV_A1");	// Play the longjump sound UNDONE: Kelly? correct sound?
			return {ItemApplyAction::Used};
		}
		return {ItemApplyAction::NotUsed};
	}
};

LINK_ENTITY_TO_CLASS(item_longjump, CItemLongJump);
