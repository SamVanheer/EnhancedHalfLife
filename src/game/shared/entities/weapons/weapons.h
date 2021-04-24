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

class CBasePlayer;
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

	static CGrenade* ShootTimed(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, float time);
	static CGrenade* ShootContact(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);

	void Explode(Vector vecSrc, Vector vecAim);
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
	void EXPORT DetonateUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT TumbleThink();

	virtual void BounceSound();
	int	BloodColor() override { return DONT_BLEED; }
	void Killed(entvars_t* pevAttacker, int iGib) override;

	bool m_fRegisteredSound;//!< whether or not this grenade has issued its DANGER sound to the world sound list yet.
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

constexpr int ITEM_FLAG_SELECTONEMPTY = 1;
constexpr int ITEM_FLAG_NOAUTORELOAD = 2;
constexpr int ITEM_FLAG_NOAUTOSWITCHEMPTY = 4;
constexpr int ITEM_FLAG_LIMITINWORLD = 8;
constexpr int ITEM_FLAG_EXHAUSTIBLE = 16; // A player can totally exhaust their ammo supply and lose this weapon

enum class WeaponState
{
	NotActive = 0,
	Active,
	OnTarget
};

struct ItemInfo
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
	const char* pszName;
	int iId;
};

extern int giAmmoIndex;

void AddAmmoNameToAmmoRegistry(const char* szAmmoname);

/**
*	@brief Items that the player has in their inventory that they can use
*/
class CBasePlayerItem : public CBaseAnimating
{
public:
	void SetObjectCollisionBox() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	/**
	*	@brief return true if the item you want the item added to the player inventory
	*/
	virtual bool AddToPlayer(CBasePlayer* pPlayer);

	/**
	*	@brief CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
	*	@return true if you want your duplicate removed from world
	*/
	virtual bool AddDuplicate(CBasePlayerItem* pItem) { return false; }
	void EXPORT DestroyItem();
	void EXPORT DefaultTouch(CBaseEntity* pOther);

	/**
	*	@brief Items that have just spawned run this think to catch them when they hit the ground.
	*	Once we're sure that the object is grounded,
	*	we change its solid type to trigger and set it in a large box that helps the player get it.
	*/
	void EXPORT FallThink();

	/**
	*	@brief make a CBasePlayerItem visible and tangible
	*/
	void EXPORT Materialize();

	/**
	*	@brief the item is trying to rematerialize, should it do so now or wait longer?
	*	the weapon desires to become visible and tangible, if the game rules allow for it
	*/
	void EXPORT AttemptToMaterialize();

	/**
	*	@brief this item is already in the world, but it is invisible and intangible. Make it visible and tangible.
	*	Copies a weapon
	*/
	CBaseEntity* Respawn() override;

	/**
	*	@brief Sets up movetype, size, solidtype for a new weapon.
	*/
	void FallInit();

	/**
	*	@brief a player is taking this weapon, should it respawn?
	*/
	void CheckRespawn();

	/**
	*	@brief returns false if struct not filled out
	*/
	virtual bool GetItemInfo(ItemInfo* p) { return false; }
	virtual bool CanDeploy() { return true; }

	/**
	*	@brief returns is deploy was successful
	*/
	virtual bool Deploy()
	{
		return true;
	}

	/**
	*	@brief can this weapon be put away right now?
	*/
	virtual bool CanHolster() { return true; }

	/**
	*	@brief Put away weapon
	*/
	virtual void Holster();
	virtual void UpdateItemInfo() {}

	/**
	*	@brief called each frame by the player PreThink
	*/
	virtual void ItemPreFrame() {}

	/**
	*	@brief called each frame by the player PostThink
	*/
	virtual void ItemPostFrame() {}

	virtual void Drop();
	virtual void Kill();
	virtual void AttachToPlayer(CBasePlayer* pPlayer);

	virtual int PrimaryAmmoIndex() { return -1; }
	virtual int SecondaryAmmoIndex() { return -1; }

	virtual bool UpdateClientData(CBasePlayer* pPlayer) { return false; }

	virtual CBasePlayerItem* GetWeaponPtr() { return nullptr; }

	virtual void GetWeaponData(weapon_data_t& data) {}
	virtual void SetWeaponData(const weapon_data_t& data) {}
	virtual void DecrementTimers() {}

	static ItemInfo ItemInfoArray[MAX_WEAPONS];
	static AmmoInfo AmmoInfoArray[MAX_AMMO_TYPES];

	CBasePlayer* m_pPlayer;
	CBasePlayerItem* m_pNext;
	int		m_iId;												// WEAPON_???

	/**
	*	@brief return 0 to MAX_ITEMS_SLOTS, used in hud
	*/
	virtual int ItemSlot() { return 0; }

	int			ItemPosition() { return ItemInfoArray[m_iId].iPosition; }
	const char* Ammo1Name() { return ItemInfoArray[m_iId].pszAmmo1; }
	int			MaxAmmo1() { return ItemInfoArray[m_iId].iMaxAmmo1; }
	const char* Ammo2Name() { return ItemInfoArray[m_iId].pszAmmo2; }
	int			MaxAmmo2() { return ItemInfoArray[m_iId].iMaxAmmo2; }
	const char* pszName() { return ItemInfoArray[m_iId].pszName; }
	int			MaxClip() { return ItemInfoArray[m_iId].iMaxClip; }
	int			Weight() { return ItemInfoArray[m_iId].iWeight; }
	int			Flags() { return ItemInfoArray[m_iId].iFlags; }
};

/**
*	@brief inventory items that
*/
class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	// generic weapon versions of CBasePlayerItem calls
	bool AddToPlayer(CBasePlayer* pPlayer) override;
	bool AddDuplicate(CBasePlayerItem* pItem) override;

	/**
	*	@brief called by the new item with the existing item as parameter
	*	@details if we call ExtractAmmo(),
	*	it's because the player is picking up this type of weapon for the first time.
	*	If it is spawned by the world, m_iDefaultAmmo will have a default ammo amount in it.
	*	if this is a weapon dropped by a dying player, has 0 m_iDefaultAmmo,
	*	which means only the ammo in the weapon clip comes along. 
	*	@return true if you can add ammo to yourself when picked up
	*/
	virtual bool ExtractAmmo(CBasePlayerWeapon* pWeapon);

	/**
	*	@brief called by the new item's class with the existing item as parameter
	*	@return true if you can add ammo to yourself when picked up
	*/
	virtual int ExtractClipAmmo(CBasePlayerWeapon* pWeapon);

	/**
	*	@brief Return true if you want to add yourself to the player
	*/
	virtual bool AddWeapon() { ExtractAmmo(this); return true; }

	// generic "shared" ammo handlers
	bool AddPrimaryAmmo(int iCount, const char* szName, int iMaxClip, int iMaxCarry);
	bool AddSecondaryAmmo(int iCount, const char* szName, int iMaxCarry);

	void UpdateItemInfo() override {}	// updates HUD state

	bool m_iPlayEmptySound;
	bool m_fFireOnEmpty;		//!< True when the gun is empty and the player is still holding down the attack key(s)
	virtual bool PlayEmptySound();
	virtual void ResetEmptySound();

	/**
	*	@brief Animate weapon model
	*/
	virtual void SendWeaponAnim(int iAnim, int body = 0);

	bool CanDeploy() override;

	/**
	*	@brief this function determines whether or not a weapon is useable by the player in its current state. 
	*	(does it have ammo loaded? do I have any ammo for the weapon?, etc)
	*/
	virtual bool IsUseable();
	bool DefaultDeploy(const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, int body = 0);
	bool DefaultReload(int iClipSize, int iAnim, float fDelay, int body = 0);

	/**
	*	@brief called each frame by the player PostThink
	*/
	void ItemPostFrame() override;
	// called by CBasePlayerWeapons ItemPostFrame()
	virtual void PrimaryAttack() {}				//!< do "+ATTACK"
	virtual void SecondaryAttack() {}			//!< do "+ATTACK2"
	virtual void Reload() {}					//!< do "+RELOAD"
	virtual void WeaponIdle() {}				//!< called when no buttons pressed

	/**
	*	@brief sends hud info to client dll, if things have changed
	*/
	bool UpdateClientData(CBasePlayer* pPlayer) override;

	/**
	*	@brief no more ammo for this gun, put it away.
	*/
	virtual void RetireWeapon();
	virtual bool ShouldWeaponIdle() { return false; }
	void Holster() override;
	virtual bool UseDecrement() { return false; }

	int	PrimaryAmmoIndex() override;
	int	SecondaryAmmoIndex() override;

	void PrintState();

	CBasePlayerItem* GetWeaponPtr() override { return (CBasePlayerItem*)this; }

	/**
	*	@brief An accurate way of calcualting the next attack time.
	*/
	float GetNextAttackDelay(float delay);

	float m_flPumpTime;
	int		m_fInSpecialReload;									//!< Are we in the middle of a reload for the shotguns
	float	m_flNextPrimaryAttack;								//!< soonest time ItemPostFrame will call PrimaryAttack
	float	m_flNextSecondaryAttack;							//!< soonest time ItemPostFrame will call SecondaryAttack
	float	m_flTimeWeaponIdle;									//!< soonest time ItemPostFrame will call WeaponIdle
	int		m_iPrimaryAmmoType;									//!< "primary" ammo index into players m_rgAmmo[]
	int		m_iSecondaryAmmoType;								//!< "secondary" ammo index into players m_rgAmmo[]
	int		m_iClip;											//!< number of shots left in the primary weapon clip, -1 it not used
	int		m_iClientClip;										//!< the last version of m_iClip sent to hud dll
	WeaponState m_iClientWeaponState;							//!< the last version of the weapon state sent to hud dll (is current weapon, is on target)
	bool	m_fInReload;										//!< Are we in the middle of a reload;

	int		m_iDefaultAmmo;//!< how much ammo you get when you pick up this weapon as placed by a level designer.

	// hle time creep vars
	float	m_flPrevPrimaryAttack;
	float	m_flLastFireTime;
};

class CBasePlayerAmmo : public CBaseEntity
{
public:
	void Spawn() override;
	void EXPORT DefaultTouch(CBaseEntity* pOther); // default weapon touch
	virtual bool AddAmmo(CBasePlayer* pOther) { return true; }

	CBaseEntity* Respawn() override;
	void EXPORT Materialize();
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
void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker);

/**
*	@brief Collects multiple small damages into a single damage
*/
void AddMultiDamage(entvars_t* pevInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType);

void DecalGunshot(TraceResult* pTrace, int iBulletType);
void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
int DamageDecal(CBaseEntity* pEntity, int bitsDamageType);

/**
*	@brief this entity is exploding, or otherwise needs to inflict damage upon entities within a certain range.
*	@details only damages ents that can clearly be seen by the explosion!
*/
void RadiusDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType);

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
	bool HasWeapon(CBasePlayerItem* pCheckItem);

	/**
	*	@brief PackWeapon: Add this weapon to the box
	*/
	bool PackWeapon(CBasePlayerItem* pWeapon);

	bool PackAmmo(string_t iszName, int iCount);

	CBasePlayerItem* m_rgpPlayerItems[MAX_ITEM_TYPES];// one slot for each 

	string_t m_rgiszAmmo[MAX_AMMO_TYPES];// ammo names
	int	m_rgAmmo[MAX_AMMO_TYPES];// ammo quantities

	int m_cAmmoTypes;// how many ammo types packed into this box (if packed by a level designer)
};

#ifdef CLIENT_DLL
/**
*	@brief Returns if it's multiplayer.
*	Mostly used by the client side weapons.
*/
bool bIsMultiplayer();

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

class CGlock : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 2; }
	bool GetItemInfo(ItemInfo* p) override;

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
	int m_iShell;

	unsigned short m_usFireGlock1;
	unsigned short m_usFireGlock2;
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

class CCrowbar : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 1; }
	void EXPORT SwingAgain();
	void EXPORT Smack();
	bool GetItemInfo(ItemInfo* p) override;

	void PrimaryAttack() override;
	bool Swing(bool fFirst);
	bool Deploy() override;
	void Holster() override;
	int m_iSwing;
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
	unsigned short m_usCrowbar;
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

class CPython : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 2; }
	bool GetItemInfo(ItemInfo* p) override;
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
	unsigned short m_usFirePython;
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

class CMP5 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 3; }
	bool GetItemInfo(ItemInfo* p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	float m_flNextAnimTime;
	int m_iShell;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usMP5;
	unsigned short m_usMP52;
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

class CCrossbow : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 3; }
	bool GetItemInfo(ItemInfo* p) override;

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
	unsigned short m_usCrossbow;
	unsigned short m_usCrossbow2;
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

class CShotgun : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif


	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 3; }
	bool GetItemInfo(ItemInfo* p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	void ItemPostFrame() override;
	float m_flNextReload;
	int m_iShell;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usDoubleFire;
	unsigned short m_usSingleFire;
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

class CRpg : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn() override;
	void Precache() override;
	void Reload() override;
	int ItemSlot() override { return 4; }
	bool GetItemInfo(ItemInfo* p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;

	bool Deploy() override;
	bool CanHolster() override;
	void Holster() override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;

	void UpdateSpot();
	bool ShouldWeaponIdle() override { return true; }

	CLaserSpot* m_pSpot;
	bool m_fSpotActive;
	int m_cActiveRockets;// how many missiles in flight from this launcher right now?

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usRpg;

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
	static CRpgRocket* CreateRpgRocket(Vector vecOrigin, Vector vecAngles, CBaseEntity* pOwner, CRpg* pLauncher);

	int m_iTrail;
	float m_flIgniteTime;
	EHANDLE m_pLauncher;// handle back to the launcher that fired me. 
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

class CGauss : public CBasePlayerWeapon
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

	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 4; }
	bool GetItemInfo(ItemInfo* p) override;
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
	void Fire(Vector vecOrigSrc, Vector vecDirShooting, float flDamage);
	float GetFullChargeTime();
	int m_iBalls;
	int m_iGlow;
	int m_iBeam;
	int m_iSoundState; // don't save this

	// was this weapon just fired primary or secondary?
	// we need to know so we can pick the right set of effects. 
	bool m_fPrimaryFire;
	AttackState m_fInAttack;

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

	float m_flStartCharge;
	float m_flAmmoStartCharge;
	float m_flPlayAftershock;
	float m_flNextAmmoBurn;//!< while charging, when to absorb another unit of player's ammo?

private:
	unsigned short m_usGaussFire;
	unsigned short m_usGaussSpin;
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

enum EGON_FIRESTATE
{
	FIRE_OFF, FIRE_CHARGE
};

enum EGON_FIREMODE
{
	FIRE_NARROW, FIRE_WIDE
};

constexpr int EGON_PRIMARY_VOLUME = 450;
constexpr std::string_view EGON_BEAM_SPRITE{"sprites/xbeam1.spr"};
constexpr std::string_view EGON_FLARE_SPRITE{"sprites/XSpark1.spr"};
constexpr std::string_view EGON_SOUND_OFF{"weapons/egon_off1.wav"};
constexpr std::string_view EGON_SOUND_RUN{"weapons/egon_run3.wav"};
constexpr std::string_view EGON_SOUND_STARTUP{"weapons/egon_windup2.wav"};

class CEgon : public CBasePlayerWeapon
{
public:
#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 4; }
	bool GetItemInfo(ItemInfo* p) override;
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

	float m_flAmmoUseTime;// since we use < 1 point of ammo per update, we subtract ammo on a timer.

	float GetPulseInterval();
	float GetDischargeInterval();

	void Fire(const Vector& vecOrigSrc, const Vector& vecDir);

	bool HasAmmo();

	void UseAmmo(int count);

	CBeam* m_pBeam;
	CBeam* m_pNoise;
	CSprite* m_pSprite;
	int m_fireState;

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

	unsigned short m_usEgonStop;

private:
	float				m_shootTime;
	EGON_FIREMODE		m_fireMode;
	float				m_shakeTime;
	bool				m_deployed;

	unsigned short m_usEgonFire;
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

class CHgun : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 4; }
	bool GetItemInfo(ItemInfo* p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	bool IsUseable() override;
	void Holster() override;
	void Reload() override;
	void WeaponIdle() override;
	float m_flNextAnimTime;

	float m_flRechargeTime;

	int m_iFirePhase;// don't save me.

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}
private:
	unsigned short m_usHornetFire;
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

class CHandGrenade : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 5; }
	bool GetItemInfo(ItemInfo* p) override;

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

	float m_flStartThrow;
	float m_flReleaseThrow;
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

class CSatchel : public CBasePlayerWeapon
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

	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 5; }
	bool GetItemInfo(ItemInfo* p) override;
	bool AddToPlayer(CBasePlayer* pPlayer) override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool AddDuplicate(CBasePlayerItem* pOriginal) override;
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

	ChargeState m_chargeReady;
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

class CTripmine : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 5; }
	bool GetItemInfo(ItemInfo* p) override;
	void SetObjectCollisionBox() override
	{
		//!!!BUGBUG - fix the model!
		pev->absmin = pev->origin + Vector(-16, -16, -5);
		pev->absmax = pev->origin + Vector(16, 16, 28);
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
	unsigned short m_usTripFire;

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

class CSqueak : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int ItemSlot() override { return 5; }
	bool GetItemInfo(ItemInfo* p) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Holster() override;
	void WeaponIdle() override;
	bool m_fJustThrown;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usSnarkFire;
};
