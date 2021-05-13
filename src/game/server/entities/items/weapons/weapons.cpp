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

#include "UserMessages.h"
#include "dll_functions.hpp"

DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
DLL_GLOBAL  const char* g_pModelNameLaser = "sprites/laserbeam.spr";
DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for the initial blood
DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for splattered blood

MULTIDAMAGE gMultiDamage;

bool bIsMultiplayer()
{
	return g_pGameRules->IsMultiplayer();
}

/**
*	@brief pass in a name and this function will tell you the maximum amount of that type of ammunition that a player can carry.
*/
int MaxAmmoCarry(string_t iszName)
{
	for (int i = 0; i < MAX_AMMO_TYPES; i++)
	{
		const auto& info = CBaseWeapon::AmmoInfoArray[i];

		if (info.pszName && !strcmp(info.pszName, STRING(iszName)))
		{
			return info.MaxCarry;
		}
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

void SpawnBlood(const Vector& vecSpot, int bloodColor, float flDamage)
{
	UTIL_BloodDrips(vecSpot, g_vecAttackDir, bloodColor, (int)flDamage);
}

void DecalGunshot(TraceResult* pTrace, int iBulletType)
{
	auto hit = CBaseEntity::InstanceOrNull(pTrace->pHit);

	if (!UTIL_IsValidEntity(hit))
		return;

	if (hit->GetSolidType() == Solid::BSP || hit->GetMovetype() == Movetype::PushStep)
	{
		// Decal the wall with a gunshot
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
			UTIL_GunshotDecalTrace(pTrace, hit->DamageDecal(DMG_BULLET));
			break;
		case BULLET_MONSTER_12MM:
			// smoke and decal
			UTIL_GunshotDecalTrace(pTrace, hit->DamageDecal(DMG_BULLET));
			break;
		case BULLET_PLAYER_CROWBAR:
			// wall decal
			UTIL_DecalTrace(pTrace, hit->DamageDecal(DMG_CLUB));
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

	WeaponInfo II;
	pEntity->Precache();
	memset(&II, 0, sizeof II);
	if (((CBaseWeapon*)pEntity)->GetWeaponInfo(II))
	{
		CBaseWeapon::WeaponInfoArray[II.iId] = II;

		if (II.pszAmmo1 && *II.pszAmmo1)
		{
			AddAmmoNameToAmmoRegistry(II.pszAmmo1, II.iMaxAmmo1);
		}

		if (II.pszAmmo2 && *II.pszAmmo2)
		{
			AddAmmoNameToAmmoRegistry(II.pszAmmo2, II.iMaxAmmo2);
		}

		memset(&II, 0, sizeof II);
	}

	UTIL_RemoveNow(pEntity);
}

void W_Precache()
{
	memset(CBaseWeapon::WeaponInfoArray, 0, sizeof(CBaseWeapon::WeaponInfoArray));
	memset(CBaseWeapon::AmmoInfoArray, 0, sizeof(CBaseWeapon::AmmoInfoArray));
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

TYPEDESCRIPTION	CBaseAmmo::m_SaveData[] =
{
	DEFINE_FIELD(CBaseAmmo, m_iAmount, FIELD_INTEGER),
	DEFINE_FIELD(CBaseAmmo, m_iszAmmoName, FIELD_STRING),
	DEFINE_FIELD(CBaseAmmo, m_iszPickupSound, FIELD_SOUNDNAME),
};

IMPLEMENT_SAVERESTORE(CBaseAmmo, CBaseItem);

void CBaseAmmo::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "ammo_amount"))
	{
		m_iAmount = std::max(0, atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "pickup_sound"))
	{
		m_iszPickupSound = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
	{
		CBaseItem::KeyValue(pkvd);
	}
}

void CBaseAmmo::Precache()
{
	CBaseItem::Precache();

	if (!IsStringNull(m_iszPickupSound))
	{
		PRECACHE_SOUND(STRING(m_iszPickupSound));
	}
}

ItemApplyResult CBaseAmmo::DefaultGiveAmmo(CBasePlayer* player, int amount, const char* ammoName)
{
	if (ammoName && *ammoName && player->GiveAmmo(amount, ammoName) != -1)
	{
		if (auto sound = STRING(m_iszPickupSound); *sound)
		{
			EmitSound(SoundChannel::Item, sound);
		}

		return {ItemApplyAction::Used};
	}
	return {ItemApplyAction::NotUsed};
}

ItemApplyResult CBaseAmmo::Apply(CBasePlayer* player)
{
	return DefaultGiveAmmo(player, m_iAmount, STRING(m_iszAmmoName));
}

LINK_ENTITY_TO_CLASS(ammo_generic, CAmmoGeneric);

void CAmmoGeneric::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "ammo_name"))
	{
		m_iszAmmoName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
	{
		CBaseAmmo::KeyValue(pkvd);
	}
}

TYPEDESCRIPTION	CBaseWeapon::m_SaveData[] =
{
	DEFINE_FIELD(CBaseWeapon, m_hPlayer, FIELD_EHANDLE),
	DEFINE_FIELD(CBaseWeapon, m_hNext, FIELD_EHANDLE),
	//DEFINE_FIELD(CBaseWeapon, m_fKnown, FIELD_INTEGER),Reset to zero on load
	DEFINE_FIELD(CBaseWeapon, m_iId, FIELD_INTEGER),
	// DEFINE_FIELD(CBaseWeapon, m_iIdPrimary, FIELD_INTEGER),
	// DEFINE_FIELD(CBaseWeapon, m_iIdSecondary, FIELD_INTEGER),

#if defined( CLIENT_WEAPONS )
	DEFINE_FIELD(CBaseWeapon, m_flNextPrimaryAttack, FIELD_FLOAT),
	DEFINE_FIELD(CBaseWeapon, m_flNextSecondaryAttack, FIELD_FLOAT),
	DEFINE_FIELD(CBaseWeapon, m_flTimeWeaponIdle, FIELD_FLOAT),
#else	// CLIENT_WEAPONS
	DEFINE_FIELD(CBaseWeapon, m_flNextPrimaryAttack, FIELD_TIME),
	DEFINE_FIELD(CBaseWeapon, m_flNextSecondaryAttack, FIELD_TIME),
	DEFINE_FIELD(CBaseWeapon, m_flTimeWeaponIdle, FIELD_TIME),
#endif	// CLIENT_WEAPONS
	DEFINE_FIELD(CBaseWeapon, m_iPrimaryAmmoType, FIELD_INTEGER),
	DEFINE_FIELD(CBaseWeapon, m_iSecondaryAmmoType, FIELD_INTEGER),
	DEFINE_FIELD(CBaseWeapon, m_iClip, FIELD_INTEGER),
	DEFINE_FIELD(CBaseWeapon, m_iDefaultPrimaryAmmo, FIELD_INTEGER),
	DEFINE_FIELD(CBaseWeapon, m_iDefaultAmmo, FIELD_INTEGER),
	//	DEFINE_FIELD( CBaseWeapon, m_iClientClip, FIELD_INTEGER )	 , reset to zero on load so hud gets updated correctly
	//  DEFINE_FIELD( CBaseWeapon, m_iClientWeaponState, FIELD_INTEGER ), reset to zero on load so hud gets updated correctly
	DEFINE_FIELD(CBaseWeapon, m_iszWorldModelName, FIELD_MODELNAME),
	DEFINE_FIELD(CBaseWeapon, m_PickupRule, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CBaseWeapon, CBaseItem);

void CBaseWeapon::SetObjectCollisionBox()
{
	pev->absmin = GetAbsOrigin() + Vector(-24, -24, 0);
	pev->absmax = GetAbsOrigin() + Vector(24, 24, 16);
}

void CBaseWeapon::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "default_primary_ammo"))
	{
		m_iDefaultAmmo = m_iDefaultPrimaryAmmo = std::max(0, atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "pickup_rule"))
	{
		if (AreStringsEqual(pkvd->szValue, "Default"))
		{
			m_PickupRule = WeaponPickupRule::Default;
		}
		else if (AreStringsEqual(pkvd->szValue, "Always"))
		{
			m_PickupRule = WeaponPickupRule::Always;
		}
		else if (AreStringsEqual(pkvd->szValue, "Never"))
		{
			m_PickupRule = WeaponPickupRule::Never;
		}
		else if (AreStringsEqual(pkvd->szValue, "NoDuplicates"))
		{
			m_PickupRule = WeaponPickupRule::NoDuplicates;
		}
		else
		{
			ALERT(at_warning, "Invalid pickup_rule value \"%s\" for \"%s\" (entity index %d)\n", pkvd->szValue, GetClassname(), entindex());
		}

		pkvd->fHandled = true;
	}
	else
	{
		CBaseItem::KeyValue(pkvd);
	}
}

void CBaseWeapon::FallInit()
{
	SetModel(GetWorldModelName());
	SetupItem(vec3_origin, vec3_origin);//pointsize until it lands on the ground.
}

CBaseWeapon* CBaseWeapon::GetItemToRespawn(const Vector& respawnPoint)
{
	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	//Don't pass the current owner since the new weapon isn't owned by that entity
	auto pNewWeapon = static_cast<CBaseWeapon*>(CBaseEntity::Create(GetClassname(), respawnPoint, GetAbsAngles(), nullptr, false));

	//Copy over item settings
	pNewWeapon->SetModel(GetWorldModelName());
	pNewWeapon->pev->sequence = pev->sequence;
	pNewWeapon->m_OriginalPosition = m_OriginalPosition;
	pNewWeapon->m_RespawnMode = m_RespawnMode;
	pNewWeapon->m_flRespawnDelay = m_flRespawnDelay;
	pNewWeapon->m_RespawnPositionMode = m_RespawnPositionMode;

	pNewWeapon->m_FallMode = m_FallMode;
	pNewWeapon->m_bCanPickUpWhileFalling = m_bCanPickUpWhileFalling;
	pNewWeapon->m_ClatterMode = m_ClatterMode;
	pNewWeapon->m_bStayVisibleDuringRespawn = m_bStayVisibleDuringRespawn;
	pNewWeapon->m_bFlashOnRespawn = m_bFlashOnRespawn;

	pNewWeapon->m_iszClatterSound = m_iszClatterSound;
	pNewWeapon->m_iszRespawnSound = m_iszRespawnSound;

	pNewWeapon->m_iszTriggerOnMaterialize = m_iszTriggerOnMaterialize;
	pNewWeapon->m_iszTriggerOnDematerialize = m_iszTriggerOnDematerialize;

	pNewWeapon->m_iDefaultAmmo = pNewWeapon->m_iDefaultPrimaryAmmo = m_iDefaultPrimaryAmmo;
	pNewWeapon->m_PickupRule = m_PickupRule;

	DispatchSpawn(pNewWeapon->edict());

	return pNewWeapon;
}

ItemApplyResult CBaseWeapon::Apply(CBasePlayer* pPlayer)
{
	const ItemApplyResult result = pPlayer->AddPlayerWeapon(this);

	if (result.Action != ItemApplyAction::NotUsed)
	{
		AttachToPlayer(pPlayer);
		pPlayer->EmitSound(SoundChannel::Item, "items/gunpickup2.wav");
	}

	return result;
}

void CBaseWeapon::DestroyWeapon()
{
	if (auto player = m_hPlayer.Get(); player)
	{
		// if attached to a player, remove. 
		player->RemovePlayerWeapon(this);
	}

	Kill();
}

void CBaseWeapon::Drop()
{
	SetTouch(nullptr);
	SetThink(&CBaseWeapon::SUB_Remove);
	pev->nextthink = gpGlobals->time + .1;
}

void CBaseWeapon::Kill()
{
	SetTouch(nullptr);
	SetThink(&CBaseWeapon::SUB_Remove);
	pev->nextthink = gpGlobals->time + .1;
}

void CBaseWeapon::AttachToPlayer(CBasePlayer* pPlayer)
{
	SetMovetype(Movetype::Follow);
	SetSolidType(Solid::Not);
	SetAimEntity(pPlayer);
	pev->effects = EF_NODRAW; // ??
	pev->modelindex = 0;// server won't send down to clients if modelindex == 0
	pev->model = iStringNull;
	SetOwner(pPlayer);
	pev->nextthink = gpGlobals->time + .1;
	SetTouch(nullptr);
}

bool CBaseWeapon::AddDuplicate(CBaseWeapon* pOriginal)
{
	if (m_iDefaultAmmo)
	{
		return ExtractAmmo(pOriginal);
	}
	else
	{
		// a dead player dropped this.
		return ExtractClipAmmo(pOriginal);
	}
}

bool CBaseWeapon::AddToPlayer(CBasePlayer* pPlayer)
{
	m_hPlayer = pPlayer;

	bool bResult = true;

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

bool CBaseWeapon::UpdateClientData(CBasePlayer* pPlayer)
{
	bool bSend = false;
	WeaponState state = WeaponState::NotActive;
	if (pPlayer->m_hActiveWeapon == this)
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
	if (this == pPlayer->m_hActiveWeapon ||
		this == pPlayer->m_hClientActiveWeapon)
	{
		if (pPlayer->m_hActiveWeapon != pPlayer->m_hClientActiveWeapon)
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
		MESSAGE_BEGIN(MessageDest::One, gmsgCurWeapon, pPlayer);
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

bool CBaseWeapon::AddPrimaryAmmo(int iCount, const char* szName)
{
	const int iMaxClip = MaxClip();

	auto player = m_hPlayer.Get();

	int iIdAmmo;

	if (iMaxClip < 1)
	{
		m_iClip = -1;
		iIdAmmo = player->GiveAmmo(iCount, szName);
	}
	else if (m_iClip == 0)
	{
		int i;
		i = std::min(m_iClip + iCount, iMaxClip) - m_iClip;
		m_iClip += i;
		iIdAmmo = player->GiveAmmo(iCount - i, szName);
	}
	else
	{
		iIdAmmo = player->GiveAmmo(iCount, szName);
	}

	// player->m_rgAmmo[m_iPrimaryAmmoType] = iMaxCarry; // hack for testing

	if (iIdAmmo > 0)
	{
		m_iPrimaryAmmoType = iIdAmmo;
		if (player->HasPlayerWeapon(this))
		{
			// play the "got ammo" sound only if we gave some ammo to a player that already had this gun.
			// if the player is just getting this gun for the first time, DefaultTouch will play the "picked up gun" sound for us.
			EmitSound(SoundChannel::Item, "items/9mmclip1.wav");
		}
	}

	return iIdAmmo > 0;
}

bool CBaseWeapon::AddSecondaryAmmo(int iCount, const char* szName)
{
	int iIdAmmo;

	iIdAmmo = m_hPlayer->GiveAmmo(iCount, szName);

	//m_hPlayer->m_rgAmmo[m_iSecondaryAmmoType] = iMax; // hack for testing

	if (iIdAmmo > 0)
	{
		m_iSecondaryAmmoType = iIdAmmo;
		EmitSound(SoundChannel::Item, "items/9mmclip1.wav");
	}
	return iIdAmmo > 0;
}

bool CBaseWeapon::IsUseable()
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

bool CBaseWeapon::ExtractAmmo(CBaseWeapon* pWeapon)
{
	bool result = false;

	if (Ammo1Name() != nullptr)
	{
		// blindly call with m_iDefaultAmmo. It's either going to be a value or zero. If it is zero,
		// we only get the ammo in the weapon's clip, which is what we want. 
		result = pWeapon->AddPrimaryAmmo(m_iDefaultAmmo, Ammo1Name());
		m_iDefaultAmmo = 0;
	}

	if (Ammo2Name() != nullptr)
	{
		result = pWeapon->AddSecondaryAmmo(0, Ammo2Name()) || result;
	}

	return result;
}

int CBaseWeapon::ExtractClipAmmo(CBaseWeapon* pWeapon)
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

	return pWeapon->m_hPlayer->GiveAmmo(iAmmo, Ammo1Name()); // , &m_iPrimaryAmmoType
}

void CBaseWeapon::RetireWeapon()
{
	auto player = m_hPlayer.Get();
	// first, no viewmodel at all.
	player->pev->viewmodel = iStringNull;
	player->pev->weaponmodel = iStringNull;
	//m_pPlayer->pev->viewmodelindex = 0;

	g_pGameRules->GetNextBestWeapon(player, this);

	//If we're still equipped and we couldn't switch to another weapon, dequip this one
	if (CanHolster() && player->m_hActiveWeapon == this)
	{
		player->SwitchWeapon(nullptr);
	}
}

float CBaseWeapon::GetNextAttackDelay(float delay)
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
	DEFINE_ARRAY(CWeaponBox, m_hPlayerWeapons, FIELD_EHANDLE, MAX_WEAPON_TYPES),
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

	SetMovetype(Movetype::Toss);
	SetSolidType(Solid::Trigger);

	SetSize(vec3_origin, vec3_origin);

	SetModel("models/w_weaponbox.mdl");
}

void CWeaponBox::Kill()
{
	CBaseWeapon* pWeapon;
	int i;

	// destroy the weapons
	for (i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		pWeapon = m_hPlayerWeapons[i];

		while (pWeapon)
		{
			pWeapon->SetThink(&CBaseWeapon::SUB_Remove);
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
			pPlayer->GiveAmmo(m_rgAmmo[i], STRING(m_rgiszAmmo[i]));

			//ALERT ( at_console, "Gave %d rounds of %s\n", m_rgAmmo[i], STRING(m_rgiszAmmo[i]) );

			// now empty the ammo from the weaponbox since we just gave it to the player
			m_rgiszAmmo[i] = iStringNull;
			m_rgAmmo[i] = 0;
		}
	}

	// go through my weapons and try to give the usable ones to the player. 
	// it's important the the player be given ammo first, so the weapons code doesn't refuse 
	// to deploy a better weapon that the player may pick up because he has no ammo for it.
	for (i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		CBaseWeapon* weapon = m_hPlayerWeapons[i];

		while (weapon)
		{
			//ALERT ( at_console, "trying to give %s\n", m_hPlayerWeapons[ i ]->GetClassname() );

			auto next = m_hPlayerWeapons[i]->m_hNext;
				
			m_hPlayerWeapons[i] = next;// unlink this weapon from the box

			if (pPlayer->AddPlayerWeapon(weapon).Action == ItemApplyAction::AttachedToPlayer)
			{
				weapon->AttachToPlayer(pPlayer);
			}
			else
			{
				//Player didn't pick it up, remove the weapon
				UTIL_Remove(weapon);
			}

			weapon = next;
		}
	}

	pOther->EmitSound(SoundChannel::Item, "items/gunpickup2.wav");
	SetTouch(nullptr);
	UTIL_Remove(this);
}

bool CWeaponBox::PackWeapon(CBaseWeapon* pWeapon)
{
	// is one of these weapons already packed in this box?
	if (HasWeapon(pWeapon))
	{
		return false;// box can only hold one of each weapon type
	}

	if (auto player = pWeapon->m_hPlayer.Get(); player)
	{
		if (!player->RemovePlayerWeapon(pWeapon))
		{
			// failed to unhook the weapon from the player!
			return false;
		}
	}

	int iWeaponSlot = pWeapon->WeaponSlot();

	if (m_hPlayerWeapons[iWeaponSlot])
	{
		// there's already one weapon in this slot, so link this into the slot's column
		pWeapon->m_hNext = m_hPlayerWeapons[iWeaponSlot];
		m_hPlayerWeapons[iWeaponSlot] = pWeapon;
	}
	else
	{
		// first weapon we have for this slot
		m_hPlayerWeapons[iWeaponSlot] = pWeapon;
		pWeapon->m_hNext = nullptr;
	}

	pWeapon->m_RespawnMode = ItemRespawnMode::Never;// never respawn
	pWeapon->SetMovetype(Movetype::None);
	pWeapon->SetSolidType(Solid::Not);
	pWeapon->pev->effects = EF_NODRAW;
	pWeapon->pev->modelindex = 0;
	pWeapon->pev->model = iStringNull;
	pWeapon->SetOwner(this);
	pWeapon->SetThink(nullptr);// crowbar may be trying to swing again, etc.
	pWeapon->SetTouch(nullptr);
	pWeapon->m_hPlayer = nullptr;

	//ALERT ( at_console, "packed %s\n", pWeapon->GetClassname() );

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

bool CWeaponBox::HasWeapon(CBaseWeapon* pCheckWeapon)
{
	CBaseWeapon* weapon = m_hPlayerWeapons[pCheckWeapon->WeaponSlot()];

	while (weapon)
	{
		if (weapon->ClassnameIs(pCheckWeapon->GetClassname()))
		{
			return true;
		}
		weapon = weapon->m_hNext;
	}

	return false;
}

bool CWeaponBox::IsEmpty()
{
	int i;

	for (i = 0; i < MAX_WEAPON_TYPES; i++)
	{
		if (m_hPlayerWeapons[i])
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
	pev->absmin = GetAbsOrigin() + Vector(-16, -16, 0);
	pev->absmax = GetAbsOrigin() + Vector(16, 16, 16);
}

void CBaseWeapon::PrintState()
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

IMPLEMENT_SAVERESTORE(CRpg, CBaseWeapon);

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

IMPLEMENT_SAVERESTORE(CShotgun, CBaseWeapon);

TYPEDESCRIPTION	CGauss::m_SaveData[] =
{
	DEFINE_FIELD(CGauss, m_fInAttack, FIELD_INTEGER),
	//	DEFINE_FIELD( CGauss, m_flStartCharge, FIELD_TIME ),
	//	DEFINE_FIELD( CGauss, m_flPlayAftershock, FIELD_TIME ),
	//	DEFINE_FIELD( CGauss, m_flNextAmmoBurn, FIELD_TIME ),
		DEFINE_FIELD(CGauss, m_fPrimaryFire, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CGauss, CBaseWeapon);

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

IMPLEMENT_SAVERESTORE(CEgon, CBaseWeapon);

TYPEDESCRIPTION	CSatchel::m_SaveData[] =
{
	DEFINE_FIELD(CSatchel, m_chargeReady, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CSatchel, CBaseWeapon);
