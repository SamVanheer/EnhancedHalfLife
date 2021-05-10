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

#pragma once

/**
*	@file
*
*	functions governing the selection/use of weapons for players
*/

#include "effects.h"
#include "CBaseItem.hpp"

class CBasePlayer;
class CBaseWeapon;
struct weapon_data_t;

/**
*	@brief removes all satchels owned by the provided player. Should only be used upon death.
*	Made this global on purpose.
*/
void DeactivateSatchels(CBasePlayer* pOwner);

/**
*	@brief called by worldspawn
*/
void W_Precache();

/**
*	@brief Contact Grenade / Timed grenade / Satchel Charge
*/
class CGrenade : public CBaseMonster
{
public:
	void Spawn() override;

	static CGrenade* ShootTimed(CBaseEntity* pOwner, const Vector& vecStart, const Vector& vecVelocity, float time);
	static CGrenade* ShootContact(CBaseEntity* pOwner, const Vector& vecStart, const Vector& vecVelocity);

	void Explode(TraceResult* pTrace, int bitsDamageType);
	void EXPORT Smoke();

	void EXPORT BounceTouch(CBaseEntity* pOther);
	void EXPORT SlideTouch(CBaseEntity* pOther);

	/**
	*	@brief Contact grenade, explode when it touches something
	*/
	void EXPORT ExplodeTouch(CBaseEntity* pOther);
	void EXPORT DangerSoundThink();

	/**
	*	@brief Timed grenade, this think is called when time runs out.
	*/
	void EXPORT PreDetonate();
	void EXPORT Detonate();
	void EXPORT DetonateUse(const UseInfo& info);
	void EXPORT TumbleThink();

	virtual void BounceSound();
	int	BloodColor() override { return DONT_BLEED; }
	void Killed(const KilledInfo& info) override;

	bool m_fRegisteredSound = false;//!< whether or not this grenade has issued its DANGER sound to the world sound list yet.
};

// constant items
constexpr int ITEM_ANTIDOTE = 0;

constexpr int WEAPON_NONE = 0;
constexpr int WEAPON_CROWBAR = 1;
constexpr int WEAPON_GLOCK = 2;
constexpr int WEAPON_PYTHON = 3;
constexpr int WEAPON_MP5 = 4;
constexpr int WEAPON_CHAINGUN = 5;
constexpr int WEAPON_CROSSBOW = 6;
constexpr int WEAPON_SHOTGUN = 7;
constexpr int WEAPON_RPG = 8;
constexpr int WEAPON_GAUSS = 9;
constexpr int WEAPON_EGON = 10;
constexpr int WEAPON_HORNETGUN = 11;
constexpr int WEAPON_HANDGRENADE = 12;
constexpr int WEAPON_TRIPMINE = 13;
constexpr int WEAPON_SATCHEL = 14;
constexpr int WEAPON_SNARK = 15;

constexpr int WEAPON_ALLWEAPONS = ~(1 << WEAPON_SUIT);

constexpr int MAX_NORMAL_BATTERY = 100;

// weapon weight factors (for auto-switching)   (-1 = noswitch)
constexpr int CROWBAR_WEIGHT = 0;
constexpr int GLOCK_WEIGHT = 10;
constexpr int PYTHON_WEIGHT = 15;
constexpr int MP5_WEIGHT = 15;
constexpr int SHOTGUN_WEIGHT = 15;
constexpr int CROSSBOW_WEIGHT = 10;
constexpr int RPG_WEIGHT = 20;
constexpr int GAUSS_WEIGHT = 20;
constexpr int EGON_WEIGHT = 20;
constexpr int HORNETGUN_WEIGHT = 10;
constexpr int HANDGRENADE_WEIGHT = 5;
constexpr int SNARK_WEIGHT = 5;
constexpr int SATCHEL_WEIGHT = -10;
constexpr int TRIPMINE_WEIGHT = -10;

// weapon clip/carry ammo capacities
constexpr int URANIUM_MAX_CARRY = 100;
constexpr int _9MM_MAX_CARRY = 250;
constexpr int _357_MAX_CARRY = 36;
constexpr int BUCKSHOT_MAX_CARRY = 125;
constexpr int BOLT_MAX_CARRY = 50;
constexpr int ROCKET_MAX_CARRY = 5;
constexpr int HANDGRENADE_MAX_CARRY = 10;
constexpr int SATCHEL_MAX_CARRY = 5;
constexpr int TRIPMINE_MAX_CARRY = 5;
constexpr int SNARK_MAX_CARRY = 15;
constexpr int HORNET_MAX_CARRY = 8;
constexpr int M203_GRENADE_MAX_CARRY = 10;

// the maximum amount of ammo each weapon's clip can hold
constexpr int WEAPON_NOCLIP = -1;

//constexpr int CROWBAR_MAX_CLIP = WEAPON_NOCLIP;
constexpr int GLOCK_MAX_CLIP = 17;
constexpr int PYTHON_MAX_CLIP = 6;
constexpr int MP5_MAX_CLIP = 50;
constexpr int MP5_DEFAULT_AMMO = 25;
constexpr int SHOTGUN_MAX_CLIP = 8;
constexpr int CROSSBOW_MAX_CLIP = 5;
constexpr int RPG_MAX_CLIP = 1;
constexpr int GAUSS_MAX_CLIP = WEAPON_NOCLIP;
constexpr int EGON_MAX_CLIP = WEAPON_NOCLIP;
constexpr int HORNETGUN_MAX_CLIP = WEAPON_NOCLIP;
constexpr int HANDGRENADE_MAX_CLIP = WEAPON_NOCLIP;
constexpr int SATCHEL_MAX_CLIP = WEAPON_NOCLIP;
constexpr int TRIPMINE_MAX_CLIP = WEAPON_NOCLIP;
constexpr int SNARK_MAX_CLIP = WEAPON_NOCLIP;

// the default amount of ammo that comes with each gun when it spawns
constexpr int GLOCK_DEFAULT_GIVE = 17;
constexpr int PYTHON_DEFAULT_GIVE = 6;
constexpr int MP5_DEFAULT_GIVE = 25;
constexpr int MP5_M203_DEFAULT_GIVE = 0;
constexpr int SHOTGUN_DEFAULT_GIVE = 12;
constexpr int CROSSBOW_DEFAULT_GIVE = 5;
constexpr int RPG_DEFAULT_GIVE = 1;
constexpr int GAUSS_DEFAULT_GIVE = 20;
constexpr int EGON_DEFAULT_GIVE = 20;
constexpr int HANDGRENADE_DEFAULT_GIVE = 5;
constexpr int SATCHEL_DEFAULT_GIVE = 1;
constexpr int TRIPMINE_DEFAULT_GIVE = 1;
constexpr int SNARK_DEFAULT_GIVE = 5;
constexpr int HIVEHAND_DEFAULT_GIVE = 8;

// The amount of ammo given to a player by an ammo item.
constexpr int AMMO_URANIUMBOX_GIVE = 20;
constexpr int AMMO_GLOCKCLIP_GIVE = GLOCK_MAX_CLIP;
constexpr int AMMO_357BOX_GIVE = PYTHON_MAX_CLIP;
constexpr int AMMO_MP5CLIP_GIVE = MP5_MAX_CLIP;
constexpr int AMMO_CHAINBOX_GIVE = 200;
constexpr int AMMO_M203BOX_GIVE = 2;
constexpr int AMMO_BUCKSHOTBOX_GIVE = 12;
constexpr int AMMO_CROSSBOWCLIP_GIVE = CROSSBOW_MAX_CLIP;
constexpr int AMMO_RPGCLIP_GIVE = RPG_MAX_CLIP;
constexpr int AMMO_SNARKBOX_GIVE = 5;

// bullet types
enum Bullet
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM, // glock
	BULLET_PLAYER_MP5, // mp5
	BULLET_PLAYER_357, // python
	BULLET_PLAYER_BUCKSHOT, // shotgun
	BULLET_PLAYER_CROWBAR, // crowbar swipe

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,
};

constexpr int WEAPON_FLAG_SELECTONEMPTY = 1;
constexpr int WEAPON_FLAG_NOAUTORELOAD = 2;
constexpr int WEAPON_FLAG_NOAUTOSWITCHEMPTY = 4;
constexpr int WEAPON_FLAG_LIMITINWORLD = 8;
constexpr int WEAPON_FLAG_EXHAUSTIBLE = 16; // A player can totally exhaust their ammo supply and lose this weapon

enum class WeaponState
{
	NotActive = 0,
	Active,
	OnTarget
};

struct WeaponInfo
{
	int		iSlot;
	int		iPosition;
	const char* pszAmmo1;	// ammo 1 type
	int		iMaxAmmo1;		// max ammo 1
	const char* pszAmmo2;	// ammo 2 type
	int		iMaxAmmo2;		// max ammo 2
	const char* pszName;
	int		iMaxClip;
	int		iId;
	int		iFlags;
	int		iWeight;// this value used to determine this weapon's importance in autoselection.
};

struct AmmoInfo
{
	int iId;
	const char* pszName;
	int MaxCarry;
};

extern int giAmmoIndex;

void AddAmmoNameToAmmoRegistry(const char* szAmmoname, int maxCarry);

/**
*	@brief Weapons that the player has in their inventory that they can use
*/
class CBaseWeapon : public CBaseItem
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	const char* GetWorldModelName() const { return STRING(m_iszWorldModelName); }

	void SetWorldModelName(const char* name)
	{
		m_iszWorldModelName = ALLOC_STRING(name);
	}

	void OnConstruct() override
	{
		CBaseItem::OnConstruct();
		m_FallMode = ItemFallMode::Fall;
		m_bCanPickUpWhileFalling = false;
	}

	void OnPostConstruct() override
	{
		CBaseItem::OnPostConstruct();

		//Initialize this to its weapon-specific default
		m_iDefaultPrimaryAmmo = m_iDefaultAmmo;
	}

	void SetObjectCollisionBox() override;

	ItemType GetType() const override final { return ItemType::Weapon; }

	void KeyValue(KeyValueData* pkvd) override;

	/**
	*	@brief return true if you want the weapon added to the player inventory
	*/
	virtual bool AddToPlayer(CBasePlayer* pPlayer);

	/**
	*	@brief CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
	*	@return true if you want your duplicate removed from world
	*/
	virtual bool AddDuplicate(CBaseWeapon* weapon);
	void EXPORT DestroyWeapon();
	ItemApplyResult Apply(CBasePlayer* pPlayer) override;

	/**
	*	@brief Sets up movetype, size, solidtype for a new weapon.
	*/
	void FallInit();

	/**
	*	@brief called by the new weapon with the existing weapon as parameter
	*	@details if we call ExtractAmmo(),
	*	it's because the player is picking up this type of weapon for the first time.
	*	If it is spawned by the world, m_iDefaultAmmo will have a default ammo amount in it.
	*	if this is a weapon dropped by a dying player, has 0 m_iDefaultAmmo,
	*	which means only the ammo in the weapon clip comes along.
	*	@return true if you can add ammo to yourself when picked up
	*/
	virtual bool ExtractAmmo(CBaseWeapon* pWeapon);

	/**
	*	@brief called by the new weapon's class with the existing weapon as parameter
	*	@return true if you can add ammo to yourself when picked up
	*/
	virtual int ExtractClipAmmo(CBaseWeapon* pWeapon);

	/**
	*	@brief Return true if you want to add yourself to the player
	*/
	virtual bool AddWeapon() { ExtractAmmo(this); return true; }

	// generic "shared" ammo handlers
	bool AddPrimaryAmmo(int iCount, const char* szName);
	bool AddSecondaryAmmo(int iCount, const char* szName);

	/**
	*	@brief returns false if struct not filled out
	*/
	virtual bool GetWeaponInfo(WeaponInfo& p) { return false; }

	/**
	*	@brief updates HUD state
	*/
	virtual void UpdateWeaponInfo() {}

	bool m_iPlayEmptySound = false;
	bool m_fFireOnEmpty = false;		//!< True when the gun is empty and the player is still holding down the attack key(s)
	virtual bool PlayEmptySound();
	virtual void ResetEmptySound();

	/**
	*	@brief Animate weapon model
	*/
	virtual void SendWeaponAnim(int iAnim, int body = 0);

	virtual bool CanDeploy();

	/**
	*	@brief returns is deploy was successful
	*/
	virtual bool Deploy()
	{
		return true;
	}

	/**
	*	@brief this function determines whether or not a weapon is useable by the player in its current state.
	*	(does it have ammo loaded? do I have any ammo for the weapon?, etc)
	*/
	virtual bool IsUseable();
	bool DefaultDeploy(const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, int body = 0);
	bool DefaultReload(int iClipSize, int iAnim, float fDelay, int body = 0);

	/**
	*	@brief can this weapon be put away right now?
	*/
	virtual bool CanHolster() { return true; }

	/**
	*	@brief Put away weapon
	*/
	virtual void Holster();

	/**
	*	@brief called each frame by the player PreThink
	*/
	virtual void WeaponPreFrame() {}

	/**
	*	@brief called each frame by the player PostThink
	*/
	virtual void WeaponPostFrame();
	// called by CBasePlayerWeapons WeaponPostFrame()
	virtual void PrimaryAttack() {}				//!< do "+ATTACK"
	virtual void SecondaryAttack() {}			//!< do "+ATTACK2"
	virtual void Reload() {}					//!< do "+RELOAD"
	virtual void WeaponIdle() {}				//!< called when no buttons pressed

	virtual void Drop();
	virtual void Kill();
	virtual void AttachToPlayer(CBasePlayer* pPlayer);

	/**
	*	@brief sends hud info to client dll, if things have changed
	*/
	virtual bool UpdateClientData(CBasePlayer* pPlayer);

	/**
	*	@brief no more ammo for this gun, put it away.
	*/
	virtual void RetireWeapon();
	virtual bool ShouldWeaponIdle() { return false; }

	virtual bool UseDecrement() { return false; }

	int	PrimaryAmmoIndex();
	int	SecondaryAmmoIndex();

	void PrintState();

	/**
	*	@brief An accurate way of calcualting the next attack time.
	*/
	float GetNextAttackDelay(float delay);

	virtual void GetWeaponData(weapon_data_t& data) {}
	virtual void SetWeaponData(const weapon_data_t& data) {}
	virtual void DecrementTimers() {}

protected:
	CBaseWeapon* GetItemToRespawn(const Vector& respawnPoint) override;

public:
	static inline WeaponInfo WeaponInfoArray[MAX_WEAPONS]{};
	static inline AmmoInfo AmmoInfoArray[MAX_AMMO_TYPES]{};

	EHandle<CBasePlayer> m_hPlayer;
	EHandle<CBaseWeapon> m_hNext;
	int m_iId = WEAPON_NONE; // WEAPON_???

	/**
	*	@brief return 0 to MAX_WEAPON_SLOTS, used in hud
	*/
	virtual int WeaponSlot() { return 0; }

	int			WeaponPosition() { return WeaponInfoArray[m_iId].iPosition; }
	const char* Ammo1Name() { return WeaponInfoArray[m_iId].pszAmmo1; }
	int			MaxAmmo1() { return WeaponInfoArray[m_iId].iMaxAmmo1; }
	const char* Ammo2Name() { return WeaponInfoArray[m_iId].pszAmmo2; }
	int			MaxAmmo2() { return WeaponInfoArray[m_iId].iMaxAmmo2; }
	const char* pszName() { return WeaponInfoArray[m_iId].pszName; }
	int			MaxClip() { return WeaponInfoArray[m_iId].iMaxClip; }
	int			Weight() { return WeaponInfoArray[m_iId].iWeight; }
	int			Flags() { return WeaponInfoArray[m_iId].iFlags; }

	float m_flPumpTime = 0;
	float m_flNextPrimaryAttack = 0;							//!< soonest time WeaponPostFrame will call PrimaryAttack
	float m_flNextSecondaryAttack = 0;							//!< soonest time WeaponPostFrame will call SecondaryAttack
	float m_flTimeWeaponIdle = 0;								//!< soonest time WeaponPostFrame will call WeaponIdle
	int m_iPrimaryAmmoType = 0;									//!< "primary" ammo index into players m_rgAmmo[]
	int m_iSecondaryAmmoType = 0;								//!< "secondary" ammo index into players m_rgAmmo[]
	int m_iClip = 0;											//!< number of shots left in the primary weapon clip, -1 it not used
	int m_iClientClip = 0;										//!< the last version of m_iClip sent to hud dll
	WeaponState m_iClientWeaponState = WeaponState::NotActive;	//!< the last version of the weapon state sent to hud dll (is current weapon, is on target)
	bool m_fInReload = false;									//!< Are we in the middle of a reload;

	int m_iDefaultPrimaryAmmo = 0; //!< how much ammo you get when you pick up this weapon as placed by a level designer.
	int m_iDefaultAmmo = 0; //!< Amount of ammo left in weapon

	// hle time creep vars
	float m_flPrevPrimaryAttack = 0;
	float m_flLastFireTime = 0;

private:
	string_t m_iszWorldModelName = iStringNull;
};

class CBaseAmmo : public CBaseItem
{
public:
	void OnConstruct() override
	{
		CBaseItem::OnConstruct();
		m_FallMode = ItemFallMode::Fall;
		m_bCanPickUpWhileFalling = true;
	}

	ItemType GetType() const override final { return ItemType::Ammo; }

	void KeyValue(KeyValueData* pkvd) override;

	void Precache() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

protected:
	ItemApplyResult DefaultGiveAmmo(CBasePlayer* player, int amount, const char* ammoName);

	ItemApplyResult Apply(CBasePlayer* player) override;

protected:
	int m_iAmount = 0;
	string_t m_iszAmmoName = iStringNull;

	string_t m_iszPickupSound = MAKE_STRING("items/9mmclip1.wav");
};

/**
*	@brief Generic ammo item
*/
class CAmmoGeneric : public CBaseAmmo
{
public:
	void KeyValue(KeyValueData* pkvd) override;
};

extern DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
extern DLL_GLOBAL	const char* g_pModelNameLaser;

extern DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
extern DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
extern DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
extern DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for blood drops
extern DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for blood spray (bigger)

/**
*	@brief resets the global multi damage accumulator
*/
void ClearMultiDamage();

/**
*	@brief inflicts contents of global multi damage register on gMultiDamage.pEntity
*/
void ApplyMultiDamage(CBaseEntity* pInflictor, CBaseEntity* pAttacker);

/**
*	@brief Collects multiple small damages into a single damage
*/
void AddMultiDamage(CBaseEntity* pInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType);

void DecalGunshot(TraceResult* pTrace, int iBulletType);
void SpawnBlood(const Vector& vecSpot, int bloodColor, float flDamage);

/**
*	@brief this entity is exploding, or otherwise needs to inflict damage upon entities within a certain range.
*	@details only damages ents that can clearly be seen by the explosion!
*/
void RadiusDamage(Vector vecSrc, CBaseEntity* pInflictor, CBaseEntity* pAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType);

struct MULTIDAMAGE
{
	CBaseEntity* pEntity;
	float			amount;
	int				type;
};

extern MULTIDAMAGE gMultiDamage;

constexpr int LOUD_GUN_VOLUME = 1000;
constexpr int NORMAL_GUN_VOLUME = 600;
constexpr int QUIET_GUN_VOLUME = 200;

constexpr int BRIGHT_GUN_FLASH = 512;
constexpr int NORMAL_GUN_FLASH = 256;
constexpr int DIM_GUN_FLASH = 128;

constexpr int BIG_EXPLOSION_VOLUME = 2048;
constexpr int NORMAL_EXPLOSION_VOLUME = 1024;
constexpr int SMALL_EXPLOSION_VOLUME = 512;

constexpr int WEAPON_ACTIVITY_VOLUME = 64;

constexpr Vector VECTOR_CONE_1DEGREES(0.00873, 0.00873, 0.00873);
constexpr Vector VECTOR_CONE_2DEGREES(0.01745, 0.01745, 0.01745);
constexpr Vector VECTOR_CONE_3DEGREES(0.02618, 0.02618, 0.02618);
constexpr Vector VECTOR_CONE_4DEGREES(0.03490, 0.03490, 0.03490);
constexpr Vector VECTOR_CONE_5DEGREES(0.04362, 0.04362, 0.04362);
constexpr Vector VECTOR_CONE_6DEGREES(0.05234, 0.05234, 0.05234);
constexpr Vector VECTOR_CONE_7DEGREES(0.06105, 0.06105, 0.06105);
constexpr Vector VECTOR_CONE_8DEGREES(0.06976, 0.06976, 0.06976);
constexpr Vector VECTOR_CONE_9DEGREES(0.07846, 0.07846, 0.07846);
constexpr Vector VECTOR_CONE_10DEGREES(0.08716, 0.08716, 0.08716);
constexpr Vector VECTOR_CONE_15DEGREES(0.13053, 0.13053, 0.13053);
constexpr Vector VECTOR_CONE_20DEGREES(0.17365, 0.17365, 0.17365);

/**
*	@brief a single entity that can store weapons and ammo.
*/
class CWeaponBox : public CBaseEntity
{
	void Precache() override;
	void Spawn() override;

	/**
	*	@brief Touch: try to add my contents to the toucher if the toucher is a player.
	*/
	void Touch(CBaseEntity* pOther) override;
	void KeyValue(KeyValueData* pkvd) override;

	/**
	*	@brief is there anything in this box?
	*/
	bool IsEmpty();
	int  GiveAmmo(int iCount, const char* szName, int iMax, int* pIndex = nullptr);
	void SetObjectCollisionBox() override;

public:
	/**
	*	@brief the think function that removes the box from the world.
	*/
	void EXPORT Kill();
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	/**
	*	@brief is a weapon of this type already packed in this box?
	*/
	bool HasWeapon(CBaseWeapon* pCheckWeapon);

	/**
	*	@brief PackWeapon: Add this weapon to the box
	*/
	bool PackWeapon(CBaseWeapon* pWeapon);

	bool PackAmmo(string_t iszName, int iCount);

	EHandle<CBaseWeapon> m_hPlayerWeapons[MAX_WEAPON_TYPES];// one slot for each 

	string_t m_rgiszAmmo[MAX_AMMO_TYPES]{};// ammo names
	int	m_rgAmmo[MAX_AMMO_TYPES]{};// ammo quantities

	int m_cAmmoTypes = 0;// how many ammo types packed into this box (if packed by a level designer)
};

/**
*	@brief Returns if it's multiplayer.
*	Mostly used by the client side weapons.
*/
bool bIsMultiplayer();

#ifdef CLIENT_DLL
/**
*	@brief Just loads a v_ model.
*/
void LoadVModel(const char* szViewModel, CBasePlayer* m_pPlayer);
#endif

enum glock_e
{
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER
};

class CGlock : public CBaseWeapon
{
public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_GLOCK;
		m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;
		SetWorldModelName("models/w_9mmhandgun.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 2; }
	bool GetWeaponInfo(WeaponInfo& p) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void GlockFire(float flSpread, float flCycleTime, bool fUseAutoAim);
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	int m_iShell = 0;

	unsigned short m_usFireGlock1 = 0;
	unsigned short m_usFireGlock2 = 0;
};

enum crowbar_e
{
	CROWBAR_IDLE = 0,
	CROWBAR_DRAW,
	CROWBAR_HOLSTER,
	CROWBAR_ATTACK1HIT,
	CROWBAR_ATTACK1MISS,
	CROWBAR_ATTACK2MISS,
	CROWBAR_ATTACK2HIT,
	CROWBAR_ATTACK3MISS,
	CROWBAR_ATTACK3HIT
};

class CCrowbar : public CBaseWeapon
{
public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_CROWBAR;
		m_iClip = -1;
		SetWorldModelName("models/w_crowbar.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 1; }
	void EXPORT SwingAgain();
	void EXPORT Smack();
	bool GetWeaponInfo(WeaponInfo& p) override;

	void PrimaryAttack() override;
	bool Swing(bool fFirst);
	bool Deploy() override;
	void Holster() override;
	int m_iSwing = 0;
	TraceResult m_trHit;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}
private:
	unsigned short m_usCrowbar = 0;
};

enum python_e
{
	PYTHON_IDLE1 = 0,
	PYTHON_FIDGET,
	PYTHON_FIRE1,
	PYTHON_RELOAD,
	PYTHON_HOLSTER,
	PYTHON_DRAW,
	PYTHON_IDLE2,
	PYTHON_IDLE3
};

class CPython : public CBaseWeapon
{
public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_PYTHON;
		m_iDefaultAmmo = PYTHON_DEFAULT_GIVE;
		SetWorldModelName("models/w_357.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 2; }
	bool GetWeaponInfo(WeaponInfo& p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Holster() override;
	void Reload() override;
	void WeaponIdle() override;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usFirePython = 0;
};

enum mp5_e
{
	MP5_LONGIDLE = 0,
	MP5_IDLE1,
	MP5_LAUNCH,
	MP5_RELOAD,
	MP5_DEPLOY,
	MP5_FIRE1,
	MP5_FIRE2,
	MP5_FIRE3,
};

class CMP5 : public CBaseWeapon
{
public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_MP5;
		m_iDefaultAmmo = MP5_DEFAULT_GIVE;
		SetWorldModelName("models/w_9mmAR.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 3; }
	bool GetWeaponInfo(WeaponInfo& p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	float m_flNextAnimTime = 0;
	int m_iShell = 0;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usMP5 = 0;
	unsigned short m_usMP52 = 0;
};

enum crossbow_e
{
	CROSSBOW_IDLE1 = 0,	// full
	CROSSBOW_IDLE2,		// empty
	CROSSBOW_FIDGET1,	// full
	CROSSBOW_FIDGET2,	// empty
	CROSSBOW_FIRE1,		// full
	CROSSBOW_FIRE2,		// reload
	CROSSBOW_FIRE3,		// empty
	CROSSBOW_RELOAD,	// from empty
	CROSSBOW_DRAW1,		// full
	CROSSBOW_DRAW2,		// empty
	CROSSBOW_HOLSTER1,	// full
	CROSSBOW_HOLSTER2,	// empty
};

class CCrossbow : public CBaseWeapon
{
public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_CROSSBOW;
		m_iDefaultAmmo = CROSSBOW_DEFAULT_GIVE;
		SetWorldModelName("models/w_crossbow.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 3; }
	bool GetWeaponInfo(WeaponInfo& p) override;

	void FireBolt();
	void FireSniperBolt();
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;
	bool Deploy() override;
	void Holster() override;
	void Reload() override;
	void WeaponIdle() override;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usCrossbow = 0;
	unsigned short m_usCrossbow2 = 0;
};

enum shotgun_e
{
	SHOTGUN_IDLE = 0,
	SHOTGUN_FIRE,
	SHOTGUN_FIRE2,
	SHOTGUN_RELOAD,
	SHOTGUN_PUMP,
	SHOTGUN_START_RELOAD,
	SHOTGUN_DRAW,
	SHOTGUN_HOLSTER,
	SHOTGUN_IDLE4,
	SHOTGUN_IDLE_DEEP
};

class CShotgun : public CBaseWeapon
{
public:
	enum class ReloadState
	{
		NotReloading = 0,
		PlayAnimation = 1,	//!< Play the shell load animation
		AddToClip = 2		//!< Update the clip value (and Hud as a result)
	};

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_SHOTGUN;
		m_iDefaultAmmo = SHOTGUN_DEFAULT_GIVE;
		SetWorldModelName("models/w_shotgun.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 3; }
	bool GetWeaponInfo(WeaponInfo& p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	void WeaponPostFrame() override;
	float m_flNextReload = 0;
	int m_iShell = 0;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

	void GetWeaponData(weapon_data_t& data) override;
	void SetWeaponData(const weapon_data_t& data) override;

	ReloadState m_fInSpecialReload = ReloadState::NotReloading; //!< Are we in the middle of a reload

private:
	unsigned short m_usDoubleFire = 0;
	unsigned short m_usSingleFire = 0;
};

class CLaserSpot : public CBaseEntity
{
	void Spawn() override;
	void Precache() override;

	int	ObjectCaps() override { return FCAP_DONT_SAVE; }

public:
	/**
	*	@brief make the laser sight invisible.
	*/
	void Suspend(float flSuspendTime);

	/**
	*	@brief bring a suspended laser sight back.
	*/
	void EXPORT Revive();

	static CLaserSpot* CreateSpot();
};

enum rpg_e
{
	RPG_IDLE = 0,
	RPG_FIDGET,
	RPG_RELOAD,		// to reload
	RPG_FIRE2,		// to empty
	RPG_HOLSTER1,	// loaded
	RPG_DRAW1,		// loaded
	RPG_HOLSTER2,	// unloaded
	RPG_DRAW_UL,	// unloaded
	RPG_IDLE_UL,	// unloaded idle
	RPG_FIDGET_UL,	// unloaded fidget
};

class CRpg : public CBaseWeapon
{
public:

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_RPG;

		if (bIsMultiplayer())
		{
			// more default ammo in multiplay. 
			m_iDefaultAmmo = RPG_DEFAULT_GIVE * 2;
		}
		else
		{
			m_iDefaultAmmo = RPG_DEFAULT_GIVE;
		}

		SetWorldModelName("models/w_rpg.mdl");
	}

	void OnRemove() override;
	void Spawn() override;
	void Precache() override;
	void Reload() override;
	int WeaponSlot() override { return 4; }
	bool GetWeaponInfo(WeaponInfo& p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;

	bool Deploy() override;
	bool CanHolster() override;
	void Holster() override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;

	void UpdateSpot();
	bool ShouldWeaponIdle() override { return true; }

	EHandle<CLaserSpot> m_hSpot;
	bool m_fSpotActive = false;
	int m_cActiveRockets = 0;// how many missiles in flight from this launcher right now?

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usRpg = 0;
};

class CRpgRocket : public CGrenade
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
	void Spawn() override;
	void Precache() override;
	void EXPORT FollowThink();
	void EXPORT IgniteThink();
	void EXPORT RocketTouch(CBaseEntity* pOther);
	static CRpgRocket* CreateRpgRocket(const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner, CRpg* pLauncher);

	int m_iTrail = 0;
	float m_flIgniteTime = 0;
	EHandle<CRpg> m_hLauncher;// handle back to the launcher that fired me. 
};

constexpr int GAUSS_PRIMARY_CHARGE_VOLUME = 256;	// how loud gauss is while charging
constexpr int GAUSS_PRIMARY_FIRE_VOLUME = 450;		// how loud gauss is when discharged

enum gauss_e
{
	GAUSS_IDLE = 0,
	GAUSS_IDLE2,
	GAUSS_FIDGET,
	GAUSS_SPINUP,
	GAUSS_SPIN,
	GAUSS_FIRE,
	GAUSS_FIRE2,
	GAUSS_HOLSTER,
	GAUSS_DRAW
};

class CGauss : public CBaseWeapon
{
public:
	enum class AttackState
	{
		NotAttacking = 0,
		SpinUp,
		Charging,
		Aftershock,
	};

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_GAUSS;
		m_iDefaultAmmo = GAUSS_DEFAULT_GIVE;
		SetWorldModelName("models/w_gauss.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 4; }
	bool GetWeaponInfo(WeaponInfo& p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;

	bool Deploy() override;
	void Holster() override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;

	/**
	*	@details since all of this code has to run and then call Fire(),
	*	it was easier at this point to rip it out of weaponidle() and make its own function then to try to merge this into Fire(),
	*	which has some identical variable names
	*/
	void StartFire();
	void Fire(const Vector& vecOrigSrc, Vector vecDirShooting, float flDamage);
	float GetFullChargeTime();
	int m_iBalls = 0;
	int m_iGlow = 0;
	int m_iBeam = 0;
	int m_iSoundState = 0; // don't save this

	// was this weapon just fired primary or secondary?
	// we need to know so we can pick the right set of effects. 
	bool m_fPrimaryFire = false;
	AttackState m_fInAttack = AttackState::NotAttacking;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

	void GetWeaponData(weapon_data_t& data) override;
	void SetWeaponData(const weapon_data_t& data) override;
	void DecrementTimers() override;

	float m_flStartCharge = 0;
	float m_flAmmoStartCharge = 0;
	float m_flPlayAftershock = 0;
	float m_flNextAmmoBurn = 0;//!< while charging, when to absorb another unit of player's ammo?

private:
	unsigned short m_usGaussFire = 0;
	unsigned short m_usGaussSpin = 0;
};

enum egon_e
{
	EGON_IDLE1 = 0,
	EGON_FIDGET1,
	EGON_ALTFIREON,
	EGON_ALTFIRECYCLE,
	EGON_ALTFIREOFF,
	EGON_FIRE1,
	EGON_FIRE2,
	EGON_FIRE3,
	EGON_FIRE4,
	EGON_DRAW,
	EGON_HOLSTER
};

constexpr int EGON_PRIMARY_VOLUME = 450;
constexpr std::string_view EGON_BEAM_SPRITE{"sprites/xbeam1.spr"};
constexpr std::string_view EGON_FLARE_SPRITE{"sprites/XSpark1.spr"};
constexpr std::string_view EGON_SOUND_OFF{"weapons/egon_off1.wav"};
constexpr std::string_view EGON_SOUND_RUN{"weapons/egon_run3.wav"};
constexpr std::string_view EGON_SOUND_STARTUP{"weapons/egon_windup2.wav"};

class CEgon : public CBaseWeapon
{
public:
	enum class FireState
	{
		Off,
		Charge
	};

	enum class FireMode
	{
		Narrow,
		Wide
	};

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_EGON;
		m_iDefaultAmmo = EGON_DEFAULT_GIVE;
		SetWorldModelName("models/w_egon.mdl");
	}

	void OnRemove() override;
	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 4; }
	bool GetWeaponInfo(WeaponInfo& p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;

	bool Deploy() override;
	void Holster() override;

	void UpdateEffect(const Vector& startPoint, const Vector& endPoint, float timeBlend);

	void CreateEffect();
	void DestroyEffect();

	void EndAttack();
	void Attack();
	void PrimaryAttack() override;
	bool ShouldWeaponIdle() override { return true; }
	void WeaponIdle() override;

	float m_flAmmoUseTime = 0;// since we use < 1 point of ammo per update, we subtract ammo on a timer.

	float GetPulseInterval();
	float GetDischargeInterval();

	void Fire(const Vector& vecOrigSrc, const Vector& vecDir);

	bool HasAmmo();

	void UseAmmo(int count);

	EHandle<CBeam> m_hBeam;
	EHandle<CBeam> m_hNoise;
	EHandle<CSprite> m_hSprite;
	FireState m_fireState = FireState::Off;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

	void GetWeaponData(weapon_data_t& data) override;
	void SetWeaponData(const weapon_data_t& data) override;

	unsigned short m_usEgonStop = 0;

private:
	float m_shootTime = 0;
	FireMode m_fireMode = FireMode::Narrow;
	float m_shakeTime = 0;
	bool m_deployed = 0;

	unsigned short m_usEgonFire = 0;
};

enum hgun_e
{
	HGUN_IDLE1 = 0,
	HGUN_FIDGETSWAY,
	HGUN_FIDGETSHAKE,
	HGUN_DOWN,
	HGUN_UP,
	HGUN_SHOOT
};

class CHgun : public CBaseWeapon
{
public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_HORNETGUN;
		m_iDefaultAmmo = HIVEHAND_DEFAULT_GIVE;
		SetWorldModelName("models/w_hgun.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 4; }
	bool GetWeaponInfo(WeaponInfo& p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	bool IsUseable() override;
	void Holster() override;
	void Reload() override;
	void WeaponIdle() override;
	float m_flNextAnimTime = 0;

	float m_flRechargeTime = 0;

	int m_iFirePhase = 0;// don't save me.

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}
private:
	unsigned short m_usHornetFire = 0;
};

enum handgrenade_e
{
	HANDGRENADE_IDLE = 0,
	HANDGRENADE_FIDGET,
	HANDGRENADE_PINPULL,
	HANDGRENADE_THROW1,	// toss
	HANDGRENADE_THROW2,	// medium
	HANDGRENADE_THROW3,	// hard
	HANDGRENADE_HOLSTER,
	HANDGRENADE_DRAW
};

class CHandGrenade : public CBaseWeapon
{
public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_HANDGRENADE;
		m_iDefaultAmmo = HANDGRENADE_DEFAULT_GIVE;
		SetWorldModelName("models/w_grenade.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 5; }
	bool GetWeaponInfo(WeaponInfo& p) override;

	void PrimaryAttack() override;
	bool Deploy() override;
	bool CanHolster() override;
	void Holster() override;
	void WeaponIdle() override;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

	void GetWeaponData(weapon_data_t& data) override;
	void SetWeaponData(const weapon_data_t& data) override;

	float m_flStartThrow = 0;
	float m_flReleaseThrow = 0;
};

enum satchel_e
{
	SATCHEL_IDLE1 = 0,
	SATCHEL_FIDGET1,
	SATCHEL_DRAW,
	SATCHEL_DROP
};

enum satchel_radio_e
{
	SATCHEL_RADIO_IDLE1 = 0,
	SATCHEL_RADIO_FIDGET1,
	SATCHEL_RADIO_DRAW,
	SATCHEL_RADIO_FIRE,
	SATCHEL_RADIO_HOLSTER
};

class CSatchel : public CBaseWeapon
{
public:
	enum class ChargeState
	{
		NoSatchelsDeployed = 0,
		SatchelsDeployed,
		Reloading,
	};

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_SATCHEL;
		m_iDefaultAmmo = SATCHEL_DEFAULT_GIVE;
		SetWorldModelName("models/w_satchel.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 5; }
	bool GetWeaponInfo(WeaponInfo& p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool AddDuplicate(CBaseWeapon* pOriginal) override;
	bool CanDeploy() override;
	bool Deploy() override;
	bool IsUseable() override;

	void Holster() override;
	void WeaponIdle() override;
	void Throw();

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

	void GetWeaponData(weapon_data_t& data) override;
	void SetWeaponData(const weapon_data_t& data) override;

	ChargeState m_chargeReady = ChargeState::NoSatchelsDeployed;
};

enum tripmine_e
{
	TRIPMINE_IDLE1 = 0,
	TRIPMINE_IDLE2,
	TRIPMINE_ARM1,
	TRIPMINE_ARM2,
	TRIPMINE_FIDGET,
	TRIPMINE_HOLSTER,
	TRIPMINE_DRAW,
	TRIPMINE_WORLD,
	TRIPMINE_GROUND,
};

class CTripmine : public CBaseWeapon
{
public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_TRIPMINE;
		m_iDefaultAmmo = TRIPMINE_DEFAULT_GIVE;
		SetWorldModelName("models/v_tripmine.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 5; }
	bool GetWeaponInfo(WeaponInfo& p) override;
	void SetObjectCollisionBox() override
	{
		//!!!BUGBUG - fix the model!
		pev->absmin = GetAbsOrigin() + Vector(-16, -16, -5);
		pev->absmax = GetAbsOrigin() + Vector(16, 16, 28);
	}

	void PrimaryAttack() override;
	bool Deploy() override;
	void Holster() override;
	void WeaponIdle() override;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usTripFire = 0;

};

enum squeak_e
{
	SQUEAK_IDLE1 = 0,
	SQUEAK_FIDGETFIT,
	SQUEAK_FIDGETNIP,
	SQUEAK_DOWN,
	SQUEAK_UP,
	SQUEAK_THROW
};

class CSqueak : public CBaseWeapon
{
public:
	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_SNARK;
		m_iDefaultAmmo = SNARK_DEFAULT_GIVE;
		SetWorldModelName("models/w_sqknest.mdl");
	}

	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 5; }
	bool GetWeaponInfo(WeaponInfo& p) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Holster() override;
	void WeaponIdle() override;
	bool m_fJustThrown = false;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usSnarkFire = 0;
};