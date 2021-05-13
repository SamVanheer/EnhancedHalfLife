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
#include "CBaseCharger.hpp"

constexpr float HEALTHKIT_DEFAULT_CAPACITY = -1;

class CHealthKit : public CItem
{
	void OnConstruct() override
	{
		CItem::OnConstruct();
		SetModelName("models/w_medkit.mdl");
	}

	void KeyValue(KeyValueData* pkvd) override;
	void Precache() override;
	ItemApplyResult Apply(CBasePlayer* pPlayer) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	float m_flCustomCapacity = HEALTHKIT_DEFAULT_CAPACITY;
};

LINK_ENTITY_TO_CLASS(item_healthkit, CHealthKit);

TYPEDESCRIPTION	CHealthKit::m_SaveData[] =
{
	DEFINE_FIELD(CHealthKit, m_flCustomCapacity, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CHealthKit, CItem);

void CHealthKit::KeyValue(KeyValueData* pkvd)
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

void CHealthKit::Precache()
{
	PRECACHE_SOUND("items/smallmedkit1.wav");
}

ItemApplyResult CHealthKit::Apply(CBasePlayer* pPlayer)
{
	if (pPlayer->pev->deadflag != DeadFlag::No)
	{
		return {ItemApplyAction::NotUsed};
	}

	float capacity = gSkillData.healthkitCapacity;

	if (m_flCustomCapacity != HEALTHKIT_DEFAULT_CAPACITY)
	{
		capacity = m_flCustomCapacity;
	}

	if (pPlayer->GiveHealth(capacity, DMG_GENERIC))
	{
		MESSAGE_BEGIN(MessageDest::One, gmsgItemPickup, pPlayer);
		WRITE_STRING(GetClassname());
		MESSAGE_END();

		pPlayer->EmitSound(SoundChannel::Item, "items/smallmedkit1.wav");

		return {ItemApplyAction::Used};
	}

	return {ItemApplyAction::NotUsed};
}


/**
*	@brief Wall mounted health kit
*/
class CWallHealth : public CBaseCharger
{
public:
	CWallHealth()
	{
		m_iTotalCapacity = gSkillData.healthchargerCapacity;
		m_iszChargeOnSound = MAKE_STRING("items/medshot4.wav");
		m_iszChargeLoopSound = MAKE_STRING("items/medcharge4.wav");
		m_iszRefuseChargeSound = MAKE_STRING("items/medshotno1.wav");
	}

protected:
	float GetCurrentValue(CBaseEntity* target) override
	{
		return target->pev->health;
	}

	bool SetCurrentValue(CBaseEntity* target, float value)
	{
		const float healthToGive = value - target->pev->health;

		return target->GiveHealth(healthToGive, DMG_GENERIC);
	}

	float GetMaximumValue(CBaseEntity* target) override { return -1; }

	float GetDefaultRechargeDelay() override { return g_pGameRules->HealthChargerRechargeTime(); }
};

LINK_ENTITY_TO_CLASS(func_healthcharger, CWallHealth);
