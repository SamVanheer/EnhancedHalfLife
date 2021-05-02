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

#include "changelevel.hpp"
#include "client.h"
#include "game.h"
#include "customentity.h"
#include "weaponinfo.h"
#include "usercmd.h"
#include "netadr.h"
#include "pm_shared.h"
#include "UserMessages.h"
#include "corpse.hpp"
#include "voice_gamemgr.h"
#include "entity_state.h"
#include "server_int.hpp"
#include "shared_interface/shared_interface.hpp"

extern DLL_GLOBAL uint32	g_ulModelIndexPlayer;
extern DLL_GLOBAL bool		g_fGameOver;
extern DLL_GLOBAL uint32	g_ulFrameCount;

extern bool giPrecacheGrunt;

qboolean ClientConnect(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128])
{
	return g_pGameRules->ClientConnected(pEntity, pszName, pszAddress, szRejectReason);

	// a client connecting during an intermission can cause problems
	//	if (intermission_running)
	//		ExitIntermission ();

}

void ClientDisconnect(edict_t* pEntity)
{
	if (g_fGameOver)
		return;

	char text[256] = "";
	if (!IsStringNull(pEntity->v.netname))
		snprintf(text, sizeof(text), "- %s has left the game\n", STRING(pEntity->v.netname));
	MESSAGE_BEGIN(MessageDest::All, gmsgSayText, nullptr);
	WRITE_BYTE(ENTINDEX(pEntity));
	WRITE_STRING(text);
	MESSAGE_END();

	// since this client isn't around to think anymore, reset their sound. 
	if (CSound* pSound = CSoundEnt::SoundPointerForIndex(CSoundEnt::ClientSoundIndex(pEntity)); pSound)
	{
		pSound->Reset();
	}

	// since the edict doesn't get deleted, fix it so it doesn't interfere.
	pEntity->v.takedamage = static_cast<int>(DamageMode::No);// don't attract autoaim
	pEntity->v.solid = Solid::Not;// nonsolid
	SET_ORIGIN(pEntity, pEntity->v.origin);

	if (auto pPlayer = reinterpret_cast<CBasePlayer*>(GET_PRIVATE(pEntity)); pPlayer)
	{
		if (pPlayer->m_pTank != nullptr)
		{
			pPlayer->m_pTank->Use({pPlayer, pPlayer, UseType::Off});
			pPlayer->m_pTank = nullptr;
		}
	}

	g_pGameRules->ClientDisconnected(pEntity);
}

// called by ClientKill and DeadThink
//TODO: this should really just be merged into PlayerDeathThink because on its own it makes little sense
void respawn(CBaseEntity* pEntity, bool fCopyCorpse)
{
	if (g_pGameRules->IsCoOp() || g_pGameRules->IsDeathmatch())
	{
		if (fCopyCorpse)
		{
			// make a copy of the dead body for appearances sake
			CopyToBodyQue(pEntity);
		}

		// respawn player
		pEntity->Spawn();
	}
	else
	{       // restart the entire server
		SERVER_COMMAND("reload\n");
	}
}

void ClientKill(edict_t* pEntity)
{
	CBasePlayer* pl = (CBasePlayer*)CBasePlayer::Instance(pEntity);

	if (pl->m_fNextSuicideTime > gpGlobals->time)
		return;  // prevent suiciding too ofter

	pl->m_fNextSuicideTime = gpGlobals->time + 1;  // don't let them suicide for 5 seconds after suiciding

	// have the player kill themself
	pl->pev->health = 0;
	pl->Killed({pl, GibType::Never});

	//	pl->pev->modelindex = g_ulModelIndexPlayer;
	//	pl->pev->frags -= 2;		// extra penalty
	//	respawn( pl );
}

void ClientPutInServer(edict_t* pEntity)
{
	CBasePlayer* pPlayer = GetClassPtr((CBasePlayer*)&pEntity->v);
	pPlayer->SetCustomDecalFrames(-1); // Assume none;

	// Allocate a CBasePlayer for pev, and call spawn
	pPlayer->Spawn();

	// Reset interpolation during first frame
	pPlayer->pev->effects |= EF_NOINTERP;

	pPlayer->pev->iuser1 = 0;	// disable any spec modes
	pPlayer->pev->iuser2 = 0;
}

/**
*	@brief String comes in as
*	say blah blah blah
*	or as
*	blah blah blah
*/
void Host_Say(CBasePlayer* player, bool teamonly)
{
	// We can get a raw string now, without the "say " prepended
	if (CMD_ARGC() == 0)
		return;

	//Not yet.
	if (player->m_flNextChatTime > gpGlobals->time)
		return;

	constexpr const char* cpSay = "say";
	constexpr const char* cpSayTeam = "say_team";
	const char* pcmd = CMD_ARGV(0);

	char* p;
	char szTemp[256];

	if (!stricmp(pcmd, cpSay) || !stricmp(pcmd, cpSayTeam))
	{
		if (CMD_ARGC() < 2)
		{
			// say with a blank message, nothing to do
			return;
		}

		p = const_cast<char*>(CMD_ARGS());
	}
	else  // Raw text, need to prepend argv[0]
	{
		if (CMD_ARGC() >= 2)
		{
			snprintf(szTemp, sizeof(szTemp), "%s %s", pcmd, CMD_ARGS());
		}
		else
		{
			// Just a one word command, use the first word...sigh
			snprintf(szTemp, sizeof(szTemp), "%s", pcmd);
		}
		p = szTemp;
	}

	// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[strlen(p) - 1] = '\0';
	}

	// make sure the text has content

	if (!p || !p[0] || !Q_UnicodeValidate(p))
		return;  // no character found, so say nothing

	// turn on color set 2  (color on,  no sound)
	char text[128];
	if (player->IsObserver() && teamonly)
		snprintf(text, sizeof(text), "%c(SPEC) %s: ", HUD_SAYTEXT_PRINTTALK, STRING(player->pev->netname));
	else if (teamonly)
		snprintf(text, sizeof(text), "%c(TEAM) %s: ", HUD_SAYTEXT_PRINTTALK, STRING(player->pev->netname));
	else
		snprintf(text, sizeof(text), "%c%s: ", HUD_SAYTEXT_PRINTTALK, STRING(player->pev->netname));

	const int j = sizeof(text) - 2 - strlen(text);  // -2 for \n and null terminator
	if ((int)strlen(p) > j)
		p[j] = '\0';

	safe_strcat(text, p);
	safe_strcat(text, "\n");

	player->m_flNextChatTime = gpGlobals->time + CHAT_INTERVAL;

	// loop through all players
	// Start with the first player.
	// This may return the world in single player if the client types something between levels or during spawn
	// so check it, or it will infinite loop

	CBasePlayer* client = nullptr;
	while (((client = (CBasePlayer*)UTIL_FindEntityByClassname(client, "player")) != nullptr) && (!IsNullEnt(client)))
	{
		if (!client->pev)
			continue;

		if (client == player)
			continue;

		if (!client->IsNetClient())	// Not a client ? (should never be true)
			continue;

		// can the receiver hear the sender? or has he muted him?
		if (g_VoiceGameMgr.PlayerHasBlockedPlayer(client, player))
			continue;

		if (!player->IsObserver() && teamonly && g_pGameRules->PlayerRelationship(client, player) != GR_TEAMMATE)
			continue;

		// Spectators can only talk to other specs
		if (player->IsObserver() && teamonly)
		{
			if (!client->IsObserver())
				continue;
		}

		MESSAGE_BEGIN(MessageDest::One, gmsgSayText, nullptr, client->pev);
		WRITE_BYTE(player->entindex());
		WRITE_STRING(text);
		MESSAGE_END();
	}

	// print to the sending client
	MESSAGE_BEGIN(MessageDest::One, gmsgSayText, nullptr, player->edict());
	WRITE_BYTE(player->entindex());
	WRITE_STRING(text);
	MESSAGE_END();

	// echo to server console
	g_engfuncs.pfnServerPrint(text);

	const char* const temp = teamonly ? cpSayTeam : cpSay;

	//TODO: these log printf calls always seem to follow the same format, so maybe add a varargs method to gamerules to log stuff this way
	// team match?
	if (g_pGameRules->IsTeamplay())
	{
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" %s \"%s\"\n",
			STRING(player->pev->netname),
			GETPLAYERUSERID(player->edict()),
			GETPLAYERAUTHID(player->edict()),
			g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(player->edict()), "model"),
			temp,
			p);
	}
	else
	{
		UTIL_LogPrintf("\"%s<%i><%s><%i>\" %s \"%s\"\n",
			STRING(player->pev->netname),
			GETPLAYERUSERID(player->edict()),
			GETPLAYERAUTHID(player->edict()),
			GETPLAYERUSERID(player->edict()),
			temp,
			p);
	}
}

// Use CMD_ARGV,  CMD_ARGV, and CMD_ARGC to get pointers the character string command.
void ClientCommand(edict_t* pEntity)
{
	// Is the client spawned yet?
	if (!pEntity->pvPrivateData)
		return;

	const char* pcmd = CMD_ARGV(0);
	const char* pstr;

	auto player = GetClassPtr<CBasePlayer>(reinterpret_cast<CBasePlayer*>(&pEntity->v));

	if (AreStringsEqual(pcmd, "say"))
	{
		Host_Say(player, false);
	}
	else if (AreStringsEqual(pcmd, "say_team"))
	{
		Host_Say(player, true);
	}
	else if (AreStringsEqual(pcmd, "fullupdate"))
	{
		player->ForceClientDllUpdate();
	}
	else if (AreStringsEqual(pcmd, "give"))
	{
		if (g_psv_cheats->value)
		{
			string_t iszItem = ALLOC_STRING(CMD_ARGV(1));	// Make a copy of the classname
			player->GiveNamedItem(STRING(iszItem));
		}
	}
	else if (AreStringsEqual(pcmd, "drop"))
	{
		// player is dropping an item. 
		player->DropPlayerItem(CMD_ARGV(1));
	}
	else if (AreStringsEqual(pcmd, "fov"))
	{
		if (g_psv_cheats->value && CMD_ARGC() > 1)
		{
			player->m_iFOV = atoi(CMD_ARGV(1));
		}
		else
		{
			CLIENT_PRINTF(pEntity, print_console, UTIL_VarArgs("\"fov\" is \"%d\"\n", (int)player->m_iFOV));
		}
	}
	else if (AreStringsEqual(pcmd, "use"))
	{
		player->SelectItem(CMD_ARGV(1));
	}
	else if (((pstr = strstr(pcmd, "weapon_")) != nullptr) && (pstr == pcmd))
	{
		player->SelectItem(pcmd);
	}
	else if (AreStringsEqual(pcmd, "lastinv"))
	{
		player->SelectLastItem();
	}
	else if (AreStringsEqual(pcmd, "spectate"))	// clients wants to become a spectator
	{
		// always allow proxies to become a spectator
		if ((player->pev->flags & FL_PROXY) || allow_spectators.value)
		{
			CBaseEntity* pSpawnSpot = g_pGameRules->GetPlayerSpawnSpot(player);
			player->StartObserver(player->pev->origin, pSpawnSpot->pev->angles);

			// notify other clients of player switching to spectator mode
			UTIL_ClientPrintAll(HUD_PRINTNOTIFY, UTIL_VarArgs("%s switched to spectator mode\n",
				(!IsStringNull(player->pev->netname) && STRING(player->pev->netname)[0] != 0) ? STRING(player->pev->netname) : "unconnected"));
		}
		else
			ClientPrint(player->pev, HUD_PRINTCONSOLE, "Spectator mode is disabled.\n");

	}
	else if (AreStringsEqual(pcmd, "specmode"))	// new spectator mode
	{
		if (player->IsObserver())
			player->Observer_SetMode(atoi(CMD_ARGV(1)));
	}
	else if (AreStringsEqual(pcmd, "closemenus"))
	{
		// just ignore it
	}
	else if (AreStringsEqual(pcmd, "follownext"))	// follow next player
	{
		if (player->IsObserver())
			player->Observer_FindNextPlayer(atoi(CMD_ARGV(1)) != 0);
	}
	else if (g_pGameRules->ClientCommand(player, pcmd))
	{
		// MenuSelect returns true only if the command is properly handled,  so don't print a warning
	}
	else
	{
		// tell the user they entered an unknown command
		char command[128];

		// check the length of the command (prevents crash)
		// max total length is 192 ...and we're adding a string below ("Unknown command: %s\n")
		safe_strcpy(command, pcmd);

		// tell the user they entered an unknown command
		ClientPrint(&pEntity->v, HUD_PRINTCONSOLE, UTIL_VarArgs("Unknown command: %s\n", command));
	}
}

void ClientUserInfoChanged(edict_t* pEntity, char* infobuffer)
{
	// Is the client spawned yet?
	if (!pEntity->pvPrivateData)
		return;

	// msg everyone if someone changes their name,  and it isn't the first time (changing no name to current name)
	if (!IsStringNull(pEntity->v.netname) && STRING(pEntity->v.netname)[0] != 0 && !AreStringsEqual(STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue(infobuffer, "name")))
	{
		char sName[256];
		char* pName = g_engfuncs.pfnInfoKeyValue(infobuffer, "name");
		safe_strcpy(sName, pName);

		// First parse the name and remove any %'s
		for (char* pApersand = sName; pApersand != nullptr && *pApersand != 0; pApersand++)
		{
			// Replace it with a space
			if (*pApersand == '%')
				*pApersand = ' ';
		}

		// Set the name
		g_engfuncs.pfnSetClientKeyValue(ENTINDEX(pEntity), infobuffer, "name", sName);

		if (gpGlobals->maxClients > 1)
		{
			char text[256];
			snprintf(text, sizeof(text), "* %s changed name to %s\n", STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue(infobuffer, "name"));
			MESSAGE_BEGIN(MessageDest::All, gmsgSayText, nullptr);
			WRITE_BYTE(ENTINDEX(pEntity));
			WRITE_STRING(text);
			MESSAGE_END();
		}

		// team match?
		if (g_pGameRules->IsTeamplay())
		{
			UTIL_LogPrintf("\"%s<%i><%s><%s>\" changed name to \"%s\"\n",
				STRING(pEntity->v.netname),
				GETPLAYERUSERID(pEntity),
				GETPLAYERAUTHID(pEntity),
				g_engfuncs.pfnInfoKeyValue(infobuffer, "model"),
				g_engfuncs.pfnInfoKeyValue(infobuffer, "name"));
		}
		else
		{
			UTIL_LogPrintf("\"%s<%i><%s><%i>\" changed name to \"%s\"\n",
				STRING(pEntity->v.netname),
				GETPLAYERUSERID(pEntity),
				GETPLAYERAUTHID(pEntity),
				GETPLAYERUSERID(pEntity),
				g_engfuncs.pfnInfoKeyValue(infobuffer, "name"));
		}
	}

	g_pGameRules->ClientUserInfoChanged(GetClassPtr((CBasePlayer*)&pEntity->v), infobuffer);
}

static bool g_serveractive = false;

void ServerDeactivate()
{
	// It's possible that the engine will call this function more times than is necessary
	//  Therefore, only run it one time for each call to ServerActivate 
	if (!g_serveractive)
	{
		return;
	}

	g_serveractive = false;

	// Peform any shutdown operations here...
	//
}

void ServerActivate(edict_t* pEdictList, int edictCount, int clientMax)
{
	// Every call to ServerActivate should be matched by a call to ServerDeactivate
	g_serveractive = true;

	// Clients have not been initialized yet
	for (int i = 0; i < edictCount; i++)
	{
		if (pEdictList[i].free)
			continue;

		// Clients aren't necessarily initialized until ClientPutInServer()
		if (i < clientMax || !pEdictList[i].pvPrivateData)
			continue;

		CBaseEntity* pClass = CBaseEntity::Instance(&pEdictList[i]);
		// Activate this entity if it's got a class & isn't dormant
		if (pClass && !(pClass->pev->flags & FL_DORMANT))
		{
			pClass->Activate();
		}
		else
		{
			ALERT(at_console, "Can't instance %s\n", STRING(pEdictList[i].v.classname));
		}
	}

	// Link user messages here to make sure first client can get them...
	LinkUserMessages();
}

void PlayerPreThink(edict_t* pEntity)
{
	if (CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pEntity); pPlayer)
		pPlayer->PreThink();
}

void PlayerPostThink(edict_t* pEntity)
{
	if (CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pEntity); pPlayer)
		pPlayer->PostThink();
}

void ParmsNewLevel()
{
}

void ParmsChangeLevel()
{
	// retrieve the pointer to the save data
	if (SAVERESTOREDATA* pSaveData = (SAVERESTOREDATA*)gpGlobals->pSaveData; pSaveData)
		pSaveData->connectionCount = CChangeLevel::BuildChangeList(pSaveData->levelList, MAX_LEVEL_CONNECTIONS);
}

void StartFrame()
{
	if (g_pGameRules)
		g_pGameRules->Think();

	if (g_fGameOver)
		return;

	gpGlobals->teamplay = teamplay.value;
	g_ulFrameCount++;
}

void ClientPrecache()
{
	// setup precaches always needed
	PRECACHE_SOUND("player/sprayer.wav");			// spray paint sound for PreAlpha

	// PRECACHE_SOUND("player/pl_jumpland2.wav");		// UNDONE: play 2x step sound

	PRECACHE_SOUND("player/pl_fallpain2.wav");
	PRECACHE_SOUND("player/pl_fallpain3.wav");

	PRECACHE_SOUND("player/pl_step1.wav");		// walk on concrete
	PRECACHE_SOUND("player/pl_step2.wav");
	PRECACHE_SOUND("player/pl_step3.wav");
	PRECACHE_SOUND("player/pl_step4.wav");

	PRECACHE_SOUND("common/npc_step1.wav");		// NPC walk on concrete
	PRECACHE_SOUND("common/npc_step2.wav");
	PRECACHE_SOUND("common/npc_step3.wav");
	PRECACHE_SOUND("common/npc_step4.wav");

	PRECACHE_SOUND("player/pl_metal1.wav");		// walk on metal
	PRECACHE_SOUND("player/pl_metal2.wav");
	PRECACHE_SOUND("player/pl_metal3.wav");
	PRECACHE_SOUND("player/pl_metal4.wav");

	PRECACHE_SOUND("player/pl_dirt1.wav");		// walk on dirt
	PRECACHE_SOUND("player/pl_dirt2.wav");
	PRECACHE_SOUND("player/pl_dirt3.wav");
	PRECACHE_SOUND("player/pl_dirt4.wav");

	PRECACHE_SOUND("player/pl_duct1.wav");		// walk in duct
	PRECACHE_SOUND("player/pl_duct2.wav");
	PRECACHE_SOUND("player/pl_duct3.wav");
	PRECACHE_SOUND("player/pl_duct4.wav");

	PRECACHE_SOUND("player/pl_grate1.wav");		// walk on grate
	PRECACHE_SOUND("player/pl_grate2.wav");
	PRECACHE_SOUND("player/pl_grate3.wav");
	PRECACHE_SOUND("player/pl_grate4.wav");

	PRECACHE_SOUND("player/pl_slosh1.wav");		// walk in shallow water
	PRECACHE_SOUND("player/pl_slosh2.wav");
	PRECACHE_SOUND("player/pl_slosh3.wav");
	PRECACHE_SOUND("player/pl_slosh4.wav");

	PRECACHE_SOUND("player/pl_tile1.wav");		// walk on tile
	PRECACHE_SOUND("player/pl_tile2.wav");
	PRECACHE_SOUND("player/pl_tile3.wav");
	PRECACHE_SOUND("player/pl_tile4.wav");
	PRECACHE_SOUND("player/pl_tile5.wav");

	PRECACHE_SOUND("player/pl_swim1.wav");		// breathe bubbles
	PRECACHE_SOUND("player/pl_swim2.wav");
	PRECACHE_SOUND("player/pl_swim3.wav");
	PRECACHE_SOUND("player/pl_swim4.wav");

	PRECACHE_SOUND("player/pl_ladder1.wav");	// climb ladder rung
	PRECACHE_SOUND("player/pl_ladder2.wav");
	PRECACHE_SOUND("player/pl_ladder3.wav");
	PRECACHE_SOUND("player/pl_ladder4.wav");

	PRECACHE_SOUND("player/pl_wade1.wav");		// wade in water
	PRECACHE_SOUND("player/pl_wade2.wav");
	PRECACHE_SOUND("player/pl_wade3.wav");
	PRECACHE_SOUND("player/pl_wade4.wav");

	PRECACHE_SOUND("debris/wood1.wav");			// hit wood texture
	PRECACHE_SOUND("debris/wood2.wav");
	PRECACHE_SOUND("debris/wood3.wav");

	PRECACHE_SOUND("plats/train_use1.wav");		// use a train

	PRECACHE_SOUND("buttons/spark5.wav");		// hit computer texture
	PRECACHE_SOUND("buttons/spark6.wav");
	PRECACHE_SOUND("debris/glass1.wav");
	PRECACHE_SOUND("debris/glass2.wav");
	PRECACHE_SOUND("debris/glass3.wav");

	PRECACHE_SOUND(SOUND_FLASHLIGHT_ON.data());
	PRECACHE_SOUND(SOUND_FLASHLIGHT_OFF.data());

	// player gib sounds
	PRECACHE_SOUND("common/bodysplat.wav");

	// player pain sounds
	PRECACHE_SOUND("player/pl_pain2.wav");
	PRECACHE_SOUND("player/pl_pain4.wav");
	PRECACHE_SOUND("player/pl_pain5.wav");
	PRECACHE_SOUND("player/pl_pain6.wav");
	PRECACHE_SOUND("player/pl_pain7.wav");

	PRECACHE_MODEL("models/player.mdl");

	// hud sounds

	PRECACHE_SOUND("common/wpn_hudoff.wav");
	PRECACHE_SOUND("common/wpn_hudon.wav");
	PRECACHE_SOUND("common/wpn_moveselect.wav");
	PRECACHE_SOUND("common/wpn_select.wav");
	PRECACHE_SOUND("common/wpn_denyselect.wav");

	// geiger sounds

	PRECACHE_SOUND("player/geiger6.wav");
	PRECACHE_SOUND("player/geiger5.wav");
	PRECACHE_SOUND("player/geiger4.wav");
	PRECACHE_SOUND("player/geiger3.wav");
	PRECACHE_SOUND("player/geiger2.wav");
	PRECACHE_SOUND("player/geiger1.wav");

	if (giPrecacheGrunt)
		UTIL_PrecacheOther("monster_human_grunt");
}

const char* GetGameDescription()
{
	if (g_pGameRules) // this function may be called before the world has spawned, and the game rules initialized
		return g_pGameRules->GetGameDescription();
	else
		return GAME_NAME.data();
}

void Sys_Error(const char* error_string)
{
	// Default case, do nothing.  MOD AUTHORS:  Add code ( e.g., _asm { int 3 }; here to cause a breakpoint for debugging your game .dlls
}

void PlayerCustomization(edict_t* pEntity, customization_t* pCust)
{
	CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(pEntity);

	if (!pPlayer)
	{
		ALERT(at_console, "PlayerCustomization:  Couldn't get player!\n");
		return;
	}

	if (!pCust)
	{
		ALERT(at_console, "PlayerCustomization:  NULL customization!\n");
		return;
	}

	switch (pCust->resource.type)
	{
	case t_decal:
		pPlayer->SetCustomDecalFrames(pCust->nUserData2); // Second int is max # of frames.
		break;
	case t_sound:
	case t_skin:
	case t_model:
		// Ignore for now.
		break;
	default:
		ALERT(at_console, "PlayerCustomization:  Unknown customization type!\n");
		break;
	}
}

// PAS and PVS routines for client messaging

void SetupVisibility(edict_t* pViewEntity, edict_t* pClient, unsigned char** pvs, unsigned char** pas)
{
	// Find the client's PVS
	edict_t* pView = pViewEntity ? pViewEntity : pClient;

	if (pClient->v.flags & FL_PROXY)
	{
		*pvs = nullptr;	// the spectator proxy sees
		*pas = nullptr;	// and hears everything
		return;
	}

	Vector org = pView->v.origin + pView->v.view_ofs;
	if (pView->v.flags & FL_DUCKING)
	{
		org = org + (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);
	}

	*pvs = ENGINE_SET_PVS(org);
	*pas = ENGINE_SET_PAS(org);
}

int AddToFullPack(entity_state_t* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet)
{
	// don't send if flagged for NODRAW and it's not the host getting the message
	if ((ent->v.effects & EF_NODRAW) &&
		(ent != host))
		return false;

	// Ignore ents without valid / visible models
	if (!ent->v.modelindex || !STRING(ent->v.model))
		return false;

	// Don't send spectators to other players
	if ((ent->v.flags & FL_SPECTATOR) && (ent != host))
	{
		return false;
	}

	// Ignore if not the host and not touching a PVS/PAS leaf
	// If pSet is nullptr, then the test will always succeed and the entity will be added to the update
	if (ent != host)
	{
		if (!ENGINE_CHECK_VISIBILITY(ent, pSet))
		{
			return false;
		}
	}

	// Don't send entity to local client if the client says it's predicting the entity itself.
	if (ent->v.flags & FL_SKIPLOCALHOST)
	{
		if ((hostflags & 1) && (ent->v.owner == host))
			return false;
	}

	if (host->v.groupinfo)
	{
		UTIL_SetGroupTrace(host->v.groupinfo, GROUP_OP_AND);

		// Should always be set, of course
		if (ent->v.groupinfo)
		{
			if (g_groupop == GROUP_OP_AND)
			{
				if (!(ent->v.groupinfo & host->v.groupinfo))
					return false;
			}
			else if (g_groupop == GROUP_OP_NAND)
			{
				if (ent->v.groupinfo & host->v.groupinfo)
					return false;
			}
		}

		UTIL_UnsetGroupTrace();
	}

	memset(state, 0, sizeof(*state));

	// Assign index so we can track this entity from frame to frame and
	//  delta from it.
	state->number = e;
	state->entityType = ENTITY_NORMAL;

	// Flag custom entities.
	if (ent->v.flags & FL_CUSTOMENTITY)
	{
		state->entityType = ENTITY_BEAM;
	}

	// 
	// Copy state data
	//

	// Round animtime to nearest millisecond
	state->animtime = (int)(1000.0 * ent->v.animtime) / 1000.0;

	memcpy(state->origin, ent->v.origin, 3 * sizeof(float));
	memcpy(state->angles, ent->v.angles, 3 * sizeof(float));
	memcpy(state->mins, ent->v.mins, 3 * sizeof(float));
	memcpy(state->maxs, ent->v.maxs, 3 * sizeof(float));

	memcpy(state->startpos, ent->v.startpos, 3 * sizeof(float));
	memcpy(state->endpos, ent->v.endpos, 3 * sizeof(float));

	state->impacttime = ent->v.impacttime;
	state->starttime = ent->v.starttime;

	state->modelindex = ent->v.modelindex;

	state->frame = ent->v.frame;

	state->skin = ent->v.skin;
	state->effects = ent->v.effects;

	// This non-player entity is being moved by the game .dll and not the physics simulation system
	//  make sure that we interpolate it's position on the client if it moves
	if (!player &&
		ent->v.animtime &&
		ent->v.velocity[0] == 0 &&
		ent->v.velocity[1] == 0 &&
		ent->v.velocity[2] == 0)
	{
		state->eflags |= EFLAG_SLERP;
	}

	state->scale = ent->v.scale;
	state->solid = static_cast<short>(ent->v.solid);
	state->colormap = ent->v.colormap;

	state->movetype = ent->v.movetype;
	state->sequence = ent->v.sequence;
	state->framerate = ent->v.framerate;
	state->body = ent->v.body;

	for (int i = 0; i < 4; i++)
	{
		state->controller[i] = ent->v.controller[i];
	}

	for (int i = 0; i < 2; i++)
	{
		state->blending[i] = ent->v.blending[i];
	}

	state->rendermode = ent->v.rendermode;
	state->renderamt = ent->v.renderamt;
	state->renderfx = ent->v.renderfx;
	state->rendercolor.r = ent->v.rendercolor.x;
	state->rendercolor.g = ent->v.rendercolor.y;
	state->rendercolor.b = ent->v.rendercolor.z;

	state->aiment = 0;
	if (ent->v.aiment)
	{
		state->aiment = ENTINDEX(ent->v.aiment);
	}

	state->owner = 0;
	if (ent->v.owner)
	{
		int owner = ENTINDEX(ent->v.owner);

		//TODO: add a helper function to verify that an index is a player index
		// Only care if owned by a player
		if (owner >= 1 && owner <= gpGlobals->maxClients)
		{
			state->owner = owner;
		}
	}

	// HACK:  Somewhat...
	// Class is overridden for non-players to signify a breakable glass object ( sort of a class? )
	if (!player)
	{
		state->playerclass = ent->v.playerclass;
	}

	// Special stuff for players only
	if (player)
	{
		memcpy(state->basevelocity, ent->v.basevelocity, 3 * sizeof(float));

		state->weaponmodel = MODEL_INDEX(STRING(ent->v.weaponmodel));
		state->gaitsequence = ent->v.gaitsequence;
		state->spectator = ent->v.flags & FL_SPECTATOR;
		state->friction = ent->v.friction;

		state->gravity = ent->v.gravity;
		//		state->team			= ent->v.team;
		//		
		state->usehull = static_cast<int>((ent->v.flags & FL_DUCKING) ? PlayerHull::Crouched : PlayerHull::Standing);
		state->health = ent->v.health;
	}

	return true;
}

void CreateBaseline(int player, int eindex, entity_state_t* baseline, edict_t* entity, int playermodelindex, Vector* player_mins, Vector* player_maxs)
{
	baseline->origin = entity->v.origin;
	baseline->angles = entity->v.angles;
	baseline->frame = entity->v.frame;
	baseline->skin = (short)entity->v.skin;

	// render information
	baseline->rendermode = entity->v.rendermode;
	baseline->renderamt = (byte)entity->v.renderamt;
	baseline->rendercolor.r = (byte)entity->v.rendercolor.x;
	baseline->rendercolor.g = (byte)entity->v.rendercolor.y;
	baseline->rendercolor.b = (byte)entity->v.rendercolor.z;
	baseline->renderfx = entity->v.renderfx;

	if (player)
	{
		baseline->mins = *player_mins;
		baseline->maxs = *player_maxs;

		baseline->colormap = eindex;
		baseline->modelindex = playermodelindex;
		baseline->friction = 1.0;
		baseline->movetype = Movetype::Walk;

		baseline->scale = entity->v.scale;
		baseline->solid = static_cast<short>(Solid::SlideBox);
		baseline->framerate = 1.0;
		baseline->gravity = 1.0;
	}
	else
	{
		baseline->mins = entity->v.mins;
		baseline->maxs = entity->v.maxs;

		baseline->colormap = 0;
		baseline->modelindex = entity->v.modelindex;//SV_ModelIndex(pr_strings + entity->v.model);
		baseline->movetype = entity->v.movetype;

		baseline->scale = entity->v.scale;
		baseline->solid = static_cast<short>(entity->v.solid);
		baseline->framerate = entity->v.framerate;
		baseline->gravity = entity->v.gravity;
	}
}

struct entity_field_alias_t
{
	char name[32];
	int	 field;
};

constexpr int FIELD_ORIGIN0 = 0;
constexpr int FIELD_ORIGIN1 = 1;
constexpr int FIELD_ORIGIN2 = 2;
constexpr int FIELD_ANGLES0 = 3;
constexpr int FIELD_ANGLES1 = 4;
constexpr int FIELD_ANGLES2 = 5;

static entity_field_alias_t entity_field_alias[] =
{
	{ "origin[0]", 0 },
	{ "origin[1]", 0 },
	{ "origin[2]", 0 },
	{ "angles[0]", 0 },
	{ "angles[1]", 0 },
	{ "angles[2]", 0 },
};

void Entity_FieldInit(delta_t* pFields)
{
	entity_field_alias[FIELD_ORIGIN0].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ORIGIN0].name);
	entity_field_alias[FIELD_ORIGIN1].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ORIGIN1].name);
	entity_field_alias[FIELD_ORIGIN2].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ORIGIN2].name);
	entity_field_alias[FIELD_ANGLES0].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ANGLES0].name);
	entity_field_alias[FIELD_ANGLES1].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ANGLES1].name);
	entity_field_alias[FIELD_ANGLES2].field = DELTA_FINDFIELD(pFields, entity_field_alias[FIELD_ANGLES2].name);
}

/**
*	@brief Callback for sending entity_state_t info over network.
*	FIXME:  Move to script
*/
void Entity_Encode(delta_t* pFields, const unsigned char* from, const unsigned char* to)
{
	static bool initialized = false;

	if (!initialized)
	{
		Entity_FieldInit(pFields);
		initialized = true;
	}

	auto f = (entity_state_t*)from;
	auto t = (entity_state_t*)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	const bool localplayer = (t->number - 1) == ENGINE_CURRENT_PLAYER();
	if (localplayer)
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}

	if ((t->impacttime != 0) && (t->starttime != 0))
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);

		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ANGLES0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ANGLES1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ANGLES2].field);
	}

	if ((t->movetype == Movetype::Follow) &&
		(t->aiment != 0))
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}
	else if (t->aiment != f->aiment)
	{
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}
}

static entity_field_alias_t player_field_alias[] =
{
	{ "origin[0]", 0 },
	{ "origin[1]", 0 },
	{ "origin[2]", 0 },
};

void Player_FieldInit(delta_t* pFields)
{
	player_field_alias[FIELD_ORIGIN0].field = DELTA_FINDFIELD(pFields, player_field_alias[FIELD_ORIGIN0].name);
	player_field_alias[FIELD_ORIGIN1].field = DELTA_FINDFIELD(pFields, player_field_alias[FIELD_ORIGIN1].name);
	player_field_alias[FIELD_ORIGIN2].field = DELTA_FINDFIELD(pFields, player_field_alias[FIELD_ORIGIN2].name);
}

/**
*	@brief Callback for sending entity_state_t for players info over network.
*/
void Player_Encode(delta_t* pFields, const unsigned char* from, const unsigned char* to)
{
	static bool initialized = false;

	if (!initialized)
	{
		Player_FieldInit(pFields);
		initialized = true;
	}

	auto f = (entity_state_t*)from;
	auto t = (entity_state_t*)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	const bool localplayer = (t->number - 1) == ENGINE_CURRENT_PLAYER();
	if (localplayer)
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}

	if ((t->movetype == Movetype::Follow) &&
		(t->aiment != 0))
	{
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}
	else if (t->aiment != f->aiment)
	{
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN0].field);
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN1].field);
		DELTA_SETBYINDEX(pFields, entity_field_alias[FIELD_ORIGIN2].field);
	}
}

constexpr int CUSTOMFIELD_ORIGIN0 = 0;
constexpr int CUSTOMFIELD_ORIGIN1 = 1;
constexpr int CUSTOMFIELD_ORIGIN2 = 2;
constexpr int CUSTOMFIELD_ANGLES0 = 3;
constexpr int CUSTOMFIELD_ANGLES1 = 4;
constexpr int CUSTOMFIELD_ANGLES2 = 5;
constexpr int CUSTOMFIELD_SKIN = 6;
constexpr int CUSTOMFIELD_SEQUENCE = 7;
constexpr int CUSTOMFIELD_ANIMTIME = 8;

entity_field_alias_t custom_entity_field_alias[] =
{
	{ "origin[0]",	0 },
	{ "origin[1]",	0 },
	{ "origin[2]",	0 },
	{ "angles[0]",	0 },
	{ "angles[1]",	0 },
	{ "angles[2]",	0 },
	{ "skin",		0 },
	{ "sequence",	0 },
	{ "animtime",	0 },
};

void Custom_Entity_FieldInit(delta_t* pFields)
{
	custom_entity_field_alias[CUSTOMFIELD_ORIGIN0].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN0].name);
	custom_entity_field_alias[CUSTOMFIELD_ORIGIN1].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN1].name);
	custom_entity_field_alias[CUSTOMFIELD_ORIGIN2].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN2].name);
	custom_entity_field_alias[CUSTOMFIELD_ANGLES0].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES0].name);
	custom_entity_field_alias[CUSTOMFIELD_ANGLES1].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES1].name);
	custom_entity_field_alias[CUSTOMFIELD_ANGLES2].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES2].name);
	custom_entity_field_alias[CUSTOMFIELD_SKIN].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_SKIN].name);
	custom_entity_field_alias[CUSTOMFIELD_SEQUENCE].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_SEQUENCE].name);
	custom_entity_field_alias[CUSTOMFIELD_ANIMTIME].field = DELTA_FINDFIELD(pFields, custom_entity_field_alias[CUSTOMFIELD_ANIMTIME].name);
}

/**
*	@brief Callback for sending entity_state_t info ( for custom entities ) over network.
*	FIXME:  Move to script
*/
void Custom_Encode(delta_t* pFields, const unsigned char* from, const unsigned char* to)
{
	static bool initialized = false;

	if (!initialized)
	{
		Custom_Entity_FieldInit(pFields);
		initialized = true;
	}

	auto f = (entity_state_t*)from;
	auto t = (entity_state_t*)to;

	const int beamType = static_cast<int>(t->rendermode) & 0x0f;

	if (beamType != BEAM_POINTS && beamType != BEAM_ENTPOINT)
	{
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN0].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN1].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN2].field);
	}

	if (beamType != BEAM_POINTS)
	{
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES0].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES1].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES2].field);
	}

	if (beamType != BEAM_ENTS && beamType != BEAM_ENTPOINT)
	{
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_SKIN].field);
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_SEQUENCE].field);
	}

	// animtime is compared by rounding first
	// see if we really shouldn't actually send it
	if ((int)f->animtime == (int)t->animtime)
	{
		DELTA_UNSETBYINDEX(pFields, custom_entity_field_alias[CUSTOMFIELD_ANIMTIME].field);
	}
}

void RegisterEncoders()
{
	DELTA_ADDENCODER("Entity_Encode", Entity_Encode);
	DELTA_ADDENCODER("Custom_Encode", Custom_Encode);
	DELTA_ADDENCODER("Player_Encode", Player_Encode);
}

int GetWeaponData(edict_t* player, weapon_data_t* info)
{
	memset(info, 0, MAX_WEAPONS * sizeof(weapon_data_t));

#if defined( CLIENT_WEAPONS )
	CBasePlayer* pl = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(player));

	if (!pl)
		return true;

	// go through all of the weapons and make a list of the ones to pack
	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (CBasePlayerItem* pPlayerItem = pl->m_hPlayerItems[i]; pPlayerItem)
		{
			// there's a weapon here. Should I pack it?
			while (pPlayerItem)
			{
				if (CBasePlayerWeapon* gun = pPlayerItem->GetWeaponPtr(); gun && gun->UseDecrement())
				{
					// Get The ID.
					ItemInfo II{};
					gun->GetItemInfo(&II);

					if (II.iId >= 0 && II.iId < MAX_WEAPONS)
					{
						weapon_data_t* item = &info[II.iId];

						item->m_iId = II.iId;
						item->m_iClip = gun->m_iClip;

						item->m_flTimeWeaponIdle = std::max(gun->m_flTimeWeaponIdle, -0.001f);
						item->m_flNextPrimaryAttack = std::max(gun->m_flNextPrimaryAttack, -0.001f);
						item->m_flNextSecondaryAttack = std::max(gun->m_flNextSecondaryAttack, -0.001f);
						item->m_fInReload = gun->m_fInReload;
						item->fuser1 = std::max(gun->pev->fuser1, -0.001f);

						gun->GetWeaponData(*item);

						// item->m_flPumpTime = std::max( gun->m_flPumpTime, -0.001f );
					}
				}
				pPlayerItem = pPlayerItem->m_hNext;
			}
		}
	}
#endif
	return true;
}

void UpdateClientData(const edict_t* ent, int sendweapons, clientdata_t* cd)
{
	if (!ent || !ent->pvPrivateData)
		return;
	entvars_t* pev = const_cast<entvars_t*>(&ent->v);
	//TODO: make this static_cast, the above private data check guards against it
	CBasePlayer* pl = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pev));
	entvars_t* pevOrg = pev;

	// if user is spectating different player in First person, override some vars
	if (pl && pl->pev->iuser1 == OBS_IN_EYE)
	{
		if (pl->m_hObserverTarget)
		{
			pev = pl->m_hObserverTarget->pev;
			//TODO: if pl is null here the target player isn't valid and shouldn't be spectated
			pl = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pev));
		}
	}

	cd->flags = pev->flags;
	cd->health = pev->health;

	cd->viewmodel = MODEL_INDEX(STRING(pev->viewmodel));

	cd->waterlevel = pev->waterlevel;
	cd->watertype = pev->watertype;
	cd->weapons = pev->weapons;

	// Vectors
	cd->origin = pev->origin;
	cd->velocity = pev->velocity;
	cd->view_ofs = pev->view_ofs;
	cd->punchangle = pev->punchangle;

	cd->bInDuck = pev->bInDuck;
	cd->flTimeStepSound = pev->flTimeStepSound;
	cd->flDuckTime = pev->flDuckTime;
	cd->flSwimTime = pev->flSwimTime;
	cd->waterjumptime = pev->teleport_time;

	safe_strcpy(cd->physinfo, ENGINE_GETPHYSINFO(ent));

	cd->maxspeed = pev->maxspeed;
	cd->fov = pl->m_iFOV;
	cd->weaponanim = pev->weaponanim;

	cd->pushmsec = pev->pushmsec;

	//Spectator mode
	// don't use spec vars from chased player
	cd->iuser1 = pevOrg->iuser1;
	cd->iuser2 = pevOrg->iuser2;

#if defined( CLIENT_WEAPONS )
	if (sendweapons)
	{
		if (pl)
		{
			cd->m_flNextAttack = pl->m_flNextAttack;
			cd->vuser1.x = pl->GetAmmoCount("9mm");
			cd->vuser1.y = pl->GetAmmoCount("357");
			cd->vuser1.z = pl->GetAmmoCount("ARgrenades");
			cd->ammo_nails = pl->GetAmmoCount("bolts");
			cd->ammo_shells = pl->GetAmmoCount("buckshot");
			cd->ammo_rockets = pl->GetAmmoCount("rockets");
			cd->ammo_cells = pl->GetAmmoCount("uranium");
			cd->vuser2.x = pl->GetAmmoCount("Hornets");

			if (pl->m_hActiveItem)
			{
				if (CBasePlayerWeapon* gun = pl->m_hActiveItem->GetWeaponPtr(); gun && gun->UseDecrement())
				{
					ItemInfo II{};
					gun->GetItemInfo(&II);

					cd->m_iId = II.iId;

					cd->vuser3.z = gun->m_iSecondaryAmmoType;
					cd->vuser4.x = gun->m_iPrimaryAmmoType;
					cd->vuser4.y = pl->m_rgAmmo[gun->m_iPrimaryAmmoType];
					cd->vuser4.z = pl->m_rgAmmo[gun->m_iSecondaryAmmoType];

					if (gun->m_iId == WEAPON_RPG)
					{
						cd->vuser2.y = ((CRpg*)gun)->m_fSpotActive;
						cd->vuser2.z = ((CRpg*)gun)->m_cActiveRockets;
					}
				}
			}
		}
	}
#endif
}

void CmdStart(const edict_t* player, const usercmd_t* cmd, unsigned int random_seed)
{
	CBasePlayer* pl = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(const_cast<edict_t*>(player)));

	if (!pl)
		return;

	if (pl->pev->groupinfo != 0)
	{
		UTIL_SetGroupTrace(pl->pev->groupinfo, GROUP_OP_AND);
	}

	pl->random_seed = random_seed;
}

void CmdEnd(const edict_t* player)
{
	CBasePlayer* pl = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(const_cast<edict_t*>(player)));

	if (!pl)
		return;
	if (pl->pev->groupinfo != 0)
	{
		UTIL_UnsetGroupTrace();
	}
}

int	ConnectionlessPacket(const netadr_t* net_from, const char* args, char* response_buffer, int* response_buffer_size)
{
	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return false;
}

int GetHullBounds(int hullnumber, Vector* mins, Vector* maxs)
{
	return Shared_GetHullBounds(hullnumber, *mins, *maxs);
}

void CreateInstancedBaselines()
{
	int iret = 0;
	entity_state_t state{};

	// Create any additional baselines here for things like grendates, etc.
	// iret = ENGINE_INSTANCE_BASELINE( pc->pev->classname, &state );

	// Destroy objects.
	//UTIL_Remove( pc );
}

int	InconsistentFile(const edict_t* player, const char* filename, char* disconnect_message)
{
	// Server doesn't care?
	if (CVAR_GET_FLOAT("mp_consistency") != 1)
		return false;

	// Default behavior is to kick the player
	snprintf(disconnect_message, MAX_INCONSISTENT_FILE_MESSAGE_LENGTH, "Server is enforcing file consistency for %s\n", filename);

	// Kick now with specified disconnect message.
	return true;
}

int AllowLagCompensation()
{
	return true;
}
