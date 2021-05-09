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
*	functions dealing with the player
*/

#include <limits>

#include "trains.h"
#include "navigation/nodes.h"
#include "shake.h"
#include "game.h"
#include "pm_shared.h"
#include "hltv.h"
#include "UserMessages.h"
#include "client.h"
#include "dll_functions.hpp"
#include "corpse.hpp"
#include "spawnpoints.hpp"

extern DLL_GLOBAL std::uint32_t	g_ulModelIndexPlayer;
extern DLL_GLOBAL bool		g_fGameOver;
int gEvilImpulse101;
extern DLL_GLOBAL int		gDisplayTitle;

bool gInitHUD = true;

constexpr int TRAIN_ACTIVE = 0x80;
constexpr int TRAIN_NEW = 0xc0;
constexpr int TRAIN_OFF = 0x00;
constexpr int TRAIN_NEUTRAL = 0x01;
constexpr int TRAIN_SLOW = 0x02;
constexpr int TRAIN_MEDIUM = 0x03;
constexpr int TRAIN_FAST = 0x04;
constexpr int TRAIN_BACK = 0x05;

// Global Savedata for player
TYPEDESCRIPTION	CBasePlayer::m_playerSaveData[] =
{
	DEFINE_FIELD(CBasePlayer, m_flFlashLightTime, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_iFlashBattery, FIELD_INTEGER),

	DEFINE_FIELD(CBasePlayer, m_afButtonLast, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_afButtonPressed, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_afButtonReleased, FIELD_INTEGER),

	DEFINE_ARRAY(CBasePlayer, m_rgItems, FIELD_INTEGER, MAX_ITEMS),
	DEFINE_FIELD(CBasePlayer, m_afPhysicsFlags, FIELD_INTEGER),

	DEFINE_FIELD(CBasePlayer, m_flTimeStepSound, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flTimeWeaponIdle, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flSwimTime, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flDuckTime, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flWallJumpTime, FIELD_TIME),

	DEFINE_FIELD(CBasePlayer, m_flSuitUpdate, FIELD_TIME),
	DEFINE_ARRAY(CBasePlayer, m_rgSuitPlayList, FIELD_INTEGER, CSUITPLAYLIST),
	DEFINE_FIELD(CBasePlayer, m_iSuitPlayNext, FIELD_INTEGER),
	DEFINE_ARRAY(CBasePlayer, m_rgiSuitNoRepeat, FIELD_INTEGER, CSUITNOREPEAT),
	DEFINE_ARRAY(CBasePlayer, m_rgflSuitNoRepeatTime, FIELD_TIME, CSUITNOREPEAT),
	DEFINE_FIELD(CBasePlayer, m_lastDamageAmount, FIELD_INTEGER),

	DEFINE_ARRAY(CBasePlayer, m_hPlayerWeapons, FIELD_EHANDLE, MAX_WEAPON_TYPES),
	DEFINE_FIELD(CBasePlayer, m_hActiveWeapon, FIELD_EHANDLE),
	DEFINE_FIELD(CBasePlayer, m_hLastWeapon, FIELD_EHANDLE),

	DEFINE_ARRAY(CBasePlayer, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_TYPES),
	DEFINE_FIELD(CBasePlayer, m_idrowndmg, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_idrownrestored, FIELD_INTEGER),

	DEFINE_FIELD(CBasePlayer, m_iTrain, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_bitsHUDDamage, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_flFallVelocity, FIELD_FLOAT),
	DEFINE_FIELD(CBasePlayer, m_iTargetVolume, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iWeaponVolume, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iExtraSoundTypes, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iWeaponFlash, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_fLongJump, FIELD_BOOLEAN),
	DEFINE_FIELD(CBasePlayer, m_fInitHUD, FIELD_BOOLEAN),
	DEFINE_FIELD(CBasePlayer, m_tbdPrev, FIELD_TIME),

	DEFINE_FIELD(CBasePlayer, m_pTank, FIELD_EHANDLE),
	DEFINE_FIELD(CBasePlayer, m_hViewEntity, FIELD_EHANDLE),
	DEFINE_FIELD(CBasePlayer, m_iHideHUD, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iFOV, FIELD_INTEGER),

	//DEFINE_FIELD( CBasePlayer, m_fDeadTime, FIELD_FLOAT ), // only used in multiplayer games
	//DEFINE_FIELD( CBasePlayer, m_fGameHUDInitialized, FIELD_INTEGER ), // only used in multiplayer games
	//DEFINE_FIELD( CBasePlayer, m_flStopExtraSoundTime, FIELD_TIME ),
	//DEFINE_FIELD( CBasePlayer, m_fKnownWeapon, FIELD_INTEGER ), // reset to zero on load
	//DEFINE_FIELD( CBasePlayer, m_iPlayerSound, FIELD_INTEGER ),	// Don't restore, set in Precache()
	//DEFINE_FIELD( CBasePlayer, m_hSndLast, FIELD_EHANDLE ),	// Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_flSndRoomtype, FIELD_FLOAT ),	// Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_flSndRange, FIELD_FLOAT ),	// Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_flgeigerRange, FIELD_FLOAT ),	// Don't restore, reset in Precache()
	//DEFINE_FIELD( CBasePlayer, m_flgeigerDelay, FIELD_FLOAT ),	// Don't restore, reset in Precache()
	//DEFINE_FIELD( CBasePlayer, m_igeigerRangePrev, FIELD_FLOAT ),	// Don't restore, reset in Precache()
	//DEFINE_FIELD( CBasePlayer, m_bStepLeft, FIELD_BOOLEAN ), // Don't need to restore
	//DEFINE_ARRAY( CBasePlayer, m_szTextureName, FIELD_CHARACTER, CBTEXTURENAMEMAX ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_chTextureType, FIELD_CHARACTER ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_fNoPlayerSound, FIELD_BOOLEAN ), // Don't need to restore, debug
	//DEFINE_FIELD( CBasePlayer, m_iClientHealth, FIELD_INTEGER ), // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_iClientBattery, FIELD_INTEGER ), // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_iClientHideHUD, FIELD_INTEGER ), // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_fWeapon, FIELD_BOOLEAN ),  // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_nCustomSprayFrames, FIELD_INTEGER ), // Don't restore, depends on server message after spawning and only matters in multiplayer
	//DEFINE_FIELD( CBasePlayer, m_vecAutoAim, FIELD_VECTOR ), // Don't save/restore - this is recomputed
	//DEFINE_ARRAY( CBasePlayer, m_rgAmmoLast, FIELD_INTEGER, MAX_AMMO_SLOTS ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_fOnTarget, FIELD_BOOLEAN ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_nCustomSprayFrames, FIELD_INTEGER ), // Don't need to restore
};

bool giPrecacheGrunt = false;

LINK_ENTITY_TO_CLASS(player, CBasePlayer);

void CBasePlayer::Pain()
{
	const float flRndSound = RANDOM_FLOAT(0, 1); //sound randomizer

	if (flRndSound <= 0.33)
		EmitSound(SoundChannel::Voice, "player/pl_pain5.wav");
	else if (flRndSound <= 0.66)
		EmitSound(SoundChannel::Voice, "player/pl_pain6.wav");
	else
		EmitSound(SoundChannel::Voice, "player/pl_pain7.wav");
}

int TrainSpeed(int iSpeed, int iMax)
{
	const float fSpeed = static_cast<float>(iSpeed) / static_cast<float>(iMax);

	int iRet = 0;

	if (iSpeed < 0)
		iRet = TRAIN_BACK;
	else if (iSpeed == 0)
		iRet = TRAIN_NEUTRAL;
	else if (fSpeed < 0.33)
		iRet = TRAIN_SLOW;
	else if (fSpeed < 0.66)
		iRet = TRAIN_MEDIUM;
	else
		iRet = TRAIN_FAST;

	return iRet;
}

void CBasePlayer::DeathSound()
{
	// water death sounds
	/*
	if (pev->waterlevel == WaterLevel::Head)
	{
		EmitSound(SoundChannel::Voice, "player/h2odeath.wav", VOL_NORM, ATTN_NONE);
		return;
	}
	*/

	// temporarily using pain sounds for death sounds
	switch (RANDOM_LONG(1, 5))
	{
	case 1:
		EmitSound(SoundChannel::Voice, "player/pl_pain5.wav");
		break;
	case 2:
		EmitSound(SoundChannel::Voice, "player/pl_pain6.wav");
		break;
	case 3:
		EmitSound(SoundChannel::Voice, "player/pl_pain7.wav");
		break;
	}

	// play one of the suit death alarms
	EMIT_GROUPNAME_SUIT(this, "HEV_DEAD");
}

bool CBasePlayer::GiveHealth(float flHealth, int bitsDamageType)
{
	return CBaseMonster::GiveHealth(flHealth, bitsDamageType);
}

Vector CBasePlayer::GetGunPosition()
{
	//	UTIL_MakeVectors(pev->v_angle);
	//	m_HackedGunPos = pev->view_ofs;
	const Vector origin = GetAbsOrigin() + pev->view_ofs;

	return origin;
}

void CBasePlayer::TraceAttack(const TraceAttackInfo& info)
{
	TraceAttackInfo adjustedInfo = info;

	if (pev->takedamage)
	{
		m_LastHitGroup = adjustedInfo.GetTraceResult().iHitgroup;

		{
			float damage = adjustedInfo.GetDamage();

			switch (adjustedInfo.GetTraceResult().iHitgroup)
			{
			case HITGROUP_GENERIC:
				break;
			case HITGROUP_HEAD:
				damage *= gSkillData.plrHead;
				break;
			case HITGROUP_CHEST:
				damage *= gSkillData.plrChest;
				break;
			case HITGROUP_STOMACH:
				damage *= gSkillData.plrStomach;
				break;
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				damage *= gSkillData.plrArm;
				break;
			case HITGROUP_LEFTLEG:
			case HITGROUP_RIGHTLEG:
				damage *= gSkillData.plrLeg;
				break;
			default:
				break;
			}

			adjustedInfo.SetDamage(damage);
		}

		SpawnBlood(info.GetTraceResult().vecEndPos, BloodColor(), adjustedInfo.GetDamage());// a little surface blood.
		TraceBleed(adjustedInfo);
		AddMultiDamage(adjustedInfo.GetAttacker(), this, adjustedInfo.GetDamage(), adjustedInfo.GetDamageTypes());
	}
}

bool CBasePlayer::TakeDamage(const TakeDamageInfo& info)
{
	const float flHealthPrev = pev->health;
	const float flRatio = PLAYER_ARMOR_RATIO;
	float flBonus = PLAYER_ARMOR_BONUS;

	TakeDamageInfo adjustedInfo = info;

	if ((adjustedInfo.GetDamageTypes() & DMG_BLAST) && g_pGameRules->IsMultiplayer())
	{
		// blasts damage armor more.
		flBonus *= 2;
	}

	// Already dead
	if (!IsAlive())
		return false;
	// go take the damage first


	CBaseEntity* pAttacker = adjustedInfo.GetAttacker();

	if (!g_pGameRules->PlayerCanTakeDamage(this, pAttacker))
	{
		// Refuse the damage
		return false;
	}

	// keep track of amount of damage last sustained
	m_lastDamageAmount = adjustedInfo.GetDamage();

	// Armor. 
	if (pev->armorvalue && !(adjustedInfo.GetDamageTypes() & (DMG_FALL | DMG_DROWN)))// armor doesn't protect against fall or drown damage!
	{
		float flNew = adjustedInfo.GetDamage() * flRatio;

		float flArmor = (adjustedInfo.GetDamage() - flNew) * flBonus;

		// Does this use more armor than we have?
		if (flArmor > pev->armorvalue)
		{
			flArmor = pev->armorvalue;
			flArmor *= (1 / flBonus);
			flNew = adjustedInfo.GetDamage() - flArmor;
			pev->armorvalue = 0;
		}
		else
			pev->armorvalue -= flArmor;

		adjustedInfo.SetDamage(flNew);
	}

	// this rounding down is critical!!! If a player ends up with 0.5 health, the engine will get that
	// as an int (zero) and think the player is dead! (this will incite a clientside screentilt, etc)
	const bool fTookDamage = CBaseMonster::TakeDamage(
		{adjustedInfo.GetInflictor(), adjustedInfo.GetAttacker(), std::floor(adjustedInfo.GetDamage()), adjustedInfo.GetDamageTypes()});

	// reset damage time countdown for each type of time based damage player just sustained

	for (int i = 0; i < CDMG_TIMEBASED; i++)
	{
		if (adjustedInfo.GetDamageTypes() & (DMG_PARALYZE << i))
			m_rgbTimeBasedDamage[i] = 0;
	}

	// tell director about it
	MESSAGE_BEGIN(MessageDest::Spectator, SVC_DIRECTOR);
	WRITE_BYTE(9);	// command length in bytes
	WRITE_BYTE(DRC_CMD_EVENT);	// take damage event
	WRITE_SHORT(this->entindex());	// index number of primary entity
	WRITE_SHORT(adjustedInfo.GetInflictor() ? adjustedInfo.GetInflictor()->entindex() : 0);	// index number of secondary entity
	WRITE_LONG(5);   // eventflags (priority and flags)
	MESSAGE_END();

	// how bad is it, doc?

	const bool ftrivial = (pev->health > 75 || m_lastDamageAmount < 5);
	const bool fmajor = (m_lastDamageAmount > 25);
	const bool fcritical = (pev->health < 30);

	// handle all bits set in this damage message,
	// let the suit give player the diagnosis

	// UNDONE: add sounds for types of damage sustained (ie: burn, shock, slash )

	// UNDONE: still need to record damage and heal messages for the following types

		// DMG_BURN	
		// DMG_FREEZE
		// DMG_BLAST
		// DMG_SHOCK

	// have suit diagnose the problem - ie: report damage type
	int bitsDamage = adjustedInfo.GetDamageTypes();
	m_bitsDamageType |= bitsDamage; // Save this so we can report it to the client
	m_bitsHUDDamage = -1;  // make sure the damage bits get resent

	bool ffound = true;

	while (fTookDamage && (!ftrivial || (bitsDamage & DMG_TIMEBASED)) && ffound && bitsDamage)
	{
		ffound = false;

		if (bitsDamage & DMG_CLUB)
		{
			if (fmajor)
				SetSuitUpdate("!HEV_DMG4", SuitSoundType::Sentence, SUIT_NEXT_IN_30SEC);	// minor fracture
			bitsDamage &= ~DMG_CLUB;
			ffound = true;
		}
		if (bitsDamage & (DMG_FALL | DMG_CRUSH))
		{
			if (fmajor)
				SetSuitUpdate("!HEV_DMG5", SuitSoundType::Sentence, SUIT_NEXT_IN_30SEC);	// major fracture
			else
				SetSuitUpdate("!HEV_DMG4", SuitSoundType::Sentence, SUIT_NEXT_IN_30SEC);	// minor fracture

			bitsDamage &= ~(DMG_FALL | DMG_CRUSH);
			ffound = true;
		}

		if (bitsDamage & DMG_BULLET)
		{
			if (m_lastDamageAmount > 5)
				SetSuitUpdate("!HEV_DMG6", SuitSoundType::Sentence, SUIT_NEXT_IN_30SEC);	// blood loss detected
			//else
			//	SetSuitUpdate("!HEV_DMG0", SuitSoundType::Sentence, SUIT_NEXT_IN_30SEC);	// minor laceration

			bitsDamage &= ~DMG_BULLET;
			ffound = true;
		}

		if (bitsDamage & DMG_SLASH)
		{
			if (fmajor)
				SetSuitUpdate("!HEV_DMG1", SuitSoundType::Sentence, SUIT_NEXT_IN_30SEC);	// major laceration
			else
				SetSuitUpdate("!HEV_DMG0", SuitSoundType::Sentence, SUIT_NEXT_IN_30SEC);	// minor laceration

			bitsDamage &= ~DMG_SLASH;
			ffound = true;
		}

		if (bitsDamage & DMG_SONIC)
		{
			if (fmajor)
				SetSuitUpdate("!HEV_DMG2", SuitSoundType::Sentence, SUIT_NEXT_IN_1MIN);	// internal bleeding
			bitsDamage &= ~DMG_SONIC;
			ffound = true;
		}

		if (bitsDamage & (DMG_POISON | DMG_PARALYZE))
		{
			SetSuitUpdate("!HEV_DMG3", SuitSoundType::Sentence, SUIT_NEXT_IN_1MIN);	// blood toxins detected
			bitsDamage &= ~(DMG_POISON | DMG_PARALYZE);
			ffound = true;
		}

		if (bitsDamage & DMG_ACID)
		{
			SetSuitUpdate("!HEV_DET1", SuitSoundType::Sentence, SUIT_NEXT_IN_1MIN);	// hazardous chemicals detected
			bitsDamage &= ~DMG_ACID;
			ffound = true;
		}

		if (bitsDamage & DMG_NERVEGAS)
		{
			SetSuitUpdate("!HEV_DET0", SuitSoundType::Sentence, SUIT_NEXT_IN_1MIN);	// biohazard detected
			bitsDamage &= ~DMG_NERVEGAS;
			ffound = true;
		}

		if (bitsDamage & DMG_RADIATION)
		{
			SetSuitUpdate("!HEV_DET2", SuitSoundType::Sentence, SUIT_NEXT_IN_1MIN);	// radiation detected
			bitsDamage &= ~DMG_RADIATION;
			ffound = true;
		}
		if (bitsDamage & DMG_SHOCK)
		{
			bitsDamage &= ~DMG_SHOCK;
			ffound = true;
		}
	}

	pev->punchangle.x = -2;

	if (fTookDamage && !ftrivial && fmajor && flHealthPrev >= 75)
	{
		// first time we take major damage...
		// turn automedic on if not on
		SetSuitUpdate("!HEV_MED1", SuitSoundType::Sentence, SUIT_NEXT_IN_30MIN);	// automedic on

		// give morphine shot if not given recently
		SetSuitUpdate("!HEV_HEAL7", SuitSoundType::Sentence, SUIT_NEXT_IN_30MIN);	// morphine shot
	}

	if (fTookDamage && !ftrivial && fcritical && flHealthPrev < 75)
	{
		// already took major damage, now it's critical...
		if (pev->health < 6)
			SetSuitUpdate("!HEV_HLTH3", SuitSoundType::Sentence, SUIT_NEXT_IN_10MIN);	// near death
		else if (pev->health < 20)
			SetSuitUpdate("!HEV_HLTH2", SuitSoundType::Sentence, SUIT_NEXT_IN_10MIN);	// health critical

		// give critical health warnings
		if (!RANDOM_LONG(0, 3) && flHealthPrev < 50)
			SetSuitUpdate("!HEV_DMG7", SuitSoundType::Sentence, SUIT_NEXT_IN_5MIN); //seek medical attention
	}

	// if we're taking time based damage, warn about its continuing effects
	if (fTookDamage && (adjustedInfo.GetDamageTypes() & DMG_TIMEBASED) && flHealthPrev < 75)
	{
		if (flHealthPrev < 50)
		{
			if (!RANDOM_LONG(0, 3))
				SetSuitUpdate("!HEV_DMG7", SuitSoundType::Sentence, SUIT_NEXT_IN_5MIN); //seek medical attention
		}
		else
			SetSuitUpdate("!HEV_HLTH1", SuitSoundType::Sentence, SUIT_NEXT_IN_10MIN);	// health dropping
	}

	return fTookDamage;
}

void CBasePlayer::PackDeadPlayerItems()
{
	// get the game rules 
	const int iWeaponRules = g_pGameRules->DeadPlayerWeapons(this);
	const int iAmmoRules = g_pGameRules->DeadPlayerAmmo(this);

	if (iWeaponRules == GR_PLR_DROP_GUN_NO && iAmmoRules == GR_PLR_DROP_AMMO_NO)
	{
		// nothing to pack. Remove the weapons and return. Don't call create on the box!
		RemoveAllItems(true);
		return;
	}

	CBaseWeapon* rgpPackWeapons[MAX_WEAPONS]{};
	int iPackAmmo[MAX_AMMO_TYPES + 1];
	int iPW = 0;// index into packweapons array
	int iPA = 0;// index into packammo array

	memset(iPackAmmo, -1, sizeof(iPackAmmo));

	// go through all of the weapons and make a list of the ones to pack
	for (int i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		if (CBaseWeapon* weapon = m_hPlayerWeapons[i]; weapon)
		{
			// there's a weapon here. Should I pack it?
			while (weapon)
			{
				switch (iWeaponRules)
				{
				case GR_PLR_DROP_GUN_ACTIVE:
					if (weapon == m_hActiveWeapon)
					{
						// this is the active weapon. Pack it.
						rgpPackWeapons[iPW++] = weapon;
					}
					break;

				case GR_PLR_DROP_GUN_ALL:
					rgpPackWeapons[iPW++] = weapon;
					break;

				default:
					break;
				}

				weapon = weapon->m_hNext;
			}
		}
	}

	// now go through ammo and make a list of which types to pack.
	if (iAmmoRules != GR_PLR_DROP_AMMO_NO)
	{
		for (int i = 0; i < MAX_AMMO_TYPES; i++)
		{
			if (m_rgAmmo[i] > 0)
			{
				// player has some ammo of this type.
				switch (iAmmoRules)
				{
				case GR_PLR_DROP_AMMO_ALL:
					iPackAmmo[iPA++] = i;
					break;

				case GR_PLR_DROP_AMMO_ACTIVE:
					if (auto activeWeapon = m_hActiveWeapon.Get(); activeWeapon && i == activeWeapon->PrimaryAmmoIndex())
					{
						// this is the primary ammo type for the active weapon
						iPackAmmo[iPA++] = i;
					}
					else if (auto activeWeapon = m_hActiveWeapon.Get(); activeWeapon && i == activeWeapon->SecondaryAmmoIndex())
					{
						// this is the secondary ammo type for the active weapon
						iPackAmmo[iPA++] = i;
					}
					break;

				default:
					break;
				}
			}
		}
	}

	// create a box to pack the stuff into.
	CWeaponBox* pWeaponBox = (CWeaponBox*)CBaseEntity::Create("weaponbox", GetAbsOrigin(), GetAbsAngles(), this);

	// don't let weaponbox tilt.
	pWeaponBox->SetAbsAngles({0, pWeaponBox->GetAbsAngles().y, 0});

	pWeaponBox->SetThink(&CWeaponBox::Kill);
	pWeaponBox->pev->nextthink = gpGlobals->time + 120;

	// back these two lists up to their first elements
	iPA = 0;
	iPW = 0;

	// pack the ammo
	while (iPackAmmo[iPA] != -1)
	{
		pWeaponBox->PackAmmo(MAKE_STRING(CBaseWeapon::AmmoInfoArray[iPackAmmo[iPA]].pszName), m_rgAmmo[iPackAmmo[iPA]]);
		iPA++;
	}

	// now pack all of the weapons in the lists
	while (rgpPackWeapons[iPW])
	{
		// weapon unhooked from the player. Pack it into der box.
		pWeaponBox->PackWeapon(rgpPackWeapons[iPW]);

		iPW++;
	}

	pWeaponBox->SetAbsVelocity(GetAbsVelocity() * 1.2);// weaponbox has player's velocity, then some.

	RemoveAllItems(true);// now strip off everything that wasn't handled by the code above.
}

void CBasePlayer::RemoveAllItems(bool removeSuit)
{
	if (auto activeWeapon = m_hActiveWeapon.Get(); activeWeapon)
	{
		ResetAutoaim();
		activeWeapon->Holster();
		m_hActiveWeapon = nullptr;
	}

	m_hLastWeapon = nullptr;

	if (m_pTank != nullptr)
	{
		m_pTank->Use({this, this, UseType::Off});
		m_pTank = nullptr;
	}

	for (int i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		CBaseWeapon* activeWeapon = m_hActiveWeapon = m_hPlayerWeapons[i];
		while (activeWeapon)
		{
			auto pPendingWeapon = activeWeapon->m_hNext;
			activeWeapon->Drop();
			activeWeapon = m_hActiveWeapon = pPendingWeapon;
		}
		m_hPlayerWeapons[i] = nullptr;
	}
	m_hActiveWeapon = nullptr;

	pev->viewmodel = iStringNull;
	pev->weaponmodel = iStringNull;

	if (removeSuit)
		pev->weapons = 0;
	else
		pev->weapons &= ~WEAPON_ALLWEAPONS;

	for (int i = 0; i < MAX_AMMO_TYPES; i++)
		m_rgAmmo[i] = 0;

	UpdateClientData();
}

void CBasePlayer::Killed(const KilledInfo& info)
{
	// Holster weapon immediately, to allow it to cleanup
	if (auto activeWeapon = m_hActiveWeapon.Get(); activeWeapon)
		activeWeapon->Holster();

	g_pGameRules->PlayerKilled(this, info.GetAttacker(), info.GetInflictor());

	if (m_pTank != nullptr)
	{
		m_pTank->Use({this, this, UseType::Off});
		m_pTank = nullptr;
	}

	// this client isn't going to be thinking for a while, so reset the sound until they respawn
	if (CSound* pSound = CSoundEnt::SoundPointerForIndex(CSoundEnt::ClientSoundIndex(edict())); pSound)
	{
		pSound->Reset();
	}

	SetAnimation(PlayerAnim::Die);

	m_iRespawnFrames = 0;

	pev->modelindex = g_ulModelIndexPlayer;    // don't use eyes

	pev->deadflag = DeadFlag::Dying;
	SetMovetype(Movetype::Toss);
	ClearBits(pev->flags, FL_ONGROUND);
	if (GetAbsVelocity().z < 10)
		SetAbsVelocity(GetAbsVelocity() + Vector{0, 0, RANDOM_FLOAT(0, 300)});

	// clear out the suit message cache so we don't keep chattering
	SetSuitUpdate(nullptr, SuitSoundType::Sentence, 0);

	// send "health" update message to zero
	m_iClientHealth = 0;
	MESSAGE_BEGIN(MessageDest::One, gmsgHealth, this);
	WRITE_SHORT(m_iClientHealth);
	MESSAGE_END();

	// Tell Ammo Hud that the player is dead
	MESSAGE_BEGIN(MessageDest::One, gmsgCurWeapon, this);
	WRITE_BYTE(static_cast<int>(WeaponState::NotActive));
	WRITE_BYTE(0XFF);
	WRITE_BYTE(0xFF);
	MESSAGE_END();

	// reset FOV
	m_iFOV = m_iClientFOV = 0;

	MESSAGE_BEGIN(MessageDest::One, gmsgSetFOV, this);
	WRITE_BYTE(0);
	MESSAGE_END();

	// UNDONE: Put this in, but add FFADE_PERMANENT and make fade time 8.8 instead of 4.12
	// UTIL_ScreenFade( edict(), Vector(128,0,0), 6, 15, 255, FFADE_OUT | FFADE_MODULATE );

	if ((pev->health < -40 && info.GetGibType() != GibType::Never) || info.GetGibType() == GibType::Always)
	{
		SetSolidType(Solid::Not);
		GibMonster();	// This clears pev->model
		pev->effects |= EF_NODRAW;
		return;
	}

	DeathSound();

	SetAbsAngles({0, GetAbsAngles().y, 0});

	SetThink(&CBasePlayer::PlayerDeathThink);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CBasePlayer::SetAnimation(PlayerAnim playerAnim)
{
	float speed = GetAbsVelocity().Length2D();

	if (pev->flags & FL_FROZEN)
	{
		speed = 0;
		playerAnim = PlayerAnim::Idle;
	}

	switch (playerAnim)
	{
	case PlayerAnim::Jump:
		m_IdealActivity = ACT_HOP;
		break;

	case PlayerAnim::SuperJump:
		m_IdealActivity = ACT_LEAP;
		break;

	case PlayerAnim::Die:
		m_IdealActivity = ACT_DIESIMPLE;
		m_IdealActivity = GetDeathActivity();
		break;

	case PlayerAnim::Attack1:
		switch (m_Activity)
		{
		case ACT_HOVER:
		case ACT_SWIM:
		case ACT_HOP:
		case ACT_LEAP:
		case ACT_DIESIMPLE:
			m_IdealActivity = m_Activity;
			break;
		default:
			m_IdealActivity = ACT_RANGE_ATTACK1;
			break;
		}
		break;
	case PlayerAnim::Idle:
	case PlayerAnim::Walk:
		if (!IsBitSet(pev->flags, FL_ONGROUND) && (m_Activity == ACT_HOP || m_Activity == ACT_LEAP))	// Still jumping
		{
			m_IdealActivity = m_Activity;
		}
		else if (pev->waterlevel > WaterLevel::Feet)
		{
			if (speed == 0)
				m_IdealActivity = ACT_HOVER;
			else
				m_IdealActivity = ACT_SWIM;
		}
		else
		{
			m_IdealActivity = ACT_WALK;
		}
		break;
	}

	char szAnim[64];
	int animDesired;

	switch (m_IdealActivity)
	{
	case ACT_HOVER:
	case ACT_LEAP:
	case ACT_SWIM:
	case ACT_HOP:
	case ACT_DIESIMPLE:
	default:
		if (m_Activity == m_IdealActivity)
			return;
		m_Activity = m_IdealActivity;

		animDesired = LookupActivity(m_Activity);
		// Already using the desired animation?
		if (pev->sequence == animDesired)
			return;

		pev->gaitsequence = 0;
		pev->sequence = animDesired;
		pev->frame = 0;
		ResetSequenceInfo();
		return;

	case ACT_RANGE_ATTACK1:
		if (IsBitSet(pev->flags, FL_DUCKING))	// crouching
			safe_strcpy(szAnim, "crouch_shoot_");
		else
			safe_strcpy(szAnim, "ref_shoot_");
		safe_strcat(szAnim, m_szAnimExtension);
		animDesired = LookupSequence(szAnim);
		if (animDesired == -1)
			animDesired = 0;

		if (pev->sequence != animDesired || !m_fSequenceLoops)
		{
			pev->frame = 0;
		}

		if (!m_fSequenceLoops)
		{
			pev->effects |= EF_NOINTERP;
		}

		m_Activity = m_IdealActivity;

		pev->sequence = animDesired;
		ResetSequenceInfo();
		break;

	case ACT_WALK:
		if (m_Activity != ACT_RANGE_ATTACK1 || m_fSequenceFinished)
		{
			if (IsBitSet(pev->flags, FL_DUCKING))	// crouching
				safe_strcpy(szAnim, "crouch_aim_");
			else
				safe_strcpy(szAnim, "ref_aim_");
			safe_strcat(szAnim, m_szAnimExtension);
			animDesired = LookupSequence(szAnim);
			if (animDesired == -1)
				animDesired = 0;
			m_Activity = ACT_WALK;
		}
		else
		{
			animDesired = pev->sequence;
		}
	}

	if (IsBitSet(pev->flags, FL_DUCKING))
	{
		if (speed == 0)
		{
			pev->gaitsequence = LookupActivity(ACT_CROUCHIDLE);
			// pev->gaitsequence	= LookupActivity( ACT_CROUCH );
		}
		else
		{
			pev->gaitsequence = LookupActivity(ACT_CROUCH);
		}
	}
	else if (speed > 220)
	{
		pev->gaitsequence = LookupActivity(ACT_RUN);
	}
	else if (speed > 0)
	{
		pev->gaitsequence = LookupActivity(ACT_WALK);
	}
	else
	{
		// pev->gaitsequence	= LookupActivity( ACT_WALK );
		pev->gaitsequence = LookupSequence("deep_idle");
	}


	// Already using the desired animation?
	if (pev->sequence == animDesired)
		return;

	//ALERT( at_console, "Set animation to %d\n", animDesired );
	// Reset to first frame of desired animation
	pev->sequence = animDesired;
	pev->frame = 0;
	ResetSequenceInfo();
}

void CBasePlayer::WaterMove()
{
	if (GetMovetype() == Movetype::Noclip)
		return;

	if (pev->health < 0)
		return;

	if (pev->waterlevel != WaterLevel::Head)
	{
		// not underwater

		// play 'up for air' sound
		if (pev->air_finished < gpGlobals->time)
			EmitSound(SoundChannel::Voice, "player/pl_wade1.wav");
		else if (pev->air_finished < gpGlobals->time + 9)
			EmitSound(SoundChannel::Voice, "player/pl_wade2.wav");

		pev->air_finished = gpGlobals->time + PLAYER_AIRTIME;
		pev->dmg = 2;

		// if we took drowning damage, give it back slowly
		if (m_idrowndmg > m_idrownrestored)
		{
			// set drowning damage bit.  hack - dmg_drownrecover actually
			// makes the time based damage code 'give back' health over time.
			// make sure counter is cleared so we start count correctly.

			// NOTE: this actually causes the count to continue restarting
			// until all drowning damage is healed.

			m_bitsDamageType |= DMG_DROWNRECOVER;
			m_bitsDamageType &= ~DMG_DROWN;
			m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;
		}

	}
	else
	{	// fully under water
		// stop restoring damage while underwater
		m_bitsDamageType &= ~DMG_DROWNRECOVER;
		m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;

		if (pev->air_finished < gpGlobals->time)		// drown!
		{
			if (pev->pain_finished < gpGlobals->time)
			{
				// take drowning damage
				pev->dmg += 1;
				if (pev->dmg > 5)
					pev->dmg = 5;
				TakeDamage({UTIL_GetWorld(), UTIL_GetWorld(), pev->dmg, DMG_DROWN});
				pev->pain_finished = gpGlobals->time + 1;

				// track drowning damage, give it back when
				// player finally takes a breath

				m_idrowndmg += pev->dmg;
			}
		}
		else
		{
			m_bitsDamageType &= ~DMG_DROWN;
		}
	}

	if (pev->waterlevel == WaterLevel::Dry)
	{
		if (IsBitSet(pev->flags, FL_INWATER))
		{
			ClearBits(pev->flags, FL_INWATER);
		}
		return;
	}

	// make bubbles

	const int air = (int)(pev->air_finished - gpGlobals->time);
	if (!RANDOM_LONG(0, 0x1f) && RANDOM_LONG(0, PLAYER_AIRTIME - 1) >= air)
	{
		switch (RANDOM_LONG(0, 3))
		{
		case 0:	EmitSound(SoundChannel::Body, "player/pl_swim1.wav", 0.8); break;
		case 1:	EmitSound(SoundChannel::Body, "player/pl_swim2.wav", 0.8); break;
		case 2:	EmitSound(SoundChannel::Body, "player/pl_swim3.wav", 0.8); break;
		case 3:	EmitSound(SoundChannel::Body, "player/pl_swim4.wav", 0.8); break;
		}
	}

	if (pev->watertype == Contents::Lava)		// do damage
	{
		if (pev->dmgtime < gpGlobals->time)
			TakeDamage({UTIL_GetWorld(), UTIL_GetWorld(), 10.0f * static_cast<int>(pev->waterlevel), DMG_BURN});
	}
	else if (pev->watertype == Contents::Slime)		// do damage
	{
		pev->dmgtime = gpGlobals->time + 1;
		TakeDamage({UTIL_GetWorld(), UTIL_GetWorld(), 4.0f * static_cast<int>(pev->waterlevel), DMG_ACID});
	}

	if (!IsBitSet(pev->flags, FL_INWATER))
	{
		SetBits(pev->flags, FL_INWATER);
		pev->dmgtime = 0;
	}
}

bool CBasePlayer::IsOnLadder()
{
	return GetMovetype() == Movetype::Fly;
}

void CBasePlayer::PlayerDeathThink()
{
	if (IsBitSet(pev->flags, FL_ONGROUND))
	{
		const float flForward = GetAbsVelocity().Length() - 20;
		if (flForward <= 0)
			SetAbsVelocity(vec3_origin);
		else
			SetAbsVelocity(flForward * GetAbsVelocity().Normalize());
	}

	if (HasWeapons())
	{
		// we drop the guns here because weapons that have an area effect and can kill their user
		// will sometimes crash coming back from CBasePlayer::Killed() if they kill their owner because the
		// player class sometimes is freed. It's safer to manipulate the weapons once we know
		// we aren't calling into any of their code anymore through the player pointer.
		PackDeadPlayerItems();
	}

	if (pev->modelindex && (!m_fSequenceFinished) && (pev->deadflag == DeadFlag::Dying))
	{
		StudioFrameAdvance();

		m_iRespawnFrames++;				// Note, these aren't necessarily real "frames", so behavior is dependent on # of client movement commands
		if (m_iRespawnFrames < 120)   // Animations should be no longer than this
			return;
	}

	// once we're done animating our death and we're on the ground, we want to set movetype to None so our dead body won't do collisions and stuff anymore
	// this prevents a bug where the dead body would go to a player's head if he walked over it while the dead player was clicking their button to respawn
	if (GetMovetype() != Movetype::None && IsBitSet(pev->flags, FL_ONGROUND))
		SetMovetype(Movetype::None);

	if (pev->deadflag == DeadFlag::Dying)
		pev->deadflag = DeadFlag::Dead;

	StopAnimation();

	pev->effects |= EF_NOINTERP;
	pev->framerate = 0.0;

	const bool fAnyButtonDown = (pev->button & ~IN_SCORE);

	// wait for all buttons released
	if (pev->deadflag == DeadFlag::Dead)
	{
		if (fAnyButtonDown)
			return;

		if (g_pGameRules->PlayerCanRespawn(this))
		{
			m_fDeadTime = gpGlobals->time;
			pev->deadflag = DeadFlag::Respawnable;
		}

		return;
	}

	// if the player has been dead for one second longer than allowed by forcerespawn, 
	// forcerespawn isn't on. Send the player off to an intermission camera until they 
	// choose to respawn.
	if (g_pGameRules->IsMultiplayer() && (gpGlobals->time > (m_fDeadTime + 6)) && !(m_afPhysicsFlags & PFLAG_OBSERVER))
	{
		// go to dead camera. 
		StartDeathCam();
	}

	if (pev->iuser1)	// player is in spectator mode
		return;

	// wait for any button down,  or mp_forcerespawn is set and the respawn time is up
	if (!fAnyButtonDown
		&& !(g_pGameRules->IsMultiplayer() && forcerespawn.value > 0 && (gpGlobals->time > (m_fDeadTime + 5))))
		return;

	pev->button = 0;
	m_iRespawnFrames = 0;

	//ALERT(at_console, "Respawn\n");

	respawn(this, !(m_afPhysicsFlags & PFLAG_OBSERVER));// don't copy a corpse if we're in deathcam.
	pev->nextthink = -1;
}

void CBasePlayer::StartDeathCam()
{
	if (pev->view_ofs == vec3_origin)
	{
		// don't accept subsequent attempts to StartDeathCam()
		return;
	}

	CBaseEntity* pSpot = UTIL_FindEntityByClassname(nullptr, "info_intermission");

	if (!IsNullEnt(pSpot))
	{
		// at least one intermission spot in the world.
		int iRand = RANDOM_LONG(0, 3);

		while (iRand > 0)
		{
			CBaseEntity* pNewSpot = UTIL_FindEntityByClassname(pSpot, "info_intermission");

			if (pNewSpot)
			{
				pSpot = pNewSpot;
			}

			iRand--;
		}

		CopyToBodyQue(this);

		SetAbsOrigin(pSpot->GetAbsOrigin());
		SetAbsAngles(pev->v_angle = pSpot->pev->v_angle);
	}
	else
	{
		// no intermission spot. Push them up in the air, looking down at their corpse
		TraceResult tr;
		CopyToBodyQue(this);
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, 128), IgnoreMonsters::Yes, this, &tr);

		SetAbsOrigin(tr.vecEndPos);
		SetAbsAngles(pev->v_angle = VectorAngles(tr.vecEndPos - GetAbsOrigin()));
	}

	// start death cam

	m_afPhysicsFlags |= PFLAG_OBSERVER;
	pev->view_ofs = vec3_origin;
	pev->fixangle = FixAngleMode::Absolute;
	SetSolidType(Solid::Not);
	SetDamageMode(DamageMode::No);
	SetMovetype(Movetype::None);
	pev->modelindex = 0;
}

void CBasePlayer::StartObserver(Vector vecPosition, Vector vecViewAngle)
{
	// clear any clientside entities attached to this player
	MESSAGE_BEGIN(MessageDest::PAS, SVC_TEMPENTITY, GetAbsOrigin());
	WRITE_BYTE(TE_KILLPLAYERATTACHMENTS);
	WRITE_BYTE((byte)entindex());
	MESSAGE_END();

	// Holster weapon immediately, to allow it to cleanup
	if (auto activeWeapon = m_hActiveWeapon.Get(); activeWeapon)
		activeWeapon->Holster();

	if (m_pTank != nullptr)
	{
		m_pTank->Use({this, this, UseType::Off});
		m_pTank = nullptr;
	}

	// clear out the suit message cache so we don't keep chattering
	SetSuitUpdate(nullptr, SuitSoundType::Sentence, 0);

	// Tell Ammo Hud that the player is dead
	MESSAGE_BEGIN(MessageDest::One, gmsgCurWeapon, this);
	WRITE_BYTE(static_cast<int>(WeaponState::NotActive));
	WRITE_BYTE(0XFF);
	WRITE_BYTE(0xFF);
	MESSAGE_END();

	// reset FOV
	m_iFOV = m_iClientFOV = 0;
	MESSAGE_BEGIN(MessageDest::One, gmsgSetFOV, this);
	WRITE_BYTE(0);
	MESSAGE_END();

	// Setup flags
	m_iHideHUD = (HIDEHUD_HEALTH | HIDEHUD_WEAPONS);
	m_afPhysicsFlags |= PFLAG_OBSERVER;
	pev->effects = EF_NODRAW;
	pev->view_ofs = vec3_origin;
	pev->v_angle = vecViewAngle;
	SetAbsAngles(vecViewAngle);
	pev->fixangle = FixAngleMode::Absolute;
	SetSolidType(Solid::Not);
	SetDamageMode(DamageMode::No);
	SetMovetype(Movetype::None);
	ClearBits(m_afPhysicsFlags, PFLAG_DUCKING);
	ClearBits(pev->flags, FL_DUCKING);
	pev->deadflag = DeadFlag::Respawnable;
	pev->health = 1;

	// Clear out the status bar
	m_fInitHUD = true;

	pev->team = 0;
	MESSAGE_BEGIN(MessageDest::All, gmsgTeamInfo);
	WRITE_BYTE(entindex());
	WRITE_STRING("");
	MESSAGE_END();

	// Remove all the player's stuff
	RemoveAllItems(false);

	// Move them to the new position
	SetAbsOrigin(vecPosition);

	// Find a player to watch
	m_flNextObserverInput = 0;
	Observer_SetMode(m_iObserverLastMode);
}

void CBasePlayer::PlayerUse()
{
	if (IsObserver())
		return;

	// Was use pressed or released?
	if (!((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_USE))
		return;

	// Hit Use on a train?
	if (m_afButtonPressed & IN_USE)
	{
		if (m_pTank != nullptr)
		{
			// Stop controlling the tank
			m_pTank->Use({this, this, UseType::Off});
			m_pTank = nullptr;
			return;
		}
		else
		{
			if (m_afPhysicsFlags & PFLAG_ONTRAIN)
			{
				m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
				m_iTrain = TRAIN_NEW | TRAIN_OFF;
				return;
			}
			else
			{	// Start controlling the train!
				CBaseEntity* pTrain = CBaseEntity::Instance(pev->groundentity);

				if (pTrain && !(pev->button & IN_JUMP) && IsBitSet(pev->flags, FL_ONGROUND) && (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) && pTrain->OnControls(this))
				{
					m_afPhysicsFlags |= PFLAG_ONTRAIN;
					m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
					m_iTrain |= TRAIN_NEW;
					EmitSound(SoundChannel::Item, "plats/train_use1.wav", 0.8);
					return;
				}
			}
		}
	}

	CBaseEntity* pObject = nullptr;
	CBaseEntity* pClosest = nullptr;
	Vector		vecLOS;
	float flMaxDot = VIEW_FIELD_NARROW;
	float flDot;

	UTIL_MakeVectors(pev->v_angle);// so we know which way we are facing

	while ((pObject = UTIL_FindEntityInSphere(pObject, GetAbsOrigin(), PLAYER_USE_SEARCH_RADIUS)) != nullptr)
	{

		if (pObject->ObjectCaps() & (FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE))
		{
			// !!!PERFORMANCE- should this check be done on a per case basis AFTER we've determined that
			// this object is actually usable? This dot is being done for every object within PLAYER_SEARCH_RADIUS
			// when player hits the use key. How many objects can be in that area, anyway? (sjb)
			vecLOS = (GetBrushModelOrigin(pObject) - (GetAbsOrigin() + pev->view_ofs));

			// This essentially moves the origin of the target to the corner nearest the player to test to see 
			// if it's "hull" is in the view cone
			vecLOS = UTIL_ClampVectorToBox(vecLOS, pObject->pev->size * 0.5);

			flDot = DotProduct(vecLOS, gpGlobals->v_forward);
			if (flDot > flMaxDot)
			{// only if the item is in front of the user
				pClosest = pObject;
				flMaxDot = flDot;
				//				ALERT( at_console, "%s : %f\n", pObject->GetClassname(), flDot );
			}
			//			ALERT( at_console, "%s : %f\n", pObject->GetClassname(), flDot );
		}
	}
	pObject = pClosest;

	// Found an object
	if (pObject)
	{
		//!!!UNDONE: traceline here to prevent USEing buttons through walls			
		int caps = pObject->ObjectCaps();

		if (m_afButtonPressed & IN_USE)
			EmitSound(SoundChannel::Item, "common/wpn_select.wav", 0.4);

		if (((pev->button & IN_USE) && (caps & FCAP_CONTINUOUS_USE)) ||
			((m_afButtonPressed & IN_USE) && (caps & (FCAP_IMPULSE_USE | FCAP_ONOFF_USE))))
		{
			if (caps & FCAP_CONTINUOUS_USE)
				m_afPhysicsFlags |= PFLAG_USING;

			pObject->Use({this, this, UseType::Set, 1});
		}
		// UNDONE: Send different USE codes for ON/OFF.  Cache last ONOFF_USE object to send 'off' if you turn away
		else if ((m_afButtonReleased & IN_USE) && (pObject->ObjectCaps() & FCAP_ONOFF_USE))	// BUGBUG This is an "off" use
		{
			pObject->Use({this, this, UseType::Set, 0});
		}
	}
	else
	{
		if (m_afButtonPressed & IN_USE)
			EmitSound(SoundChannel::Item, "common/wpn_denyselect.wav", 0.4);
	}
}

void CBasePlayer::Jump()
{
	if (IsBitSet(pev->flags, FL_WATERJUMP))
		return;

	if (pev->waterlevel >= WaterLevel::Waist)
	{
		return;
	}

	// jump velocity is sqrt( height * gravity * 2)

	// If this isn't the first frame pressing the jump button, break out.
	if (!IsBitSet(m_afButtonPressed, IN_JUMP))
		return;         // don't pogo stick

	if (!(pev->flags & FL_ONGROUND) || !pev->groundentity)
	{
		return;
	}

	// many features in this function use v_forward, so makevectors now.
	UTIL_MakeVectors(GetAbsAngles());

	// ClearBits(pev->flags, FL_ONGROUND);		// don't stairwalk

	SetAnimation(PlayerAnim::Jump);

	if (m_fLongJump &&
		(pev->button & IN_DUCK) &&
		(pev->flDuckTime > 0) &&
		GetAbsVelocity().Length() > 50)
	{
		SetAnimation(PlayerAnim::SuperJump);
	}

	// If you're standing on a conveyor, add it's velocity to yours (for momentum)
	auto pGround = InstanceOrNull(pev->groundentity);
	if (pGround && (pGround->pev->flags & FL_CONVEYOR))
	{
		SetAbsVelocity(GetAbsVelocity() + pev->basevelocity);
	}
}

/**
*	@brief This is a glorious hack to find free space when you've crouched into some solid space
*	Our crouching collisions do not work correctly for some reason and this is easier than fixing the problem :(
*/
void FixPlayerCrouchStuck(CBasePlayer* pPlayer)
{
	TraceResult trace;

	Vector origin = pPlayer->GetAbsOrigin();

	// Move up as many as 18 pixels if the player is stuck.
	for (int i = 0; i < 18; i++)
	{
		UTIL_TraceHull(origin, origin, IgnoreMonsters::No, Hull::Head, pPlayer, &trace);
		if (trace.fStartSolid)
			++origin.z;
		else
			break;
	}

	pPlayer->SetAbsOrigin(origin);
}

void CBasePlayer::Duck()
{
	if (pev->button & IN_DUCK)
	{
		if (m_IdealActivity != ACT_LEAP)
		{
			SetAnimation(PlayerAnim::Walk);
		}
	}
}

int  CBasePlayer::Classify()
{
	return CLASS_PLAYER;
}

void CBasePlayer::AddPoints(int score, bool bAllowNegativeScore)
{
	// Positive score always adds
	if (score < 0)
	{
		if (!bAllowNegativeScore)
		{
			if (pev->frags < 0)		// Can't go more negative
				return;

			if (-score > pev->frags)	// Will this go negative?
			{
				score = -pev->frags;		// Sum will be 0
			}
		}
	}

	pev->frags += score;

	MESSAGE_BEGIN(MessageDest::All, gmsgScoreInfo);
	WRITE_BYTE(entindex());
	WRITE_SHORT(pev->frags);
	WRITE_SHORT(m_iDeaths);
	WRITE_SHORT(0);
	WRITE_SHORT(g_pGameRules->GetTeamIndex(m_szTeamName) + 1);
	MESSAGE_END();
}

void CBasePlayer::AddPointsToTeam(int score, bool bAllowNegativeScore)
{
	int index = entindex();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

		if (pPlayer && i != index)
		{
			if (g_pGameRules->PlayerRelationship(this, pPlayer) == GR_TEAMMATE)
			{
				pPlayer->AddPoints(score, bAllowNegativeScore);
			}
		}
	}
}

void CBasePlayer::InitStatusBar()
{
	m_flStatusBarDisappearDelay = 0;
	m_SbarString1[0] = m_SbarString0[0] = 0;
}

void CBasePlayer::UpdateStatusBar()
{
	int newSBarState[SBAR_END]{};
	char sbuf0[SBAR_STRING_SIZE]{};
	char sbuf1[SBAR_STRING_SIZE]{};

	safe_strcpy(sbuf0, m_SbarString0);
	safe_strcpy(sbuf1, m_SbarString1);

	// Find an ID Target
	TraceResult tr;
	UTIL_MakeVectors(pev->v_angle + pev->punchangle);
	const Vector vecSrc = EyePosition();
	const Vector vecEnd = vecSrc + (gpGlobals->v_forward * MAX_ID_RANGE);
	UTIL_TraceLine(vecSrc, vecEnd, IgnoreMonsters::No, this, &tr);

	if (tr.flFraction != 1.0)
	{
		if (auto pEntity = CBaseEntity::InstanceOrWorld(tr.pHit); !IsNullEnt(pEntity))
		{
			if (pEntity->IsPlayer())
			{
				newSBarState[SBAR_ID_TARGETNAME] = pEntity->entindex();
				safe_strcpy(sbuf1, "1 %p1\n2 Health: %i2%%\n3 Armor: %i3%%");

				// allies and medics get to see the targets health
				if (g_pGameRules->PlayerRelationship(this, pEntity) == GR_TEAMMATE)
				{
					newSBarState[SBAR_ID_TARGETHEALTH] = 100 * (pEntity->pev->health / pEntity->pev->max_health);
					newSBarState[SBAR_ID_TARGETARMOR] = pEntity->pev->armorvalue; //No need to get it % based since 100 it's the max.
				}

				m_flStatusBarDisappearDelay = gpGlobals->time + 1.0;
			}
		}
		else if (m_flStatusBarDisappearDelay > gpGlobals->time)
		{
			// hold the values for a short amount of time after viewing the object
			newSBarState[SBAR_ID_TARGETNAME] = m_izSBarState[SBAR_ID_TARGETNAME];
			newSBarState[SBAR_ID_TARGETHEALTH] = m_izSBarState[SBAR_ID_TARGETHEALTH];
			newSBarState[SBAR_ID_TARGETARMOR] = m_izSBarState[SBAR_ID_TARGETARMOR];
		}
	}

	bool bForceResend = false;

	if (strcmp(sbuf0, m_SbarString0))
	{
		MESSAGE_BEGIN(MessageDest::One, gmsgStatusText, this);
		WRITE_BYTE(0);
		WRITE_STRING(sbuf0);
		MESSAGE_END();

		safe_strcpy(m_SbarString0, sbuf0);

		// make sure everything's resent
		bForceResend = true;
	}

	if (strcmp(sbuf1, m_SbarString1))
	{
		MESSAGE_BEGIN(MessageDest::One, gmsgStatusText, this);
		WRITE_BYTE(1);
		WRITE_STRING(sbuf1);
		MESSAGE_END();

		safe_strcpy(m_SbarString1, sbuf1);

		// make sure everything's resent
		bForceResend = true;
	}

	// Check values and send if they don't match
	for (int i = 1; i < SBAR_END; i++)
	{
		if (newSBarState[i] != m_izSBarState[i] || bForceResend)
		{
			MESSAGE_BEGIN(MessageDest::One, gmsgStatusValue, this);
			WRITE_BYTE(i);
			WRITE_SHORT(newSBarState[i]);
			MESSAGE_END();

			m_izSBarState[i] = newSBarState[i];
		}
	}
}

void CBasePlayer::PreThink()
{
	const int buttonsChanged = (m_afButtonLast ^ pev->button);	// These buttons have changed this frame

	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	m_afButtonPressed = buttonsChanged & pev->button;		// The changed ones still down are "pressed"
	m_afButtonReleased = buttonsChanged & (~pev->button);	// The ones not down are "released"

	g_pGameRules->PlayerThink(this);

	if (g_fGameOver)
		return;         // intermission or finale

	UTIL_MakeVectors(pev->v_angle);             // is this still used?

	WeaponPreFrame();
	WaterMove();

	if (g_pGameRules && g_pGameRules->AllowFlashlight())
		m_iHideHUD &= ~HIDEHUD_FLASHLIGHT;
	else
		m_iHideHUD |= HIDEHUD_FLASHLIGHT;

	if (m_bResetViewEntity)
	{
		m_bResetViewEntity = false;

		CBaseEntity* viewEntity = m_hViewEntity;

		if (viewEntity)
		{
			SET_VIEW(edict(), viewEntity->edict());
		}
	}

	// JOHN: checks if new client data (for HUD and view control) needs to be sent to the client
	UpdateClientData();

	CheckTimeBasedDamage();

	CheckSuitUpdate();

	// Observer Button Handling
	if (IsObserver())
	{
		Observer_HandleButtons();
		Observer_CheckTarget();
		Observer_CheckProperties();
		pev->impulse = 0;
		return;
	}

	if (pev->deadflag >= DeadFlag::Dying)
	{
		PlayerDeathThink();
		return;
	}

	// So the correct flags get sent to client asap.
	//
	if (m_afPhysicsFlags & PFLAG_ONTRAIN)
		pev->flags |= FL_ONTRAIN;
	else
		pev->flags &= ~FL_ONTRAIN;

	// Train speed control
	if (m_afPhysicsFlags & PFLAG_ONTRAIN)
	{
		CBaseEntity* pTrain = CBaseEntity::Instance(pev->groundentity);

		if (!pTrain)
		{
			TraceResult trainTrace;
			// Maybe this is on the other side of a level transition
			UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -38), IgnoreMonsters::Yes, this, &trainTrace);

			// HACKHACK - Just look for the func_tracktrain classname
			if (trainTrace.flFraction != 1.0 && trainTrace.pHit)
				pTrain = CBaseEntity::Instance(trainTrace.pHit);


			if (!pTrain || !(pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) || !pTrain->OnControls(this))
			{
				//ALERT( at_error, "In train mode with no train!\n" );
				m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
				m_iTrain = TRAIN_NEW | TRAIN_OFF;
				return;
			}
		}
		else if (!IsBitSet(pev->flags, FL_ONGROUND) || IsBitSet(pTrain->pev->spawnflags, SF_TRACKTRAIN_NOCONTROL) || (pev->button & (IN_MOVELEFT | IN_MOVERIGHT)))
		{
			// Turn off the train if you jump, strafe, or the train controls go dead
			m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
			m_iTrain = TRAIN_NEW | TRAIN_OFF;
			return;
		}

		SetAbsVelocity(vec3_origin);
		float vel = 0;
		if (m_afButtonPressed & IN_FORWARD)
		{
			vel = 1;
			pTrain->Use({this, this, UseType::Set, (float)vel});
		}
		else if (m_afButtonPressed & IN_BACK)
		{
			vel = -1;
			pTrain->Use({this, this, UseType::Set, (float)vel});
		}

		if (vel)
		{
			m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
			m_iTrain |= TRAIN_ACTIVE | TRAIN_NEW;
		}

	}
	else if (m_iTrain & TRAIN_ACTIVE)
		m_iTrain = TRAIN_NEW; // turn off train

	if (pev->button & IN_JUMP)
	{
		// If on a ladder, jump off the ladder
		// else Jump
		Jump();
	}


	// If trying to duck, already ducked, or in the process of ducking
	if ((pev->button & IN_DUCK) || IsBitSet(pev->flags, FL_DUCKING) || (m_afPhysicsFlags & PFLAG_DUCKING))
		Duck();

	if (!IsBitSet(pev->flags, FL_ONGROUND))
	{
		m_flFallVelocity = -GetAbsVelocity().z;
	}

	// StudioFrameAdvance( );//!!!HACKHACK!!! Can't be hit by traceline when not animating?

	// Clear out ladder pointer
	m_hEnemy = nullptr;

	if (m_afPhysicsFlags & PFLAG_ONBARNACLE)
	{
		SetAbsVelocity(vec3_origin);
	}
}

void CBasePlayer::CheckTimeBasedDamage()
{
	if (!(m_bitsDamageType & DMG_TIMEBASED))
		return;

	// only check for time based damage approx. every 2 seconds
	if (fabs(gpGlobals->time - m_tbdPrev) < 2.0)
		return;

	m_tbdPrev = gpGlobals->time;

	for (int i = 0; i < CDMG_TIMEBASED; i++)
	{
		// make sure bit is set for damage type
		if (m_bitsDamageType & (DMG_PARALYZE << i))
		{
			byte bDuration = 0;

			switch (i)
			{
			case itbd_Paralyze:
				// UNDONE - flag movement as half-speed
				bDuration = PARALYZE_DURATION;
				break;
			case itbd_NerveGas:
				//				TakeDamage(pev, pev, NERVEGAS_DAMAGE, DMG_GENERIC);	
				bDuration = NERVEGAS_DURATION;
				break;
			case itbd_Poison:
				TakeDamage({this, this, POISON_DAMAGE, DMG_GENERIC});
				bDuration = POISON_DURATION;
				break;
			case itbd_Radiation:
				//				TakeDamage(pev, pev, RADIATION_DAMAGE, DMG_GENERIC);
				bDuration = RADIATION_DURATION;
				break;
			case itbd_DrownRecover:
				// NOTE: this hack is actually used to RESTORE health
				// after the player has been drowning and finally takes a breath
				if (m_idrowndmg > m_idrownrestored)
				{
					int idif = std::min(m_idrowndmg - m_idrownrestored, 10);

					GiveHealth(idif, DMG_GENERIC);
					m_idrownrestored += idif;
				}
				bDuration = 4;	// get up to 5*10 = 50 points back
				break;
			case itbd_Acid:
				//				TakeDamage(pev, pev, ACID_DAMAGE, DMG_GENERIC);
				bDuration = ACID_DURATION;
				break;
			case itbd_SlowBurn:
				//				TakeDamage(pev, pev, SLOWBURN_DAMAGE, DMG_GENERIC);
				bDuration = SLOWBURN_DURATION;
				break;
			case itbd_SlowFreeze:
				//				TakeDamage(pev, pev, SLOWFREEZE_DAMAGE, DMG_GENERIC);
				bDuration = SLOWFREEZE_DURATION;
				break;
			default:
				bDuration = 0;
			}

			if (m_rgbTimeBasedDamage[i])
			{
				// use up an antitoxin on poison or nervegas after a few seconds of damage					
				if (((i == itbd_NerveGas) && (m_rgbTimeBasedDamage[i] < NERVEGAS_DURATION)) ||
					((i == itbd_Poison) && (m_rgbTimeBasedDamage[i] < POISON_DURATION)))
				{
					if (m_rgItems[ITEM_ANTIDOTE])
					{
						m_rgbTimeBasedDamage[i] = 0;
						m_rgItems[ITEM_ANTIDOTE]--;
						SetSuitUpdate("!HEV_HEAL4", SuitSoundType::Sentence, SUIT_REPEAT_OK);
					}
				}


				// decrement damage duration, detect when done.
				if (!m_rgbTimeBasedDamage[i] || --m_rgbTimeBasedDamage[i] == 0)
				{
					m_rgbTimeBasedDamage[i] = 0;
					// if we're done, clear damage bits
					m_bitsDamageType &= ~(DMG_PARALYZE << i);
				}
			}
			else
				// first time taking this damage type - init damage duration
				m_rgbTimeBasedDamage[i] = bDuration;
		}
	}
}

void CBasePlayer::UpdateGeigerCounter()
{
	// delay per update ie: don't flood net with these msgs
	if (gpGlobals->time < m_flgeigerDelay)
		return;

	m_flgeigerDelay = gpGlobals->time + PLAYER_GEIGER_DELAY;

	// send range to radition source to client

	const byte range = (byte)(m_flgeigerRange / 4);

	if (range != m_igeigerRangePrev)
	{
		m_igeigerRangePrev = range;

		MESSAGE_BEGIN(MessageDest::One, gmsgGeigerRange, this);
		WRITE_BYTE(range);
		MESSAGE_END();
	}

	// reset counter and semaphore
	if (!RANDOM_LONG(0, 3))
		m_flgeigerRange = 1000;
}

void CBasePlayer::CheckSuitUpdate()
{
	// Ignore suit updates if no suit
	if (!(pev->weapons & (1 << WEAPON_SUIT)))
		return;

	// if in range of radiation source, ping geiger counter
	UpdateGeigerCounter();

	if (g_pGameRules->IsMultiplayer())
	{
		// don't bother updating HEV voice in multiplayer.
		return;
	}

	if (gpGlobals->time >= m_flSuitUpdate && m_flSuitUpdate > 0)
	{
		int isentence = 0;
		int isearch = m_iSuitPlayNext;

		// play a sentence off of the end of the queue
		for (int i = 0; i < CSUITPLAYLIST; i++)
		{
			if (isentence = m_rgSuitPlayList[isearch])
				break;

			if (++isearch == CSUITPLAYLIST)
				isearch = 0;
		}

		if (isentence)
		{
			m_rgSuitPlayList[isearch] = 0;
			if (isentence > 0)
			{
				// play sentence number

				char sentence[CBSENTENCENAME_MAX + 1];
				safe_strcpy(sentence, "!");
				safe_strcat(sentence, gszallsentencenames[isentence]);
				EMIT_SOUND_SUIT(this, sentence);
			}
			else
			{
				// play sentence group
				EMIT_GROUPID_SUIT(this, -isentence);
			}
			m_flSuitUpdate = gpGlobals->time + PLAYER_SUIT_UPDATE_TIME;
		}
		else
			// queue is empty, don't check 
			m_flSuitUpdate = 0;
	}
}

void CBasePlayer::SetSuitUpdate(const char* name, SuitSoundType type, int iNoRepeatTime)
{
	// Ignore suit updates if no suit
	if (!(pev->weapons & (1 << WEAPON_SUIT)))
		return;

	if (g_pGameRules->IsMultiplayer())
	{
		// due to static channel design, etc. We don't play HEV sounds in multiplayer right now.
		return;
	}

	// if name == nullptr, then clear out the queue
	if (!name)
	{
		for (int i = 0; i < CSUITPLAYLIST; i++)
			m_rgSuitPlayList[i] = 0;
		return;
	}

	// get sentence or group number
	int isentence;
	if (type == SuitSoundType::Sentence)
	{
		isentence = SENTENCEG_Lookup(name, nullptr, 0);
		if (isentence < 0)
			return;
	}
	else
		// mark group number as negative
		isentence = -SENTENCEG_GetIndex(name);

	// check norepeat list - this list lets us cancel
	// the playback of words or sentences that have already
	// been played within a certain time.
	int iempty = -1;
	for (int i = 0; i < CSUITNOREPEAT; i++)
	{
		if (isentence == m_rgiSuitNoRepeat[i])
		{
			// this sentence or group is already in 
			// the norepeat list

			if (m_rgflSuitNoRepeatTime[i] < gpGlobals->time)
			{
				// norepeat time has expired, clear it out
				m_rgiSuitNoRepeat[i] = 0;
				m_rgflSuitNoRepeatTime[i] = 0.0;
				iempty = i;
				break;
			}
			else
			{
				// don't play, still marked as norepeat
				return;
			}
		}
		// keep track of empty slot
		if (!m_rgiSuitNoRepeat[i])
			iempty = i;
	}

	// sentence is not in norepeat list, save if norepeat time was given

	if (iNoRepeatTime)
	{
		if (iempty < 0)
			iempty = RANDOM_LONG(0, CSUITNOREPEAT - 1); // pick random slot to take over
		m_rgiSuitNoRepeat[iempty] = isentence;
		m_rgflSuitNoRepeatTime[iempty] = iNoRepeatTime + gpGlobals->time;
	}

	// find empty spot in queue, or overwrite last spot
	m_rgSuitPlayList[m_iSuitPlayNext++] = isentence;
	if (m_iSuitPlayNext == CSUITPLAYLIST)
		m_iSuitPlayNext = 0;

	if (m_flSuitUpdate <= gpGlobals->time)
	{
		if (m_flSuitUpdate == 0)
			// play queue is empty, don't delay too long before playback
			m_flSuitUpdate = gpGlobals->time + PLAYER_SUIT_FIRST_UPDATE_TIME;
		else
			m_flSuitUpdate = gpGlobals->time + PLAYER_SUIT_UPDATE_TIME;
	}
}

/**
*	@brief Check for turning off powerups
*/
static void CheckPowerups(CBaseEntity* pEntity)
{
	if (pEntity->pev->health <= 0)
		return;

	pEntity->pev->modelindex = g_ulModelIndexPlayer;    // don't use eyes
}

void CBasePlayer::UpdatePlayerSound()
{
	CSound* pSound = CSoundEnt::SoundPointerForIndex(CSoundEnt::ClientSoundIndex(edict()));

	if (!pSound)
	{
		ALERT(at_console, "Client lost reserved sound!\n");
		return;
	}

	pSound->m_iType = bits_SOUND_NONE;

	// now calculate the best target volume for the sound. If the player's weapon
	// is louder than his body/movement, use the weapon volume, else, use the body volume.
	int iBodyVolume;
	if (IsBitSet(pev->flags, FL_ONGROUND))
	{
		iBodyVolume = GetAbsVelocity().Length();

		// clamp the noise that can be made by the body, in case a push trigger,
		// weapon recoil, or anything shoves the player abnormally fast. 
		if (iBodyVolume > 512)
		{
			iBodyVolume = 512;
		}
	}
	else
	{
		iBodyVolume = 0;
	}

	if (pev->button & IN_JUMP)
	{
		iBodyVolume += 100;
	}

	// convert player move speed and actions into sound audible by monsters.
	if (m_iWeaponVolume > iBodyVolume)
	{
		m_iTargetVolume = m_iWeaponVolume;

		// OR in the bits for COMBAT sound if the weapon is being louder than the player. 
		pSound->m_iType |= bits_SOUND_COMBAT;
	}
	else
	{
		m_iTargetVolume = iBodyVolume;
	}

	// decay weapon volume over time so bits_SOUND_COMBAT stays set for a while
	m_iWeaponVolume -= 250 * gpGlobals->frametime;
	//TODO: this assignment is pointless
	int iVolume;
	if (m_iWeaponVolume < 0)
	{
		iVolume = 0;
	}


	// if target volume is greater than the player sound's current volume, we paste the new volume in 
	// immediately. If target is less than the current volume, current volume is not set immediately to the
	// lower volume, rather works itself towards target volume over time. This gives monsters a much better chance
	// to hear a sound, especially if they don't listen every frame.
	iVolume = pSound->m_iVolume;

	if (m_iTargetVolume > iVolume)
	{
		iVolume = m_iTargetVolume;
	}
	else if (iVolume > m_iTargetVolume)
	{
		iVolume -= 250 * gpGlobals->frametime;

		if (iVolume < m_iTargetVolume)
		{
			iVolume = 0;
		}
	}

	if (m_fNoPlayerSound)
	{
		// debugging flag, lets players move around and shoot without monsters hearing.
		iVolume = 0;
	}

	if (gpGlobals->time > m_flStopExtraSoundTime)
	{
		// since the extra sound that a weapon emits only lasts for one client frame, we keep that sound around for a server frame or two 
		// after actual emission to make sure it gets heard.
		m_iExtraSoundTypes = 0;
	}

	if (pSound)
	{
		pSound->m_vecOrigin = GetAbsOrigin();
		pSound->m_iType |= (bits_SOUND_PLAYER | m_iExtraSoundTypes);
		pSound->m_iVolume = iVolume;
	}

	// keep track of virtual muzzle flash
	m_iWeaponFlash -= 256 * gpGlobals->frametime;
	if (m_iWeaponFlash < 0)
		m_iWeaponFlash = 0;

	//UTIL_MakeVectors(GetAbsAngles());
	//gpGlobals->v_forward.z = 0;

	// Below are a couple of useful little bits that make it easier to determine just how much noise the 
	// player is making. 
	// UTIL_ParticleEffect ( GetAbsOrigin() + gpGlobals->v_forward * iVolume, vec3_origin, 255, 25 );
	//ALERT ( at_console, "%d/%d\n", iVolume, m_iTargetVolume );
}

void CBasePlayer::PostThink()
{
	// intermission or finale, or dead
	if (!g_fGameOver && IsAlive())
	{
		// Handle Tank controlling
		if (m_pTank != nullptr)
		{ // if they've moved too far from the gun,  or selected a weapon, unuse the gun
			if (m_pTank->OnControls(this) && IsStringNull(pev->weaponmodel))
			{
				m_pTank->Use({this, this, UseType::Set, 2});	// try fire the gun
			}
			else
			{  // they've moved off the platform
				m_pTank->Use({this, this, UseType::Off});
				m_pTank = nullptr;
			}
		}

		// do weapon stuff
		WeaponPostFrame();

		// check to see if player landed hard enough to make a sound
		// falling farther than half of the maximum safe distance, but not as far a max safe distance will
		// play a bootscrape sound, and no damage will be inflicted. Fallling a distance shorter than half
		// of maximum safe distance will make no sound. Falling farther than max safe distance will play a 
		// fallpain sound, and damage will be inflicted based on how far the player fell

		if ((IsBitSet(pev->flags, FL_ONGROUND)) && (pev->health > 0) && m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD)
		{
			// ALERT ( at_console, "%f\n", m_flFallVelocity );

			if (pev->watertype == Contents::Water)
			{
				// Did he hit the world or a non-moving entity?
				// BUG - this happens all the time in water, especially when 
				// BUG - water has current force
				// if ( !pev->groundentity || VARS(pev->groundentity)->velocity.z == 0 )
					// EmitSound(SoundChannel::Body, "player/pl_wade1.wav");
			}
			else if (m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED)
			{// after this point, we start doing damage

				const float flFallDamage = g_pGameRules->PlayerFallDamage(this);

				if (flFallDamage > pev->health)
				{//splat
					// note: play on item channel because we play footstep landing on body channel
					EmitSound(SoundChannel::Item, "common/bodysplat.wav");
				}

				if (flFallDamage > 0)
				{
					TakeDamage({UTIL_GetWorld(), UTIL_GetWorld(), flFallDamage, DMG_FALL});
					pev->punchangle.x = 0;
				}
			}

			if (IsAlive())
			{
				SetAnimation(PlayerAnim::Walk);
			}
		}

		if (IsBitSet(pev->flags, FL_ONGROUND))
		{
			if (m_flFallVelocity > 64 && !g_pGameRules->IsMultiplayer())
			{
				CSoundEnt::InsertSound(bits_SOUND_PLAYER, GetAbsOrigin(), m_flFallVelocity, 0.2);
				// ALERT( at_console, "fall %f\n", m_flFallVelocity );
			}
			m_flFallVelocity = 0;
		}

		// select the proper animation for the player character	
		if (IsAlive())
		{
			if (!GetAbsVelocity().x && !GetAbsVelocity().y)
				SetAnimation(PlayerAnim::Idle);
			else if ((GetAbsVelocity().x || GetAbsVelocity().y) && (IsBitSet(pev->flags, FL_ONGROUND)))
				SetAnimation(PlayerAnim::Walk);
			else if (pev->waterlevel > WaterLevel::Feet)
				SetAnimation(PlayerAnim::Walk);
		}

		StudioFrameAdvance();
		CheckPowerups(this);

		UpdatePlayerSound();
	}

#if defined( CLIENT_WEAPONS )
	// Decay timers on weapons
// go through all of the weapons and make a list of the ones to pack
	for (int i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		if (auto weapon = m_hPlayerWeapons[i].Get(); weapon)
		{
			while (weapon)
			{
				if (weapon->UseDecrement())
				{
					weapon->m_flNextPrimaryAttack = std::max(weapon->m_flNextPrimaryAttack - gpGlobals->frametime, -1.0f);
					weapon->m_flNextSecondaryAttack = std::max(weapon->m_flNextSecondaryAttack - gpGlobals->frametime, -0.001f);

					if (weapon->m_flTimeWeaponIdle != 1000)
					{
						weapon->m_flTimeWeaponIdle = std::max(weapon->m_flTimeWeaponIdle - gpGlobals->frametime, -0.001f);
					}

					if (weapon->pev->fuser1 != 1000)
					{
						weapon->pev->fuser1 = std::max(weapon->pev->fuser1 - gpGlobals->frametime, -0.001f);
					}

					// Only decrement if not flagged as NO_DECREMENT
//					if ( weapon->m_flPumpTime != 1000 )
				//	{
				//		weapon->m_flPumpTime	= std::max( weapon->m_flPumpTime - gpGlobals->frametime, -0.001f );
				//	}

					weapon->DecrementTimers();
				}

				weapon = weapon->m_hNext;
			}
		}
	}

	m_flNextAttack -= gpGlobals->frametime;
	if (m_flNextAttack < -0.001)
		m_flNextAttack = -0.001;
#endif

	// Track button info so we can detect 'pressed' and 'released' buttons next frame
	m_afButtonLast = pev->button;
}

void CBasePlayer::Spawn()
{
	SetClassname("player");
	pev->health = 100;
	pev->armorvalue = 0;
	SetDamageMode(DamageMode::Aim);
	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Walk);
	pev->max_health = pev->health;
	pev->flags &= FL_PROXY;	// keep proxy flag sey by engine
	pev->flags |= FL_CLIENT;
	pev->air_finished = gpGlobals->time + PLAYER_AIRTIME;
	pev->dmg = 2;				// initial water damage
	pev->effects = 0;
	pev->deadflag = DeadFlag::No;
	pev->dmg_take = 0;
	pev->dmg_save = 0;
	pev->friction = 1.0;
	pev->gravity = 1.0;
	m_bitsHUDDamage = -1;
	m_bitsDamageType = 0;
	m_afPhysicsFlags = 0;
	m_fLongJump = false;// no longjump module. 

	g_engfuncs.pfnSetPhysicsKeyValue(edict(), "slj", "0");
	g_engfuncs.pfnSetPhysicsKeyValue(edict(), "hl", "1");

	m_iFOV = 0;// init field of view.
	m_iClientFOV = -1; // make sure fov reset is sent

	m_flNextDecalTime = 0;// let this player decal as soon as he spawns.

	m_flgeigerDelay = gpGlobals->time + 2.0;	// wait a few seconds until user-defined message registrations
												// are recieved by all clients

	m_flTimeStepSound = 0;
	m_bStepLeft = false;
	m_flFieldOfView = 0.5;// some monsters use this to determine whether or not the player is looking at them.

	m_bloodColor = BLOOD_COLOR_RED;
	m_flNextAttack = UTIL_WeaponTimeBase();

	m_iFlashBattery = 99;
	m_flFlashLightTime = 1; // force first message

// dont let uninitialized value here hurt the player
	m_flFallVelocity = 0;

	g_pGameRules->SetDefaultPlayerTeam(this);
	g_pGameRules->GetPlayerSpawnSpot(this);

	SetModel("models/player.mdl");
	g_ulModelIndexPlayer = pev->modelindex;
	pev->sequence = LookupActivity(ACT_IDLE);

	if (IsBitSet(pev->flags, FL_DUCKING))
		SetSize(VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	else
		SetSize(VEC_HULL_MIN, VEC_HULL_MAX);

	pev->view_ofs = VEC_VIEW;
	Precache();
	m_HackedGunPos = Vector(0, 32, 0);

	if (m_iPlayerSound == SOUNDLIST_EMPTY)
	{
		ALERT(at_console, "Couldn't alloc player sound slot!\n");
	}

	m_fNoPlayerSound = false;// normal sound behavior.

	m_hLastWeapon = nullptr;
	m_fInitHUD = true;
	m_iClientHideHUD = -1;  // force this to be recalculated
	m_fWeapon = false;
	m_hClientActiveWeapon = nullptr;
	m_iClientBattery = -1;

	// reset all ammo values to 0
	for (int i = 0; i < MAX_AMMO_TYPES; i++)
	{
		m_rgAmmo[i] = 0;
		m_rgAmmoLast[i] = 0;  // client ammo values also have to be reset  (the death hud clear messages does on the client side)
	}

	m_lastx = m_lasty = 0;

	m_flNextChatTime = gpGlobals->time;

	g_pGameRules->PlayerSpawn(this);
}

void CBasePlayer::Precache()
{
	// in the event that the player JUST spawned, and the level node graph
	// was loaded, fix all of the node graph pointers before the game starts.

	// !!!BUGBUG - now that we have multiplayer, this needs to be moved!
	if (WorldGraph.m_fGraphPresent && !WorldGraph.m_fGraphPointersSet)
	{
		if (!WorldGraph.SetGraphPointers())
		{
			ALERT(at_console, "**Graph pointers were not set!\n");
		}
		else
		{
			ALERT(at_console, "**Graph Pointers Set!\n");
		}
	}

	// SOUNDS / MODELS ARE PRECACHED in ClientPrecache() (game specific)
	// because they need to precache before any clients have connected

	// init geiger counter vars during spawn and each time
	// we cross a level transition

	m_flgeigerRange = 1000;
	m_igeigerRangePrev = 1000;

	m_bitsDamageType = 0;
	m_bitsHUDDamage = -1;

	m_iClientBattery = -1;

	m_iTrain |= TRAIN_NEW;

	// Make sure any necessary user messages have been registered
	LinkUserMessages();

	if (gInitHUD)
		m_fInitHUD = true;
}

bool CBasePlayer::Save(CSave& save)
{
	if (!CBaseMonster::Save(save))
		return false;

	return save.WriteFields("PLAYER", this, m_playerSaveData, ArraySize(m_playerSaveData));
}

bool CBasePlayer::Restore(CRestore& restore)
{
	if (!CBaseMonster::Restore(restore))
		return false;

	const bool status = restore.ReadFields("PLAYER", this, m_playerSaveData, ArraySize(m_playerSaveData));

	SAVERESTOREDATA* pSaveData = (SAVERESTOREDATA*)gpGlobals->pSaveData;
	// landmark isn't present.
	if (!pSaveData->fUseLandmark)
	{
		ALERT(at_console, "No Landmark:%s\n", pSaveData->szLandmarkName);

		// default to normal spawn
		CBaseEntity* pSpawnSpot = EntSelectSpawnPoint(this);
		SetAbsOrigin(pSpawnSpot->GetAbsOrigin() + vec3_up);
		SetAbsAngles(pSpawnSpot->GetAbsAngles());
	}
	pev->v_angle.z = 0;	// Clear out roll
	SetAbsAngles(pev->v_angle);

	pev->fixangle = FixAngleMode::Absolute;           // turn this way immediately

// Copied from spawn() for now
	m_bloodColor = BLOOD_COLOR_RED;

	g_ulModelIndexPlayer = pev->modelindex;

	if (IsBitSet(pev->flags, FL_DUCKING))
	{
		// Use the crouch HACK
		//FixPlayerCrouchStuck(this);
		// Don't need to do this with new player prediction code.
		SetSize(VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	}
	else
	{
		SetSize(VEC_HULL_MIN, VEC_HULL_MAX);
	}

	g_engfuncs.pfnSetPhysicsKeyValue(edict(), "hl", "1");

	if (m_fLongJump)
	{
		g_engfuncs.pfnSetPhysicsKeyValue(edict(), "slj", "1");
	}
	else
	{
		g_engfuncs.pfnSetPhysicsKeyValue(edict(), "slj", "0");
	}

#if defined( CLIENT_WEAPONS )
	// HACK:	This variable is saved/restored in CBaseMonster as a time variable, but we're using it
	//			as just a counter.  Ideally, this needs its own variable that's saved as a plain float.
	//			Barring that, we clear it out here instead of using the incorrect restored time value.
	m_flNextAttack = UTIL_WeaponTimeBase();
#endif

	m_bResetViewEntity = true;

	m_bRestored = true;

	return status;
}

bool CBasePlayer::HasWeapons()
{
	for (int i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		if (m_hPlayerWeapons[i])
		{
			return true;
		}
	}

	return false;
}

const char* CBasePlayer::TeamID()
{
	if (pev == nullptr)		// Not fully connected yet
		return "";

	// return their team name
	return m_szTeamName;
}

/**
*	@brief !!!UNDONE:ultra temporary SprayCan entity to apply decal frame at a time. For PreAlpha CD
*/
class CSprayCan : public CBaseEntity
{
public:
	void	Spawn(CBaseEntity* pOwner);
	void	Think() override;

	int	ObjectCaps() override { return FCAP_DONT_SAVE; }
};

void CSprayCan::Spawn(CBaseEntity* pOwner)
{
	SetAbsOrigin(pOwner->GetAbsOrigin() + Vector(0, 0, 32));
	SetAbsAngles(pOwner->pev->v_angle);
	SetOwner(pOwner);
	pev->frame = 0;

	pev->nextthink = gpGlobals->time + 0.1;
	EmitSound(SoundChannel::Voice, "player/sprayer.wav");
}

void CSprayCan::Think()
{
	auto pPlayer = (CBasePlayer*)GetOwner();

	const int nFrames = pPlayer ? pPlayer->GetCustomDecalFrames() : -1;

	const int playernum = pPlayer ? pPlayer->entindex() : 0;

	// ALERT(at_console, "Spray by player %i, %i of %i\n", playernum, (int)(pev->frame + 1), nFrames);

	TraceResult	tr;
	UTIL_MakeVectors(GetAbsAngles());
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + gpGlobals->v_forward * 128, IgnoreMonsters::Yes, pPlayer, &tr);

	// No customization present.
	if (nFrames == -1)
	{
		UTIL_DecalTrace(&tr, DECAL_LAMBDA6);
		UTIL_Remove(this);
	}
	else
	{
		UTIL_PlayerDecalTrace(&tr, playernum, pev->frame, true);
		// Just painted last custom frame.
		if (pev->frame++ >= (nFrames - 1))
			UTIL_Remove(this);
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

class	CBloodSplat : public CBaseEntity
{
public:
	void	Spawn(CBaseEntity* pOwner);
	void	Spray();
};

void CBloodSplat::Spawn(CBaseEntity* pOwner)
{
	SetAbsOrigin(pOwner->GetAbsOrigin() + Vector(0, 0, 32));
	SetAbsAngles(pOwner->pev->v_angle);
	SetOwner(pOwner);

	SetThink(&CBloodSplat::Spray);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CBloodSplat::Spray()
{
	if (g_Language != LANGUAGE_GERMAN)
	{
		TraceResult	tr;
		UTIL_MakeVectors(GetAbsAngles());
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + gpGlobals->v_forward * 128, IgnoreMonsters::Yes, GetOwner(), &tr);

		UTIL_BloodDecalTrace(&tr, BLOOD_COLOR_RED);
	}
	SetThink(&CBloodSplat::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CBasePlayer::GiveNamedItem(const char* pszName)
{
	string_t istr = MAKE_STRING(pszName);

	auto pEntity = UTIL_CreateNamedEntity(istr);
	if (IsNullEnt(pEntity))
	{
		ALERT(at_console, "NULL Ent in GiveNamedItem!\n");
		return;
	}

	//Only allow player to give items
	//TODO: don't rely on dynamic_cast for this
	if (auto item = dynamic_cast<CBaseItem*>(pEntity); item)
	{
		item->SetAbsOrigin(GetAbsOrigin());
		item->m_RespawnMode = ItemRespawnMode::Never;

		DispatchSpawn(item->edict());
		item->Touch(this);
	}
	else
	{
		UTIL_Remove(item);
	}
}

bool CBasePlayer::FlashlightIsOn()
{
	return IsBitSet(pev->effects, EF_DIMLIGHT);
}

void CBasePlayer::FlashlightTurnOn()
{
	if (!g_pGameRules->AllowFlashlight())
	{
		return;
	}

	if ((pev->weapons & (1 << WEAPON_SUIT)))
	{
		EmitSound(SoundChannel::Weapon, SOUND_FLASHLIGHT_ON.data());
		SetBits(pev->effects, EF_DIMLIGHT);
		MESSAGE_BEGIN(MessageDest::One, gmsgFlashlight, this);
		WRITE_BYTE(1);
		WRITE_BYTE(m_iFlashBattery);
		MESSAGE_END();

		m_flFlashLightTime = FLASH_DRAIN_TIME + gpGlobals->time;
	}
}

void CBasePlayer::FlashlightTurnOff()
{
	EmitSound(SoundChannel::Weapon, SOUND_FLASHLIGHT_OFF.data());
	ClearBits(pev->effects, EF_DIMLIGHT);
	MESSAGE_BEGIN(MessageDest::One, gmsgFlashlight, this);
	WRITE_BYTE(0);
	WRITE_BYTE(m_iFlashBattery);
	MESSAGE_END();

	m_flFlashLightTime = FLASH_CHARGE_TIME + gpGlobals->time;
}

void CBasePlayer::ForceClientDllUpdate()
{
	m_iClientHealth = -1;
	m_iClientBattery = -1;
	m_iTrain |= TRAIN_NEW;  // Force new train message.
	m_fWeapon = false;          // Force weapon send
	m_fKnownWeapon = false;    // Force weaponinit messages.
	m_fInitHUD = true;		// Force HUD gmsgResetHUD message

	// Now force all the necessary messages
	//  to be sent.
	UpdateClientData();
}

void CBasePlayer::ImpulseCommands()
{
	// Handle use events
	PlayerUse();

	const int iImpulse = pev->impulse;
	switch (iImpulse)
	{
	case 99:
	{
		bool iOn;

		if (!gmsgLogo)
		{
			iOn = true;
			gmsgLogo = REG_USER_MSG("Logo", 1);
		}
		else
		{
			iOn = false;
		}

		ASSERT(gmsgLogo > 0);
		// send "health" update message
		MESSAGE_BEGIN(MessageDest::One, gmsgLogo, this);
		WRITE_BYTE(iOn);
		MESSAGE_END();

		if (!iOn)
			gmsgLogo = 0;
		break;
	}
	case 100:
		// temporary flashlight for level designers
		if (FlashlightIsOn())
		{
			FlashlightTurnOff();
		}
		else
		{
			FlashlightTurnOn();
		}
		break;

	case	201:// paint decal
	{
		if (gpGlobals->time < m_flNextDecalTime)
		{
			// too early!
			break;
		}

		TraceResult	tr;// UNDONE: kill me! This is temporary for PreAlpha CDs
		UTIL_MakeVectors(pev->v_angle);
		UTIL_TraceLine(GetAbsOrigin() + pev->view_ofs, GetAbsOrigin() + pev->view_ofs + gpGlobals->v_forward * 128, IgnoreMonsters::Yes, this, &tr);

		if (tr.flFraction != 1.0)
		{// line hit something, so paint a decal
			m_flNextDecalTime = gpGlobals->time + decalfrequency.value;
			CSprayCan* pCan = GetClassPtr((CSprayCan*)nullptr);
			pCan->Spawn(this);
		}

		break;
	}

	default:
		// check all of the cheat impulse commands now
		CheatImpulseCommands(iImpulse);
		break;
	}

	pev->impulse = 0;
}

void CBasePlayer::CheatImpulseCommands(int iImpulse)
{
	if (!g_psv_cheats->value)
	{
		return;
	}

	switch (iImpulse)
	{
	case 76:
	{
		if (!giPrecacheGrunt)
		{
			giPrecacheGrunt = true;
			ALERT(at_console, "You must now restart to use Grunt-o-matic.\n");
		}
		else
		{
			UTIL_MakeVectors(Vector(0, pev->v_angle.y, 0));
			Create("monster_human_grunt", GetAbsOrigin() + gpGlobals->v_forward * 128, GetAbsAngles());
		}
		break;
	}

	case 101:
		gEvilImpulse101 = true;
		GiveNamedItem("item_suit");
		GiveNamedItem("item_battery");
		GiveNamedItem("weapon_crowbar");
		GiveNamedItem("weapon_9mmhandgun");
		GiveNamedItem("ammo_9mmclip");
		GiveNamedItem("weapon_shotgun");
		GiveNamedItem("ammo_buckshot");
		GiveNamedItem("weapon_9mmAR");
		GiveNamedItem("ammo_9mmAR");
		GiveNamedItem("ammo_ARgrenades");
		GiveNamedItem("weapon_handgrenade");
		GiveNamedItem("weapon_tripmine");
		GiveNamedItem("weapon_357");
		GiveNamedItem("ammo_357");
		GiveNamedItem("weapon_crossbow");
		GiveNamedItem("ammo_crossbow");
		GiveNamedItem("weapon_egon");
		GiveNamedItem("weapon_gauss");
		GiveNamedItem("ammo_gaussclip");
		GiveNamedItem("weapon_rpg");
		GiveNamedItem("ammo_rpgclip");
		GiveNamedItem("weapon_satchel");
		GiveNamedItem("weapon_snark");
		GiveNamedItem("weapon_hornetgun");

		gEvilImpulse101 = false;
		break;

	case 102:
		// Gibbage!!!
		CGib::SpawnRandomGibs(this, 1, true);
		break;

	case 103:
	{
		// What the hell are you doing?
		if (auto pEntity = UTIL_FindEntityForward(this); pEntity)
		{
			CBaseMonster* pMonster = pEntity->MyMonsterPointer();
			if (pMonster)
				pMonster->ReportAIState();
		}
		break;
	}

	case 104:
		// Dump all of the global state varaibles (and global entity names)
		gGlobalState.DumpGlobals();
		break;

	case	105:// player makes no sound for monsters to hear.
	{
		if (m_fNoPlayerSound)
		{
			ALERT(at_console, "Player is audible\n");
			m_fNoPlayerSound = false;
		}
		else
		{
			ALERT(at_console, "Player is silent\n");
			m_fNoPlayerSound = true;
		}
		break;
	}

	case 106:
	{
		// Give me the classname and targetname of this entity.
		if (auto pEntity = UTIL_FindEntityForward(this); pEntity)
		{
			ALERT(at_console, "Classname: %s", pEntity->GetClassname());

			if (!IsStringNull(pEntity->pev->targetname))
			{
				ALERT(at_console, " - Targetname: %s\n", STRING(pEntity->pev->targetname));
			}
			else
			{
				ALERT(at_console, " - TargetName: No Targetname\n");
			}

			ALERT(at_console, "Model: %s\n", STRING(pEntity->pev->model));
			if (!IsStringNull(pEntity->pev->globalname))
				ALERT(at_console, "Globalname: %s\n", STRING(pEntity->pev->globalname));
		}
		break;
	}

	case 107:
	{
		TraceResult tr;

		Vector start = GetAbsOrigin() + pev->view_ofs;
		Vector end = start + gpGlobals->v_forward * 1024;
		UTIL_TraceLine(start, end, IgnoreMonsters::Yes, this, &tr);

		auto pWorld = InstanceOrWorld(tr.pHit);

		const char* pTextureName = TRACE_TEXTURE(pWorld->edict(), start, end);
		if (pTextureName)
			ALERT(at_console, "Texture: %s\n", pTextureName);
		break;
	}

	case	195:// show shortest paths for entire level to nearest node
	{
		Create("node_viewer_fly", GetAbsOrigin(), GetAbsAngles());
		break;
	}

	case	196:// show shortest paths for entire level to nearest node
	{
		Create("node_viewer_large", GetAbsOrigin(), GetAbsAngles());
		break;
	}

	case	197:// show shortest paths for entire level to nearest node
	{
		Create("node_viewer_human", GetAbsOrigin(), GetAbsAngles());
		break;
	}

	case	199:// show nearest node and all connections
	{
		ALERT(at_console, "%d\n", WorldGraph.FindNearestNode(GetAbsOrigin(), bits_NODE_GROUP_REALM));
		WorldGraph.ShowNodeConnections(WorldGraph.FindNearestNode(GetAbsOrigin(), bits_NODE_GROUP_REALM));
		break;
	}

	case	202:// Random blood splatter
	{
		TraceResult tr;
		UTIL_MakeVectors(pev->v_angle);
		UTIL_TraceLine(GetAbsOrigin() + pev->view_ofs, GetAbsOrigin() + pev->view_ofs + gpGlobals->v_forward * 128, IgnoreMonsters::Yes, this, &tr);

		if (tr.flFraction != 1.0)
		{// line hit something, so paint a decal
			CBloodSplat* pBlood = GetClassPtr((CBloodSplat*)nullptr);
			pBlood->Spawn(this);
		}
		break;
	}

	case	203:// remove creature.
	{
		if (auto pEntity = UTIL_FindEntityForward(this); pEntity)
		{
			if (pEntity->pev->takedamage)
				pEntity->SetThink(&CBaseEntity::SUB_Remove);
		}
		break;
	}
	}
}

ItemApplyResult CBasePlayer::AddPlayerWeapon(CBaseWeapon* weapon)
{
	CBaseWeapon* pInsert = m_hPlayerWeapons[weapon->WeaponSlot()];

	while (pInsert)
	{
		if (pInsert->ClassnameIs(weapon->GetClassname()))
		{
			int flags = 0;
			if (weapon->AddDuplicate(pInsert))
			{
				g_pGameRules->PlayerGotItem(*this, *weapon);

				flags |= ItemFlag::PlayerGotItemHandled;

				// ugly hack to update clip w/o an update clip message
				pInsert->UpdateWeaponInfo();
				if (auto activeWeapon = m_hActiveWeapon.Get(); activeWeapon)
					activeWeapon->UpdateWeaponInfo();
			}
			//Respawns the weapon if needed
			return {ItemApplyAction::Used, flags | ItemFlag::AlwaysRemoveItem};
		}
		pInsert = pInsert->m_hNext;
	}

	if (weapon->AddToPlayer(this))
	{
		g_pGameRules->PlayerGotItem(*this, *weapon);

		weapon->m_hNext = m_hPlayerWeapons[weapon->WeaponSlot()];
		m_hPlayerWeapons[weapon->WeaponSlot()] = weapon;

		// should we switch to this weapon?
		if (g_pGameRules->ShouldSwitchWeapon(this, weapon))
		{
			SwitchWeapon(weapon);
		}

		return {ItemApplyAction::AttachedToPlayer, ItemFlag::PlayerGotItemHandled};
	}

	return {ItemApplyAction::NotUsed};
}

bool CBasePlayer::RemovePlayerWeapon(CBaseWeapon* weapon)
{
	if (m_hActiveWeapon == weapon)
	{
		ResetAutoaim();
		weapon->Holster();
		weapon->pev->nextthink = 0;// crowbar may be trying to swing again, etc.
		weapon->SetThink(nullptr);
		m_hActiveWeapon = nullptr;
		pev->viewmodel = iStringNull;
		pev->weaponmodel = iStringNull;
	}

	if (m_hLastWeapon == weapon)
		m_hLastWeapon = nullptr;

	CBaseWeapon* pPrev = m_hPlayerWeapons[weapon->WeaponSlot()];

	if (pPrev == weapon)
	{
		m_hPlayerWeapons[weapon->WeaponSlot()] = weapon->m_hNext;
		return true;
	}
	else
	{
		while (pPrev && pPrev->m_hNext != weapon)
		{
			pPrev = pPrev->m_hNext;
		}
		if (pPrev)
		{
			pPrev->m_hNext = weapon->m_hNext;
			return true;
		}
	}

	return false;
}

int CBasePlayer::GiveAmmo(int iCount, const char* szName)
{
	if (!szName)
	{
		// no ammo.
		return -1;
	}

	const int i = GetAmmoIndex(szName);

	if (i < 0 || i >= MAX_AMMO_TYPES)
		return -1;

	const int iMax = CBaseWeapon::AmmoInfoArray[i].MaxCarry;

	if (!g_pGameRules->CanHaveAmmo(this, szName, iMax))
	{
		// game rules say I can't have any more of this ammo type.
		return -1;
	}

	const int iAdd = std::min(iCount, iMax - m_rgAmmo[i]);
	if (iAdd < 1)
		return i;

	m_rgAmmo[i] += iAdd;

	if (gmsgAmmoPickup)  // make sure the ammo messages have been linked first
	{
		// Send the message that ammo has been picked up
		MESSAGE_BEGIN(MessageDest::One, gmsgAmmoPickup, this);
		WRITE_BYTE(GetAmmoIndex(szName));		// ammo ID
		WRITE_BYTE(iAdd);		// amount
		MESSAGE_END();
	}

	return i;
}

void CBasePlayer::WeaponPreFrame()
{
#if defined( CLIENT_WEAPONS )
	if (m_flNextAttack > 0)
#else
	if (gpGlobals->time < m_flNextAttack)
#endif
	{
		return;
	}

	if (auto activeWeapon = m_hActiveWeapon.Get(); activeWeapon)
	{
		activeWeapon->WeaponPreFrame();
	}
}

void CBasePlayer::WeaponPostFrame()
{
	// check if the player is using a tank
	if (m_pTank != nullptr)
		return;

#if defined( CLIENT_WEAPONS )
	if (m_flNextAttack > 0)
#else
	if (gpGlobals->time < m_flNextAttack)
#endif
	{
		return;
	}

	ImpulseCommands();

	if (auto activeWeapon = m_hActiveWeapon.Get(); activeWeapon)
	{
		activeWeapon->WeaponPostFrame();
	}
}

int CBasePlayer::AmmoInventory(int iAmmoIndex)
{
	if (iAmmoIndex == -1)
	{
		return -1;
	}

	return m_rgAmmo[iAmmoIndex];
}

int CBasePlayer::GetAmmoIndex(const char* psz)
{
	if (!psz)
		return -1;

	for (int i = 1; i < MAX_AMMO_TYPES; i++)
	{
		if (!CBaseWeapon::AmmoInfoArray[i].pszName)
			continue;

		if (stricmp(psz, CBaseWeapon::AmmoInfoArray[i].pszName) == 0)
			return i;
	}

	return -1;
}

void CBasePlayer::SendAmmoUpdate()
{
	for (int i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if (m_rgAmmo[i] != m_rgAmmoLast[i])
		{
			m_rgAmmoLast[i] = m_rgAmmo[i];

			ASSERT(m_rgAmmo[i] >= 0);
			ASSERT(m_rgAmmo[i] < 255);

			// send "Ammo" update message
			MESSAGE_BEGIN(MessageDest::One, gmsgAmmoX, this);
			WRITE_BYTE(i);
			WRITE_BYTE(std::max(std::min(m_rgAmmo[i], 254), 0));  // clamp the value to one byte
			MESSAGE_END();
		}
	}
}

void CBasePlayer::UpdateClientData()
{
	const bool fullHUDInitRequired = m_fInitHUD != false;

	if (m_fInitHUD)
	{
		m_fInitHUD = false;
		gInitHUD = false;

		MESSAGE_BEGIN(MessageDest::One, gmsgResetHUD, this);
		WRITE_BYTE(0);
		MESSAGE_END();

		if (!m_fGameHUDInitialized)
		{
			MESSAGE_BEGIN(MessageDest::One, gmsgInitHUD, this);
			MESSAGE_END();

			g_pGameRules->InitHUD(this);
			m_fGameHUDInitialized = true;

			m_iObserverLastMode = OBS_ROAMING;

			if (g_pGameRules->IsMultiplayer())
			{
				FireTargets("game_playerjoin", this, this, UseType::Toggle, 0);
			}
		}

		FireTargets("game_playerspawn", this, this, UseType::Toggle, 0);

		InitStatusBar();
	}

	if (m_iHideHUD != m_iClientHideHUD)
	{
		MESSAGE_BEGIN(MessageDest::One, gmsgHideWeapon, this);
		WRITE_BYTE(m_iHideHUD);
		MESSAGE_END();

		m_iClientHideHUD = m_iHideHUD;
	}

	if (m_iFOV != m_iClientFOV)
	{
		MESSAGE_BEGIN(MessageDest::One, gmsgSetFOV, this);
		WRITE_BYTE(m_iFOV);
		MESSAGE_END();

		// cache FOV change at end of function, so weapon updates can see that FOV has changed
	}

	// HACKHACK -- send the message to display the game title
	if (gDisplayTitle)
	{
		MESSAGE_BEGIN(MessageDest::One, gmsgShowGameTitle, this);
		WRITE_BYTE(0);
		MESSAGE_END();
		gDisplayTitle = 0;
	}

	if (pev->health != m_iClientHealth)
	{
		// make sure that no negative health values are sent
		int iHealth = std::clamp(static_cast<int>(pev->health), 0, static_cast<int>(std::numeric_limits<short>::max()));
		if (pev->health > 0.0f && pev->health <= 1.0f)
			iHealth = 1;

		// send "health" update message
		MESSAGE_BEGIN(MessageDest::One, gmsgHealth, this);
		WRITE_SHORT(iHealth);
		MESSAGE_END();

		m_iClientHealth = pev->health;
	}

	if (pev->armorvalue != m_iClientBattery)
	{
		m_iClientBattery = pev->armorvalue;

		ASSERT(gmsgBattery > 0);
		// send "health" update message
		MESSAGE_BEGIN(MessageDest::One, gmsgBattery, this);
		WRITE_SHORT((int)pev->armorvalue);
		MESSAGE_END();
	}

	if (pev->dmg_take || pev->dmg_save || m_bitsHUDDamage != m_bitsDamageType)
	{
		// Comes from inside me if not set
		Vector damageOrigin = GetAbsOrigin();
		// send "damage" message
		// causes screen to flash, and pain compass to show direction of damage
		if (pev->dmg_inflictor)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(pev->dmg_inflictor);
			if (pEntity)
				damageOrigin = pEntity->Center();
		}

		// only send down damage type that have hud art
		const int visibleDamageBits = m_bitsDamageType & DMG_SHOWNHUD;

		MESSAGE_BEGIN(MessageDest::One, gmsgDamage, this);
		WRITE_BYTE(pev->dmg_save);
		WRITE_BYTE(pev->dmg_take);
		WRITE_LONG(visibleDamageBits);
		WRITE_COORD(damageOrigin.x);
		WRITE_COORD(damageOrigin.y);
		WRITE_COORD(damageOrigin.z);
		MESSAGE_END();

		pev->dmg_take = 0;
		pev->dmg_save = 0;
		m_bitsHUDDamage = m_bitsDamageType;

		// Clear off non-time-based damage indicators
		m_bitsDamageType &= DMG_TIMEBASED;
	}

	if (m_bRestored)
	{
		//Always tell client about battery state
		MESSAGE_BEGIN(MessageDest::One, gmsgFlashBattery, this);
		WRITE_BYTE(m_iFlashBattery);
		MESSAGE_END();

		//Tell client the flashlight is on
		if (FlashlightIsOn())
		{
			MESSAGE_BEGIN(MessageDest::One, gmsgFlashlight, this);
			WRITE_BYTE(1);
			WRITE_BYTE(m_iFlashBattery);
			MESSAGE_END();
		}
	}

	// Update Flashlight
	if ((m_flFlashLightTime) && (m_flFlashLightTime <= gpGlobals->time))
	{
		if (FlashlightIsOn())
		{
			if (m_iFlashBattery)
			{
				m_flFlashLightTime = FLASH_DRAIN_TIME + gpGlobals->time;
				m_iFlashBattery--;

				if (!m_iFlashBattery)
					FlashlightTurnOff();
			}
		}
		else
		{
			if (m_iFlashBattery < 100)
			{
				m_flFlashLightTime = FLASH_CHARGE_TIME + gpGlobals->time;
				m_iFlashBattery++;
			}
			else
				m_flFlashLightTime = 0;
		}

		MESSAGE_BEGIN(MessageDest::One, gmsgFlashBattery, this);
		WRITE_BYTE(m_iFlashBattery);
		MESSAGE_END();
	}

	if (m_iTrain & TRAIN_NEW)
	{
		ASSERT(gmsgTrain > 0);
		// send "health" update message
		MESSAGE_BEGIN(MessageDest::One, gmsgTrain, this);
		WRITE_BYTE(m_iTrain & 0xF);
		MESSAGE_END();

		m_iTrain &= ~TRAIN_NEW;
	}

	//
	// New Weapon?
	//
	if (!m_fKnownWeapon)
	{
		m_fKnownWeapon = true;

		// WeaponInit Message
		// byte  = # of weapons
		//
		// for each weapon:
		// byte		name str length (not including null)
		// bytes... name
		// byte		Ammo Type
		// byte		Ammo2 Type
		// byte		bucket
		// byte		bucket pos
		// byte		flags	
		// ????		Icons

			// Send ALL the weapon info now
		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			const WeaponInfo& II = CBaseWeapon::WeaponInfoArray[i];

			if (!II.iId)
				continue;

			const char* pszName;
			if (!II.pszName)
				pszName = "Empty";
			else
				pszName = II.pszName;

			MESSAGE_BEGIN(MessageDest::One, gmsgWeaponList, this);
			WRITE_STRING(pszName);			// string	weapon name
			WRITE_BYTE(GetAmmoIndex(II.pszAmmo1));	// byte		Ammo Type
			WRITE_BYTE(II.iMaxAmmo1);				// byte     Max Ammo 1
			WRITE_BYTE(GetAmmoIndex(II.pszAmmo2));	// byte		Ammo2 Type
			WRITE_BYTE(II.iMaxAmmo2);				// byte     Max Ammo 2
			WRITE_BYTE(II.iSlot);					// byte		bucket
			WRITE_BYTE(II.iPosition);				// byte		bucket pos
			WRITE_BYTE(II.iId);						// byte		id (bit index into pev->weapons)
			WRITE_BYTE(II.iFlags);					// byte		Flags
			MESSAGE_END();
		}
	}

	SendAmmoUpdate();

	// Update all the weapons
	for (int i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		if (auto weapon = m_hPlayerWeapons[i].Get(); weapon)  // each weapon updates it's successors
			weapon->UpdateClientData(this);
	}

	//Active weapon is becoming null, or we're sending all HUD state to client
	//Only if we're not in Observer mode, which uses the target player's weapon
	if (pev->iuser1 == OBS_NONE && !m_hActiveWeapon && ((m_hClientActiveWeapon != m_hActiveWeapon) || fullHUDInitRequired))
	{
		//Tell ammo hud that we have no weapon selected
		MESSAGE_BEGIN(MessageDest::One, gmsgCurWeapon, this);
		WRITE_BYTE(static_cast<int>(WeaponState::NotActive));
		WRITE_BYTE(0);
		WRITE_BYTE(0);
		MESSAGE_END();
	}

	// Cache and client weapon change
	m_hClientActiveWeapon = m_hActiveWeapon;
	m_iClientFOV = m_iFOV;

	// Update Status Bar
	if (m_flNextSBarUpdateTime < gpGlobals->time)
	{
		UpdateStatusBar();
		m_flNextSBarUpdateTime = gpGlobals->time + 0.2;
	}

	//Handled anything that needs resetting
	m_bRestored = false;
}

bool CBasePlayer::BecomeProne()
{
	m_afPhysicsFlags |= PFLAG_ONBARNACLE;
	return true;
}

void CBasePlayer::BarnacleVictimBitten(CBaseEntity* pBarnacle)
{
	TakeDamage({pBarnacle, pBarnacle, pev->health + pev->armorvalue, DMG_SLASH | DMG_ALWAYSGIB});
}

void CBasePlayer::BarnacleVictimReleased()
{
	m_afPhysicsFlags &= ~PFLAG_ONBARNACLE;
}

int CBasePlayer::Illumination()
{
	int iIllum = CBaseEntity::Illumination();

	iIllum += m_iWeaponFlash;
	if (iIllum > 255)
		return 255;
	return iIllum;
}

void CBasePlayer::EnableControl(bool fControl)
{
	if (!fControl)
		pev->flags |= FL_FROZEN;
	else
		pev->flags &= ~FL_FROZEN;
}

Vector CBasePlayer::GetAutoaimVector(float flDelta)
{
	if (g_SkillLevel == SkillLevel::Hard)
	{
		UTIL_MakeVectors(pev->v_angle + pev->punchangle);
		return gpGlobals->v_forward;
	}

	Vector vecSrc = GetGunPosition();
	const float flDist = WORLD_SIZE;

	// always use non-sticky autoaim
	// UNDONE: use sever variable to chose!
	if (true || g_SkillLevel == SkillLevel::Medium)
	{
		m_vecAutoAim = vec3_origin;
		// flDelta *= 0.5;
	}

	const bool m_fOldTargeting = m_fOnTarget;
	Vector angles = AutoaimDeflection(vecSrc, flDist, flDelta);

	// update ontarget if changed
	if (!g_pGameRules->AllowAutoTargetCrosshair())
		m_fOnTarget = false;
	else if (m_fOldTargeting != m_fOnTarget)
	{
		m_hActiveWeapon->UpdateWeaponInfo();
	}

	if (angles.x > 180)
		angles.x -= 360;
	if (angles.x < -180)
		angles.x += 360;
	if (angles.y > 180)
		angles.y -= 360;
	if (angles.y < -180)
		angles.y += 360;

	if (angles.x > 25)
		angles.x = 25;
	if (angles.x < -25)
		angles.x = -25;
	if (angles.y > 12)
		angles.y = 12;
	if (angles.y < -12)
		angles.y = -12;

	// always use non-sticky autoaim
	// UNDONE: use server variable to choose!
	if (false || g_SkillLevel == SkillLevel::Easy)
	{
		m_vecAutoAim = m_vecAutoAim * 0.67 + angles * 0.33;
	}
	else
	{
		m_vecAutoAim = angles * 0.9;
	}

	// m_vecAutoAim = m_vecAutoAim * 0.99;

	// Don't send across network if sv_aim is 0
	if (g_psv_aim->value != 0)
	{
		if (m_vecAutoAim.x != m_lastx ||
			m_vecAutoAim.y != m_lasty)
		{
			SET_CROSSHAIRANGLE(edict(), -m_vecAutoAim.x, m_vecAutoAim.y);

			m_lastx = m_vecAutoAim.x;
			m_lasty = m_vecAutoAim.y;
		}
	}
	else
	{
		ResetAutoaim();
	}

	// ALERT( at_console, "%f %f\n", angles.x, angles.y );

	UTIL_MakeVectors(pev->v_angle + pev->punchangle + m_vecAutoAim);
	return gpGlobals->v_forward;
}

Vector CBasePlayer::AutoaimDeflection(const Vector& vecSrc, float flDist, float flDelta)
{
	if (g_psv_aim->value == 0)
	{
		m_fOnTarget = false;
		return vec3_origin;
	}

	UTIL_MakeVectors(pev->v_angle + pev->punchangle + m_vecAutoAim);

	// try all possible entities
	Vector bestdir = gpGlobals->v_forward;

	m_fOnTarget = false;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + bestdir * flDist, IgnoreMonsters::No, this, &tr);

	if (tr.pHit && tr.pHit->v.takedamage != static_cast<int>(DamageMode::No))
	{
		// don't look through water
		if (!((pev->waterlevel != WaterLevel::Head && tr.pHit->v.waterlevel == WaterLevel::Head)
			|| (pev->waterlevel == WaterLevel::Head && tr.pHit->v.waterlevel == WaterLevel::Dry)))
		{
			if (tr.pHit->v.takedamage == static_cast<int>(DamageMode::Aim))
				m_fOnTarget = true;

			return m_vecAutoAim;
		}
	}

	float bestdot = flDelta; // +- 10 degrees
	CBaseEntity* bestent = nullptr;

	for (int i = 1; i < gpGlobals->maxEntities; i++)
	{
		auto entity = UTIL_EntityByIndex(i);

		if (IsNullEnt(entity))	// Not in use
			continue;

		if (entity->pev->takedamage != static_cast<int>(DamageMode::Aim))
			continue;
		if (entity == this)
			continue;
		//		if (pev->team > 0 && pEdict->v.team == pev->team)
		//			continue;	// don't aim at teammate
		if (!g_pGameRules->ShouldAutoAim(this, entity))
			continue;

		if (!entity->IsAlive())
			continue;

		// don't look through water
		if ((pev->waterlevel != WaterLevel::Head && entity->pev->waterlevel == WaterLevel::Head)
			|| (pev->waterlevel == WaterLevel::Head && entity->pev->waterlevel == WaterLevel::Dry))
			continue;

		const Vector center = entity->BodyTarget(vecSrc);

		const Vector dir = (center - vecSrc).Normalize();

		// make sure it's in front of the player
		if (DotProduct(dir, gpGlobals->v_forward) < 0)
			continue;

		float dot = fabs(DotProduct(dir, gpGlobals->v_right))
			+ fabs(DotProduct(dir, gpGlobals->v_up)) * 0.5;

		// tweek for distance
		dot *= 1.0 + 0.2 * ((center - vecSrc).Length() / flDist);

		if (dot > bestdot)
			continue;	// to far to turn

		UTIL_TraceLine(vecSrc, center, IgnoreMonsters::No, this, &tr);
		if (tr.flFraction != 1.0 && InstanceOrNull(tr.pHit) != entity)
		{
			// ALERT( at_console, "hit %s, can't see %s\n", STRING( tr.pHit->v.classname ), STRING( pEdict->v.classname ) );
			continue;
		}

		// don't shoot at friends
		if (GetRelationship(entity) < Relationship::None)
		{
			if (!entity->IsPlayer() && !g_pGameRules->IsDeathmatch())
				// ALERT( at_console, "friend\n");
				continue;
		}

		// can shoot at this one
		bestdot = dot;
		bestent = entity;
		bestdir = dir;
	}

	if (bestent)
	{
		bestdir = VectorAngles(bestdir);
		bestdir.x = -bestdir.x;
		bestdir = bestdir - pev->v_angle - pev->punchangle;

		if (bestent->pev->takedamage == static_cast<int>(DamageMode::Aim))
			m_fOnTarget = true;

		return bestdir;
	}

	return vec3_origin;
}

void CBasePlayer::ResetAutoaim()
{
	if (m_vecAutoAim.x != 0 || m_vecAutoAim.y != 0)
	{
		m_vecAutoAim = vec3_origin;
		SET_CROSSHAIRANGLE(edict(), 0, 0);
	}
	m_fOnTarget = false;
}

void CBasePlayer::SetCustomDecalFrames(int nFrames)
{
	if (nFrames > 0 &&
		nFrames < 8)
		m_nCustomSprayFrames = nFrames;
	else
		m_nCustomSprayFrames = -1;
}

int CBasePlayer::GetCustomDecalFrames()
{
	return m_nCustomSprayFrames;
}

void CBasePlayer::DropPlayerWeapon(const char* pszWeaponName)
{
	if (!g_pGameRules->IsMultiplayer() || (weaponstay.value > 0))
	{
		// no dropping in single player.
		return;
	}

	if (!strlen(pszWeaponName))
	{
		// if this string has no length, the client didn't type a name!
		// assume player wants to drop the active weapon.
		// make the string null to make future operations in this function easier
		pszWeaponName = nullptr;
	}

	for (int i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		auto pWeapon = m_hPlayerWeapons[i].Get();

		while (pWeapon)
		{
			if (pszWeaponName)
			{
				// try to match by name. 
				if (!strcmp(pszWeaponName, pWeapon->GetClassname()))
				{
					// match! 
					break;
				}
			}
			else
			{
				// trying to drop active weapon
				if (pWeapon == m_hActiveWeapon)
				{
					// active weapon!
					break;
				}
			}

			pWeapon = pWeapon->m_hNext;
		}


		// if we land here with a valid pWeapon pointer, that's because we found the 
		// weapon we want to drop and hit a BREAK;  pWeapon is the weapon.
		if (pWeapon)
		{
			if (!g_pGameRules->GetNextBestWeapon(this, pWeapon))
				return; // can't drop the weapon they asked for, may be our last weapon or something we can't holster

			UTIL_MakeVectors(GetAbsAngles());

			pev->weapons &= ~(1 << pWeapon->m_iId);// take weapon off hud

			CWeaponBox* pWeaponBox = (CWeaponBox*)CBaseEntity::Create("weaponbox", GetAbsOrigin() + gpGlobals->v_forward * 10, GetAbsAngles(), this);
			pWeaponBox->SetAbsAngles({0, pWeaponBox->GetAbsAngles().y, 0});
			pWeaponBox->PackWeapon(pWeapon);
			pWeaponBox->SetAbsVelocity(gpGlobals->v_forward * 300 + gpGlobals->v_forward * 100);

			// drop half of the ammo for this weapon.
			int	iAmmoIndex;

			iAmmoIndex = GetAmmoIndex(pWeapon->Ammo1Name()); // ???

			if (iAmmoIndex != -1)
			{
				// this weapon weapon uses ammo, so pack an appropriate amount.
				if (pWeapon->Flags() & WEAPON_FLAG_EXHAUSTIBLE)
				{
					// pack up all the ammo, this weapon is its own ammo type
					pWeaponBox->PackAmmo(MAKE_STRING(pWeapon->Ammo1Name()), m_rgAmmo[iAmmoIndex]);
					m_rgAmmo[iAmmoIndex] = 0;

				}
				else
				{
					// pack half of the ammo
					pWeaponBox->PackAmmo(MAKE_STRING(pWeapon->Ammo1Name()), m_rgAmmo[iAmmoIndex] / 2);
					m_rgAmmo[iAmmoIndex] /= 2;
				}

			}

			return;// we're done, so stop searching with the FOR loop.
		}
	}
}

bool CBasePlayer::HasPlayerWeapon(CBaseWeapon* pCheckWeapon)
{
	for (CBaseWeapon* weapon = m_hPlayerWeapons[pCheckWeapon->WeaponSlot()]; weapon; weapon = weapon->m_hNext)
	{
		if (weapon->ClassnameIs(pCheckWeapon->GetClassname()))
		{
			return true;
		}
	}

	return false;
}

bool CBasePlayer::HasNamedPlayerWeapon(const char* pszWeaponName)
{
	for (int i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		for (CBaseWeapon* weapon = m_hPlayerWeapons[i]; weapon; weapon = weapon->m_hNext)
		{
			if (!strcmp(pszWeaponName, weapon->GetClassname()))
			{
				return true;
			}
		}
	}

	return false;
}

bool CBasePlayer::SwitchWeapon(CBaseWeapon* pWeapon)
{
	if (pWeapon && !pWeapon->CanDeploy())
	{
		return false;
	}

	ResetAutoaim();

	if (auto activeWeapon = m_hActiveWeapon.Get(); activeWeapon)
	{
		activeWeapon->Holster();
	}

	m_hActiveWeapon = pWeapon;

	if (pWeapon)
	{
		pWeapon->Deploy();
	}

	return true;
}

void CBasePlayer::SetPrefsFromUserinfo(char* infobuffer)
{
	const char* value = g_engfuncs.pfnInfoKeyValue(infobuffer, "cl_autowepswitch");

	if (*value)
	{
		m_iAutoWepSwitch = atoi(value);
	}
	else
	{
		m_iAutoWepSwitch = 1;
	}
}

class CStripWeapons : public CPointEntity
{
public:
	void	Use(const UseInfo& info) override;

private:
};

LINK_ENTITY_TO_CLASS(player_weaponstrip, CStripWeapons);

void CStripWeapons::Use(const UseInfo& info)
{
	CBasePlayer* pPlayer = nullptr;

	if (auto activator = info.GetActivator(); activator && activator->IsPlayer())
	{
		pPlayer = (CBasePlayer*)activator;
	}
	else if (!g_pGameRules->IsMultiplayer())
	{
		pPlayer = UTIL_GetLocalPlayer();
	}

	if (pPlayer)
		pPlayer->RemoveAllItems(false);
}

/**
*	@brief Multiplayer intermission spots.
*/
class CInfoIntermission :public CPointEntity
{
	void Spawn() override;
	void Think() override;
};

void CInfoIntermission::Spawn()
{
	SetAbsOrigin(GetAbsOrigin());
	SetSolidType(Solid::Not);
	pev->effects = EF_NODRAW;
	pev->v_angle = vec3_origin;

	pev->nextthink = gpGlobals->time + 2;// let targets spawn!
}

void CInfoIntermission::Think()
{
	// find my target
	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));

	if (!IsNullEnt(pTarget))
	{
		pev->v_angle = VectorAngles((pTarget->GetAbsOrigin() - GetAbsOrigin()).Normalize());
		pev->v_angle.x = -pev->v_angle.x;
	}
}

LINK_ENTITY_TO_CLASS(info_intermission, CInfoIntermission);
