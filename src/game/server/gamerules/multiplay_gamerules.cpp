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

#include "gamerules.h"

#include "game.h"
#include "CBaseItem.hpp"
#include "items.h"
#include "voice_gamemgr.h"
#include "hltv.h"
#include "UserMessages.h"
#include "mapcycle.hpp"

extern DLL_GLOBAL bool	g_fGameOver;

constexpr int ITEM_RESPAWN_TIME = 30;
constexpr int WEAPON_RESPAWN_TIME = 20;
constexpr int AMMO_RESPAWN_TIME = 20;

float g_flIntermissionStartTime = 0;

class CMultiplayGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	bool		CanPlayerHearPlayer(CBasePlayer* pListener, CBasePlayer* pTalker) override
	{
		if (g_pGameRules->IsTeamplay())
		{
			if (g_pGameRules->PlayerRelationship(pListener, pTalker) != GR_TEAMMATE)
			{
				return false;
			}
		}

		return true;
	}
};

static CMultiplayGameMgrHelper g_GameMgrHelper;

CHalfLifeMultiplay::CHalfLifeMultiplay()
{
	g_VoiceGameMgr.Init(&g_GameMgrHelper, gpGlobals->maxClients);

	RefreshSkillData();
	m_flIntermissionEndTime = 0;
	g_flIntermissionStartTime = 0;

	// 11/8/98
	// Modified by YWB:  Server .cfg file is now a cvar, so that 
	//  server ops can run multiple game servers, with different server .cfg files,
	//  from a single installed directory.
	// Mapcyclefile is already a cvar.

	// 3/31/99
	// Added lservercfg file cvar, since listen and dedicated servers should not
	// share a single config file. (sjb)
	if (IS_DEDICATED_SERVER())
	{
		// this code has been moved into engine, to only run server.cfg once
	}
	else
	{
		// listen server
		const char* lservercfgfile = CVAR_GET_STRING("lservercfgfile");

		if (lservercfgfile && lservercfgfile[0])
		{
			char szCommand[256];

			ALERT(at_console, "Executing listen server config file\n");
			snprintf(szCommand, sizeof(szCommand), "exec %s\n", lservercfgfile);
			SERVER_COMMAND(szCommand);
		}
	}
}

bool CHalfLifeMultiplay::ClientCommand(CBasePlayer* pPlayer, const char* pcmd)
{
	if (g_VoiceGameMgr.ClientCommand(pPlayer, pcmd))
		return true;

	return CGameRules::ClientCommand(pPlayer, pcmd);
}

void CHalfLifeMultiplay::ClientUserInfoChanged(CBasePlayer* pPlayer, char* infobuffer)
{
	pPlayer->SetPrefsFromUserinfo(infobuffer);
}

void CHalfLifeMultiplay::RefreshSkillData()
{
	// load all default values
	CGameRules::RefreshSkillData();

	// override some values for multiplay.

		// suitcharger
	gSkillData.suitchargerCapacity = 30;

	// Crowbar whack
	gSkillData.plrDmgCrowbar = 25;

	// Glock Round
	gSkillData.plrDmg9MM = 12;

	// 357 Round
	gSkillData.plrDmg357 = 40;

	// MP5 Round
	gSkillData.plrDmgMP5 = 12;

	// M203 grenade
	gSkillData.plrDmgM203Grenade = 100;

	// Shotgun buckshot
	gSkillData.plrDmgBuckshot = 20;// fewer pellets in deathmatch

	// Crossbow
	gSkillData.plrDmgCrossbowClient = 20;

	// RPG
	gSkillData.plrDmgRPG = 120;

	// Egon
	gSkillData.plrDmgEgonWide = 20;
	gSkillData.plrDmgEgonNarrow = 10;

	// Hand Grendade
	gSkillData.plrDmgHandGrenade = 100;

	// Satchel Charge
	gSkillData.plrDmgSatchel = 120;

	// Tripmine
	gSkillData.plrDmgTripmine = 150;

	// hornet
	gSkillData.plrDmgHornet = 10;
}

// longest the intermission can last, in seconds
constexpr int MAX_INTERMISSION_TIME = 120;

void CHalfLifeMultiplay::Think()
{
	g_VoiceGameMgr.Update(gpGlobals->frametime);

	///// Check game rules /////
	static int last_frags;
	static int last_time;

	int frags_remaining = 0;
	int time_remaining = 0;

	if (g_fGameOver)   // someone else quit the game already
	{
		// bounds check
		int time = (int)CVAR_GET_FLOAT("mp_chattime");
		if (time < 1)
			CVAR_SET_STRING("mp_chattime", "1");
		else if (time > MAX_INTERMISSION_TIME)
			CVAR_SET_STRING("mp_chattime", UTIL_dtos1(MAX_INTERMISSION_TIME));

		m_flIntermissionEndTime = g_flIntermissionStartTime + mp_chattime.value;

		// check to see if we should change levels now
		if (m_flIntermissionEndTime < gpGlobals->time)
		{
			if (m_iEndIntermissionButtonHit  // check that someone has pressed a key, or the max intermission time is over
				|| ((g_flIntermissionStartTime + MAX_INTERMISSION_TIME) < gpGlobals->time))
				ChangeLevel(); // intermission is over
		}

		return;
	}

	float flTimeLimit = timelimit.value * 60;
	float flFragLimit = fraglimit.value;

	time_remaining = (int)(flTimeLimit ? (flTimeLimit - gpGlobals->time) : 0);

	if (flTimeLimit != 0 && gpGlobals->time >= flTimeLimit)
	{
		GoToIntermission();
		return;
	}

	if (flFragLimit)
	{
		int bestfrags = std::numeric_limits<int>::max();
		int remain;

		// check if any player is over the frag limit
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

			if (pPlayer && pPlayer->pev->frags >= flFragLimit)
			{
				GoToIntermission();
				return;
			}


			if (pPlayer)
			{
				remain = flFragLimit - pPlayer->pev->frags;
				if (remain < bestfrags)
				{
					bestfrags = remain;
				}
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

bool CHalfLifeMultiplay::IsMultiplayer()
{
	return true;
}

bool CHalfLifeMultiplay::IsDeathmatch()
{
	return true;
}

bool CHalfLifeMultiplay::IsCoOp()
{
	return gpGlobals->coop;
}

bool CHalfLifeMultiplay::ShouldSwitchWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pWeapon)
{
	if (!pWeapon->CanDeploy())
	{
		// that weapon can't deploy anyway.
		return false;
	}

	auto activeItem = pPlayer->m_hActiveItem.Get();

	if (!activeItem)
	{
		// player doesn't have an active item!
		return true;
	}

	if (!activeItem->CanHolster())
	{
		// can't put away the active item.
		return false;
	}

	//Never switch
	if (pPlayer->m_iAutoWepSwitch == 0)
	{
		return false;
	}

	//Only switch if not attacking
	if (pPlayer->m_iAutoWepSwitch == 2 && (pPlayer->m_afButtonLast & (IN_ATTACK | IN_ATTACK2)))
	{
		return false;
	}

	if (pWeapon->Weight() > activeItem->Weight())
	{
		return true;
	}

	return false;
}

bool CHalfLifeMultiplay::GetNextBestWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pCurrentWeapon)
{

	CBasePlayerItem* pCheck;
	CBasePlayerItem* pBest;// this will be used in the event that we don't find a weapon in the same category.
	int iBestWeight;
	int i;

	iBestWeight = -1;// no weapon lower than -1 can be autoswitched to
	pBest = nullptr;

	if (!pCurrentWeapon->CanHolster())
	{
		// can't put this gun away right now, so can't switch.
		return false;
	}

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		pCheck = pPlayer->m_hPlayerItems[i];

		while (pCheck)
		{
			if (pCheck->Weight() > -1 && pCheck->Weight() == pCurrentWeapon->Weight() && pCheck != pCurrentWeapon)
			{
				// this weapon is from the same category. 
				if (pCheck->CanDeploy())
				{
					if (pPlayer->SwitchWeapon(pCheck))
					{
						return true;
					}
				}
			}
			else if (pCheck->Weight() > iBestWeight && pCheck != pCurrentWeapon)// don't reselect the weapon we're trying to get rid of
			{
				//ALERT ( at_console, "Considering %s\n", pCheck->GetClassname() );
				// we keep updating the 'best' weapon just in case we can't find a weapon of the same weight
				// that the player was using. This will end up leaving the player with his heaviest-weighted 
				// weapon. 
				if (pCheck->CanDeploy())
				{
					// if this weapon is useable, flag it as the best
					iBestWeight = pCheck->Weight();
					pBest = pCheck;
				}
			}

			pCheck = pCheck->m_hNext;
		}
	}

	// if we make it here, we've checked all the weapons and found no useable 
	// weapon in the same catagory as the current weapon. 

	// if pBest is null, we didn't find ANYTHING. Shouldn't be possible- should always 
	// at least get the crowbar, but ya never know.
	if (!pBest)
	{
		return false;
	}

	pPlayer->SwitchWeapon(pBest);

	return true;
}

bool CHalfLifeMultiplay::ClientConnected(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128])
{
	g_VoiceGameMgr.ClientConnected(pEntity);
	return true;
}

void CHalfLifeMultiplay::UpdateGameMode(CBasePlayer* pPlayer)
{
	MESSAGE_BEGIN(MessageDest::One, gmsgGameMode, pPlayer);
	WRITE_BYTE(0);  // game mode none
	MESSAGE_END();
}

void CHalfLifeMultiplay::InitHUD(CBasePlayer* pl)
{
	// notify other clients of player joining the game
	UTIL_ClientPrintAll(HUD_PRINTNOTIFY, UTIL_VarArgs("%s has joined the game\n",
		(!IsStringNull(pl->pev->netname) && STRING(pl->pev->netname)[0] != 0) ? STRING(pl->pev->netname) : "unconnected"));

	LogPrintf(pl, "entered the game");

	UpdateGameMode(pl);

	// sending just one score makes the hud scoreboard active;  otherwise
	// it is just disabled for single play
	MESSAGE_BEGIN(MessageDest::One, gmsgScoreInfo, pl);
	WRITE_BYTE(pl->entindex());
	WRITE_SHORT(0);
	WRITE_SHORT(0);
	WRITE_SHORT(0);
	WRITE_SHORT(0);
	MESSAGE_END();

	SendMOTDToClient(pl);

	// loop through all active players and send their score info to the new client
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		// FIXME:  Probably don't need to cast this just to read m_iDeaths
		CBasePlayer* plr = UTIL_PlayerByIndex(i);

		if (plr)
		{
			MESSAGE_BEGIN(MessageDest::One, gmsgScoreInfo, pl);
			WRITE_BYTE(i);	// client number
			WRITE_SHORT(plr->pev->frags);
			WRITE_SHORT(plr->m_iDeaths);
			WRITE_SHORT(0);
			WRITE_SHORT(GetTeamIndex(plr->m_szTeamName) + 1);
			MESSAGE_END();
		}
	}

	if (g_fGameOver)
	{
		MESSAGE_BEGIN(MessageDest::One, SVC_INTERMISSION, pl);
		MESSAGE_END();
	}
}

void CHalfLifeMultiplay::ClientDisconnected(edict_t* pClient)
{
	if (pClient)
	{
		CBasePlayer* pPlayer = (CBasePlayer*)CBaseEntity::Instance(pClient);

		if (pPlayer)
		{
			FireTargets("game_playerleave", pPlayer, pPlayer, UseType::Toggle, 0);

			LogPrintf(pPlayer, "disconnected");

			pPlayer->RemoveAllItems(true);// destroy all of the players weapons and items
		}
	}
}

float CHalfLifeMultiplay::PlayerFallDamage(CBasePlayer* pPlayer)
{
	int iFallDamage = (int)falldamage.value;

	switch (iFallDamage)
	{
	case 1://progressive
		pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
		return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
		break;
	default:
	case 0:// fixed
		return 10;
		break;
	}
}

bool CHalfLifeMultiplay::PlayerCanTakeDamage(CBasePlayer* pPlayer, CBaseEntity* pAttacker)
{
	return true;
}

void CHalfLifeMultiplay::PlayerThink(CBasePlayer* pPlayer)
{
	if (g_fGameOver)
	{
		// check for button presses
		if (pPlayer->m_afButtonPressed & (IN_DUCK | IN_ATTACK | IN_ATTACK2 | IN_USE | IN_JUMP))
			m_iEndIntermissionButtonHit = true;

		// clear attack/use commands from player
		pPlayer->m_afButtonPressed = 0;
		pPlayer->pev->button = 0;
		pPlayer->m_afButtonReleased = 0;
	}
}

void CHalfLifeMultiplay::PlayerSpawn(CBasePlayer* pPlayer)
{
	bool		addDefault;
	CBaseEntity* pWeaponEntity = nullptr;

	//Ensure the player switches to the Glock on spawn regardless of setting
	const int originalAutoWepSwitch = pPlayer->m_iAutoWepSwitch;
	pPlayer->m_iAutoWepSwitch = 1;

	pPlayer->pev->weapons |= (1 << WEAPON_SUIT);

	addDefault = true;

	while (pWeaponEntity = UTIL_FindEntityByClassname(pWeaponEntity, "game_player_equip"))
	{
		pWeaponEntity->Touch(pPlayer);
		addDefault = false;
	}

	if (addDefault)
	{
		pPlayer->GiveNamedItem("weapon_crowbar");
		pPlayer->GiveNamedItem("weapon_9mmhandgun");
		pPlayer->GiveAmmo(68, "9mm", _9MM_MAX_CARRY);// 4 full reloads
	}

	pPlayer->m_iAutoWepSwitch = originalAutoWepSwitch;
}

bool CHalfLifeMultiplay::PlayerCanRespawn(CBasePlayer* pPlayer)
{
	return true;
}

float CHalfLifeMultiplay::PlayerSpawnTime(CBasePlayer* pPlayer)
{
	return gpGlobals->time;//now!
}

bool CHalfLifeMultiplay::AllowAutoTargetCrosshair()
{
	return (aimcrosshair.value != 0);
}

int CHalfLifeMultiplay::PointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled)
{
	return 1;
}

void CHalfLifeMultiplay::PlayerKilled(CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* pInflictor)
{
	if (!pVictim)
		return;

	DeathNotice(pVictim, pKiller, pInflictor);

	pVictim->m_iDeaths += 1;

	FireTargets("game_playerdie", pVictim, pVictim, UseType::Toggle, 0);
	CBasePlayer* peKiller = nullptr;
	if (pKiller && pKiller->IsPlayer())
		peKiller = (CBasePlayer*)pKiller;

	if (pVictim == pKiller)
	{  // killed self
		pKiller->pev->frags -= 1;
	}
	else if (pKiller)
	{
		if (pKiller->IsPlayer())
		{
			// if a player dies in a deathmatch game and the killer is a client, award the killer some points
			pKiller->pev->frags += PointsForKill(peKiller, pVictim);

			FireTargets("game_playerkill", pKiller, pKiller, UseType::Toggle, 0);
		}
		else
		{
			// killed by the world
			pKiller->pev->frags -= 1;
		}
	}

	// update the scores
	// killed scores
	MESSAGE_BEGIN(MessageDest::All, gmsgScoreInfo);
	WRITE_BYTE(pVictim->entindex());
	WRITE_SHORT(pVictim->pev->frags);
	WRITE_SHORT(pVictim->m_iDeaths);
	WRITE_SHORT(0);
	WRITE_SHORT(GetTeamIndex(pVictim->m_szTeamName) + 1);
	MESSAGE_END();

	// killers score, if it's a player
	if (pKiller && pKiller->IsPlayer())
	{
		CBasePlayer* PK = (CBasePlayer*)pKiller;

		MESSAGE_BEGIN(MessageDest::All, gmsgScoreInfo);
		WRITE_BYTE(PK->entindex());
		WRITE_SHORT(PK->pev->frags);
		WRITE_SHORT(PK->m_iDeaths);
		WRITE_SHORT(0);
		WRITE_SHORT(GetTeamIndex(PK->m_szTeamName) + 1);
		MESSAGE_END();

		// let the killer paint another decal as soon as he'd like.
		PK->m_flNextDecalTime = gpGlobals->time;
	}

	if (pVictim->HasNamedPlayerItem("weapon_satchel"))
	{
		DeactivateSatchels(pVictim);
	}
}

void CHalfLifeMultiplay::DeathNotice(CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* pInflictor)
{
	// Work out what killed the player, and send a message to all clients about it
	const char* killer_weapon_name = "world";		// by default, the player is killed by the world
	int killer_index = 0;

	// Hack to fix name change
	const char* tau = "tau_cannon";
	const char* gluon = "gluon gun";

	if (pKiller->pev->flags & FL_CLIENT)
	{
		killer_index = pKiller->entindex();

		if (pInflictor)
		{
			if (pInflictor == pKiller)
			{
				// If the inflictor is the killer,  then it must be their current weapon doing the damage
				CBasePlayer* pPlayer = (CBasePlayer*)pKiller;

				if (auto activeItem = pPlayer->m_hActiveItem.Get(); activeItem)
				{
					killer_weapon_name = activeItem->pszName();
				}
			}
			else
			{
				killer_weapon_name = pInflictor->GetClassname();  // it's just that easy
			}
		}
	}
	else
	{
		killer_weapon_name = pInflictor->GetClassname();
	}

	// strip the monster_* or weapon_* from the inflictor's classname
	if (strncmp(killer_weapon_name, "weapon_", 7) == 0)
		killer_weapon_name += 7;
	else if (strncmp(killer_weapon_name, "monster_", 8) == 0)
		killer_weapon_name += 8;
	else if (strncmp(killer_weapon_name, "func_", 5) == 0)
		killer_weapon_name += 5;

	MESSAGE_BEGIN(MessageDest::All, gmsgDeathMsg);
	WRITE_BYTE(killer_index);				// the killer
	WRITE_BYTE(pVictim->entindex());		// the victim
	WRITE_STRING(killer_weapon_name);		// what they were killed by (should this be a string?)
	MESSAGE_END();

	// replace the code names with the 'real' names
	if (!strcmp(killer_weapon_name, "egon"))
		killer_weapon_name = gluon;
	else if (!strcmp(killer_weapon_name, "gauss"))
		killer_weapon_name = tau;

	if (pVictim == pKiller)
	{
		// killed self
		LogPrintf(pVictim, "committed suicide with \"%s\"", killer_weapon_name);
	}
	else if (pKiller->pev->flags & FL_CLIENT)
	{
		// team match?
		if (IsTeamplay())
		{
			LogPrintf(pKiller, "killed \"%s<%i><%s><%s>\" with \"%s\"",
				STRING(pVictim->pev->netname),
				GETPLAYERUSERID(pVictim->edict()),
				GETPLAYERAUTHID(pVictim->edict()),
				g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pVictim->edict()), "model"),
				killer_weapon_name);
		}
		else
		{
			LogPrintf(pKiller, "killed \"%s<%i><%s><%i>\" with \"%s\"",
				STRING(pVictim->pev->netname),
				GETPLAYERUSERID(pVictim->edict()),
				GETPLAYERAUTHID(pVictim->edict()),
				GETPLAYERUSERID(pVictim->edict()),
				killer_weapon_name);
		}
	}
	else
	{
		// killed by the world
		LogPrintf(pVictim, "committed suicide with \"%s\" (world)", killer_weapon_name);
	}

	MESSAGE_BEGIN(MessageDest::Spectator, SVC_DIRECTOR);
	WRITE_BYTE(9);	// command length in bytes
	WRITE_BYTE(DRC_CMD_EVENT);	// player killed
	WRITE_SHORT(pVictim->entindex());	// index number of primary entity
	if (pInflictor)
		WRITE_SHORT(pInflictor->entindex());	// index number of secondary entity
	else
		WRITE_SHORT(pKiller->entindex());		// index number of secondary entity
	WRITE_LONG(7 | DRC_FLAG_DRAMATIC);   // eventflags (priority and flags)
	MESSAGE_END();

	//  Print a standard message
		// TODO: make this go direct to console
	return; // just remove for now
/*
	char	szText[ 128 ];

	if ( pKiller->flags & FL_MONSTER )
	{
		// killed by a monster
		safe_strcpy ( szText, STRING( pVictim->pev->netname ) );
		safe_strcat ( szText, " was killed by a monster.\n" );
		return;
	}

	if ( pKiller == pVictim->pev )
	{
		safe_strcpy ( szText, STRING( pVictim->pev->netname ) );
		safe_strcat ( szText, " commited suicide.\n" );
	}
	else if ( pKiller->flags & FL_CLIENT )
	{
		safe_strcpy ( szText, STRING( pKiller->netname ) );

		safe_strcat( szText, " : " );
		safe_strcat( szText, killer_weapon_name );
		safe_strcat( szText, " : " );

		safe_strcat ( szText, STRING( pVictim->pev->netname ) );
		safe_strcat ( szText, "\n" );
	}
	else if ( pKiller->ClassnameIs( "worldspawn" ) )
	{
		safe_strcpy ( szText, STRING( pVictim->pev->netname ) );
		safe_strcat ( szText, " fell or drowned or something.\n" );
	}
	else if ( pKiller->solid == Solid::BSP )
	{
		safe_strcpy ( szText, STRING( pVictim->pev->netname ) );
		safe_strcat ( szText, " was mooshed.\n" );
	}
	else
	{
		safe_strcpy ( szText, STRING( pVictim->pev->netname ) );
		safe_strcat ( szText, " died mysteriously.\n" );
	}

	UTIL_ClientPrintAll( szText );
*/
}

bool CHalfLifeMultiplay::CanHaveItem(CBasePlayer& player, CBaseItem& item)
{
	switch (item.GetType())
	{
	case ItemType::Weapon:
	{
		auto& weapon = static_cast<CBasePlayerItem&>(item);

		if (weaponstay.value > 0)
		{
			if (!(weapon.Flags() & ITEM_FLAG_LIMITINWORLD))
			{
				// check if the player already has this weapon
				for (int i = 0; i < MAX_ITEM_TYPES; i++)
				{
					CBasePlayerItem* it = player.m_hPlayerItems[i];

					while (it != nullptr)
					{
						if (it->m_iId == weapon.m_iId)
						{
							return false;
						}

						it = it->m_hNext;
					}
				}
			}
		}
	}

	[[fallthrough]];

	default: return CGameRules::CanHaveItem(player, item);
	}
}

void CHalfLifeMultiplay::PlayerGotItem(CBasePlayer& player, CBaseItem& item)
{
	//Nothing
}

bool CHalfLifeMultiplay::ItemShouldRespawn(CBaseItem& item)
{
	return item.m_RespawnMode != ItemRespawnMode::Never;
}

float CHalfLifeMultiplay::ItemRespawnTime(CBaseItem& item)
{
	if (item.m_flRespawnDelay != ITEM_DEFAULT_RESPAWN_DELAY)
	{
		return gpGlobals->time + item.m_flRespawnDelay;
	}

	switch (item.GetType())
	{
	case ItemType::PickupItem: return gpGlobals->time + ITEM_RESPAWN_TIME;
	case ItemType::Ammo: return gpGlobals->time + AMMO_RESPAWN_TIME;

	case ItemType::Weapon:
	{
		auto& weapon = static_cast<CBasePlayerItem&>(item);
		if (weaponstay.value > 0)
		{
			// make sure it's only certain weapons
			if (!(weapon.Flags() & ITEM_FLAG_LIMITINWORLD))
			{
				return gpGlobals->time + 0;		// weapon respawns almost instantly
			}
		}

		return gpGlobals->time + WEAPON_RESPAWN_TIME;
	}

	default:
	{
		ALERT(at_error, "CHalfLifeMultiplay::ItemRespawnTime: Unknown item type \"%d\"\n", item.GetType());
		return -1;
	}
	}
}

// when we are within this close to running out of entities,  items 
// marked with the ITEM_FLAG_LIMITINWORLD will delay their respawn
constexpr int ENTITY_INTOLERANCE = 100;

float CHalfLifeMultiplay::ItemTryRespawn(CBaseItem& item)
{
	switch (item.GetType())
	{
	case ItemType::PickupItem: break;
	case ItemType::Ammo: break;

	case ItemType::Weapon:
	{
		auto& weapon = static_cast<CBasePlayerItem&>(item);

		if (weapon.m_iId && (weapon.Flags() & ITEM_FLAG_LIMITINWORLD))
		{
			if (NUMBER_OF_ENTITIES() >= (gpGlobals->maxEntities - ENTITY_INTOLERANCE))
			{
				// we're past the entity tolerance level,  so delay the respawn
				//TODO: this results in time + time + delay
				return ItemRespawnTime(weapon);
			}
		}

		break;
	}

	default:
	{
		ALERT(at_error, "CHalfLifeMultiplay::ItemTryRespawn: Unknown item type \"%d\"\n", item.GetType());
		return -1;
	}
	}

	//Respawn immediately
	return 0;
}

void CHalfLifeMultiplay::PlayerGotAmmo(CBasePlayer* pPlayer, char* szName, int iCount)
{
}

bool CHalfLifeMultiplay::IsAllowedToSpawn(CBaseEntity* pEntity)
{
	//	if ( pEntity->pev->flags & FL_MONSTER )
	//		return false;

	return true;
}

float CHalfLifeMultiplay::HealthChargerRechargeTime()
{
	return 60;
}


float CHalfLifeMultiplay::HEVChargerRechargeTime()
{
	return 30;
}

int CHalfLifeMultiplay::DeadPlayerWeapons(CBasePlayer* pPlayer)
{
	return GR_PLR_DROP_GUN_ACTIVE;
}

int CHalfLifeMultiplay::DeadPlayerAmmo(CBasePlayer* pPlayer)
{
	return GR_PLR_DROP_AMMO_ACTIVE;
}

CBaseEntity* CHalfLifeMultiplay::GetPlayerSpawnSpot(CBasePlayer* pPlayer)
{
	CBaseEntity* pSpawnSpot = CGameRules::GetPlayerSpawnSpot(pPlayer);
	if (IsMultiplayer() && !IsStringNull(pSpawnSpot->pev->target))
	{
		FireTargets(STRING(pSpawnSpot->pev->target), pPlayer, pPlayer, UseType::Toggle, 0);
	}

	return pSpawnSpot;
}

int CHalfLifeMultiplay::PlayerRelationship(CBaseEntity* pPlayer, CBaseEntity* pTarget)
{
	// half life deathmatch has only enemies
	return GR_NOTTEAMMATE;
}

bool CHalfLifeMultiplay::PlayFootstepSounds(CBasePlayer* pl, float fvol)
{
	if (g_footsteps && g_footsteps->value == 0)
		return false;

	if (pl->IsOnLadder() || pl->GetAbsVelocity().Length2D() > 220)
		return true;  // only make step sounds in multiplayer if the player is moving fast enough

	return false;
}

bool CHalfLifeMultiplay::AllowFlashlight()
{
	return flashlight.value != 0;
}

bool CHalfLifeMultiplay::AllowMonsters()
{
	return (allowmonsters.value != 0);
}

constexpr int INTERMISSION_TIME = 6;

void CHalfLifeMultiplay::GoToIntermission()
{
	if (g_fGameOver)
		return;  // intermission has already been triggered, so ignore.

	MESSAGE_BEGIN(MessageDest::All, SVC_INTERMISSION);
	MESSAGE_END();

	// bounds check
	int time = (int)CVAR_GET_FLOAT("mp_chattime");
	if (time < 1)
		CVAR_SET_STRING("mp_chattime", "1");
	else if (time > MAX_INTERMISSION_TIME)
		CVAR_SET_STRING("mp_chattime", UTIL_dtos1(MAX_INTERMISSION_TIME));

	m_flIntermissionEndTime = gpGlobals->time + ((int)mp_chattime.value);
	g_flIntermissionStartTime = gpGlobals->time;

	g_fGameOver = true;
	m_iEndIntermissionButtonHit = false;
}

void CHalfLifeMultiplay::ChangeLevel()
{
	static char szPreviousMapCycleFile[256];
	static mapcycle_t mapcycle;

	char szNextMap[MAX_MAPNAME_LENGTH];
	char szFirstMapInList[MAX_MAPNAME_LENGTH];
	char szCommands[1500];
	char szRules[1500];
	int minplayers = 0, maxplayers = 0;
	safe_strcpy(szFirstMapInList, "hldm1");  // the absolute default level is hldm1

	int	curplayers;
	bool do_cycle = true;

	// find the map to change to
	const char* mapcfile = CVAR_GET_STRING("mapcyclefile");
	ASSERT(mapcfile != nullptr);

	szCommands[0] = '\0';
	szRules[0] = '\0';

	curplayers = UTIL_CountPlayers();

	// Has the map cycle filename changed?
	if (stricmp(mapcfile, szPreviousMapCycleFile))
	{
		safe_strcpy(szPreviousMapCycleFile, mapcfile);

		DestroyMapCycle(&mapcycle);

		if (!ReloadMapCycleFile(mapcfile, &mapcycle) || (!mapcycle.items))
		{
			ALERT(at_console, "Unable to load map cycle file %s\n", mapcfile);
			do_cycle = false;
		}
	}

	if (do_cycle && mapcycle.items)
	{
		bool keeplooking = false;
		bool found = false;
		mapcycle_item_t* item;

		// Assume current map
		safe_strcpy(szNextMap, STRING(gpGlobals->mapname));
		safe_strcpy(szFirstMapInList, STRING(gpGlobals->mapname));

		// Traverse list
		for (item = mapcycle.next_item; item->next != mapcycle.next_item; item = item->next)
		{
			keeplooking = false;

			ASSERT(item != nullptr);

			if (item->minplayers != 0)
			{
				if (curplayers >= item->minplayers)
				{
					found = true;
					minplayers = item->minplayers;
				}
				else
				{
					keeplooking = true;
				}
			}

			if (item->maxplayers != 0)
			{
				if (curplayers <= item->maxplayers)
				{
					found = true;
					maxplayers = item->maxplayers;
				}
				else
				{
					keeplooking = true;
				}
			}

			if (keeplooking)
				continue;

			found = true;
			break;
		}

		if (!found)
		{
			item = mapcycle.next_item;
		}

		// Increment next item pointer
		mapcycle.next_item = item->next;

		// Perform logic on current item
		safe_strcpy(szNextMap, item->mapname);

		ExtractCommandString(item->rulebuffer, szCommands, sizeof(szCommands));
		safe_strcpy(szRules, item->rulebuffer);
	}

	if (!IS_MAP_VALID(szNextMap))
	{
		safe_strcpy(szNextMap, szFirstMapInList);
	}

	g_fGameOver = true;

	ALERT(at_console, "CHANGE LEVEL: %s\n", szNextMap);
	if (minplayers || maxplayers)
	{
		ALERT(at_console, "PLAYER COUNT:  min %i max %i current %i\n", minplayers, maxplayers, curplayers);
	}
	if (strlen(szRules) > 0)
	{
		ALERT(at_console, "RULES:  %s\n", szRules);
	}

	CHANGE_LEVEL(szNextMap, nullptr);
	if (strlen(szCommands) > 0)
	{
		SERVER_COMMAND(szCommands);
	}
}

constexpr int MAX_MOTD_CHUNK = 60;
constexpr int MAX_MOTD_LENGTH = 1536; // (MAX_MOTD_CHUNK * 4)

void CHalfLifeMultiplay::SendMOTDToClient(CBasePlayer* player)
{
	// read from the MOTD.txt file
	int char_count = 0;

	auto [fileBuffer, size] = FileSystem_LoadFileIntoBuffer(CVAR_GET_STRING("motdfile"));

	char* const aFileList = reinterpret_cast<char*>(fileBuffer.get());
	char* pFileList = aFileList;

	// send the server name
	MESSAGE_BEGIN(MessageDest::One, gmsgServerName, player);
	WRITE_STRING(CVAR_GET_STRING("hostname"));
	MESSAGE_END();

	// Send the message of the day
	// read it chunk-by-chunk,  and send it in parts

	while (pFileList && *pFileList && char_count < MAX_MOTD_LENGTH)
	{
		char chunk[MAX_MOTD_CHUNK + 1]{};

		if (strlen(pFileList) < MAX_MOTD_CHUNK)
		{
			safe_strcpy(chunk, pFileList);
		}
		else
		{
			safe_strcpy(chunk, pFileList);
		}

		char_count += strlen(chunk);
		if (char_count < MAX_MOTD_LENGTH)
			pFileList = aFileList + char_count;
		else
			*pFileList = 0;

		MESSAGE_BEGIN(MessageDest::One, gmsgMOTD, player);
		WRITE_BYTE(*pFileList ? false : true);	// false means there is still more message to come
		WRITE_STRING(chunk);
		MESSAGE_END();
	}
}
