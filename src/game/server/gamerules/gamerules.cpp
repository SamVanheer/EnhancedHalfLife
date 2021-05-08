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
#include "teamplay_gamerules.h"
#include "game.h"
#include "CBaseItem.hpp"
#include "UserMessages.h"
#include "spawnpoints.hpp"

extern DLL_GLOBAL bool	g_fGameOver;

bool CGameRules::CanHaveAmmo(CBasePlayer* pPlayer, const char* pszAmmoName, int iMaxCarry)
{
	int iAmmoIndex;

	if (pszAmmoName)
	{
		iAmmoIndex = pPlayer->GetAmmoIndex(pszAmmoName);

		if (iAmmoIndex > -1)
		{
			if (pPlayer->AmmoInventory(iAmmoIndex) < iMaxCarry)
			{
				// player has room for more of this type of ammo
				return true;
			}
		}
	}

	return false;
}

CBaseEntity* CGameRules::GetPlayerSpawnSpot(CBasePlayer* pPlayer)
{
	CBaseEntity* pSpawnSpot = EntSelectSpawnPoint(pPlayer);

	pPlayer->SetAbsOrigin(pSpawnSpot->GetAbsOrigin() + vec3_up);
	pPlayer->pev->v_angle = vec3_origin;
	pPlayer->SetAbsVelocity(vec3_origin);
	pPlayer->SetAbsAngles(pSpawnSpot->GetAbsAngles());
	pPlayer->pev->punchangle = vec3_origin;
	pPlayer->pev->fixangle = FixAngleMode::Absolute;

	return pSpawnSpot;
}

bool CGameRules::CanHaveItem(CBasePlayer& player, CBaseItem& item)
{
	switch (item.GetType())
	{
	case ItemType::PickupItem: return true;
	case ItemType::Ammo: return true;

	case ItemType::Weapon:
	{
		auto& weapon = static_cast<CBasePlayerItem&>(item);

		// only living players can have items
		if (player.pev->deadflag != DeadFlag::No)
			return false;

		if (weapon.Ammo1Name())
		{
			if (!CanHaveAmmo(&player, weapon.Ammo1Name(), weapon.MaxAmmo1()))
			{
				// we can't carry anymore ammo for this gun. We can only 
				// have the gun if we aren't already carrying one of this type
				if (player.HasPlayerItem(&weapon))
				{
					return false;
				}
			}
		}
		else
		{
			// weapon doesn't use ammo, don't take another if you already have it.
			if (player.HasPlayerItem(&weapon))
			{
				return false;
			}
		}

		// note: will fall through to here if GetItemInfo doesn't fill the struct!
		return true;
	}

	default:
	{
		ALERT(at_error, "CGameRules::CanHaveItem: Unknown item type \"%d\"\n", item.GetType());
		return false;
	}
	}
}

void CGameRules::RefreshSkillData()
{
	auto skill = static_cast<SkillLevel>(CVAR_GET_FLOAT("skill"));

	skill = std::clamp(skill, SkillLevel::Easy, SkillLevel::Hard);

	gSkillData.Level = g_SkillLevel = skill;

	ALERT(at_console, "\nGAME SKILL LEVEL:%d\n", static_cast<int>(skill));

	//Agrunt		
	gSkillData.agruntHealth = GetSkillCvar("sk_agrunt_health");
	gSkillData.agruntDmgPunch = GetSkillCvar("sk_agrunt_dmg_punch");

	// Apache 
	gSkillData.apacheHealth = GetSkillCvar("sk_apache_health");

	// Barney
	gSkillData.barneyHealth = GetSkillCvar("sk_barney_health");

	// Big Momma
	gSkillData.bigmommaHealthFactor = GetSkillCvar("sk_bigmomma_health_factor");
	gSkillData.bigmommaDmgSlash = GetSkillCvar("sk_bigmomma_dmg_slash");
	gSkillData.bigmommaDmgBlast = GetSkillCvar("sk_bigmomma_dmg_blast");
	gSkillData.bigmommaRadiusBlast = GetSkillCvar("sk_bigmomma_radius_blast");

	// Bullsquid
	gSkillData.bullsquidHealth = GetSkillCvar("sk_bullsquid_health");
	gSkillData.bullsquidDmgBite = GetSkillCvar("sk_bullsquid_dmg_bite");
	gSkillData.bullsquidDmgWhip = GetSkillCvar("sk_bullsquid_dmg_whip");
	gSkillData.bullsquidDmgSpit = GetSkillCvar("sk_bullsquid_dmg_spit");

	// Gargantua
	gSkillData.gargantuaHealth = GetSkillCvar("sk_gargantua_health");
	gSkillData.gargantuaDmgSlash = GetSkillCvar("sk_gargantua_dmg_slash");
	gSkillData.gargantuaDmgFire = GetSkillCvar("sk_gargantua_dmg_fire");
	gSkillData.gargantuaDmgStomp = GetSkillCvar("sk_gargantua_dmg_stomp");

	// Hassassin
	gSkillData.hassassinHealth = GetSkillCvar("sk_hassassin_health");

	// Headcrab
	gSkillData.headcrabHealth = GetSkillCvar("sk_headcrab_health");
	gSkillData.headcrabDmgBite = GetSkillCvar("sk_headcrab_dmg_bite");

	// Hgrunt 
	gSkillData.hgruntHealth = GetSkillCvar("sk_hgrunt_health");
	gSkillData.hgruntDmgKick = GetSkillCvar("sk_hgrunt_kick");
	gSkillData.hgruntShotgunPellets = GetSkillCvar("sk_hgrunt_pellets");
	gSkillData.hgruntGrenadeSpeed = GetSkillCvar("sk_hgrunt_gspeed");

	// Houndeye
	gSkillData.houndeyeHealth = GetSkillCvar("sk_houndeye_health");
	gSkillData.houndeyeDmgBlast = GetSkillCvar("sk_houndeye_dmg_blast");

	// ISlave
	gSkillData.slaveHealth = GetSkillCvar("sk_islave_health");
	gSkillData.slaveDmgClaw = GetSkillCvar("sk_islave_dmg_claw");
	gSkillData.slaveDmgClawrake = GetSkillCvar("sk_islave_dmg_clawrake");
	gSkillData.slaveDmgZap = GetSkillCvar("sk_islave_dmg_zap");

	// Icthyosaur
	gSkillData.ichthyosaurHealth = GetSkillCvar("sk_ichthyosaur_health");
	gSkillData.ichthyosaurDmgShake = GetSkillCvar("sk_ichthyosaur_shake");

	// Leech
	gSkillData.leechHealth = GetSkillCvar("sk_leech_health");

	gSkillData.leechDmgBite = GetSkillCvar("sk_leech_dmg_bite");

	// Controller
	gSkillData.controllerHealth = GetSkillCvar("sk_controller_health");
	gSkillData.controllerDmgZap = GetSkillCvar("sk_controller_dmgzap");
	gSkillData.controllerSpeedBall = GetSkillCvar("sk_controller_speedball");
	gSkillData.controllerDmgBall = GetSkillCvar("sk_controller_dmgball");

	// Nihilanth
	gSkillData.nihilanthHealth = GetSkillCvar("sk_nihilanth_health");
	gSkillData.nihilanthZap = GetSkillCvar("sk_nihilanth_zap");

	// Scientist
	gSkillData.scientistHealth = GetSkillCvar("sk_scientist_health");

	// Snark
	gSkillData.snarkHealth = GetSkillCvar("sk_snark_health");
	gSkillData.snarkDmgBite = GetSkillCvar("sk_snark_dmg_bite");
	gSkillData.snarkDmgPop = GetSkillCvar("sk_snark_dmg_pop");

	// Zombie
	gSkillData.zombieHealth = GetSkillCvar("sk_zombie_health");
	gSkillData.zombieDmgOneSlash = GetSkillCvar("sk_zombie_dmg_one_slash");
	gSkillData.zombieDmgBothSlash = GetSkillCvar("sk_zombie_dmg_both_slash");

	//Turret
	gSkillData.turretHealth = GetSkillCvar("sk_turret_health");

	// MiniTurret
	gSkillData.miniturretHealth = GetSkillCvar("sk_miniturret_health");

	// Sentry Turret
	gSkillData.sentryHealth = GetSkillCvar("sk_sentry_health");

	// PLAYER WEAPONS

		// Crowbar whack
	gSkillData.plrDmgCrowbar = GetSkillCvar("sk_plr_crowbar");

	// Glock Round
	gSkillData.plrDmg9MM = GetSkillCvar("sk_plr_9mm_bullet");

	// 357 Round
	gSkillData.plrDmg357 = GetSkillCvar("sk_plr_357_bullet");

	// MP5 Round
	gSkillData.plrDmgMP5 = GetSkillCvar("sk_plr_9mmAR_bullet");

	// M203 grenade
	gSkillData.plrDmgM203Grenade = GetSkillCvar("sk_plr_9mmAR_grenade");

	// Shotgun buckshot
	gSkillData.plrDmgBuckshot = GetSkillCvar("sk_plr_buckshot");

	// Crossbow
	gSkillData.plrDmgCrossbowClient = GetSkillCvar("sk_plr_xbow_bolt_client");
	gSkillData.plrDmgCrossbowMonster = GetSkillCvar("sk_plr_xbow_bolt_monster");

	// RPG
	gSkillData.plrDmgRPG = GetSkillCvar("sk_plr_rpg");

	// Gauss gun
	gSkillData.plrDmgGauss = GetSkillCvar("sk_plr_gauss");

	// Egon Gun
	gSkillData.plrDmgEgonNarrow = GetSkillCvar("sk_plr_egon_narrow");
	gSkillData.plrDmgEgonWide = GetSkillCvar("sk_plr_egon_wide");

	// Hand Grendade
	gSkillData.plrDmgHandGrenade = GetSkillCvar("sk_plr_hand_grenade");

	// Satchel Charge
	gSkillData.plrDmgSatchel = GetSkillCvar("sk_plr_satchel");

	// Tripmine
	gSkillData.plrDmgTripmine = GetSkillCvar("sk_plr_tripmine");

	// MONSTER WEAPONS
	gSkillData.monDmg12MM = GetSkillCvar("sk_12mm_bullet");
	gSkillData.monDmgMP5 = GetSkillCvar("sk_9mmAR_bullet");
	gSkillData.monDmg9MM = GetSkillCvar("sk_9mm_bullet");

	// MONSTER HORNET
	gSkillData.monDmgHornet = GetSkillCvar("sk_hornet_dmg");

	// PLAYER HORNET
	gSkillData.plrDmgHornet = GetSkillCvar("sk_plr_hornet_dmg");


	// HEALTH/CHARGE
	gSkillData.suitchargerCapacity = GetSkillCvar("sk_suitcharger");
	gSkillData.batteryCapacity = GetSkillCvar("sk_battery");
	gSkillData.healthchargerCapacity = GetSkillCvar("sk_healthcharger");
	gSkillData.healthkitCapacity = GetSkillCvar("sk_healthkit");
	gSkillData.scientistHeal = GetSkillCvar("sk_scientist_heal");

	// monster damage adj
	gSkillData.monHead = GetSkillCvar("sk_monster_head");
	gSkillData.monChest = GetSkillCvar("sk_monster_chest");
	gSkillData.monStomach = GetSkillCvar("sk_monster_stomach");
	gSkillData.monLeg = GetSkillCvar("sk_monster_leg");
	gSkillData.monArm = GetSkillCvar("sk_monster_arm");

	// player damage adj
	gSkillData.plrHead = GetSkillCvar("sk_player_head");
	gSkillData.plrChest = GetSkillCvar("sk_player_chest");
	gSkillData.plrStomach = GetSkillCvar("sk_player_stomach");
	gSkillData.plrLeg = GetSkillCvar("sk_player_leg");
	gSkillData.plrArm = GetSkillCvar("sk_player_arm");
}

void CGameRules::LogPrintf(CBaseEntity* player, const char* format, ...)
{
	char message[1024];

	va_list list;

	va_start(list, format);
	vsnprintf(message, sizeof(message), format, list);
	va_end(list);

	// Print to server console
	ALERT(at_logged, "\"%s<%i><%s><%i>\" %s\n",
		STRING(player->pev->netname),
		GETPLAYERUSERID(player->edict()),
		GETPLAYERAUTHID(player->edict()),
		GETPLAYERUSERID(player->edict()),
		message);
}

CGameRules* InstallGameRules()
{
	SERVER_COMMAND("exec game.cfg\n");
	SERVER_EXECUTE();

	if (!gpGlobals->deathmatch)
	{
		// generic half-life
		return new CHalfLifeRules;
	}
	else
	{
		if (teamplay.value > 0)
		{
			// teamplay
			return new CHalfLifeTeamplay;
		}
		if ((int)gpGlobals->deathmatch == 1)
		{
			// vanilla deathmatch
			return new CHalfLifeMultiplay;
		}
		else
		{
			// vanilla deathmatch??
			return new CHalfLifeMultiplay;
		}
	}
}
