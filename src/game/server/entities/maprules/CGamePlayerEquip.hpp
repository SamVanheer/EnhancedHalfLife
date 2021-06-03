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

#include "CRulePointEntity.hpp"
#include "CGamePlayerEquip.generated.hpp"

constexpr int SF_PLAYEREQUIP_USEONLY = 0x0001;
constexpr int MAX_EQUIP = 32;

/**
*	@brief Sets the default player equipment
*	@details Flag: USE Only
*/
class EHL_CLASS("EntityName": "game_player_equip") CGamePlayerEquip : public CRulePointEntity
{
	EHL_GENERATED_BODY()

public:
	void		KeyValue(KeyValueData* pkvd) override;
	void		Touch(CBaseEntity* pOther) override;
	void		Use(const UseInfo& info) override;

	inline bool	UseOnly() { return (pev->spawnflags & SF_PLAYEREQUIP_USEONLY) != 0; }

private:

	void		EquipPlayer(CBaseEntity* pPlayer);

	EHL_FIELD("Persisted": true)
	string_t m_weaponNames[MAX_EQUIP]{};

	EHL_FIELD("Persisted": true)
	int m_weaponCount[MAX_EQUIP]{};
};
