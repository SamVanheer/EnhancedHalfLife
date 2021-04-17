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

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"skill.h"
#include	"items.h"
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

bool CHalfLifeRules::FShouldSwitchWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pWeapon)
{
	if (!pPlayer->m_pActiveItem)
	{
		// player doesn't have an active item!
		return true;
	}

	if (!pPlayer->m_pActiveItem->CanHolster())
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

float CHalfLifeRules::FlPlayerFallDamage(CBasePlayer* pPlayer)
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

bool CHalfLifeRules::FPlayerCanRespawn(CBasePlayer* pPlayer)
{
	return true;
}

float CHalfLifeRules::FlPlayerSpawnTime(CBasePlayer* pPlayer)
{
	return gpGlobals->time;//now!
}

int CHalfLifeRules::IPointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled)
{
	return 1;
}

void CHalfLifeRules::PlayerKilled(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor)
{
}

void CHalfLifeRules::DeathNotice(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor)
{
}

void CHalfLifeRules::PlayerGotWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pWeapon)
{
}

float CHalfLifeRules::FlWeaponRespawnTime(CBasePlayerItem* pWeapon)
{
	return -1;
}

float CHalfLifeRules::FlWeaponTryRespawn(CBasePlayerItem* pWeapon)
{
	return 0;
}

Vector CHalfLifeRules::VecWeaponRespawnSpot(CBasePlayerItem* pWeapon)
{
	return pWeapon->pev->origin;
}

int CHalfLifeRules::WeaponShouldRespawn(CBasePlayerItem* pWeapon)
{
	return GR_WEAPON_RESPAWN_NO;
}

bool CHalfLifeRules::CanHaveItem(CBasePlayer* pPlayer, CItem* pItem)
{
	return true;
}

void CHalfLifeRules::PlayerGotItem(CBasePlayer* pPlayer, CItem* pItem)
{
}

int CHalfLifeRules::ItemShouldRespawn(CItem* pItem)
{
	return GR_ITEM_RESPAWN_NO;
}

float CHalfLifeRules::FlItemRespawnTime(CItem* pItem)
{
	return -1;
}

Vector CHalfLifeRules::VecItemRespawnSpot(CItem* pItem)
{
	return pItem->pev->origin;
}

bool CHalfLifeRules::IsAllowedToSpawn(CBaseEntity* pEntity)
{
	return true;
}

void CHalfLifeRules::PlayerGotAmmo(CBasePlayer* pPlayer, char* szName, int iCount)
{
}

int CHalfLifeRules::AmmoShouldRespawn(CBasePlayerAmmo* pAmmo)
{
	return GR_AMMO_RESPAWN_NO;
}

float CHalfLifeRules::FlAmmoRespawnTime(CBasePlayerAmmo* pAmmo)
{
	return -1;
}

Vector CHalfLifeRules::VecAmmoRespawnSpot(CBasePlayerAmmo* pAmmo)
{
	return pAmmo->pev->origin;
}

float CHalfLifeRules::FlHealthChargerRechargeTime()
{
	return 0;// don't recharge
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

bool CHalfLifeRules::FAllowMonsters()
{
	return true;
}
