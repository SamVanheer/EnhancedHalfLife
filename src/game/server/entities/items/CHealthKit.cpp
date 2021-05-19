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

#include "CHealthKit.hpp"
#include "UserMessages.h"

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
	CItem::Precache();
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
