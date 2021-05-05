/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

#include "usercmd.h"
#include "entity_state.h"
#include "demo_api.h"
#include "pm_defs.h"
#include "event_api.h"
#include "r_efx.h"

#include "cl_dll.h"
#include "com_weapons.h"
#include "demo.h"

extern int g_iUser1;

// Pool of client side entities/entvars_t
//For weapons + local player
static edict_t	edicts[MAX_WEAPONS + 1];
static int			num_ents = 0;

// The entity we'll use to represent the local client
static CBasePlayer	player;

// Local version of game .dll global variables ( time, etc. )
static globalvars_t	Globals;

static CBasePlayerWeapon* g_pWpns[MAX_WEAPONS];

float g_flApplyVel = 0.0;
int   g_irunninggausspred = 0;

Vector previousorigin;

// HLDM Weapon placeholder entities.
CGlock g_Glock;
CCrowbar g_Crowbar;
CPython g_Python;
CMP5 g_Mp5;
CCrossbow g_Crossbow;
CShotgun g_Shotgun;
CRpg g_Rpg;
CGauss g_Gauss;
CEgon g_Egon;
CHgun g_HGun;
CHandGrenade g_HandGren;
CSatchel g_Satchel;
CTripmine g_Tripmine;
CSqueak g_Snark;

/**
*	@brief Print debug messages to console
*/
void AlertMessage(ALERT_TYPE atype, const char* szFmt, ...)
{
	va_list		argptr;
	static char	string[1024];

	va_start(argptr, szFmt);
	vsnprintf(string, sizeof(string), szFmt, argptr);
	va_end(argptr);

	gEngfuncs.Con_Printf("cl:  ");
	gEngfuncs.Con_Printf(string);
}

bool bIsMultiplayer()
{
	return gEngfuncs.GetMaxClients() != 1;
}

void LoadVModel(const char* szViewModel, CBasePlayer* m_pPlayer)
{
	gEngfuncs.CL_LoadModel(szViewModel, reinterpret_cast<int*>(&m_pPlayer->pev->viewmodel));
}

/**
*	@brief Links the raw entity to an entvars_t holder.
*	If a player is passed in as the owner, then we set up the m_pPlayer field.
*/
void HUD_PrepEntity(CBaseEntity* pEntity, CBasePlayer* pWeaponOwner)
{
	memset(&edicts[num_ents], 0, sizeof(edict_t));

	auto edict = &edicts[num_ents];
	auto pev = &edict->v;
	pev->pContainingEntity = edict;

	++num_ents;

	pEntity->pev = pev;
	edict->pvPrivateData = pEntity;

	pEntity->Precache();
	pEntity->Spawn();

	if (pWeaponOwner)
	{
		ItemInfo info;

		memset(&info, 0, sizeof(info));

		((CBasePlayerWeapon*)pEntity)->m_hPlayer = pWeaponOwner;

		((CBasePlayerWeapon*)pEntity)->GetItemInfo(&info);

		CBasePlayerItem::ItemInfoArray[info.iId] = info;

		if (info.pszAmmo1 && *info.pszAmmo1)
		{
			AddAmmoNameToAmmoRegistry(info.pszAmmo1);
		}

		if (info.pszAmmo2 && *info.pszAmmo2)
		{
			AddAmmoNameToAmmoRegistry(info.pszAmmo2);
		}

		g_pWpns[info.iId] = (CBasePlayerWeapon*)pEntity;
	}
}

/**
*	@brief If weapons code "kills" an entity, just set its effects to EF_NODRAW
*/
void CBaseEntity::Killed(const KilledInfo& info)
{
	pev->effects |= EF_NODRAW;
}

bool CBasePlayerWeapon::DefaultDeploy(const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, int body)
{
	if (!CanDeploy())
		return false;

	gEngfuncs.CL_LoadModel(szViewModel, reinterpret_cast<int*>(&m_hPlayer->pev->viewmodel));

	SendWeaponAnim(iAnim, body);

	g_irunninggausspred = false;
	m_hPlayer->m_flNextAttack = 0.5;
	m_flTimeWeaponIdle = 1.0;
	return true;
}

bool CBasePlayerWeapon::PlayEmptySound()
{
	if (m_iPlayEmptySound)
	{
		HUD_PlaySound("weapons/357_cock1.wav", 0.8);
		m_iPlayEmptySound = false;
		return false; //TODO: incorrect?
	}
	return false;
}

void CBasePlayerWeapon::Holster()
{
	m_fInReload = false; // cancel any reload in progress.
	g_irunninggausspred = false;
	m_hPlayer->pev->viewmodel = iStringNull;
}

void CBasePlayerWeapon::SendWeaponAnim(int iAnim, int body)
{
	m_hPlayer->pev->weaponanim = iAnim;

	HUD_SendWeaponAnim(iAnim, body, false);
}

/**
*	@brief Only produces random numbers to match the server ones.
*/
Vector CBasePlayer::FireBulletsPlayer(std::uint32_t cShots, const Vector& vecSrc, const Vector& vecDirShooting, const Vector& vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage)
{
	const int shared_rand = random_seed;

	float x = 0, y = 0, z;

	for (std::uint32_t iShot = 1; iShot <= cShots; iShot++)
	{
		//Use player's random seed.
		// get circular gaussian spread
		x = UTIL_SharedRandomFloat(shared_rand + iShot, -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + (1 + iShot), -0.5, 0.5);
		y = UTIL_SharedRandomFloat(shared_rand + (2 + iShot), -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + (3 + iShot), -0.5, 0.5);
		z = x * x + y * y;
	}

	return Vector(x * vecSpread.x, y * vecSpread.y, 0.0);
}

void CBasePlayer::SelectItem(const char* pstr)
{
	//TODO: needs to be merged with server version
	if (!pstr)
		return;

	CBasePlayerItem* pItem = nullptr;

	if (!pItem)
		return;


	if (pItem == m_hActiveItem)
		return;

	if (m_hActiveItem)
		m_hActiveItem->Holster();

	m_hLastItem = m_hActiveItem;
	m_hActiveItem = pItem;

	if (m_hActiveItem)
	{
		m_hActiveItem->Deploy();
	}
}

void CBasePlayer::Killed(const KilledInfo& info)
{
	// Holster weapon immediately, to allow it to cleanup
	if (auto activeItem = m_hActiveItem.Get(); activeItem)
		activeItem->Holster();

	g_irunninggausspred = false;
}

void CBasePlayer::Spawn()
{
	if (auto activeItem = m_hActiveItem.Get(); activeItem)
		activeItem->Deploy();

	g_irunninggausspred = false;
}

void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IgnoreMonsters igmon, CBaseEntity* pIgnore, TraceResult* ptr)
{
	//Don't actually trace, but act like the trace didn't hit anything.
	memset(ptr, 0, sizeof(*ptr));
	ptr->flFraction = 1.0;
}

/**
*	@brief For debugging, draw a box around a player made out of particles
*/
void UTIL_ParticleBox(CBasePlayer* player, const Vector& mins, const Vector& maxs, float life, unsigned char r, unsigned char g, unsigned char b)
{
	const Vector mmin = player->GetAbsOrigin() + mins;
	const Vector mmax = player->GetAbsOrigin() + maxs;

	gEngfuncs.pEfxAPI->R_ParticleBox(mmin, mmax, 5.0, 0, 255, 0);
}

/**
*	@brief For debugging, draw boxes for other collidable players
*/
void UTIL_ParticleBoxes()
{
	int idx;
	physent_t* pe;
	cl_entity_t* player;
	Vector mins, maxs;

	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	player = gEngfuncs.GetLocalPlayer();
	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(player->index - 1);

	for (idx = 1; idx < 100; idx++)
	{
		pe = gEngfuncs.pEventAPI->EV_GetPhysent(idx);
		if (!pe)
			break;

		if (pe->info >= 1 && pe->info <= gEngfuncs.GetMaxClients())
		{
			mins = pe->origin + pe->mins;
			maxs = pe->origin + pe->maxs;

			gEngfuncs.pEfxAPI->R_ParticleBox(mins, maxs, 0, 0, 255, 2.0);
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

/**
*	@brief For debugging, draw a line made out of particles
*/
void UTIL_ParticleLine(CBasePlayer* player, const Vector& start, const Vector& end, float life, unsigned char r, unsigned char g, unsigned char b)
{
	gEngfuncs.pEfxAPI->R_ParticleLine(start, end, r, g, b, life);
}

/**
*	@brief Set up weapons, player and functions needed to run weapons code client-side.
*/
void HUD_InitClientWeapons()
{
	static bool initialized = false;
	if (initialized)
		return;

	initialized = true;

	// Set up pointer ( dummy object )
	gpGlobals = &Globals;

	// Fill in current time ( probably not needed )
	gpGlobals->time = gEngfuncs.GetClientTime();

	// Fake functions
	g_engfuncs.pfnPrecacheModel = stub_PrecacheModel;
	g_engfuncs.pfnPrecacheSound = stub_PrecacheSound;
	g_engfuncs.pfnPrecacheEvent = stub_PrecacheEvent;
	g_engfuncs.pfnNameForFunction = stub_NameForFunction;
	g_engfuncs.pfnSetModel = stub_SetModel;
	g_engfuncs.pfnSetClientMaxspeed = HUD_SetMaxSpeed;

	// Handled locally
	g_engfuncs.pfnPlaybackEvent = HUD_PlaybackEvent;
	g_engfuncs.pfnAlertMessage = AlertMessage;

	// Pass through to engine
	g_engfuncs.pfnPrecacheEvent = gEngfuncs.pfnPrecacheEvent;
	g_engfuncs.pfnRandomFloat = gEngfuncs.pfnRandomFloat;
	g_engfuncs.pfnRandomLong = gEngfuncs.pfnRandomLong;
	g_engfuncs.pfnCVarGetPointer = gEngfuncs.pfnGetCvarPointer;
	g_engfuncs.pfnCVarGetString = gEngfuncs.pfnGetCvarString;
	g_engfuncs.pfnCVarGetFloat = gEngfuncs.pfnGetCvarFloat;

	// Allocate a slot for the local player
	HUD_PrepEntity(&player, nullptr);

	// Allocate slot(s) for each weapon that we are going to be predicting
	HUD_PrepEntity(&g_Glock, &player);
	HUD_PrepEntity(&g_Crowbar, &player);
	HUD_PrepEntity(&g_Python, &player);
	HUD_PrepEntity(&g_Mp5, &player);
	HUD_PrepEntity(&g_Crossbow, &player);
	HUD_PrepEntity(&g_Shotgun, &player);
	HUD_PrepEntity(&g_Rpg, &player);
	HUD_PrepEntity(&g_Gauss, &player);
	HUD_PrepEntity(&g_Egon, &player);
	HUD_PrepEntity(&g_HGun, &player);
	HUD_PrepEntity(&g_HandGren, &player);
	HUD_PrepEntity(&g_Satchel, &player);
	HUD_PrepEntity(&g_Tripmine, &player);
	HUD_PrepEntity(&g_Snark, &player);
}

/**
*	@brief Returns the last position that we stored for egon beam endpoint.
*/
void HUD_GetLastOrg(Vector& org)
{
	// Return last origin
	org = previousorigin;
}

/**
*	@brief Remember our exact predicted origin so we can draw the egon to the right position.
*/
void HUD_SetLastOrg()
{
	// Offset final origin by view_offset
	previousorigin = g_finalstate->playerstate.origin + g_finalstate->client.view_ofs;
}

/**
*	@brief Run Weapon firing code on client
*/
void HUD_WeaponsPostThink(local_state_t* from, local_state_t* to, usercmd_t* cmd, double time, unsigned int random_seed)
{
	int i;
	int buttonsChanged;
	CBasePlayerWeapon* pWeapon = nullptr;
	CBasePlayerWeapon* pCurrent;
	weapon_data_t nulldata, * pfrom, * pto;
	static int lasthealth;

	memset(&nulldata, 0, sizeof(nulldata));

	HUD_InitClientWeapons();

	// Get current clock
	gpGlobals->time = time;

	// Fill in data based on selected weapon
	// FIXME, make this a method in each weapon?  where you pass in an entity_state_t *?
	switch (from->client.m_iId)
	{
	case WEAPON_CROWBAR:
		pWeapon = &g_Crowbar;
		break;

	case WEAPON_GLOCK:
		pWeapon = &g_Glock;
		break;

	case WEAPON_PYTHON:
		pWeapon = &g_Python;
		break;

	case WEAPON_MP5:
		pWeapon = &g_Mp5;
		break;

	case WEAPON_CROSSBOW:
		pWeapon = &g_Crossbow;
		break;

	case WEAPON_SHOTGUN:
		pWeapon = &g_Shotgun;
		break;

	case WEAPON_RPG:
		pWeapon = &g_Rpg;
		break;

	case WEAPON_GAUSS:
		pWeapon = &g_Gauss;
		break;

	case WEAPON_EGON:
		pWeapon = &g_Egon;
		break;

	case WEAPON_HORNETGUN:
		pWeapon = &g_HGun;
		break;

	case WEAPON_HANDGRENADE:
		pWeapon = &g_HandGren;
		break;

	case WEAPON_SATCHEL:
		pWeapon = &g_Satchel;
		break;

	case WEAPON_TRIPMINE:
		pWeapon = &g_Tripmine;
		break;

	case WEAPON_SNARK:
		pWeapon = &g_Snark;
		break;
	}

	// Store pointer to our destination entity_state_t so we can get our origin, etc. from it
	//  for setting up events on the client
	g_finalstate = to;

	// If we are running events/etc. go ahead and see if we
	//  managed to die between last frame and this one
	// If so, run the appropriate player killed or spawn function
	if (g_runfuncs)
	{
		if (to->client.health <= 0 && lasthealth > 0)
		{
			player.Killed({nullptr, nullptr, GibType::Normal});

		}
		else if (to->client.health > 0 && lasthealth <= 0)
		{
			player.Spawn();
		}

		lasthealth = to->client.health;
	}

	// We are not predicting the current weapon, just bow out here.
	if (!pWeapon)
		return;

	for (i = 0; i < MAX_WEAPONS; i++)
	{
		pCurrent = g_pWpns[i];
		if (!pCurrent)
		{
			continue;
		}

		pfrom = &from->weapondata[i];

		pCurrent->m_fInReload = pfrom->m_fInReload;
		//		pCurrent->m_flPumpTime			= pfrom->m_flPumpTime;
		pCurrent->m_iClip = pfrom->m_iClip;
		pCurrent->m_flNextPrimaryAttack = pfrom->m_flNextPrimaryAttack;
		pCurrent->m_flNextSecondaryAttack = pfrom->m_flNextSecondaryAttack;
		pCurrent->m_flTimeWeaponIdle = pfrom->m_flTimeWeaponIdle;
		pCurrent->pev->fuser1 = pfrom->fuser1;
		pCurrent->SetWeaponData(*pfrom);

		pCurrent->m_iSecondaryAmmoType = (int)from->client.vuser3[2];
		pCurrent->m_iPrimaryAmmoType = (int)from->client.vuser4[0];
		player.m_rgAmmo[pCurrent->m_iPrimaryAmmoType] = (int)from->client.vuser4[1];
		player.m_rgAmmo[pCurrent->m_iSecondaryAmmoType] = (int)from->client.vuser4[2];
	}

	// For random weapon events, use this seed to seed random # generator
	player.random_seed = random_seed;

	// Get old buttons from previous state.
	player.m_afButtonLast = from->playerstate.oldbuttons;

	// Which buttsons chave changed
	buttonsChanged = (player.m_afButtonLast ^ cmd->buttons);	// These buttons have changed this frame

	// Debounced button codes for pressed/released
	// The changed ones still down are "pressed"
	player.m_afButtonPressed = buttonsChanged & cmd->buttons;
	// The ones not down are "released"
	player.m_afButtonReleased = buttonsChanged & (~cmd->buttons);

	// Set player variables that weapons code might check/alter
	player.pev->button = cmd->buttons;

	player.SetAbsVelocity(from->client.velocity);
	player.pev->flags = from->client.flags;

	player.pev->deadflag = from->client.deadflag;
	player.pev->waterlevel = static_cast<WaterLevel>(from->client.waterlevel);
	player.pev->maxspeed = from->client.maxspeed;
	player.m_iFOV = from->client.fov;
	player.pev->weaponanim = from->client.weaponanim;
	player.pev->viewmodel = from->client.viewmodel;
	player.m_flNextAttack = from->client.m_flNextAttack;

	//Stores all our ammo info, so the client side weapons can use them.
	player.SetAmmoCount("9mm", (int)from->client.vuser1[0]);
	player.SetAmmoCount("357", (int)from->client.vuser1[1]);
	player.SetAmmoCount("ARgrenades", (int)from->client.vuser1[2]);
	player.SetAmmoCount("bolts", (int)from->client.ammo_nails); //is an int anyways...
	player.SetAmmoCount("buckshot", (int)from->client.ammo_shells);
	player.SetAmmoCount("uranium", (int)from->client.ammo_cells);
	player.SetAmmoCount("Hornets", (int)from->client.vuser2[0]);
	player.SetAmmoCount("rockets", (int)from->client.ammo_rockets);


	// Point to current weapon object
	if (from->client.m_iId)
	{
		player.m_hActiveItem = g_pWpns[from->client.m_iId];
	}

	if (player.m_hActiveItem->m_iId == WEAPON_RPG)
	{
		((CRpg*)player.m_hActiveItem.Get())->m_fSpotActive = ((int)from->client.vuser2[1]) != 0;
		((CRpg*)player.m_hActiveItem.Get())->m_cActiveRockets = (int)from->client.vuser2[2];
	}

	// Don't go firing anything if we have died or are spectating
	// Or if we don't have a weapon model deployed
	if ((player.pev->deadflag != DeadFlag::DeaderThanDead) &&
		!CL_IsDead() && !IsStringNull(player.pev->viewmodel) && !g_iUser1)
	{
		if (player.m_flNextAttack <= 0)
		{
			pWeapon->ItemPostFrame();
		}
	}

	// Assume that we are not going to switch weapons
	to->client.m_iId = from->client.m_iId;

	// Now see if we issued a changeweapon command ( and we're not dead )
	if (cmd->weaponselect && (player.pev->deadflag != DeadFlag::DeaderThanDead))
	{
		// Switched to a different weapon?
		if (from->weapondata[cmd->weaponselect].m_iId == cmd->weaponselect)
		{
			CBasePlayerWeapon* pNew = g_pWpns[cmd->weaponselect];
			if (pNew && (pNew != pWeapon))
			{
				// Put away old weapon
				if (auto activeItem = player.m_hActiveItem.Get(); activeItem)
					activeItem->Holster();

				player.m_hLastItem = player.m_hActiveItem;
				player.m_hActiveItem = pNew;

				// Deploy new weapon
				if (auto activeItem = player.m_hActiveItem.Get(); activeItem)
				{
					activeItem->Deploy();
				}

				// Update weapon id so we can predict things correctly.
				to->client.m_iId = cmd->weaponselect;
			}
		}
	}

	// Copy in results of prediction code
	to->client.viewmodel = player.pev->viewmodel;
	to->client.fov = player.m_iFOV;
	to->client.weaponanim = player.pev->weaponanim;
	to->client.m_flNextAttack = player.m_flNextAttack;
	to->client.maxspeed = player.pev->maxspeed;

	//HL Weapons
	to->client.vuser1[0] = player.GetAmmoCount("9mm");
	to->client.vuser1[1] = player.GetAmmoCount("357");
	to->client.vuser1[2] = player.GetAmmoCount("ARgrenades");

	to->client.ammo_nails = player.GetAmmoCount("bolts");
	to->client.ammo_shells = player.GetAmmoCount("buckshot");
	to->client.ammo_cells = player.GetAmmoCount("uranium");
	to->client.vuser2[0] = player.GetAmmoCount("Hornets");
	to->client.ammo_rockets = player.GetAmmoCount("rockets");

	if (player.m_hActiveItem->m_iId == WEAPON_RPG)
	{
		from->client.vuser2[1] = ((CRpg*)player.m_hActiveItem.Get())->m_fSpotActive;
		from->client.vuser2[2] = ((CRpg*)player.m_hActiveItem.Get())->m_cActiveRockets;
	}

	// Make sure that weapon animation matches what the game .dll is telling us
	//  over the wire ( fixes some animation glitches )
	if (g_runfuncs && (HUD_GetWeaponAnim() != to->client.weaponanim))
	{
		//Make sure the 357 has the right body
		g_Python.pev->body = bIsMultiplayer() ? 1 : 0;

		// Force a fixed anim down to viewmodel
		HUD_SendWeaponAnim(to->client.weaponanim, pWeapon->pev->body, true);
	}

	for (i = 0; i < MAX_WEAPONS; i++)
	{
		pCurrent = g_pWpns[i];

		pto = &to->weapondata[i];

		if (!pCurrent)
		{
			memset(pto, 0, sizeof(weapon_data_t));
			continue;
		}

		pto->m_fInReload = pCurrent->m_fInReload;
		//		pto->m_flPumpTime				= pCurrent->m_flPumpTime;
		pto->m_iClip = pCurrent->m_iClip;
		pto->m_flNextPrimaryAttack = pCurrent->m_flNextPrimaryAttack;
		pto->m_flNextSecondaryAttack = pCurrent->m_flNextSecondaryAttack;
		pto->m_flTimeWeaponIdle = pCurrent->m_flTimeWeaponIdle;
		pto->fuser1 = pCurrent->pev->fuser1;
		pCurrent->SetWeaponData(*pto);

		// Decrement weapon counters, server does this at same time ( during post think, after doing everything else )
		pto->m_flNextReload -= cmd->msec / 1000.0;
		pto->m_fNextAimBonus -= cmd->msec / 1000.0;
		pto->m_flNextPrimaryAttack -= cmd->msec / 1000.0;
		pto->m_flNextSecondaryAttack -= cmd->msec / 1000.0;
		pto->m_flTimeWeaponIdle -= cmd->msec / 1000.0;
		pto->fuser1 -= cmd->msec / 1000.0;

		to->client.vuser3[2] = pCurrent->m_iSecondaryAmmoType;
		to->client.vuser4[0] = pCurrent->m_iPrimaryAmmoType;
		to->client.vuser4[1] = player.m_rgAmmo[pCurrent->m_iPrimaryAmmoType];
		to->client.vuser4[2] = player.m_rgAmmo[pCurrent->m_iSecondaryAmmoType];

		/*		if ( pto->m_flPumpTime != -9999 )
				{
					pto->m_flPumpTime -= cmd->msec / 1000.0;
					if ( pto->m_flPumpTime < -0.001 )
						pto->m_flPumpTime = -0.001;
				}*/

		if (pto->m_fNextAimBonus < -1.0)
		{
			pto->m_fNextAimBonus = -1.0;
		}

		if (pto->m_flNextPrimaryAttack < -1.0)
		{
			pto->m_flNextPrimaryAttack = -1.0;
		}

		if (pto->m_flNextSecondaryAttack < -0.001)
		{
			pto->m_flNextSecondaryAttack = -0.001;
		}

		if (pto->m_flTimeWeaponIdle < -0.001)
		{
			pto->m_flTimeWeaponIdle = -0.001;
		}

		if (pto->m_flNextReload < -0.001)
		{
			pto->m_flNextReload = -0.001;
		}

		if (pto->fuser1 < -0.001)
		{
			pto->fuser1 = -0.001;
		}
	}

	// m_flNextAttack is now part of the weapons, but is part of the player instead
	to->client.m_flNextAttack -= cmd->msec / 1000.0;
	if (to->client.m_flNextAttack < -0.001)
	{
		to->client.m_flNextAttack = -0.001;
	}

	to->client.fuser2 -= cmd->msec / 1000.0;
	if (to->client.fuser2 < -0.001)
	{
		to->client.fuser2 = -0.001;
	}

	to->client.fuser3 -= cmd->msec / 1000.0;
	if (to->client.fuser3 < -0.001)
	{
		to->client.fuser3 = -0.001;
	}

	// Store off the last position from the predicted state.
	HUD_SetLastOrg();

	// Wipe it so we can't use it after this frame
	g_finalstate = nullptr;
}

void DLLEXPORT HUD_PostRunCmd(local_state_t* from, local_state_t* to, usercmd_t* cmd, int runfuncs, double time, unsigned int random_seed)
{
	g_runfuncs = runfuncs != 0;

#if defined( CLIENT_WEAPONS )
	if (cl_lw && cl_lw->value)
	{
		HUD_WeaponsPostThink(from, to, cmd, time, random_seed);
	}
	else
#endif
	{
		to->client.fov = g_lastFOV;
	}

	if (g_irunninggausspred == 1)
	{
		Vector forward;
		gEngfuncs.pfnAngleVectors(v_angles, forward, nullptr, nullptr);
		to->client.velocity = to->client.velocity - forward * g_flApplyVel * 5;
		g_irunninggausspred = false;
	}

	// All games can use FOV state
	g_lastFOV = to->client.fov;
}
