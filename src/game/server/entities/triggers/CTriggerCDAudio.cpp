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

#include "CTriggerCDAudio.hpp"

LINK_ENTITY_TO_CLASS(trigger_cdaudio, CTriggerCDAudio);

// !!!HACK - overloaded HEALTH to avoid adding new field
void CTriggerCDAudio::Touch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{// only clients may trigger these events
		return;
	}

	PlayTrack();
}

void CTriggerCDAudio::Spawn()
{
	InitTrigger();
}

void CTriggerCDAudio::Use(const UseInfo& info)
{
	PlayTrack();
}

void CTriggerCDAudio::PlayTrack()
{
	PlayCDTrack((int)pev->health);

	SetTouch(nullptr);
	UTIL_Remove(this);
}

void PlayCDTrack(int iTrack)
{
	if (iTrack < -1 || iTrack > 30)
	{
		ALERT(at_console, "TriggerCDAudio - Track %d out of range\n");
		return;
	}

	char string[64];

	if (iTrack == -1)
	{
		safe_strcpy(string, "cd stop\n");
	}
	else
	{
		snprintf(string, sizeof(string), "cd play %3d\n", iTrack);
	}

	for (auto player : UTIL_AllPlayers())
	{
		CLIENT_COMMAND(player->edict(), string);
	}
}
