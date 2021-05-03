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
*	precaches and defs for entities and other data that must always be available.
*/

#include "navigation/nodes.h"
#include "client.h"
#include "teamplay_gamerules.h"
#include "dll_functions.hpp"
#include "corpse.hpp"
#include "spawnpoints.hpp"

extern DLL_GLOBAL	int			gDisplayTitle;

LINK_ENTITY_TO_CLASS(worldspawn, CWorld);

constexpr int SF_WORLD_DARK = 0x0001;		//!< Fade from black at startup
constexpr int SF_WORLD_TITLE = 0x0002;		//!< Display game title at startup
constexpr int SF_WORLD_FORCETEAM = 0x0004;	//!< Force teams

extern DLL_GLOBAL bool		g_fGameOver;

CWorld::~CWorld()
{
	g_IsStartingNewMap = true;
}

void CWorld::Spawn()
{
	g_fGameOver = false;
	Precache();
}

void CWorld::Precache()
{
	g_pLastSpawn = nullptr;

#if 1
	CVAR_SET_STRING("sv_gravity", "800"); // 67ft/sec
	CVAR_SET_STRING("sv_stepsize", "18");
#else
	CVAR_SET_STRING("sv_gravity", "384"); // 32ft/sec
	CVAR_SET_STRING("sv_stepsize", "24");
#endif

	CVAR_SET_STRING("room_type", "0");// clear DSP

	// Set up game rules
	if (g_pGameRules)
	{
		delete g_pGameRules;
	}

	g_pGameRules = InstallGameRules();

	//!!!UNDONE why is there so much Spawn code in the Precache function? I'll just keep it here 

	///!!!LATER - do we want a sound ent in deathmatch? (sjb)
	//pSoundEnt = CBaseEntity::Create( "soundent", vec3_origin, vec3_origin, this );
	pSoundEnt = GetClassPtr((CSoundEnt*)nullptr);
	pSoundEnt->Spawn();

	if (!pSoundEnt)
	{
		ALERT(at_console, "**COULD NOT CREATE SOUNDENT**\n");
	}

	InitBodyQue();

	// init sentence group playback stuff from sentences.txt.
	// ok to call this multiple times, calls after first are ignored.

	SENTENCEG_Init();

	// init texture type array from materials.txt

	TEXTURETYPE_Init();


	// the area based ambient sounds MUST be the first precache_sounds

	// player precaches     
	W_Precache();									// get weapon precaches

	ClientPrecache();

	// sounds used from C physics code
	PRECACHE_SOUND("common/null.wav");				// clears sound channels

	PRECACHE_SOUND("items/suitchargeok1.wav");//!!! temporary sound for respawning weapons.
	PRECACHE_SOUND("items/gunpickup2.wav");// player picks up a gun.

	PRECACHE_SOUND("common/bodydrop3.wav");// dead bodies hitting the ground (animation events)
	PRECACHE_SOUND("common/bodydrop4.wav");

	g_Language = (int)CVAR_GET_FLOAT("sv_language");
	if (g_Language == LANGUAGE_GERMAN)
	{
		PRECACHE_MODEL("models/germangibs.mdl");
	}
	else
	{
		PRECACHE_MODEL("models/hgibs.mdl");
		PRECACHE_MODEL("models/agibs.mdl");
	}

	PRECACHE_SOUND("weapons/ric1.wav");
	PRECACHE_SOUND("weapons/ric2.wav");
	PRECACHE_SOUND("weapons/ric3.wav");
	PRECACHE_SOUND("weapons/ric4.wav");
	PRECACHE_SOUND("weapons/ric5.wav");
	//
	// Setup light animation tables. 'a' is total darkness, 'z' is maxbright.
	//

		// 0 normal
	LIGHT_STYLE(0, "m");

	// 1 FLICKER (first variety)
	LIGHT_STYLE(1, "mmnmmommommnonmmonqnmmo");

	// 2 SLOW STRONG PULSE
	LIGHT_STYLE(2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

	// 3 CANDLE (first variety)
	LIGHT_STYLE(3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

	// 4 FAST STROBE
	LIGHT_STYLE(4, "mamamamamama");

	// 5 GENTLE PULSE 1
	LIGHT_STYLE(5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");

	// 6 FLICKER (second variety)
	LIGHT_STYLE(6, "nmonqnmomnmomomno");

	// 7 CANDLE (second variety)
	LIGHT_STYLE(7, "mmmaaaabcdefgmmmmaaaammmaamm");

	// 8 CANDLE (third variety)
	LIGHT_STYLE(8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

	// 9 SLOW STROBE (fourth variety)
	LIGHT_STYLE(9, "aaaaaaaazzzzzzzz");

	// 10 FLUORESCENT FLICKER
	LIGHT_STYLE(10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	LIGHT_STYLE(11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

	// 12 UNDERWATER LIGHT MUTATION
	// this light only distorts the lightmap - no contribution
	// is made to the brightness of affected surfaces
	LIGHT_STYLE(12, "mmnnmmnnnmmnn");

	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	LIGHT_STYLE(63, "a");

	InitializeDecalsList();

	// init the WorldGraph.
	WorldGraph.InitGraph();

	// make sure the .NOD file is newer than the .BSP file.
	if (!WorldGraph.CheckNODFile(STRING(gpGlobals->mapname)))
	{// NOD file is not present, or is older than the BSP file.
		WorldGraph.AllocNodes();
	}
	else
	{// Load the node graph for this level
		if (!WorldGraph.LoadGraph(STRING(gpGlobals->mapname)))
		{// couldn't load, so alloc and prepare to build a graph.
			ALERT(at_console, "*Error opening .NOD file\n");
			WorldGraph.AllocNodes();
		}
		else
		{
			ALERT(at_console, "\n*Graph Loaded!\n");
		}
	}

	if (pev->speed > 0)
		CVAR_SET_FLOAT("sv_zmax", pev->speed);
	else
		CVAR_SET_FLOAT("sv_zmax", 4096);

	if (!IsStringNull(pev->netname))
	{
		ALERT(at_aiconsole, "Chapter title: %s\n", STRING(pev->netname));
		CBaseEntity* pEntity = CBaseEntity::Create("env_message", vec3_origin, vec3_origin, nullptr);
		if (pEntity)
		{
			pEntity->SetThink(&CBaseEntity::SUB_CallUseToggle);
			pEntity->pev->message = pev->netname;
			pev->netname = iStringNull;
			pEntity->pev->nextthink = gpGlobals->time + 0.3;
			pEntity->pev->spawnflags = SF_MESSAGE_ONCE;
		}
	}

	if (pev->spawnflags & SF_WORLD_DARK)
		CVAR_SET_FLOAT("v_dark", 1.0);
	else
		CVAR_SET_FLOAT("v_dark", 0.0);

	if (pev->spawnflags & SF_WORLD_TITLE)
		gDisplayTitle = true;		// display the game title if this key is set
	else
		gDisplayTitle = false;

	if (pev->spawnflags & SF_WORLD_FORCETEAM)
	{
		CVAR_SET_FLOAT("mp_defaultteam", 1);
	}
	else
	{
		CVAR_SET_FLOAT("mp_defaultteam", 0);
	}
}

void CWorld::KeyValue(KeyValueData* pkvd)
{
	//"wad" is ignored
	if (AreStringsEqual(pkvd->szKeyName, "skyname"))
	{
		// Sent over net now.
		CVAR_SET_STRING("sv_skyname", pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "sounds"))
	{
		gpGlobals->cdAudioTrack = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "WaveHeight"))
	{
		// Sent over net now.
		pev->scale = atof(pkvd->szValue) * (1.0 / 8.0);
		pkvd->fHandled = true;
		CVAR_SET_FLOAT("sv_wateramp", pev->scale);
	}
	else if (AreStringsEqual(pkvd->szKeyName, "MaxRange"))
	{
		pev->speed = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "chaptertitle"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "startdark"))
	{
		// UNDONE: This is a gross hack!!! The CVAR is NOT sent over the client/server link
		// but it will work for single player
		const int flag = atoi(pkvd->szValue);
		pkvd->fHandled = true;
		if (flag)
			pev->spawnflags |= SF_WORLD_DARK;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "newunit"))
	{
		// Single player only.  Clear save directory if set
		if (atoi(pkvd->szValue))
			CVAR_SET_FLOAT("sv_newunit", 1);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "gametitle"))
	{
		if (atoi(pkvd->szValue))
			pev->spawnflags |= SF_WORLD_TITLE;

		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "mapteams"))
	{
		pev->team = static_cast<int>(ALLOC_STRING(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "defaultteam"))
	{
		if (atoi(pkvd->szValue))
		{
			pev->spawnflags |= SF_WORLD_FORCETEAM;
		}
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

int CWorld::DamageDecal(int bitsDamageType)
{
	return DECAL_GUNSHOT1 + RANDOM_LONG(0, 4);
}
