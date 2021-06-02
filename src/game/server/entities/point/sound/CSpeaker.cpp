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

#include "CSpeaker.hpp"

void CSpeaker::Spawn()
{
	const char* szSoundFile = STRING(pev->message);

	if (!m_preset && (IsStringNull(pev->message) || strlen(szSoundFile) < 1))
	{
		ALERT(at_error, "SPEAKER with no Level/Sentence! at: %f, %f, %f\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
		pev->nextthink = gpGlobals->time + 0.1;
		SetThink(&CSpeaker::SUB_Remove);
		return;
	}
	SetSolidType(Solid::Not);
	SetMovetype(Movetype::None);

	SetThink(&CSpeaker::SpeakerThink);
	pev->nextthink = 0.0;

	// allow on/off switching via 'use' function.

	SetUse(&CSpeaker::ToggleUse);

	Precache();
}

void CSpeaker::Precache()
{
	if (!IsBitSet(pev->spawnflags, SPEAKER_START_SILENT))
		// set first announcement time for random n second
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(5.0, 15.0);
}

void CSpeaker::SpeakerThink()
{
	// Wait for the talkmonster to finish first.
	if (gpGlobals->time <= CTalkMonster::g_talkWaitTime)
	{
		pev->nextthink = CTalkMonster::g_talkWaitTime + RANDOM_FLOAT(5, 10);
		return;
	}

	const char* szSoundFile = nullptr;

	if (m_preset)
	{
		// go lookup preset text, assign szSoundFile
		switch (m_preset)
		{
		case 1: szSoundFile = "C1A0_"; break;
		case 2: szSoundFile = "C1A1_"; break;
		case 3: szSoundFile = "C1A2_"; break;
		case 4: szSoundFile = "C1A3_"; break;
		case 5: szSoundFile = "C1A4_"; break;
		case 6: szSoundFile = "C2A1_"; break;
		case 7: szSoundFile = "C2A2_"; break;
		case 8: szSoundFile = "C2A3_"; break;
		case 9: szSoundFile = "C2A4_"; break;
		case 10: szSoundFile = "C2A5_"; break;
		case 11: szSoundFile = "C3A1_"; break;
		case 12: szSoundFile = "C3A2_"; break;
		}
	}
	else
		szSoundFile = STRING(pev->message);

	//No sound to play
	if (!szSoundFile)
	{
		return;
	}

	const float flvolume = pev->health * 0.1;
	const float flattenuation = 0.3;
	const int flags = 0;
	const int pitch = PITCH_NORM;

	if (szSoundFile[0] == '!')
	{
		// play single sentence, one shot
		UTIL_EmitAmbientSound(this, GetAbsOrigin(), szSoundFile,
			flvolume, flattenuation, flags, pitch);

		// shut off and reset
		pev->nextthink = 0.0;
	}
	else
	{
		// make random announcement from sentence group

		if (SENTENCEG_PlayRndSz(this, szSoundFile, flvolume, flattenuation, pitch, flags) < 0)
			ALERT(at_console, "Level Design Error!\nSPEAKER has bad sentence group name: %s\n", szSoundFile);

		// set next announcement time for random 5 to 10 minute delay
		pev->nextthink = gpGlobals->time +
			RANDOM_FLOAT(ANNOUNCE_MINUTES_MIN * 60.0, ANNOUNCE_MINUTES_MAX * 60.0);

		CTalkMonster::g_talkWaitTime = gpGlobals->time + 5;		// time delay until it's ok to speak: used so that two NPCs don't talk at once
	}
}

void CSpeaker::ToggleUse(const UseInfo& info)
{
	const bool fActive = (pev->nextthink > 0.0);

	// fActive is true only if an announcement is pending

	if (info.GetUseType() != UseType::Toggle)
	{
		// ignore if we're just turning something on that's already on, or
		// turning something off that's already off.
		if ((fActive && info.GetUseType() == UseType::On) || (!fActive && info.GetUseType() == UseType::Off))
			return;
	}

	if (info.GetUseType() == UseType::On)
	{
		// turn on announcements
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}

	if (info.GetUseType() == UseType::Off)
	{
		// turn off announcements
		pev->nextthink = 0.0;
		return;

	}

	// Toggle announcements
	if (fActive)
	{
		// turn off announcements
		pev->nextthink = 0.0;
	}
	else
	{
		// turn on announcements
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CSpeaker::KeyValue(KeyValueData* pkvd)
{
	// preset
	if (AreStringsEqual(pkvd->szKeyName, "preset"))
	{
		m_preset = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}
