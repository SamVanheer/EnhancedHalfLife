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

#include "CBasePlayer.hpp"
#include "CItem.hpp"
#include "skill.h"
#include "UserMessages.h"
#include "weapons.h"

constexpr float BATTERY_DEFAULT_CAPACITY = -1;

class EHL_CLASS() CItemBattery : public CItem
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
		CItem::Precache();
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	ItemApplyResult Apply(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->deadflag != DeadFlag::No)
		{
			return {ItemApplyAction::NotUsed};
		}

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			pPlayer->HasSuit())
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
