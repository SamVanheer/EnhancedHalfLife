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

#include "CBaseItem.hpp"
#include "weapons.hpp"

#include "CBaseWeapon.generated.hpp"

class CBasePlayer;
struct weapon_data_t;

constexpr int WEAPON_FLAG_SELECTONEMPTY = 1;
constexpr int WEAPON_FLAG_NOAUTORELOAD = 2;
constexpr int WEAPON_FLAG_NOAUTOSWITCHEMPTY = 4;
constexpr int WEAPON_FLAG_LIMITINWORLD = 8;
constexpr int WEAPON_FLAG_EXHAUSTIBLE = 16; // A player can totally exhaust their ammo supply and lose this weapon

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

enum class WeaponPickupRule
{
	Default,		//!< Use default pickup rules
	Always,			//!< Player can always pick up this weapon, even if they already have it and have maximum ammo
	Never,			//!< Player can never pick up this weapon (ornament, prefer cheaper entities)
	NoDuplicates,	//!< Player can pick up the weapon if they don't have it already
};

extern int giAmmoIndex;

void AddAmmoNameToAmmoRegistry(const char* szAmmoname, int maxCarry);

/**
*	@brief Weapons that the player has in their inventory that they can use
*/
class EHL_CLASS() CBaseWeapon : public CBaseItem
{
	EHL_GENERATED_BODY()

public:
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

	void KeyValue(KeyValueData * pkvd) override;

	/**
	*	@brief return true if you want the weapon added to the player inventory
	*/
	virtual bool AddToPlayer(CBasePlayer * pPlayer);

	/**
	*	@brief CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
	*	@return true if you want your duplicate removed from world
	*/
	virtual bool AddDuplicate(CBaseWeapon * weapon);
	void EXPORT DestroyWeapon();
	ItemApplyResult Apply(CBasePlayer * pPlayer) override;

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
	virtual bool ExtractAmmo(CBaseWeapon * pWeapon);

	/**
	*	@brief called by the new weapon's class with the existing weapon as parameter
	*	@return true if you can add ammo to yourself when picked up
	*/
	virtual int ExtractClipAmmo(CBaseWeapon * pWeapon);

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
	virtual bool GetWeaponInfo(WeaponInfo & p) { return false; }

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
	virtual void AttachToPlayer(CBasePlayer * pPlayer);

	/**
	*	@brief sends hud info to client dll, if things have changed
	*/
	virtual bool UpdateClientData(CBasePlayer * pPlayer);

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

	virtual void GetWeaponData(weapon_data_t & data) {}
	virtual void SetWeaponData(const weapon_data_t & data) {}
	virtual void DecrementTimers() {}

protected:
	CBaseWeapon* GetItemToRespawn(const Vector & respawnPoint) override;

public:
	static inline WeaponInfo WeaponInfoArray[MAX_WEAPONS]{};
	static inline AmmoInfo AmmoInfoArray[MAX_AMMO_TYPES]{};

	EHL_FIELD(Persisted)
		EHandle<CBasePlayer> m_hPlayer;

	EHL_FIELD(Persisted)
		EHandle<CBaseWeapon> m_hNext;

	EHL_FIELD(Persisted)
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

	//TODO: move to shotgun class
	EHL_FIELD(Persisted, Type=Time)
	float m_flPumpTime = 0;

#ifdef CLIENT_WEAPONS
	EHL_FIELD(Persisted)
#else
	EHL_FIELD(Persisted, Type=Time)
#endif
	float m_flNextPrimaryAttack = 0;							//!< soonest time WeaponPostFrame will call PrimaryAttack

#ifdef CLIENT_WEAPONS
	EHL_FIELD(Persisted)
#else
	EHL_FIELD(Persisted, Type=Time)
#endif
	float m_flNextSecondaryAttack = 0;							//!< soonest time WeaponPostFrame will call SecondaryAttack

#ifdef CLIENT_WEAPONS
	EHL_FIELD(Persisted)
#else
	EHL_FIELD(Persisted, Type=Time)
#endif
	float m_flTimeWeaponIdle = 0;								//!< soonest time WeaponPostFrame will call WeaponIdle

	EHL_FIELD(Persisted)
	int m_iPrimaryAmmoType = 0;									//!< "primary" ammo index into players m_rgAmmo[]

	EHL_FIELD(Persisted)
	int m_iSecondaryAmmoType = 0;								//!< "secondary" ammo index into players m_rgAmmo[]

	EHL_FIELD(Persisted)
	int m_iClip = 0;											//!< number of shots left in the primary weapon clip, -1 it not used

		//reset to zero on load so hud gets updated correctly
	int m_iClientClip = 0;										//!< the last version of m_iClip sent to hud dll

	//reset to zero on load so hud gets updated correctly
	WeaponState m_iClientWeaponState = WeaponState::NotActive;	//!< the last version of the weapon state sent to hud dll (is current weapon, is on target)

	EHL_FIELD(Persisted)
	bool m_fInReload = false;									//!< Are we in the middle of a reload;

	EHL_FIELD(Persisted)
	int m_iDefaultPrimaryAmmo = 0; //!< how much ammo you get when you pick up this weapon as placed by a level designer.

	EHL_FIELD(Persisted)
	int m_iDefaultAmmo = 0; //!< Amount of ammo left in weapon

		//Handled in CBasePlayer::AddPlayerWeapon
	EHL_FIELD(Persisted)
	WeaponPickupRule m_PickupRule = WeaponPickupRule::Default;

	// hle time creep vars
	float m_flPrevPrimaryAttack = 0;
	float m_flLastFireTime = 0;

private:
	EHL_FIELD(Persisted, Type = ModelName)
	string_t m_iszWorldModelName = iStringNull;
};
