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

constexpr int SF_PLAYEREQUIP_USEONLY = 0x0001;
constexpr int MAX_EQUIP = 32;

/**
*	@brief Sets the default player equipment
*	@details Flag: USE Only
*/
class CGamePlayerEquip : public CRulePointEntity
{
public:
	void		KeyValue(KeyValueData* pkvd) override;
	void		Touch(CBaseEntity* pOther) override;
	void		Use(const UseInfo& info) override;

	inline bool	UseOnly() { return (pev->spawnflags & SF_PLAYEREQUIP_USEONLY) != 0; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

private:

	void		EquipPlayer(CBaseEntity* pPlayer);

	string_t m_weaponNames[MAX_EQUIP]{};
	int m_weaponCount[MAX_EQUIP]{};
};
