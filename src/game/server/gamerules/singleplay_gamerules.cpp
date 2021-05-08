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

#include "gamerules.h"
#include "CBaseItem.hpp"
#include "items.h"
#include "UserMessages.h"

extern DLL_GLOBAL bool	g_fGameOver;

CHalfLifeRules::CHalfLifeRules()
{
	RefreshSkillData();
}

void CHalfLifeRules::Think()
{
}

bool CHalfLifeRules::IsMultiplayer()
{
	return false;
}

bool CHalfLifeRules::IsDeathmatch()
{
	return false;
}

bool CHalfLifeRules::IsCoOp()
{
	return false;
}

bool CHalfLifeRules::ShouldSwitchWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pWeapon)
{
	auto activeItem = pPlayer->m_hActiveItem.Get();

	if (!activeItem)
	{
		// player doesn't have an active item!
		return true;
	}

	if (!activeItem->CanHolster())
	{
		return false;
	}

	return true;
}

bool CHalfLifeRules::GetNextBestWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pCurrentWeapon)
{
	return false;
}

bool CHalfLifeRules::ClientConnected(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128])
{
	return true;
}

void CHalfLifeRules::InitHUD(CBasePlayer* pl)
{
}

void CHalfLifeRules::ClientDisconnected(edict_t* pClient)
{
}

float CHalfLifeRules::PlayerFallDamage(CBasePlayer* pPlayer)
{
	// subtract off the speed at which a player is allowed to fall without being hurt,
	// so damage will be based on speed beyond that, not the entire fall
	pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
	return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
}

void CHalfLifeRules::PlayerSpawn(CBasePlayer* pPlayer)
{
}

bool CHalfLifeRules::AllowAutoTargetCrosshair()
{
	return (g_SkillLevel == SkillLevel::Easy);
}

void CHalfLifeRules::PlayerThink(CBasePlayer* pPlayer)
{
}

bool CHalfLifeRules::PlayerCanRespawn(CBasePlayer* pPlayer)
{
	return true;
}

float CHalfLifeRules::PlayerSpawnTime(CBasePlayer* pPlayer)
{
	return gpGlobals->time;//now!
}

int CHalfLifeRules::PointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled)
{
	return 1;
}

void CHalfLifeRules::PlayerKilled(CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* pInflictor)
{
}

void CHalfLifeRules::DeathNotice(CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* pInflictor)
{
}

void CHalfLifeRules::PlayerGotItem(CBasePlayer& player, CBaseItem& item)
{
	//Nothing
}

bool CHalfLifeRules::ItemShouldRespawn(CBaseItem& item)
{
	//Items don't respawn in singleplayer by default
	return item.m_RespawnMode == ItemRespawnMode::Always;
}

float CHalfLifeRules::ItemRespawnTime(CBaseItem& item)
{
	//Allow respawn if it has a custom delay
	if (item.m_flRespawnDelay != ITEM_DEFAULT_RESPAWN_DELAY)
	{
		return gpGlobals->time + item.m_flRespawnDelay;
	}

	//No respawn
	return -1;
}

float CHalfLifeRules::ItemTryRespawn(CBaseItem& item)
{
	//If it has a custom delay then it can spawn when it first checks, otherwise never respawn
	if (item.m_flRespawnDelay != ITEM_DEFAULT_RESPAWN_DELAY)
	{
		return 0;
	}

	return -1;
}

bool CHalfLifeRules::IsAllowedToSpawn(CBaseEntity* pEntity)
{
	return true;
}

void CHalfLifeRules::PlayerGotAmmo(CBasePlayer* pPlayer, char* szName, int iCount)
{
}

float CHalfLifeRules::HealthChargerRechargeTime()
{
	return -1;// don't recharge
}

int CHalfLifeRules::DeadPlayerWeapons(CBasePlayer* pPlayer)
{
	return GR_PLR_DROP_GUN_NO;
}

int CHalfLifeRules::DeadPlayerAmmo(CBasePlayer* pPlayer)
{
	return GR_PLR_DROP_AMMO_NO;
}

int CHalfLifeRules::PlayerRelationship(CBaseEntity* pPlayer, CBaseEntity* pTarget)
{
	// why would a single player in half life need this? 
	return GR_NOTTEAMMATE;
}

bool CHalfLifeRules::AllowMonsters()
{
	return true;
}
