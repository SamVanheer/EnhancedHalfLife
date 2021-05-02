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

/**
*	@file
*
*	battery-related code
*/

#include "CBaseCharger.hpp"

class CRecharge : public CBaseCharger
{
public:
	CRecharge()
	{
		m_iTotalCapacity = gSkillData.suitchargerCapacity;
		m_iszChargeOnSound = MAKE_STRING("items/suitchargeok1.wav");
		m_iszChargeLoopSound = MAKE_STRING("items/suitcharge1.wav");
		m_iszRefuseChargeSound = MAKE_STRING("items/suitchargeno1.wav");
	}

protected:
	float GetCurrentValue(CBaseEntity* target) override
	{
		return target->pev->armorvalue;
	}

	bool SetCurrentValue(CBaseEntity* target, float value)
	{
		target->pev->armorvalue = value;
		return true;
	}

	float GetMaximumValue(CBaseEntity* target) override { return 100; }

	float GetDefaultRechargeDelay() override { return g_pGameRules->HEVChargerRechargeTime(); }
};

LINK_ENTITY_TO_CLASS(func_recharge, CRecharge);
