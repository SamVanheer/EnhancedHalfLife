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

#include <limits>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "game.h"
#include "UserMessages.h"
#include "voice_gamemgr.h"

static char team_names[MAX_TEAMS][TEAM_NAME_LENGTH];
static int team_scores[MAX_TEAMS];
static int num_teams = 0;

extern DLL_GLOBAL bool		g_fGameOver;

CHalfLifeTeamplay::CHalfLifeTeamplay()
{
	m_DisableDeathMessages = false;
	m_DisableDeathPenalty = false;

	memset(team_names, 0, sizeof(team_names));
	memset(team_scores, 0, sizeof(team_scores));
	num_teams = 0;

	// Copy over the team from the server config
	m_szTeamList[0] = 0;

	// Cache this because the team code doesn't want to deal with changing this in the middle of a game
	safe_strcpy(m_szTeamList, teamlist.string);

	CBaseEntity* pWorld = UTIL_GetWorld();
	if (pWorld && pWorld->pev->team)
	{
		if (teamoverride.value)
		{
			const char* pTeamList = STRING(static_cast<string_t>(pWorld->pev->team));
			if (pTeamList && strlen(pTeamList))
			{
				safe_strcpy(m_szTeamList, pTeamList);
			}
		}
	}
	// Has the server set teams
	if (strlen(m_szTeamList))
		m_teamLimit = true;
	else
		m_teamLimit = false;

	RecountTeams();
}

void CHalfLifeTeamplay::Think()
{
	///// Check game rules /////
	static int last_frags;
	static int last_time;

	int frags_remaining = 0;
	int time_remaining = 0;

	g_VoiceGameMgr.Update(gpGlobals->frametime);

	if (g_fGameOver)   // someone else quit the game already
	{
		CHalfLifeMultiplay::Think();
		return;
	}

	float flTimeLimit = CVAR_GET_FLOAT("mp_timelimit") * 60;

	time_remaining = (int)(flTimeLimit ? (flTimeLimit - gpGlobals->time) : 0);

	if (flTimeLimit != 0 && gpGlobals->time >= flTimeLimit)
	{
		GoToIntermission();
		return;
	}

	float flFragLimit = fraglimit.value;
	if (flFragLimit)
	{
		int bestfrags = std::numeric_limits<int>::max();
		int remain;

		// check if any team is over the frag limit
		for (int i = 0; i < num_teams; i++)
		{
			if (team_scores[i] >= flFragLimit)
			{
				GoToIntermission();
				return;
			}

			remain = flFragLimit - team_scores[i];
			if (remain < bestfrags)
			{
				bestfrags = remain;
			}
		}
		frags_remaining = bestfrags;
	}

	// Updates when frags change
	if (frags_remaining != last_frags)
	{
		g_engfuncs.pfnCvar_DirectSet(&fragsleft, UTIL_VarArgs("%i", frags_remaining));
	}

	// Updates once per second
	if (timeleft.value != last_time)
	{
		g_engfuncs.pfnCvar_DirectSet(&timeleft, UTIL_VarArgs("%i", time_remaining));
	}

	last_frags = frags_remaining;
	last_time = time_remaining;
}

bool CHalfLifeTeamplay::ClientCommand(CBasePlayer* pPlayer, const char* pcmd)
{
	if (g_VoiceGameMgr.ClientCommand(pPlayer, pcmd))
		return true;

	if (AreStringsEqual(pcmd, "menuselect"))
	{
		if (CMD_ARGC() < 2)
			return true;

		int slot = atoi(CMD_ARGV(1));

		// select the item from the current menu

		return true;
	}

	return false;
}

void CHalfLifeTeamplay::UpdateGameMode(CBasePlayer* pPlayer)
{
	MESSAGE_BEGIN(MessageDest::One, gmsgGameMode, nullptr, pPlayer->edict());
	WRITE_BYTE(1);  // game mode teamplay
	MESSAGE_END();
}

const char* CHalfLifeTeamplay::SetDefaultPlayerTeam(CBasePlayer* pPlayer)
{
	// copy out the team name from the model
	char* mdls = g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "model");
	safe_strcpy(pPlayer->m_szTeamName, mdls);

	RecountTeams();

	// update the current player of the team he is joining
	if (pPlayer->m_szTeamName[0] == '\0' || !IsValidTeam(pPlayer->m_szTeamName) || defaultteam.value)
	{
		const char* pTeamName = nullptr;

		if (defaultteam.value)
		{
			pTeamName = team_names[0];
		}
		else
		{
			pTeamName = TeamWithFewestPlayers();
		}
		safe_strcpy(pPlayer->m_szTeamName, pTeamName);
	}

	return pPlayer->m_szTeamName;
}

void CHalfLifeTeamplay::InitHUD(CBasePlayer* pPlayer)
{
	int i;

	SetDefaultPlayerTeam(pPlayer);
	CHalfLifeMultiplay::InitHUD(pPlayer);

	// Send down the team names
	MESSAGE_BEGIN(MessageDest::One, gmsgTeamNames, nullptr, pPlayer->edict());
	WRITE_BYTE(num_teams);
	for (i = 0; i < num_teams; i++)
	{
		WRITE_STRING(team_names[i]);
	}
	MESSAGE_END();

	RecountTeams();

	char* mdls = g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "model");
	// update the current player of the team he is joining
	char text[1024];
	if (!strcmp(mdls, pPlayer->m_szTeamName))
	{
		snprintf(text, sizeof(text), "* you are on team \'%s\'\n", pPlayer->m_szTeamName);
	}
	else
	{
		snprintf(text, sizeof(text), "* assigned to team %s\n", pPlayer->m_szTeamName);
	}

	ChangePlayerTeam(pPlayer, pPlayer->m_szTeamName, false, false);
	UTIL_SayText(text, pPlayer);
	int clientIndex = pPlayer->entindex();
	RecountTeams();
	// update this player with all the other players team info
	// loop through all active players and send their team info to the new client
	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* plr = UTIL_PlayerByIndex(i);
		if (plr && IsValidTeam(plr->TeamID()))
		{
			MESSAGE_BEGIN(MessageDest::One, gmsgTeamInfo, nullptr, pPlayer->edict());
			WRITE_BYTE(plr->entindex());
			WRITE_STRING(plr->TeamID());
			MESSAGE_END();
		}
	}
}

void CHalfLifeTeamplay::ChangePlayerTeam(CBasePlayer* pPlayer, const char* pTeamName, bool bKill, bool bGib)
{
	int damageFlags = DMG_GENERIC;
	int clientIndex = pPlayer->entindex();

	if (!bGib)
	{
		damageFlags |= DMG_NEVERGIB;
	}
	else
	{
		damageFlags |= DMG_ALWAYSGIB;
	}

	if (bKill)
	{
		// kill the player,  remove a death,  and let them start on the new team
		m_DisableDeathMessages = true;
		m_DisableDeathPenalty = true;

		CBaseEntity* pWorld = UTIL_GetWorld();
		pPlayer->TakeDamage({pWorld, pWorld, 900, damageFlags});

		m_DisableDeathMessages = false;
		m_DisableDeathPenalty = false;
	}

	// copy out the team name from the model
	safe_strcpy(pPlayer->m_szTeamName, pTeamName);

	g_engfuncs.pfnSetClientKeyValue(clientIndex, g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "model", pPlayer->m_szTeamName);
	g_engfuncs.pfnSetClientKeyValue(clientIndex, g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "team", pPlayer->m_szTeamName);

	// notify everyone's HUD of the team change
	MESSAGE_BEGIN(MessageDest::All, gmsgTeamInfo);
	WRITE_BYTE(clientIndex);
	WRITE_STRING(pPlayer->m_szTeamName);
	MESSAGE_END();

	MESSAGE_BEGIN(MessageDest::All, gmsgScoreInfo);
	WRITE_BYTE(clientIndex);
	WRITE_SHORT(pPlayer->pev->frags);
	WRITE_SHORT(pPlayer->m_iDeaths);
	WRITE_SHORT(0);
	WRITE_SHORT(g_pGameRules->GetTeamIndex(pPlayer->m_szTeamName) + 1);
	MESSAGE_END();
}

void CHalfLifeTeamplay::ClientUserInfoChanged(CBasePlayer* pPlayer, char* infobuffer)
{
	char text[1024];

	// prevent skin/color/model changes
	char* mdls = g_engfuncs.pfnInfoKeyValue(infobuffer, "model");

	if (!stricmp(mdls, pPlayer->m_szTeamName))
		return;

	if (defaultteam.value)
	{
		int clientIndex = pPlayer->entindex();

		g_engfuncs.pfnSetClientKeyValue(clientIndex, g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "model", pPlayer->m_szTeamName);
		g_engfuncs.pfnSetClientKeyValue(clientIndex, g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "team", pPlayer->m_szTeamName);
		snprintf(text, sizeof(text), "* Not allowed to change teams in this game!\n");
		UTIL_SayText(text, pPlayer);
		return;
	}

	if (defaultteam.value || !IsValidTeam(mdls))
	{
		int clientIndex = pPlayer->entindex();

		g_engfuncs.pfnSetClientKeyValue(clientIndex, g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "model", pPlayer->m_szTeamName);
		snprintf(text, sizeof(text), "* Can't change team to \'%s\'\n", mdls);
		UTIL_SayText(text, pPlayer);
		snprintf(text, sizeof(text), "* Server limits teams to \'%s\'\n", m_szTeamList);
		UTIL_SayText(text, pPlayer);
		return;
	}
	// notify everyone of the team change
	snprintf(text, sizeof(text), "* %s has changed to team \'%s\'\n", STRING(pPlayer->pev->netname), mdls);
	UTIL_SayTextAll(text, pPlayer);

	UTIL_LogPrintf("\"%s<%i><%s><%s>\" joined team \"%s\"\n",
		STRING(pPlayer->pev->netname),
		GETPLAYERUSERID(pPlayer->edict()),
		GETPLAYERAUTHID(pPlayer->edict()),
		pPlayer->m_szTeamName,
		mdls);

	ChangePlayerTeam(pPlayer, mdls, true, true);
	// recound stuff
	RecountTeams(true);
}

void CHalfLifeTeamplay::DeathNotice(CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* pInflictor)
{
	if (m_DisableDeathMessages)
		return;

	if (pVictim && pKiller && pKiller->pev->flags & FL_CLIENT)
	{
		if ((pKiller != pVictim) && (PlayerRelationship(pVictim, pKiller) == GR_TEAMMATE))
		{
			MESSAGE_BEGIN(MessageDest::All, gmsgDeathMsg);
			WRITE_BYTE(pKiller->entindex());	// the killer
			WRITE_BYTE(pVictim->entindex());	// the victim
			WRITE_STRING("teammate");			// flag this as a teammate kill
			MESSAGE_END();
			return;
		}
	}

	CHalfLifeMultiplay::DeathNotice(pVictim, pKiller, pInflictor);
}

void CHalfLifeTeamplay::PlayerKilled(CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* pInflictor)
{
	if (!m_DisableDeathPenalty)
	{
		CHalfLifeMultiplay::PlayerKilled(pVictim, pKiller, pInflictor);
		RecountTeams();
	}
}

bool CHalfLifeTeamplay::IsTeamplay()
{
	return true;
}

bool CHalfLifeTeamplay::PlayerCanTakeDamage(CBasePlayer* pPlayer, CBaseEntity* pAttacker)
{
	if (pAttacker && PlayerRelationship(pPlayer, pAttacker) == GR_TEAMMATE)
	{
		// my teammate hit me.
		if ((friendlyfire.value == 0) && (pAttacker != pPlayer))
		{
			// friendly fire is off, and this hit came from someone other than myself,  then don't get hurt
			return false;
		}
	}

	return CHalfLifeMultiplay::PlayerCanTakeDamage(pPlayer, pAttacker);
}

int CHalfLifeTeamplay::PlayerRelationship(CBaseEntity* pPlayer, CBaseEntity* pTarget)
{
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if (!pPlayer || !pTarget || !pTarget->IsPlayer())
		return GR_NOTTEAMMATE;

	if ((*GetTeamID(pPlayer) != '\0') && (*GetTeamID(pTarget) != '\0') && !stricmp(GetTeamID(pPlayer), GetTeamID(pTarget)))
	{
		return GR_TEAMMATE;
	}

	return GR_NOTTEAMMATE;
}

bool CHalfLifeTeamplay::ShouldAutoAim(CBasePlayer* pPlayer, CBaseEntity* target)
{
	// always autoaim, unless target is a teammate
	if (target && target->IsPlayer())
	{
		if (PlayerRelationship(pPlayer, target) == GR_TEAMMATE)
			return false; // don't autoaim at teammates
	}

	return CHalfLifeMultiplay::ShouldAutoAim(pPlayer, target);
}

int CHalfLifeTeamplay::PointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled)
{
	if (!pKilled)
		return 0;

	if (!pAttacker)
		return 1;

	if (pAttacker != pKilled && PlayerRelationship(pAttacker, pKilled) == GR_TEAMMATE)
		return -1;

	return 1;
}

const char* CHalfLifeTeamplay::GetTeamID(CBaseEntity* pEntity)
{
	if (pEntity == nullptr || pEntity->pev == nullptr)
		return "";

	// return their team name
	return pEntity->TeamID();
}

int CHalfLifeTeamplay::GetTeamIndex(std::string_view teamName)
{
	if (!teamName.empty())
	{
		// try to find existing team
		for (int tm = 0; tm < num_teams; tm++)
		{
			if (UTIL_IEquals(team_names[tm], teamName))
				return tm;
		}
	}

	return -1;	// No match
}

const char* CHalfLifeTeamplay::GetIndexedTeamName(int teamIndex)
{
	if (teamIndex < 0 || teamIndex >= num_teams)
		return "";

	return team_names[teamIndex];
}

bool CHalfLifeTeamplay::IsValidTeam(const char* pTeamName)
{
	if (!m_teamLimit)	// Any team is valid if the teamlist isn't set
		return true;

	return GetTeamIndex(pTeamName) != -1;
}

const char* CHalfLifeTeamplay::TeamWithFewestPlayers()
{
	int i;
	int minPlayers = MAX_TEAMS;
	int teamCount[MAX_TEAMS];
	char* pTeamName = nullptr;

	memset(teamCount, 0, MAX_TEAMS * sizeof(int));

	// loop through all clients, count number of players on each team
	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* plr = UTIL_PlayerByIndex(i);

		if (plr)
		{
			int team = GetTeamIndex(plr->TeamID());
			if (team >= 0)
				teamCount[team] ++;
		}
	}

	// Find team with least players
	for (i = 0; i < num_teams; i++)
	{
		if (teamCount[i] < minPlayers)
		{
			minPlayers = teamCount[i];
			pTeamName = team_names[i];
		}
	}

	return pTeamName;
}

void CHalfLifeTeamplay::RecountTeams(bool bResendInfo)
{
	// loop through all teams, recounting everything
	num_teams = 0;

	const std::string_view teamList{m_szTeamList};

	std::size_t previousIndex = 0;
	std::size_t nextIndex = 0;

	while (true)
	{
		nextIndex = teamList.find(';', nextIndex);

		//Last entry
		if (nextIndex == std::string_view::npos)
		{
			nextIndex = teamList.length();
		}

		const auto teamName{teamList.substr(previousIndex, nextIndex - previousIndex)};

		//TODO: improve empty entry handling. Report as warning, maybe keep parsing?
		if (teamName.empty())
		{
			break;
		}

		if (GetTeamIndex(teamName) < 0)
		{
			safe_strcpy(team_names[num_teams], teamName);
			num_teams++;
		}

		if (nextIndex == teamList.length())
		{
			break;
		}

		//Advance past the delimiter
		++nextIndex;
		previousIndex = nextIndex;
	}

	if (num_teams < 2)
	{
		num_teams = 0;
		m_teamLimit = false;
	}

	// Sanity check
	memset(team_scores, 0, sizeof(team_scores));

	// loop through all clients
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* plr = UTIL_PlayerByIndex(i);

		if (plr)
		{
			const char* pTeamName = plr->TeamID();
			// try add to existing team
			int tm = GetTeamIndex(pTeamName);

			if (tm < 0) // no team match found
			{
				if (!m_teamLimit)
				{
					// add to new team
					tm = num_teams;
					num_teams++;
					team_scores[tm] = 0;
					safe_strcpy(team_names[tm], pTeamName);
				}
			}

			if (tm >= 0)
			{
				team_scores[tm] += plr->pev->frags;
			}

			if (bResendInfo) //Someone's info changed, let's send the team info again.
			{
				if (plr && IsValidTeam(plr->TeamID()))
				{
					MESSAGE_BEGIN(MessageDest::All, gmsgTeamInfo, nullptr);
					WRITE_BYTE(plr->entindex());
					WRITE_STRING(plr->TeamID());
					MESSAGE_END();
				}
			}
		}
	}
}
