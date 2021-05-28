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

#include "CMessage.hpp"

LINK_ENTITY_TO_CLASS(env_message, CMessage);

void CMessage::Spawn()
{
	Precache();

	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);

	switch (pev->impulse)
	{
	case 1: // Medium radius
		pev->speed = ATTN_STATIC;
		break;

	case 2:	// Large radius
		pev->speed = ATTN_NORM;
		break;

	case 3:	//EVERYWHERE
		pev->speed = ATTN_NONE;
		break;

	default:
	case 0: // Small radius
		pev->speed = ATTN_IDLE;
		break;
	}
	pev->impulse = 0;

	// No volume, use normal
	if (pev->scale <= 0)
		pev->scale = 1.0;
}

void CMessage::Precache()
{
	if (!IsStringNull(m_iszMessageSound))
		PRECACHE_SOUND(STRING(m_iszMessageSound));
}

void CMessage::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "messagesound"))
	{
		m_iszMessageSound = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "messagevolume"))
	{
		pev->scale = atof(pkvd->szValue) * 0.1;
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "messageattenuation"))
	{
		pev->impulse = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CMessage::Use(const UseInfo& info)
{
	if (pev->spawnflags & SF_MESSAGE_ALL)
		UTIL_ShowMessageAll(STRING(pev->message));
	else
	{
		CBasePlayer* pPlayer = nullptr;

		if (auto activator = info.GetActivator(); activator && activator->IsPlayer())
			pPlayer = static_cast<CBasePlayer*>(activator);
		else if (!g_pGameRules->IsMultiplayer())
		{
			pPlayer = UTIL_GetLocalPlayer();
		}
		if (pPlayer)
			UTIL_ShowMessage(STRING(pev->message), static_cast<CBasePlayer*>(pPlayer));
	}
	if (!IsStringNull(m_iszMessageSound))
	{
		EmitSound(SoundChannel::Body, STRING(m_iszMessageSound), pev->scale, pev->speed);
	}
	if (pev->spawnflags & SF_MESSAGE_ONCE)
		UTIL_Remove(this);

	SUB_UseTargets(this, UseType::Toggle, 0);
}
