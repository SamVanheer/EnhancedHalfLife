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

#include "CBaseCharger.hpp"
#include "CWallHealth.generated.hpp"

/**
*	@brief Wall mounted health kit
*/
class EHL_CLASS(EntityName=func_healthcharger) CWallHealth : public CBaseCharger
{
	EHL_GENERATED_BODY()

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
