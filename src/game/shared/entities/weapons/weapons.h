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

void DeactivateSatchels( CBasePlayer *pOwner );
void W_Precache();

// Contact Grenade / Timed grenade / Satchel Charge
class CGrenade : public CBaseMonster
{
public:
	void Spawn() override;

	enum SATCHELCODE { SATCHEL_DETONATE = 0, SATCHEL_RELEASE };

	static CGrenade *ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	static CGrenade *ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static CGrenade *ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static void UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code );

	void Explode( Vector vecSrc, Vector vecAim );
	void Explode( TraceResult *pTrace, int bitsDamageType );
	void EXPORT Smoke();

	void EXPORT BounceTouch( CBaseEntity *pOther );
	void EXPORT SlideTouch( CBaseEntity *pOther );
	void EXPORT ExplodeTouch( CBaseEntity *pOther );
	void EXPORT DangerSoundThink();
	void EXPORT PreDetonate();
	void EXPORT Detonate();
	void EXPORT DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT TumbleThink();

	virtual void BounceSound();
	int	BloodColor() override { return DONT_BLEED; }
	void Killed( entvars_t *pevAttacker, int iGib ) override;

	bool m_fRegisteredSound;// whether or not this grenade has issued its DANGER sound to the world sound list yet.
};


// constant items
constexpr int ITEM_HEALTHKIT = 1;
constexpr int ITEM_ANTIDOTE = 2;
constexpr int ITEM_SECURITY = 3;
constexpr int ITEM_BATTERY = 4;

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
	const char	*pszAmmo1;	// ammo 1 type
	int		iMaxAmmo1;		// max ammo 1
	const char	*pszAmmo2;	// ammo 2 type
	int		iMaxAmmo2;		// max ammo 2
	const char	*pszName;
	int		iMaxClip;
	int		iId;
	int		iFlags;
	int		iWeight;// this value used to determine this weapon's importance in autoselection.
};

struct AmmoInfo
{
	const char *pszName;
	int iId;
};

extern int giAmmoIndex;

void AddAmmoNameToAmmoRegistry(const char* szAmmoname);

// Items that the player has in their inventory that they can use
class CBasePlayerItem : public CBaseAnimating
{
public:
	void SetObjectCollisionBox() override;

	bool Save(CSave& save) override;
	bool Restore( CRestore &restore ) override;
	
	static	TYPEDESCRIPTION m_SaveData[];

	virtual bool AddToPlayer( CBasePlayer *pPlayer );	// return true if the item you want the item added to the player inventory
	virtual bool AddDuplicate( CBasePlayerItem *pItem ) { return false; }	// return true if you want your duplicate removed from world
	void EXPORT DestroyItem();
	void EXPORT DefaultTouch( CBaseEntity *pOther );	// default weapon touch
	void EXPORT FallThink ();// when an item is first spawned, this think is run to determine when the object has hit the ground.
	void EXPORT Materialize();// make a weapon visible and tangible
	void EXPORT AttemptToMaterialize();  // the weapon desires to become visible and tangible, if the game rules allow for it
	CBaseEntity* Respawn () override;// copy a weapon
	void FallInit();
	void CheckRespawn();
	virtual bool GetItemInfo(ItemInfo *p) { return false; }	// returns 0 if struct not filled out
	virtual bool CanDeploy() { return true; }
	virtual bool Deploy( )								// returns is deploy was successful
		 { return true; }

	virtual bool CanHolster() { return true; }// can this weapon be put away right now?
	virtual void Holster( int skiplocal = 0 );
	virtual void UpdateItemInfo() {}

	virtual void ItemPreFrame()	{}		// called each frame by the player PreThink
	virtual void ItemPostFrame() {}		// called each frame by the player PostThink

	virtual void Drop();
	virtual void Kill();
	virtual void AttachToPlayer ( CBasePlayer *pPlayer );

	virtual int PrimaryAmmoIndex() { return -1; }
	virtual int SecondaryAmmoIndex() { return -1; }

	virtual bool UpdateClientData( CBasePlayer *pPlayer ) { return false; }

	virtual CBasePlayerItem *GetWeaponPtr() { return nullptr; }

	virtual void GetWeaponData(weapon_data_t& data) {}
	virtual void SetWeaponData(const weapon_data_t& data) {}

	static ItemInfo ItemInfoArray[ MAX_WEAPONS ];
	static AmmoInfo AmmoInfoArray[MAX_AMMO_TYPES];

	CBasePlayer	*m_pPlayer;
	CBasePlayerItem *m_pNext;
	int		m_iId;												// WEAPON_???

	virtual int iItemSlot() { return 0; }			// return 0 to MAX_ITEMS_SLOTS, used in hud

	int			iItemPosition() { return ItemInfoArray[ m_iId ].iPosition; }
	const char	*pszAmmo1()	{ return ItemInfoArray[ m_iId ].pszAmmo1; }
	int			iMaxAmmo1()	{ return ItemInfoArray[ m_iId ].iMaxAmmo1; }
	const char	*pszAmmo2()	{ return ItemInfoArray[ m_iId ].pszAmmo2; }
	int			iMaxAmmo2()	{ return ItemInfoArray[ m_iId ].iMaxAmmo2; }
	const char	*pszName()	{ return ItemInfoArray[ m_iId ].pszName; }
	int			iMaxClip()	{ return ItemInfoArray[ m_iId ].iMaxClip; }
	int			iWeight()		{ return ItemInfoArray[ m_iId ].iWeight; }
	int			iFlags()		{ return ItemInfoArray[ m_iId ].iFlags; }

	// int		m_iIdPrimary;										// Unique Id for primary ammo
	// int		m_iIdSecondary;										// Unique Id for secondary ammo
};


// inventory items that 
class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	bool Save(CSave& save) override;
	bool Restore( CRestore &restore ) override;
	
	static	TYPEDESCRIPTION m_SaveData[];

	// generic weapon versions of CBasePlayerItem calls
	bool AddToPlayer( CBasePlayer *pPlayer ) override;
	bool AddDuplicate( CBasePlayerItem *pItem ) override;

	virtual bool ExtractAmmo( CBasePlayerWeapon *pWeapon ); //{ return true; }			// Return true if you can add ammo to yourself when picked up
	virtual int ExtractClipAmmo( CBasePlayerWeapon *pWeapon );// { return true; }			// Return true if you can add ammo to yourself when picked up

	virtual int AddWeapon() { ExtractAmmo( this ); return true; }	// Return true if you want to add yourself to the player

	// generic "shared" ammo handlers
	bool AddPrimaryAmmo( int iCount, const char *szName, int iMaxClip, int iMaxCarry );
	bool AddSecondaryAmmo( int iCount, const char *szName, int iMaxCarry );

	void UpdateItemInfo() override {}	// updates HUD state

	int m_iPlayEmptySound;
	int m_fFireOnEmpty;		// True when the gun is empty and the player is still holding down the
							// attack key(s)
	virtual bool PlayEmptySound();
	virtual void ResetEmptySound();

	virtual void SendWeaponAnim( int iAnim, int skiplocal = 1, int body = 0 );  // skiplocal is 1 if client is predicting weapon animations

	bool CanDeploy() override;
	virtual bool IsUseable();
	bool DefaultDeploy(const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt, int skiplocal = 0, int body = 0 );
	bool DefaultReload( int iClipSize, int iAnim, float fDelay, int body = 0 );

	void ItemPostFrame() override;	// called each frame by the player PostThink
	// called by CBasePlayerWeapons ItemPostFrame()
	virtual void PrimaryAttack() {}				// do "+ATTACK"
	virtual void SecondaryAttack() {}			// do "+ATTACK2"
	virtual void Reload() {}						// do "+RELOAD"
	virtual void WeaponIdle() {}					// called when no buttons pressed
	bool UpdateClientData( CBasePlayer *pPlayer ) override;		// sends hud info to client dll, if things have changed
	virtual void RetireWeapon();
	virtual bool ShouldWeaponIdle() {return false; }
	void Holster( int skiplocal = 0 ) override;
	virtual bool UseDecrement() { return false; }
	
	int	PrimaryAmmoIndex() override;
	int	SecondaryAmmoIndex() override;

	void PrintState();

	CBasePlayerItem *GetWeaponPtr() override { return (CBasePlayerItem *)this; }
	float GetNextAttackDelay( float delay );

	float m_flPumpTime;
	int		m_fInSpecialReload;									// Are we in the middle of a reload for the shotguns
	float	m_flNextPrimaryAttack;								// soonest time ItemPostFrame will call PrimaryAttack
	float	m_flNextSecondaryAttack;							// soonest time ItemPostFrame will call SecondaryAttack
	float	m_flTimeWeaponIdle;									// soonest time ItemPostFrame will call WeaponIdle
	int		m_iPrimaryAmmoType;									// "primary" ammo index into players m_rgAmmo[]
	int		m_iSecondaryAmmoType;								// "secondary" ammo index into players m_rgAmmo[]
	int		m_iClip;											// number of shots left in the primary weapon clip, -1 it not used
	int		m_iClientClip;										// the last version of m_iClip sent to hud dll
	WeaponState m_iClientWeaponState;							// the last version of the weapon state sent to hud dll (is current weapon, is on target)
	int		m_fInReload;										// Are we in the middle of a reload;

	int		m_iDefaultAmmo;// how much ammo you get when you pick up this weapon as placed by a level designer.
	
	// hle time creep vars
	float	m_flPrevPrimaryAttack;
	float	m_flLastFireTime;			

};


class CBasePlayerAmmo : public CBaseEntity
{
public:
	void Spawn() override;
	void EXPORT DefaultTouch( CBaseEntity *pOther ); // default weapon touch
	virtual bool AddAmmo( CBaseEntity *pOther ) { return true; }

	CBaseEntity* Respawn() override;
	void EXPORT Materialize();
};


extern DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
extern DLL_GLOBAL	const char *g_pModelNameLaser;

extern DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
extern DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
extern DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
extern DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for blood drops
extern DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for blood spray (bigger)

void ClearMultiDamage();
void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker );
void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType);

void DecalGunshot( TraceResult *pTrace, int iBulletType );
void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
int DamageDecal( CBaseEntity *pEntity, int bitsDamageType );
void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );

struct MULTIDAMAGE
{
	CBaseEntity		*pEntity;
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

//=========================================================
// CWeaponBox - a single entity that can store weapons
// and ammo. 
//=========================================================
class CWeaponBox : public CBaseEntity
{
	void Precache() override;
	void Spawn() override;
	void Touch( CBaseEntity *pOther ) override;
	void KeyValue( KeyValueData *pkvd ) override;
	bool IsEmpty();
	int  GiveAmmo( int iCount, const char *szName, int iMax, int *pIndex = nullptr);
	void SetObjectCollisionBox() override;

public:
	void EXPORT Kill ();
	bool Save(CSave& save) override;
	bool Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];

	bool HasWeapon( CBasePlayerItem *pCheckItem );
	bool PackWeapon( CBasePlayerItem *pWeapon );
	bool PackAmmo( string_t iszName, int iCount );
	
	CBasePlayerItem	*m_rgpPlayerItems[MAX_ITEM_TYPES];// one slot for each 

	string_t m_rgiszAmmo[MAX_AMMO_TYPES];// ammo names
	int	m_rgAmmo[MAX_AMMO_TYPES];// ammo quantities

	int m_cAmmoTypes;// how many ammo types packed into this box (if packed by a level designer)
};

#ifdef CLIENT_DLL
bool bIsMultiplayer ();
void LoadVModel ( const char *szViewModel, CBasePlayer *m_pPlayer );
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
	int iItemSlot() override { return 2; }
	bool GetItemInfo(ItemInfo *p) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void GlockFire( float flSpread, float flCycleTime, bool fUseAutoAim );
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
	int iItemSlot() override { return 1; }
	void EXPORT SwingAgain();
	void EXPORT Smack();
	bool GetItemInfo(ItemInfo *p) override;

	void PrimaryAttack() override;
	bool Swing( bool fFirst );
	bool Deploy() override;
	void Holster( int skiplocal = 0 ) override;
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
	int iItemSlot() override { return 2; }
	bool GetItemInfo(ItemInfo *p) override;
	bool AddToPlayer( CBasePlayer *pPlayer ) override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Holster( int skiplocal = 0 ) override;
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
	int iItemSlot() override { return 3; }
	bool GetItemInfo(ItemInfo *p) override;
	bool AddToPlayer( CBasePlayer *pPlayer ) override;

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
	int iItemSlot( ) override { return 3; }
	bool GetItemInfo(ItemInfo *p) override;

	void FireBolt();
	void FireSniperBolt();
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool AddToPlayer( CBasePlayer *pPlayer ) override;
	bool Deploy( ) override;
	void Holster( int skiplocal = 0 ) override;
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
	bool Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif


	void Spawn() override;
	void Precache() override;
	int iItemSlot( ) override { return 3; }
	bool GetItemInfo(ItemInfo *p) override;
	bool AddToPlayer( CBasePlayer *pPlayer ) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy( ) override;
	void Reload() override;
	void WeaponIdle() override;
	void ItemPostFrame() override;
	int m_fInReload;
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
	void Suspend( float flSuspendTime );
	void EXPORT Revive();
	
	static CLaserSpot *CreateSpot();
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
	bool Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn() override;
	void Precache() override;
	void Reload() override;
	int iItemSlot() override { return 4; }
	bool GetItemInfo(ItemInfo *p) override;
	bool AddToPlayer( CBasePlayer *pPlayer ) override;

	bool Deploy() override;
	bool CanHolster() override;
	void Holster( int skiplocal = 0 ) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;

	void UpdateSpot();
	bool ShouldWeaponIdle() override { return true; }

	CLaserSpot *m_pSpot;
	int m_fSpotActive;
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
	bool Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];
	void Spawn() override;
	void Precache() override;
	void EXPORT FollowThink();
	void EXPORT IgniteThink();
	void EXPORT RocketTouch( CBaseEntity *pOther );
	static CRpgRocket *CreateRpgRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CRpg *pLauncher );

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

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 4; }
	bool GetItemInfo(ItemInfo *p) override;
	bool AddToPlayer( CBasePlayer *pPlayer ) override;

	bool Deploy() override;
	void Holster( int skiplocal = 0  ) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;
	
	void StartFire();
	void Fire( Vector vecOrigSrc, Vector vecDirShooting, float flDamage );
	float GetFullChargeTime();
	int m_iBalls;
	int m_iGlow;
	int m_iBeam;
	int m_iSoundState; // don't save this

	// was this weapon just fired primary or secondary?
	// we need to know so we can pick the right set of effects. 
	bool m_fPrimaryFire;
	int m_fInAttack;

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

constexpr int	EGON_PRIMARY_VOLUME = 450;
#define EGON_BEAM_SPRITE		"sprites/xbeam1.spr"
#define EGON_FLARE_SPRITE		"sprites/XSpark1.spr"
#define EGON_SOUND_OFF			"weapons/egon_off1.wav"
#define EGON_SOUND_RUN			"weapons/egon_run3.wav"
#define EGON_SOUND_STARTUP		"weapons/egon_windup2.wav"

class CEgon : public CBasePlayerWeapon
{
public:
#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 4; }
	bool GetItemInfo(ItemInfo *p) override;
	bool AddToPlayer( CBasePlayer *pPlayer ) override;

	bool Deploy() override;
	void Holster( int skiplocal = 0 ) override;

	void UpdateEffect( const Vector &startPoint, const Vector &endPoint, float timeBlend );

	void CreateEffect ();
	void DestroyEffect ();

	void EndAttack();
	void Attack();
	void PrimaryAttack() override;
	bool ShouldWeaponIdle() override { return true; }
	void WeaponIdle() override;

	float m_flAmmoUseTime;// since we use < 1 point of ammo per update, we subtract ammo on a timer.

	float GetPulseInterval();
	float GetDischargeInterval();

	void Fire( const Vector &vecOrigSrc, const Vector &vecDir );

	bool HasAmmo();

	void UseAmmo( int count );

	CBeam				*m_pBeam;
	CBeam				*m_pNoise;
	CSprite				*m_pSprite;
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
	int iItemSlot() override { return 4; }
	bool GetItemInfo(ItemInfo *p) override;
	bool AddToPlayer( CBasePlayer *pPlayer ) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	bool IsUseable() override;
	void Holster( int skiplocal = 0 ) override;
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
	int iItemSlot() override { return 5; }
	bool GetItemInfo(ItemInfo *p) override;

	void PrimaryAttack() override;
	bool Deploy() override;
	bool CanHolster() override;
	void Holster( int skiplocal = 0 ) override;
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

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 5; }
	bool GetItemInfo(ItemInfo *p) override;
	bool AddToPlayer( CBasePlayer *pPlayer ) override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool AddDuplicate( CBasePlayerItem *pOriginal ) override;
	bool CanDeploy() override;
	bool Deploy() override;
	bool IsUseable() override;
	
	void Holster( int skiplocal = 0 ) override;
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

	int m_chargeReady;
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
	int iItemSlot() override { return 5; }
	bool GetItemInfo(ItemInfo *p) override;
	void SetObjectCollisionBox() override
	{
		//!!!BUGBUG - fix the model!
		pev->absmin = pev->origin + Vector(-16, -16, -5);
		pev->absmax = pev->origin + Vector(16, 16, 28); 
	}

	void PrimaryAttack() override;
	bool Deploy() override;
	void Holster( int skiplocal = 0 ) override;
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
	int iItemSlot() override { return 5; }
	bool GetItemInfo(ItemInfo *p) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Holster( int skiplocal = 0 ) override;
	void WeaponIdle() override;
	int m_fJustThrown;

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
