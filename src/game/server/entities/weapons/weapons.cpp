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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "soundent.h"
#include "decals.hpp"
#include "gamerules.h"
#include "UserMessages.h"

extern int gEvilImpulse101;

DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
DLL_GLOBAL  const char* g_pModelNameLaser = "sprites/laserbeam.spr";
DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for the initial blood
DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for splattered blood

ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];
AmmoInfo CBasePlayerItem::AmmoInfoArray[MAX_AMMO_TYPES];

MULTIDAMAGE gMultiDamage;

/**
*	@brief pass in a name and this function will tell you the maximum amount of that type of ammunition that a player can carry.
*/
int MaxAmmoCarry(string_t iszName)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (CBasePlayerItem::ItemInfoArray[i].pszAmmo1 && !strcmp(STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo1))
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo1;
		if (CBasePlayerItem::ItemInfoArray[i].pszAmmo2 && !strcmp(STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo2))
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo2;
	}

	ALERT(at_console, "MaxAmmoCarry() doesn't recognize '%s'!\n", STRING(iszName));
	return -1;
}

void ClearMultiDamage()
{
	gMultiDamage.pEntity = nullptr;
	gMultiDamage.amount = 0;
	gMultiDamage.type = 0;
}

void ApplyMultiDamage(CBaseEntity* pInflictor, CBaseEntity* pAttacker)
{
	Vector		vecSpot1;//where blood comes from
	Vector		vecDir;//direction blood should go
	TraceResult	tr;

	if (!gMultiDamage.pEntity)
		return;

	gMultiDamage.pEntity->TakeDamage({pInflictor, pAttacker, gMultiDamage.amount, gMultiDamage.type});
}

void AddMultiDamage(CBaseEntity* pInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType)
{
	if (!pEntity)
		return;

	gMultiDamage.type |= bitsDamageType;

	if (pEntity != gMultiDamage.pEntity)
	{
		ApplyMultiDamage(pInflictor, pInflictor); // UNDONE: wrong attacker!
		gMultiDamage.pEntity = pEntity;
		gMultiDamage.amount = 0;
	}

	gMultiDamage.amount += flDamage;
}

void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage)
{
	UTIL_BloodDrips(vecSpot, g_vecAttackDir, bloodColor, (int)flDamage);
}

int DamageDecal(CBaseEntity* pEntity, int bitsDamageType)
{
	if (!pEntity)
		return (DECAL_GUNSHOT1 + RANDOM_LONG(0, 4));

	return pEntity->DamageDecal(bitsDamageType);
}

void DecalGunshot(TraceResult* pTrace, int iBulletType)
{
	if (!UTIL_IsValidEntity(pTrace->pHit))
		return;

	if (VARS(pTrace->pHit)->solid == Solid::BSP || VARS(pTrace->pHit)->movetype == Movetype::PushStep)
	{
		CBaseEntity* pEntity = nullptr;
		// Decal the wall with a gunshot
		if (!IsNullEnt(pTrace->pHit))
			pEntity = CBaseEntity::Instance(pTrace->pHit);

		switch (iBulletType)
		{
		case BULLET_PLAYER_9MM:
		case BULLET_MONSTER_9MM:
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_PLAYER_BUCKSHOT:
		case BULLET_PLAYER_357:
		default:
			// smoke and decal
			UTIL_GunshotDecalTrace(pTrace, DamageDecal(pEntity, DMG_BULLET));
			break;
		case BULLET_MONSTER_12MM:
			// smoke and decal
			UTIL_GunshotDecalTrace(pTrace, DamageDecal(pEntity, DMG_BULLET));
			break;
		case BULLET_PLAYER_CROWBAR:
			// wall decal
			UTIL_DecalTrace(pTrace, DamageDecal(pEntity, DMG_CLUB));
			break;
		}
	}
}

void EjectBrass(const Vector& vecOrigin, const Vector& vecVelocity, float rotation, int model, int soundtype)
{
	// FIX: when the player shoots, their gun isn't in the same position as it is on the model other players see.

	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, vecOrigin);
	WRITE_BYTE(TE_MODEL);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_COORD(vecVelocity.x);
	WRITE_COORD(vecVelocity.y);
	WRITE_COORD(vecVelocity.z);
	WRITE_ANGLE(rotation);
	WRITE_SHORT(model);
	WRITE_BYTE(soundtype);
	WRITE_BYTE(25);// 2.5 seconds
	MESSAGE_END();
}


#if 0
// UNDONE: This is no longer used?
void ExplodeModel(const Vector& vecOrigin, float speed, int model, int count)
{
	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, vecOrigin);
	WRITE_BYTE(TE_EXPLODEMODEL);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_COORD(speed);
	WRITE_SHORT(model);
	WRITE_SHORT(count);
	WRITE_BYTE(15);// 1.5 seconds
	MESSAGE_END();
}
#endif

/**
*	@brief Precaches the weapon and queues the weapon info for sending to clients
*/
void UTIL_PrecacheOtherWeapon(const char* szClassname)
{
	auto pEntity = UTIL_CreateNamedEntity(MAKE_STRING(szClassname));
	if (IsNullEnt(pEntity))
	{
		ALERT(at_console, "NULL Ent in UTIL_PrecacheOtherWeapon\n");
		return;
	}

	ItemInfo II;
	pEntity->Precache();
	memset(&II, 0, sizeof II);
	if (((CBasePlayerItem*)pEntity)->GetItemInfo(&II))
	{
		CBasePlayerItem::ItemInfoArray[II.iId] = II;

		if (II.pszAmmo1 && *II.pszAmmo1)
		{
			AddAmmoNameToAmmoRegistry(II.pszAmmo1);
		}

		if (II.pszAmmo2 && *II.pszAmmo2)
		{
			AddAmmoNameToAmmoRegistry(II.pszAmmo2);
		}

		memset(&II, 0, sizeof II);
	}

	REMOVE_ENTITY(pEntity->edict());
}

void W_Precache()
{
	memset(CBasePlayerItem::ItemInfoArray, 0, sizeof(CBasePlayerItem::ItemInfoArray));
	memset(CBasePlayerItem::AmmoInfoArray, 0, sizeof(CBasePlayerItem::AmmoInfoArray));
	giAmmoIndex = 0;

	// custom items...

	// common world objects
	UTIL_PrecacheOther("item_suit");
	UTIL_PrecacheOther("item_battery");
	UTIL_PrecacheOther("item_antidote");
	UTIL_PrecacheOther("item_longjump");

	// shotgun
	UTIL_PrecacheOtherWeapon("weapon_shotgun");
	UTIL_PrecacheOther("ammo_buckshot");

	// crowbar
	UTIL_PrecacheOtherWeapon("weapon_crowbar");

	// glock
	UTIL_PrecacheOtherWeapon("weapon_9mmhandgun");
	UTIL_PrecacheOther("ammo_9mmclip");

	// mp5
	UTIL_PrecacheOtherWeapon("weapon_9mmAR");
	UTIL_PrecacheOther("ammo_9mmAR");
	UTIL_PrecacheOther("ammo_ARgrenades");

	// python
	UTIL_PrecacheOtherWeapon("weapon_357");
	UTIL_PrecacheOther("ammo_357");

	// gauss
	UTIL_PrecacheOtherWeapon("weapon_gauss");
	UTIL_PrecacheOther("ammo_gaussclip");

	// rpg
	UTIL_PrecacheOtherWeapon("weapon_rpg");
	UTIL_PrecacheOther("ammo_rpgclip");

	// crossbow
	UTIL_PrecacheOtherWeapon("weapon_crossbow");
	UTIL_PrecacheOther("ammo_crossbow");

	// egon
	UTIL_PrecacheOtherWeapon("weapon_egon");

	// tripmine
	UTIL_PrecacheOtherWeapon("weapon_tripmine");

	// satchel charge
	UTIL_PrecacheOtherWeapon("weapon_satchel");

	// hand grenade
	UTIL_PrecacheOtherWeapon("weapon_handgrenade");

	// squeak grenade
	UTIL_PrecacheOtherWeapon("weapon_snark");

	// hornetgun
	UTIL_PrecacheOtherWeapon("weapon_hornetgun");

	if (g_pGameRules->IsMultiplayer())
	{
		UTIL_PrecacheOther("weaponbox");// container for dropped deathmatch weapons
	}

	g_sModelIndexFireball = PRECACHE_MODEL("sprites/zerogxplode.spr");// fireball
	g_sModelIndexWExplosion = PRECACHE_MODEL("sprites/WXplo1.spr");// underwater fireball
	g_sModelIndexSmoke = PRECACHE_MODEL("sprites/steam1.spr");// smoke
	g_sModelIndexBubbles = PRECACHE_MODEL("sprites/bubble.spr");//bubbles
	g_sModelIndexBloodSpray = PRECACHE_MODEL("sprites/bloodspray.spr"); // initial blood
	g_sModelIndexBloodDrop = PRECACHE_MODEL("sprites/blood.spr"); // splattered blood 

	g_sModelIndexLaser = PRECACHE_MODEL(g_pModelNameLaser);
	g_sModelIndexLaserDot = PRECACHE_MODEL("sprites/laserdot.spr");


	// used by explosions
	PRECACHE_MODEL("models/grenade.mdl");
	PRECACHE_MODEL("sprites/explode1.spr");

	PRECACHE_SOUND("weapons/debris1.wav");// explosion aftermaths
	PRECACHE_SOUND("weapons/debris2.wav");// explosion aftermaths
	PRECACHE_SOUND("weapons/debris3.wav");// explosion aftermaths

	PRECACHE_SOUND("weapons/grenade_hit1.wav");//grenade
	PRECACHE_SOUND("weapons/grenade_hit2.wav");//grenade
	PRECACHE_SOUND("weapons/grenade_hit3.wav");//grenade

	PRECACHE_SOUND("weapons/bullet_hit1.wav");	// hit by bullet
	PRECACHE_SOUND("weapons/bullet_hit2.wav");	// hit by bullet

	PRECACHE_SOUND("items/weapondrop1.wav");// weapon falls to the ground

}

TYPEDESCRIPTION	CBasePlayerItem::m_SaveData[] =
{
	DEFINE_FIELD(CBasePlayerItem, m_hPlayer, FIELD_EHANDLE),
	DEFINE_FIELD(CBasePlayerItem, m_hNext, FIELD_EHANDLE),
	//DEFINE_FIELD( CBasePlayerItem, m_fKnown, FIELD_INTEGER ),Reset to zero on load
	DEFINE_FIELD(CBasePlayerItem, m_iId, FIELD_INTEGER),
	// DEFINE_FIELD( CBasePlayerItem, m_iIdPrimary, FIELD_INTEGER ),
	// DEFINE_FIELD( CBasePlayerItem, m_iIdSecondary, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE(CBasePlayerItem, CBaseAnimating);

TYPEDESCRIPTION	CBasePlayerWeapon::m_SaveData[] =
{
#if defined( CLIENT_WEAPONS )
	DEFINE_FIELD(CBasePlayerWeapon, m_flNextPrimaryAttack, FIELD_FLOAT),
	DEFINE_FIELD(CBasePlayerWeapon, m_flNextSecondaryAttack, FIELD_FLOAT),
	DEFINE_FIELD(CBasePlayerWeapon, m_flTimeWeaponIdle, FIELD_FLOAT),
#else	// CLIENT_WEAPONS
	DEFINE_FIELD(CBasePlayerWeapon, m_flNextPrimaryAttack, FIELD_TIME),
	DEFINE_FIELD(CBasePlayerWeapon, m_flNextSecondaryAttack, FIELD_TIME),
	DEFINE_FIELD(CBasePlayerWeapon, m_flTimeWeaponIdle, FIELD_TIME),
#endif	// CLIENT_WEAPONS
	DEFINE_FIELD(CBasePlayerWeapon, m_iPrimaryAmmoType, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iSecondaryAmmoType, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iClip, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iDefaultAmmo, FIELD_INTEGER),
	//	DEFINE_FIELD( CBasePlayerWeapon, m_iClientClip, FIELD_INTEGER )	 , reset to zero on load so hud gets updated correctly
	//  DEFINE_FIELD( CBasePlayerWeapon, m_iClientWeaponState, FIELD_INTEGER ), reset to zero on load so hud gets updated correctly
};

IMPLEMENT_SAVERESTORE(CBasePlayerWeapon, CBasePlayerItem);

void CBasePlayerItem::SetObjectCollisionBox()
{
	pev->absmin = pev->origin + Vector(-24, -24, 0);
	pev->absmax = pev->origin + Vector(24, 24, 16);
}

void CBasePlayerItem::FallInit()
{
	pev->movetype = Movetype::Toss;
	pev->solid = Solid::BBox;

	SetAbsOrigin(pev->origin);
	SetSize(vec3_origin, vec3_origin);//pointsize until it lands on the ground.

	SetTouch(&CBasePlayerItem::DefaultTouch);
	SetThink(&CBasePlayerItem::FallThink);

	pev->nextthink = gpGlobals->time + 0.1;
}

void CBasePlayerItem::FallThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->flags & FL_ONGROUND)
	{
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		if (!IsNullEnt(pev->owner))
		{
			int pitch = 95 + RANDOM_LONG(0, 29);
			EmitSound(SoundChannel::Voice, "items/weapondrop1.wav", VOL_NORM, ATTN_NORM, pitch);
		}

		// lie flat
		pev->angles.x = 0;
		pev->angles.z = 0;

		Materialize();
	}
}

void CBasePlayerItem::Materialize()
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		EmitSound(SoundChannel::Weapon, "items/suitchargeok1.wav", VOL_NORM, ATTN_NORM, 150);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	pev->solid = Solid::Trigger;

	SetAbsOrigin(pev->origin);// link into world.
	SetTouch(&CBasePlayerItem::DefaultTouch);
	SetThink(nullptr);

}

void CBasePlayerItem::AttemptToMaterialize()
{
	float time = g_pGameRules->WeaponTryRespawn(this);

	if (time == 0)
	{
		Materialize();
		return;
	}

	pev->nextthink = gpGlobals->time + time;
}

void CBasePlayerItem::CheckRespawn()
{
	switch (g_pGameRules->WeaponShouldRespawn(this))
	{
	case GR_WEAPON_RESPAWN_YES:
		Respawn();
		break;
	case GR_WEAPON_RESPAWN_NO:
		return;
		break;
	}
}

CBaseEntity* CBasePlayerItem::Respawn()
{
	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	CBaseEntity* pNewWeapon = CBaseEntity::Create(STRING(pev->classname), g_pGameRules->WeaponRespawnSpot(this), pev->angles, InstanceOrNull(pev->owner));

	if (pNewWeapon)
	{
		pNewWeapon->pev->effects |= EF_NODRAW;// invisible for now
		pNewWeapon->SetTouch(nullptr);// no touch
		pNewWeapon->SetThink(&CBasePlayerItem::AttemptToMaterialize);

		DROP_TO_FLOOR(edict());

		// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.
		pNewWeapon->pev->nextthink = g_pGameRules->WeaponRespawnTime(this);
	}
	else
	{
		ALERT(at_console, "Respawn failed to create %s!\n", STRING(pev->classname));
	}

	return pNewWeapon;
}

void CBasePlayerItem::DefaultTouch(CBaseEntity* pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
		return;

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;

	// can I have this?
	if (!g_pGameRules->CanHavePlayerItem(pPlayer, this))
	{
		if (gEvilImpulse101)
		{
			UTIL_Remove(this);
		}
		return;
	}

	if (pPlayer->AddPlayerItem(this))
	{
		AttachToPlayer(pPlayer);
		pPlayer->EmitSound(SoundChannel::Item, "items/gunpickup2.wav");
	}

	SUB_UseTargets(pOther, UseType::Toggle, 0); // UNDONE: when should this happen?
}

void CBasePlayerItem::DestroyItem()
{
	if (auto player = m_hPlayer.Get(); player)
	{
		// if attached to a player, remove. 
		player->RemovePlayerItem(this);
	}

	Kill();
}

bool CBasePlayerItem::AddToPlayer(CBasePlayer* pPlayer)
{
	m_hPlayer = pPlayer;

	return true;
}

void CBasePlayerItem::Drop()
{
	SetTouch(nullptr);
	SetThink(&CBasePlayerItem::SUB_Remove);
	pev->nextthink = gpGlobals->time + .1;
}

void CBasePlayerItem::Kill()
{
	SetTouch(nullptr);
	SetThink(&CBasePlayerItem::SUB_Remove);
	pev->nextthink = gpGlobals->time + .1;
}

void CBasePlayerItem::Holster()
{
	auto player = m_hPlayer.Get();
	player->pev->viewmodel = iStringNull;
	player->pev->weaponmodel = iStringNull;
}

void CBasePlayerItem::AttachToPlayer(CBasePlayer* pPlayer)
{
	pev->movetype = Movetype::Follow;
	pev->solid = Solid::Not;
	pev->aiment = pPlayer->edict();
	pev->effects = EF_NODRAW; // ??
	pev->modelindex = 0;// server won't send down to clients if modelindex == 0
	pev->model = iStringNull;
	pev->owner = pPlayer->edict();
	pev->nextthink = gpGlobals->time + .1;
	SetTouch(nullptr);
}

bool CBasePlayerWeapon::AddDuplicate(CBasePlayerItem* pOriginal)
{
	if (m_iDefaultAmmo)
	{
		return ExtractAmmo((CBasePlayerWeapon*)pOriginal);
	}
	else
	{
		// a dead player dropped this.
		return ExtractClipAmmo((CBasePlayerWeapon*)pOriginal);
	}
}

bool CBasePlayerWeapon::AddToPlayer(CBasePlayer* pPlayer)
{
	bool bResult = CBasePlayerItem::AddToPlayer(pPlayer);

	pPlayer->pev->weapons |= (1 << m_iId);

	if (!m_iPrimaryAmmoType)
	{
		m_iPrimaryAmmoType = pPlayer->GetAmmoIndex(Ammo1Name());
		m_iSecondaryAmmoType = pPlayer->GetAmmoIndex(Ammo2Name());
	}


	if (bResult)
		return AddWeapon();
	return false;
}

bool CBasePlayerWeapon::UpdateClientData(CBasePlayer* pPlayer)
{
	bool bSend = false;
	WeaponState state = WeaponState::NotActive;
	if (pPlayer->m_hActiveItem == this)
	{
		if (pPlayer->m_fOnTarget)
			state = WeaponState::OnTarget;
		else
			state = WeaponState::Active;
	}

	// Forcing send of all data!
	if (!pPlayer->m_fWeapon)
	{
		bSend = true;
	}

	// This is the current or last weapon, so the state will need to be updated
	if (this == pPlayer->m_hActiveItem ||
		this == pPlayer->m_hClientActiveItem)
	{
		if (pPlayer->m_hActiveItem != pPlayer->m_hClientActiveItem)
		{
			bSend = true;
		}
	}

	// If the ammo, state, or fov has changed, update the weapon
	if (m_iClip != m_iClientClip ||
		state != m_iClientWeaponState ||
		pPlayer->m_iFOV != pPlayer->m_iClientFOV)
	{
		bSend = true;
	}

	if (bSend)
	{
		MESSAGE_BEGIN(MessageDest::One, gmsgCurWeapon, nullptr, pPlayer->pev);
		WRITE_BYTE(static_cast<int>(state));
		WRITE_BYTE(m_iId);
		WRITE_BYTE(m_iClip);
		MESSAGE_END();

		m_iClientClip = m_iClip;
		m_iClientWeaponState = state;
		pPlayer->m_fWeapon = true;
	}

	if (auto next = m_hNext.Get(); next)
		next->UpdateClientData(pPlayer);

	return true;
}

void CBasePlayerWeapon::SendWeaponAnim(int iAnim, int body)
{
	const bool skiplocal = UseDecrement();

	auto player = m_hPlayer.Get();

	player->pev->weaponanim = iAnim;

#if defined( CLIENT_WEAPONS )
	if (skiplocal && ENGINE_CANSKIP(player->edict()))
		return;
#endif

	MESSAGE_BEGIN(MessageDest::One, SVC_WEAPONANIM, nullptr, player->pev);
	WRITE_BYTE(iAnim);						// sequence number
	WRITE_BYTE(pev->body);					// weaponmodel bodygroup.
	MESSAGE_END();
}

bool CBasePlayerWeapon::AddPrimaryAmmo(int iCount, const char* szName, int iMaxClip, int iMaxCarry)
{
	auto player = m_hPlayer.Get();

	int iIdAmmo;

	if (iMaxClip < 1)
	{
		m_iClip = -1;
		iIdAmmo = player->GiveAmmo(iCount, szName, iMaxCarry);
	}
	else if (m_iClip == 0)
	{
		int i;
		i = std::min(m_iClip + iCount, iMaxClip) - m_iClip;
		m_iClip += i;
		iIdAmmo = player->GiveAmmo(iCount - i, szName, iMaxCarry);
	}
	else
	{
		iIdAmmo = player->GiveAmmo(iCount, szName, iMaxCarry);
	}

	// player->m_rgAmmo[m_iPrimaryAmmoType] = iMaxCarry; // hack for testing

	if (iIdAmmo > 0)
	{
		m_iPrimaryAmmoType = iIdAmmo;
		if (player->HasPlayerItem(this))
		{
			// play the "got ammo" sound only if we gave some ammo to a player that already had this gun.
			// if the player is just getting this gun for the first time, DefaultTouch will play the "picked up gun" sound for us.
			EmitSound(SoundChannel::Item, "items/9mmclip1.wav");
		}
	}

	return iIdAmmo > 0;
}

bool CBasePlayerWeapon::AddSecondaryAmmo(int iCount, const char* szName, int iMax)
{
	int iIdAmmo;

	iIdAmmo = m_hPlayer->GiveAmmo(iCount, szName, iMax);

	//m_hPlayer->m_rgAmmo[m_iSecondaryAmmoType] = iMax; // hack for testing

	if (iIdAmmo > 0)
	{
		m_iSecondaryAmmoType = iIdAmmo;
		EmitSound(SoundChannel::Item, "items/9mmclip1.wav");
	}
	return iIdAmmo > 0;
}

bool CBasePlayerWeapon::IsUseable()
{
	if (m_iClip > 0)
	{
		return true;
	}

	//Player has unlimited ammo for this weapon or does not use magazines
	if (MaxAmmo1() == WEAPON_NOCLIP)
	{
		return true;
	}

	auto player = m_hPlayer.Get();

	if (player->m_rgAmmo[PrimaryAmmoIndex()] > 0)
	{
		return true;
	}

	if (Ammo2Name())
	{
		//Player has unlimited ammo for this weapon or does not use magazines
		if (MaxAmmo2() == WEAPON_NOCLIP)
		{
			return true;
		}

		if (player->m_rgAmmo[SecondaryAmmoIndex()] > 0)
		{
			return true;
		}
	}

	// clip is empty (or nonexistant) and the player has no more ammo of this type. 
	return false;
}

bool CBasePlayerWeapon::DefaultDeploy(const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, int body)
{
	if (!CanDeploy())
		return false;

	auto player = m_hPlayer.Get();

	player->pev->viewmodel = MAKE_STRING(szViewModel);
	player->pev->weaponmodel = MAKE_STRING(szWeaponModel);
	safe_strcpy(player->m_szAnimExtension, szAnimExt);
	SendWeaponAnim(iAnim, body);

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
	m_flLastFireTime = 0.0;

	return true;
}

bool CBasePlayerWeapon::PlayEmptySound()
{
	if (m_iPlayEmptySound)
	{
		m_hPlayer->EmitSound(SoundChannel::Weapon, "weapons/357_cock1.wav", 0.8);
		m_iPlayEmptySound = false;
		return false;
	}
	return false;
}

int CBasePlayerWeapon::PrimaryAmmoIndex()
{
	return m_iPrimaryAmmoType;
}

int CBasePlayerWeapon::SecondaryAmmoIndex()
{
	return m_iSecondaryAmmoType;
}

void CBasePlayerWeapon::Holster()
{
	auto player = m_hPlayer.Get();

	m_fInReload = false; // cancel any reload in progress.
	player->pev->viewmodel = iStringNull;
	player->pev->weaponmodel = iStringNull;
}

void CBasePlayerAmmo::Spawn()
{
	pev->movetype = Movetype::Toss;
	pev->solid = Solid::Trigger;
	SetSize(Vector(-16, -16, 0), Vector(16, 16, 16));
	SetAbsOrigin(pev->origin);

	SetTouch(&CBasePlayerAmmo::DefaultTouch);
}

CBaseEntity* CBasePlayerAmmo::Respawn()
{
	pev->effects |= EF_NODRAW;
	SetTouch(nullptr);

	SetAbsOrigin(g_pGameRules->AmmoRespawnSpot(this));// move to wherever I'm supposed to repawn.

	SetThink(&CBasePlayerAmmo::Materialize);
	pev->nextthink = g_pGameRules->AmmoRespawnTime(this);

	return this;
}

void CBasePlayerAmmo::Materialize()
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		EmitSound(SoundChannel::Weapon, "items/suitchargeok1.wav", VOL_NORM, ATTN_NORM, 150);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch(&CBasePlayerAmmo::DefaultTouch);
}

void CBasePlayerAmmo::DefaultTouch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{
		return;
	}

	auto player = static_cast<CBasePlayer*>(pOther);

	if (AddAmmo(player))
	{
		if (g_pGameRules->AmmoShouldRespawn(this) == GR_AMMO_RESPAWN_YES)
		{
			Respawn();
		}
		else
		{
			SetTouch(nullptr);
			SetThink(&CBasePlayerAmmo::SUB_Remove);
			pev->nextthink = gpGlobals->time + .1;
		}
	}
	else if (gEvilImpulse101)
	{
		// evil impulse 101 hack, kill always
		SetTouch(nullptr);
		SetThink(&CBasePlayerAmmo::SUB_Remove);
		pev->nextthink = gpGlobals->time + .1;
	}
}

bool CBasePlayerWeapon::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	bool result = false;

	if (Ammo1Name() != nullptr)
	{
		// blindly call with m_iDefaultAmmo. It's either going to be a value or zero. If it is zero,
		// we only get the ammo in the weapon's clip, which is what we want. 
		result = pWeapon->AddPrimaryAmmo(m_iDefaultAmmo, Ammo1Name(), MaxClip(), MaxAmmo1());
		m_iDefaultAmmo = 0;
	}

	if (Ammo2Name() != nullptr)
	{
		result = pWeapon->AddSecondaryAmmo(0, Ammo2Name(), MaxAmmo2()) || result;
	}

	return result;
}

int CBasePlayerWeapon::ExtractClipAmmo(CBasePlayerWeapon* pWeapon)
{
	int			iAmmo;

	if (m_iClip == WEAPON_NOCLIP)
	{
		iAmmo = 0;// guns with no clips always come empty if they are second-hand
	}
	else
	{
		iAmmo = m_iClip;
	}

	return pWeapon->m_hPlayer->GiveAmmo(iAmmo, Ammo1Name(), MaxAmmo1()); // , &m_iPrimaryAmmoType
}

void CBasePlayerWeapon::RetireWeapon()
{
	auto player = m_hPlayer.Get();
	// first, no viewmodel at all.
	player->pev->viewmodel = iStringNull;
	player->pev->weaponmodel = iStringNull;
	//m_pPlayer->pev->viewmodelindex = 0;

	g_pGameRules->GetNextBestWeapon(player, this);

	//If we're still equipped and we couldn't switch to another weapon, dequip this one
	if (CanHolster() && player->m_hActiveItem == this)
	{
		player->SwitchWeapon(nullptr);
	}
}

float CBasePlayerWeapon::GetNextAttackDelay(float delay)
{
	if (m_flLastFireTime == 0 || m_flNextPrimaryAttack == -1)
	{
		// At this point, we are assuming that the client has stopped firing
		// and we are going to reset our book keeping variables.
		m_flLastFireTime = gpGlobals->time;
		m_flPrevPrimaryAttack = delay;
	}
	// calculate the time between this shot and the previous
	float flTimeBetweenFires = gpGlobals->time - m_flLastFireTime;
	float flCreep = 0.0f;
	if (flTimeBetweenFires > 0)
		flCreep = flTimeBetweenFires - m_flPrevPrimaryAttack; // postive or negative

// save the last fire time
	m_flLastFireTime = gpGlobals->time;

	float flNextAttack = UTIL_WeaponTimeBase() + delay - flCreep;
	// we need to remember what the m_flNextPrimaryAttack time is set to for each shot, 
	// store it as m_flPrevPrimaryAttack.
	m_flPrevPrimaryAttack = flNextAttack - UTIL_WeaponTimeBase();
	// 	char szMsg[256];
	// 	snprintf( szMsg, sizeof(szMsg), "next attack time: %0.4f\n", gpGlobals->time + flNextAttack );
	// 	OutputDebugString( szMsg );
	return flNextAttack;
}

LINK_ENTITY_TO_CLASS(weaponbox, CWeaponBox);

TYPEDESCRIPTION	CWeaponBox::m_SaveData[] =
{
	DEFINE_ARRAY(CWeaponBox, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_TYPES),
	DEFINE_ARRAY(CWeaponBox, m_rgiszAmmo, FIELD_STRING, MAX_AMMO_TYPES),
	DEFINE_ARRAY(CWeaponBox, m_hPlayerItems, FIELD_EHANDLE, MAX_ITEM_TYPES),
	DEFINE_FIELD(CWeaponBox, m_cAmmoTypes, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CWeaponBox, CBaseEntity);

void CWeaponBox::Precache()
{
	PRECACHE_MODEL("models/w_weaponbox.mdl");
}

void CWeaponBox::KeyValue(KeyValueData* pkvd)
{
	if (m_cAmmoTypes < MAX_AMMO_TYPES)
	{
		PackAmmo(ALLOC_STRING(pkvd->szKeyName), atoi(pkvd->szValue));
		m_cAmmoTypes++;// count this new ammo type.

		pkvd->fHandled = true;
	}
	else
	{
		ALERT(at_console, "WeaponBox too full! only %d ammotypes allowed\n", MAX_AMMO_TYPES);
	}
}

void CWeaponBox::Spawn()
{
	Precache();

	pev->movetype = Movetype::Toss;
	pev->solid = Solid::Trigger;

	SetSize(vec3_origin, vec3_origin);

	SetModel("models/w_weaponbox.mdl");
}

void CWeaponBox::Kill()
{
	CBasePlayerItem* pWeapon;
	int i;

	// destroy the weapons
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		pWeapon = m_hPlayerItems[i];

		while (pWeapon)
		{
			pWeapon->SetThink(&CBasePlayerItem::SUB_Remove);
			pWeapon->pev->nextthink = gpGlobals->time + 0.1;
			pWeapon = pWeapon->m_hNext;
		}
	}

	// remove the box
	UTIL_Remove(this);
}

void CWeaponBox::Touch(CBaseEntity* pOther)
{
	if (!(pev->flags & FL_ONGROUND))
	{
		return;
	}

	if (!pOther->IsPlayer())
	{
		// only players may touch a weaponbox.
		return;
	}

	if (!pOther->IsAlive())
	{
		// no dead guys.
		return;
	}

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;
	int i;

	// dole out ammo
	for (i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if (!IsStringNull(m_rgiszAmmo[i]))
		{
			// there's some ammo of this type. 
			pPlayer->GiveAmmo(m_rgAmmo[i], STRING(m_rgiszAmmo[i]), MaxAmmoCarry(m_rgiszAmmo[i]));

			//ALERT ( at_console, "Gave %d rounds of %s\n", m_rgAmmo[i], STRING(m_rgiszAmmo[i]) );

			// now empty the ammo from the weaponbox since we just gave it to the player
			m_rgiszAmmo[i] = iStringNull;
			m_rgAmmo[i] = 0;
		}
	}

	// go through my weapons and try to give the usable ones to the player. 
	// it's important the the player be given ammo first, so the weapons code doesn't refuse 
	// to deploy a better weapon that the player may pick up because he has no ammo for it.
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (CBasePlayerItem* pItem = m_hPlayerItems[i]; pItem)
		{
			// have at least one weapon in this slot
			while (pItem)
			{
				//ALERT ( at_console, "trying to give %s\n", STRING( m_rgpPlayerItems[ i ]->pev->classname ) );

				pItem = m_hPlayerItems[i] = m_hPlayerItems[i]->m_hNext;// unlink this weapon from the box

				if (pPlayer->AddPlayerItem(pItem))
				{
					pItem->AttachToPlayer(pPlayer);
				}
			}
		}
	}

	pOther->EmitSound(SoundChannel::Item, "items/gunpickup2.wav");
	SetTouch(nullptr);
	UTIL_Remove(this);
}

bool CWeaponBox::PackWeapon(CBasePlayerItem* pWeapon)
{
	// is one of these weapons already packed in this box?
	if (HasWeapon(pWeapon))
	{
		return false;// box can only hold one of each weapon type
	}

	if (auto player = pWeapon->m_hPlayer.Get(); player)
	{
		if (!player->RemovePlayerItem(pWeapon))
		{
			// failed to unhook the weapon from the player!
			return false;
		}
	}

	int iWeaponSlot = pWeapon->ItemSlot();

	if (m_hPlayerItems[iWeaponSlot])
	{
		// there's already one weapon in this slot, so link this into the slot's column
		pWeapon->m_hNext = m_hPlayerItems[iWeaponSlot];
		m_hPlayerItems[iWeaponSlot] = pWeapon;
	}
	else
	{
		// first weapon we have for this slot
		m_hPlayerItems[iWeaponSlot] = pWeapon;
		pWeapon->m_hNext = nullptr;
	}

	pWeapon->pev->spawnflags |= SF_NORESPAWN;// never respawn
	pWeapon->pev->movetype = Movetype::None;
	pWeapon->pev->solid = Solid::Not;
	pWeapon->pev->effects = EF_NODRAW;
	pWeapon->pev->modelindex = 0;
	pWeapon->pev->model = iStringNull;
	pWeapon->pev->owner = edict();
	pWeapon->SetThink(nullptr);// crowbar may be trying to swing again, etc.
	pWeapon->SetTouch(nullptr);
	pWeapon->m_hPlayer = nullptr;

	//ALERT ( at_console, "packed %s\n", STRING(pWeapon->pev->classname) );

	return true;
}

bool CWeaponBox::PackAmmo(string_t iszName, int iCount)
{
	int iMaxCarry;

	if (IsStringNull(iszName))
	{
		// error here
		ALERT(at_console, "NULL String in PackAmmo!\n");
		return false;
	}

	iMaxCarry = MaxAmmoCarry(iszName);

	if (iMaxCarry != -1 && iCount > 0)
	{
		//ALERT ( at_console, "Packed %d rounds of %s\n", iCount, STRING(iszName) );
		GiveAmmo(iCount, STRING(iszName), iMaxCarry);
		return true;
	}

	return false;
}

int CWeaponBox::GiveAmmo(int iCount, const char* szName, int iMax, int* pIndex)
{
	int i;

	for (i = 1; i < MAX_AMMO_TYPES && !IsStringNull(m_rgiszAmmo[i]); i++)
	{
		if (stricmp(szName, STRING(m_rgiszAmmo[i])) == 0)
		{
			if (pIndex)
				*pIndex = i;

			int iAdd = std::min(iCount, iMax - m_rgAmmo[i]);
			if (iCount == 0 || iAdd > 0)
			{
				m_rgAmmo[i] += iAdd;

				return i;
			}
			return -1;
		}
	}
	if (i < MAX_AMMO_TYPES)
	{
		if (pIndex)
			*pIndex = i;

		m_rgiszAmmo[i] = MAKE_STRING(szName);
		m_rgAmmo[i] = iCount;

		return i;
	}
	ALERT(at_console, "out of named ammo slots\n");
	return i;
}

bool CWeaponBox::HasWeapon(CBasePlayerItem* pCheckItem)
{
	CBasePlayerItem* pItem = m_hPlayerItems[pCheckItem->ItemSlot()];

	while (pItem)
	{
		if (pItem->ClassnameIs(STRING(pCheckItem->pev->classname)))
		{
			return true;
		}
		pItem = pItem->m_hNext;
	}

	return false;
}

bool CWeaponBox::IsEmpty()
{
	int i;

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_hPlayerItems[i])
		{
			return false;
		}
	}

	for (i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if (!IsStringNull(m_rgiszAmmo[i]))
		{
			// still have a bit of this type of ammo
			return false;
		}
	}

	return true;
}

void CWeaponBox::SetObjectCollisionBox()
{
	pev->absmin = pev->origin + Vector(-16, -16, 0);
	pev->absmax = pev->origin + Vector(16, 16, 16);
}

void CBasePlayerWeapon::PrintState()
{
	ALERT(at_console, "primary:  %f\n", m_flNextPrimaryAttack);
	ALERT(at_console, "idle   :  %f\n", m_flTimeWeaponIdle);

	//	ALERT( at_console, "nextrl :  %f\n", m_flNextReload );
	//	ALERT( at_console, "nextpum:  %f\n", m_flPumpTime );

	//	ALERT( at_console, "m_frt  :  %f\n", m_fReloadTime );
	ALERT(at_console, "m_finre:  %i\n", m_fInReload);
	//	ALERT( at_console, "m_finsr:  %i\n", m_fInSpecialReload );

	ALERT(at_console, "m_iclip:  %i\n", m_iClip);
}

TYPEDESCRIPTION	CRpg::m_SaveData[] =
{
	DEFINE_FIELD(CRpg, m_fSpotActive, FIELD_BOOLEAN),
	DEFINE_FIELD(CRpg, m_cActiveRockets, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CRpg, CBasePlayerWeapon);

TYPEDESCRIPTION	CRpgRocket::m_SaveData[] =
{
	DEFINE_FIELD(CRpgRocket, m_flIgniteTime, FIELD_TIME),
	DEFINE_FIELD(CRpgRocket, m_hLauncher, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CRpgRocket, CGrenade);

TYPEDESCRIPTION	CShotgun::m_SaveData[] =
{
	DEFINE_FIELD(CShotgun, m_flNextReload, FIELD_TIME),
	DEFINE_FIELD(CShotgun, m_fInSpecialReload, FIELD_INTEGER),
	DEFINE_FIELD(CShotgun, m_flNextReload, FIELD_TIME),
	// DEFINE_FIELD( CShotgun, m_iShell, FIELD_INTEGER ),
	DEFINE_FIELD(CShotgun, m_flPumpTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CShotgun, CBasePlayerWeapon);

TYPEDESCRIPTION	CGauss::m_SaveData[] =
{
	DEFINE_FIELD(CGauss, m_fInAttack, FIELD_INTEGER),
	//	DEFINE_FIELD( CGauss, m_flStartCharge, FIELD_TIME ),
	//	DEFINE_FIELD( CGauss, m_flPlayAftershock, FIELD_TIME ),
	//	DEFINE_FIELD( CGauss, m_flNextAmmoBurn, FIELD_TIME ),
		DEFINE_FIELD(CGauss, m_fPrimaryFire, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CGauss, CBasePlayerWeapon);

TYPEDESCRIPTION	CEgon::m_SaveData[] =
{
	//	DEFINE_FIELD( CEgon, m_hBeam, FIELD_EHANDLE ),
	//	DEFINE_FIELD( CEgon, m_hNoise, FIELD_EHANDLE ),
	//	DEFINE_FIELD( CEgon, m_hSprite, FIELD_EHANDLE ),
		DEFINE_FIELD(CEgon, m_shootTime, FIELD_TIME),
		DEFINE_FIELD(CEgon, m_fireState, FIELD_INTEGER),
		DEFINE_FIELD(CEgon, m_fireMode, FIELD_INTEGER),
		DEFINE_FIELD(CEgon, m_shakeTime, FIELD_TIME),
		DEFINE_FIELD(CEgon, m_flAmmoUseTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CEgon, CBasePlayerWeapon);

TYPEDESCRIPTION	CSatchel::m_SaveData[] =
{
	DEFINE_FIELD(CSatchel, m_chargeReady, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CSatchel, CBasePlayerWeapon);
