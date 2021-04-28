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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBaseCharger.hpp"

TYPEDESCRIPTION CBaseCharger::m_SaveData[] =
{
	DEFINE_FIELD(CBaseCharger, m_flNextCharge, FIELD_TIME),
	DEFINE_FIELD(CBaseCharger, m_flSoundTime, FIELD_TIME),
	DEFINE_FIELD(CBaseCharger, m_State, FIELD_INTEGER),
	DEFINE_FIELD(CBaseCharger, m_flRechargeDelay, FIELD_FLOAT),
	DEFINE_FIELD(CBaseCharger, m_flChargeInterval, FIELD_FLOAT),
	DEFINE_FIELD(CBaseCharger, m_iCurrentCapacity, FIELD_INTEGER),
	DEFINE_FIELD(CBaseCharger, m_iPitch, FIELD_INTEGER),
	DEFINE_FIELD(CBaseCharger, m_iTotalCapacity, FIELD_INTEGER),
	DEFINE_FIELD(CBaseCharger, m_iszChargeOnSound, FIELD_STRING),
	DEFINE_FIELD(CBaseCharger, m_iszChargeLoopSound, FIELD_STRING),
	DEFINE_FIELD(CBaseCharger, m_iszRefuseChargeSound, FIELD_STRING),
	DEFINE_FIELD(CBaseCharger, m_iszRechargeSound, FIELD_STRING),
};

//TODO: was CBaseEntity before, needs investigating
IMPLEMENT_SAVERESTORE(CBaseCharger, CBaseToggle);

void CBaseCharger::KeyValue(KeyValueData* pkvd)
{
	//TODO: are these really needed?
	if (AreStringsEqual(pkvd->szKeyName, "style") ||
		AreStringsEqual(pkvd->szKeyName, "height") ||
		AreStringsEqual(pkvd->szKeyName, "value1") ||
		AreStringsEqual(pkvd->szKeyName, "value2") ||
		AreStringsEqual(pkvd->szKeyName, "value3"))
	{
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "recharge_delay"))
	{
		m_flRechargeDelay = std::max(0.0, atof(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "charge_per_use"))
	{
		//Constrained in Spawn
		m_iChargePerUse = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "charge_interval"))
	{
		//Constrained in Spawn
		m_flChargeInterval = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "initial_capacity"))
	{
		m_iCurrentCapacity = std::max(CHARGER_INFINITE_CAPACITY, atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "sound_pitch"))
	{
		m_iPitch = std::clamp(atoi(pkvd->szValue), 0, 255);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "total_capacity"))
	{
		//Allow a limited capacity charger to recharge to unlimited
		m_iTotalCapacity = std::max(CHARGER_INFINITE_CAPACITY, atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "charge_on_sound"))
	{
		m_iszChargeOnSound = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "charge_loop_sound"))
	{
		m_iszChargeLoopSound = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "refuse_charge_sound"))
	{
		m_iszRefuseChargeSound = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "recharge_sound"))
	{
		m_iszRechargeSound = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CBaseCharger::Spawn()
{
	Precache();

	pev->solid = Solid::BSP;
	pev->movetype = Movetype::Push;

	UTIL_SetOrigin(pev, pev->origin);		// set size and link into world
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (m_iCurrentCapacity == CHARGER_NOT_INITIALIZED)
	{
		m_iCurrentCapacity = m_iTotalCapacity;
	}

	//Ensure charge value is valid
	//Allow 0 to make cosmetic chargers
	m_iChargePerUse = std::max(0, m_iChargePerUse);

	//Allow at most continuous charging
	m_flChargeInterval = std::max(0.0f, m_flChargeInterval);

	CheckIfOutOfCharge();
}

void CBaseCharger::Precache()
{
	if (!IsStringNull(m_iszChargeOnSound))
	{
		PRECACHE_SOUND(STRING(m_iszChargeOnSound));
	}

	if (!IsStringNull(m_iszChargeLoopSound))
	{
		PRECACHE_SOUND(STRING(m_iszChargeLoopSound));
	}

	if (!IsStringNull(m_iszRefuseChargeSound))
	{
		PRECACHE_SOUND(STRING(m_iszRefuseChargeSound));
	}

	if (!IsStringNull(m_iszRechargeSound))
	{
		PRECACHE_SOUND(STRING(m_iszRechargeSound));
	}
}

void CBaseCharger::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Make sure that we have a caller
	// if it's not a player, ignore
	if (!pActivator || !pActivator->IsPlayer())
		return;

	CheckIfOutOfCharge();

	// if the player doesn't have the suit, or there is no juice left, make the deny noise
	if ((m_iCurrentCapacity == 0) || (!(pActivator->pev->weapons & (1 << WEAPON_SUIT))))
	{
		if (m_flSoundTime <= gpGlobals->time)
		{
			m_flSoundTime = gpGlobals->time + 0.62;

			if (!IsStringNull(m_iszRefuseChargeSound))
			{
				EmitSound(SoundChannel::Item, STRING(m_iszRefuseChargeSound), 0.85, ATTN_NORM, m_iPitch);
			}
		}
		return;
	}

	pev->nextthink = pev->ltime + 0.25;
	SetThink(&CBaseCharger::Off);

	// Time to recharge yet?
	if (m_flNextCharge >= gpGlobals->time)
		return;

	m_hActivator = pActivator;

	// Play the on sound or the looping charging sound
	if (m_State == ChargerState::Off)
	{
		m_State = ChargerState::Starting;

		if (!IsStringNull(m_iszChargeOnSound))
		{
			EmitSound(SoundChannel::Item, STRING(m_iszChargeOnSound), 0.85, ATTN_NORM, m_iPitch);
		}

		m_flSoundTime = 0.56 + gpGlobals->time;
	}
	if ((m_State == ChargerState::Starting) && (m_flSoundTime <= gpGlobals->time))
	{
		m_State = ChargerState::Charging;

		if (!IsStringNull(m_iszChargeLoopSound))
		{
			EmitSound(SoundChannel::Static, STRING(m_iszChargeLoopSound), 0.85, ATTN_NORM, m_iPitch);
		}
	}

	const float maximumValue = GetMaximumValue(pActivator);
	float currentValue = GetCurrentValue(pActivator);

	// charge the player
	if (maximumValue == -1 || currentValue < maximumValue)
	{
		const int chargeAvailable = m_iCurrentCapacity != CHARGER_INFINITE_CAPACITY ? std::min(m_iChargePerUse, m_iCurrentCapacity) : m_iChargePerUse;
		int maxChargeToGive;

		if (maximumValue == -1)
		{
			maxChargeToGive = chargeAvailable;
			currentValue += chargeAvailable;
		}
		else
		{
			//If the charge space left is a fractional value then we should overcharge by a unit to cap it off
			const int chargeSpaceLeft = static_cast<int>(std::ceil(maximumValue - currentValue));
			maxChargeToGive = std::min(chargeAvailable, chargeSpaceLeft);

			//Avoid rounding errors by clamping
			currentValue = std::min(maximumValue, currentValue + maxChargeToGive);
		}

		if (SetCurrentValue(pActivator, currentValue) && m_iCurrentCapacity != CHARGER_INFINITE_CAPACITY)
		{
			m_iCurrentCapacity -= maxChargeToGive;
		}
	}

	// govern the rate of charge
	m_flNextCharge = gpGlobals->time + m_flChargeInterval;
}

void CBaseCharger::Recharge()
{
	m_iCurrentCapacity = m_iTotalCapacity;
	pev->frame = 0;
	SetThink(&CBaseCharger::SUB_DoNothing);

	if (!IsStringNull(m_iszRechargeSound))
	{
		EmitSound(SoundChannel::Item, STRING(m_iszRechargeSound), 0.85, ATTN_NORM, m_iPitch);
	}
}

void CBaseCharger::Off()
{
	// Stop looping sound.
	if (m_State == ChargerState::Charging)
	{
		if (!IsStringNull(m_iszChargeLoopSound))
		{
			StopSound(SoundChannel::Static, STRING(m_iszChargeLoopSound));
		}
	}

	m_State = ChargerState::Off;

	if (m_flRechargeDelay == -1)
	{
		m_flRechargeDelay = GetDefaultRechargeDelay();
	}

	if (m_iCurrentCapacity == 0 && m_flRechargeDelay >= 0)
	{
		//Don't reset the think time, otherwise we'll never recharge on constant use input
		if (m_pfnThink != &CBaseCharger::Recharge)
		{
			//Delay can't be 0, so just make it a really small value
			pev->nextthink = pev->ltime + std::max(0.01f, m_flRechargeDelay);
			SetThink(&CBaseCharger::Recharge);
		}
	}
	else
		SetThink(&CBaseCharger::SUB_DoNothing);
}

void CBaseCharger::CheckIfOutOfCharge()
{
	// if there is no juice left, turn it off
	if (m_iCurrentCapacity == 0)
	{
		pev->frame = 1;
		Off();
	}
	else
	{
		pev->frame = 0;
	}
}