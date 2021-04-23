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
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "items.h"
#include "gamerules.h"
#include "UserMessages.h"

class CHealthKit : public CItem
{
	void Spawn() override;
	void Precache() override;
	bool MyTouch(CBasePlayer* pPlayer) override;
};

LINK_ENTITY_TO_CLASS(item_healthkit, CHealthKit);

void CHealthKit::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_medkit.mdl");

	CItem::Spawn();
}

void CHealthKit::Precache()
{
	PRECACHE_MODEL("models/w_medkit.mdl");
	PRECACHE_SOUND("items/smallmedkit1.wav");
}

bool CHealthKit::MyTouch(CBasePlayer* pPlayer)
{
	if (pPlayer->pev->deadflag != DEAD_NO)
	{
		return false;
	}

	if (pPlayer->TakeHealth(gSkillData.healthkitCapacity, DMG_GENERIC))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, nullptr, pPlayer->pev);
		WRITE_STRING(STRING(pev->classname));
		MESSAGE_END();

		pPlayer->EmitSound(CHAN_ITEM, "items/smallmedkit1.wav");

		if (g_pGameRules->ItemShouldRespawn(this))
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);
		}

		return true;
	}

	return false;
}

enum class ChargerState
{
	Off,
	Starting,
	Charging
};

/**
*	@brief Wall mounted health kit
*/
class CWallHealth : public CBaseToggle
{
public:
	void Spawn() override;
	void Precache() override;
	void EXPORT Off();
	void EXPORT Recharge();
	void KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	int	ObjectCaps() override { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	float m_flNextCharge;
	int		m_iReactivate; //!< DeathMatch Delay until reactvated
	int		m_iJuice;
	ChargerState m_iOn;
	float   m_flSoundTime;
};

TYPEDESCRIPTION CWallHealth::m_SaveData[] =
{
	DEFINE_FIELD(CWallHealth, m_flNextCharge, FIELD_TIME),
	DEFINE_FIELD(CWallHealth, m_iReactivate, FIELD_INTEGER),
	DEFINE_FIELD(CWallHealth, m_iJuice, FIELD_INTEGER),
	DEFINE_FIELD(CWallHealth, m_iOn, FIELD_INTEGER),
	DEFINE_FIELD(CWallHealth, m_flSoundTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CWallHealth, CBaseEntity);

LINK_ENTITY_TO_CLASS(func_healthcharger, CWallHealth);

void CWallHealth::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "style") ||
		AreStringsEqual(pkvd->szKeyName, "height") ||
		AreStringsEqual(pkvd->szKeyName, "value1") ||
		AreStringsEqual(pkvd->szKeyName, "value2") ||
		AreStringsEqual(pkvd->szKeyName, "value3"))
	{
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "dmdelay"))
	{
		m_iReactivate = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CWallHealth::Spawn()
{
	Precache();

	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);		// set size and link into world
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	SET_MODEL(ENT(pev), STRING(pev->model));
	m_iJuice = gSkillData.healthchargerCapacity;
	pev->frame = 0;

}

void CWallHealth::Precache()
{
	PRECACHE_SOUND("items/medshot4.wav");
	PRECACHE_SOUND("items/medshotno1.wav");
	PRECACHE_SOUND("items/medcharge4.wav");
}

void CWallHealth::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Make sure that we have a caller
	if (!pActivator)
		return;
	// if it's not a player, ignore
	if (!pActivator->IsPlayer())
		return;

	// if there is no juice left, turn it off
	if (m_iJuice <= 0)
	{
		pev->frame = 1;
		Off();
	}

	// if the player doesn't have the suit, or there is no juice left, make the deny noise
	if ((m_iJuice <= 0) || (!(pActivator->pev->weapons & (1 << WEAPON_SUIT))))
	{
		if (m_flSoundTime <= gpGlobals->time)
		{
			m_flSoundTime = gpGlobals->time + 0.62;
			EmitSound(CHAN_ITEM, "items/medshotno1.wav");
		}
		return;
	}

	pev->nextthink = pev->ltime + 0.25;
	SetThink(&CWallHealth::Off);

	// Time to recharge yet?

	if (m_flNextCharge >= gpGlobals->time)
		return;

	// Play the on sound or the looping charging sound
	if (m_iOn == ChargerState::Off)
	{
		m_iOn = ChargerState::Starting;
		EmitSound(CHAN_ITEM, "items/medshot4.wav");
		m_flSoundTime = 0.56 + gpGlobals->time;
	}
	if ((m_iOn == ChargerState::Starting) && (m_flSoundTime <= gpGlobals->time))
	{
		m_iOn = ChargerState::Charging;
		EmitSound(CHAN_STATIC, "items/medcharge4.wav");
	}


	// charge the player
	if (pActivator->TakeHealth(1, DMG_GENERIC))
	{
		m_iJuice--;
	}

	// govern the rate of charge
	m_flNextCharge = gpGlobals->time + 0.1;
}

void CWallHealth::Recharge()
{
	EmitSound(CHAN_ITEM, "items/medshot4.wav");
	m_iJuice = gSkillData.healthchargerCapacity;
	pev->frame = 0;
	SetThink(&CWallHealth::SUB_DoNothing);
}

void CWallHealth::Off()
{
	// Stop looping sound.
	if (m_iOn == ChargerState::Charging)
		StopSound(CHAN_STATIC, "items/medcharge4.wav");

	m_iOn = ChargerState::Off;

	if ((!m_iJuice) && ((m_iReactivate = g_pGameRules->HealthChargerRechargeTime()) > 0))
	{
		pev->nextthink = pev->ltime + m_iReactivate;
		SetThink(&CWallHealth::Recharge);
	}
	else
		SetThink(&CWallHealth::SUB_DoNothing);
}
