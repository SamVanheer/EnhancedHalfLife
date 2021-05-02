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

#include "sound/materials.hpp"

//
// Player PHYSICS FLAGS bits
//
constexpr int PFLAG_ONLADDER = 1 << 0;
constexpr int PFLAG_ONSWING = 1 << 0;
constexpr int PFLAG_ONTRAIN = 1 << 1;
constexpr int PFLAG_ONBARNACLE = 1 << 2;
constexpr int PFLAG_DUCKING = 1 << 3;		//!< In the process of ducking, but totally squatted yet
constexpr int PFLAG_USING = 1 << 4;		//!< Using a continuous entity
constexpr int PFLAG_OBSERVER = 1 << 5;	//!< player is locked in stationary cam mode. Spectators can move, observers can't.

constexpr int CSUITPLAYLIST = 4;		//!< max of 4 suit sentences queued up at any time

enum class SuitSoundType
{
	Sentence,
	Group
};

constexpr int SUIT_REPEAT_OK = 0;
constexpr int SUIT_NEXT_IN_30SEC = 30;
constexpr int SUIT_NEXT_IN_1MIN = 60;
constexpr int SUIT_NEXT_IN_5MIN = 300;
constexpr int SUIT_NEXT_IN_10MIN = 600;
constexpr int SUIT_NEXT_IN_30MIN = 1800;
constexpr int SUIT_NEXT_IN_1HOUR = 3600;

constexpr int CSUITNOREPEAT = 32;

enum class PlayerAnim
{
	Idle,
	Walk,
	Jump,
	SuperJump,
	Die,
	Attack1,
};

constexpr int MAX_ID_RANGE = 2048;
constexpr int SBAR_STRING_SIZE = 128;

enum sbar_data
{
	SBAR_ID_TARGETNAME = 1,
	SBAR_ID_TARGETHEALTH,
	SBAR_ID_TARGETARMOR,
	SBAR_END,
};

constexpr float CHAT_INTERVAL = 1.0f;

/**
*	@brief generic player
*	This is Half-Life player entity
*/
class CBasePlayer : public CBaseMonster
{
public:

	// Spectator camera
	/**
	*	@brief Find the next client in the game for this player to spectate
	*/
	void Observer_FindNextPlayer(bool bReverse);

	/**
	*	@brief Handle buttons in observer mode
	*/
	void Observer_HandleButtons();

	/**
	*	@brief Attempt to change the observer mode
	*/
	void Observer_SetMode(int iMode);
	void Observer_CheckTarget();
	void Observer_CheckProperties();
	EHANDLE	m_hObserverTarget;
	float m_flNextObserverInput = 0;
	int m_iObserverWeapon = 0;	//!< weapon of current tracked target
	int m_iObserverLastMode = 0; //!< last used observer mode
	bool IsObserver() { return pev->iuser1 != 0; }

	int random_seed = 0; //!< Seed that is shared between client & server for shared weapons code

	int m_iPlayerSound = 0; //!< the index of the sound list slot reserved for this player
	int m_iTargetVolume = 0; //!< ideal sound volume. 
	int m_iWeaponVolume = 0; //!< how loud the player's weapon is right now.
	int m_iExtraSoundTypes = 0; //!< additional classification for this weapon's sound
	int m_iWeaponFlash = 0; //!< brightness of the weapon flash
	float m_flStopExtraSoundTime = 0;

	float m_flFlashLightTime = 0; //!< Time until next battery draw/Recharge
	int m_iFlashBattery = 0; //!< Flashlight Battery Draw

	int m_afButtonLast = 0;
	int m_afButtonPressed = 0;
	int m_afButtonReleased = 0;

	EHANDLE m_hSndLast; //!< last sound entity to modify player room type
	float m_flSndRoomtype = 0; //!< last roomtype set by sound entity
	float m_flSndRange = 0; //!< dist from player to sound entity

	float m_flFallVelocity = 0;

	int m_rgItems[MAX_ITEMS]{};
	bool m_fKnownItem = false; //!< True when a new item needs to be added

	unsigned int m_afPhysicsFlags = 0; //!< physics flags - set when 'normal' physics should be revisited or overriden
	float m_fNextSuicideTime = 0; //!< the time after which the player can next use the suicide command

	// these are time-sensitive things that we keep track of
	float m_flTimeStepSound = 0; //!< when the last stepping sound was made
	float m_flTimeWeaponIdle = 0; //!< when to play another weapon idle animation.
	float m_flSwimTime = 0; //!< how long player has been underwater
	float m_flDuckTime = 0; //!< how long we've been ducking
	float m_flWallJumpTime = 0; //!< how long until next walljump

	float m_flSuitUpdate = 0; //!< when to play next suit update
	int m_rgSuitPlayList[CSUITPLAYLIST]{}; //!< next sentencenum to play for suit update
	int m_iSuitPlayNext = 0; //!< next sentence slot for queue storage;
	int m_rgiSuitNoRepeat[CSUITNOREPEAT]{}; //!< suit sentence no repeat list
	float m_rgflSuitNoRepeatTime[CSUITNOREPEAT]{}; //!< how long to wait before allowing repeat
	int m_lastDamageAmount = 0; //!< Last damage taken
	float m_tbdPrev = 0; //!< Time-based damage timer

	float m_flgeigerRange = 0; //!< range to nearest radiation source
	float m_flgeigerDelay = 0; //!< delay per update of range msg to client
	int m_igeigerRangePrev = 0;
	int m_iStepLeft = 0; //!< alternate left/right foot stepping sound TODO should be bool
	char m_szTextureName[CBTEXTURENAMEMAX]{}; //!< current texture name we're standing on
	char m_chTextureType = '\0'; //!< current texture type

	int m_idrowndmg = 0; //!< track drowning damage taken
	int m_idrownrestored = 0; //!< track drowning damage restored

	int m_bitsHUDDamage = 0; //!< Damage bits for the current fame. These get sent to the hude via the DAMAGE message TODO typo
	bool m_fInitHUD = false; //!< True when deferred HUD restart msg needs to be sent
	bool m_fGameHUDInitialized = false;
	int m_iTrain = 0; //!< Train control position
	bool m_fWeapon = false; //!< Set this to false to force a reset of the current weapon HUD info

	EHANDLE m_pTank; //!< the tank which the player is currently controlling,  nullptr if no tank
	EHANDLE m_hViewEntity; //!< The view entity being used, or null if the player is using itself as the view entity
	bool m_bResetViewEntity = false; //!<True if the player's view needs to be set back to the view entity
	float m_fDeadTime = 0; //!< the time at which the player died  (used in PlayerDeathThink())

	bool m_fNoPlayerSound = false; //!< a debugging feature. Player makes no sound if this is true. 
	bool m_fLongJump = false; //!< does this player have the longjump module?

	int m_iClientHealth = 0; //!< the health currently known by the client.  If this changes, send a new
	int m_iClientBattery = 0; //!< the Battery currently known by the client.  If this changes, send a new
	int m_iHideHUD = 0; //!< the players hud weapon info is to be hidden
	int m_iClientHideHUD = 0;
	int m_iFOV = 0; //!< field of view
	int m_iClientFOV = 0; //!< client's known FOV
	// usable player items 
	EHandle<CBasePlayerItem> m_hPlayerItems[MAX_ITEM_TYPES];
	EHandle<CBasePlayerItem> m_hActiveItem;
	EHandle<CBasePlayerItem> m_hClientActiveItem;  //!< client version of the active item
	EHandle<CBasePlayerItem> m_hLastItem;
	// shared ammo slots
	int	m_rgAmmo[MAX_AMMO_TYPES]{};
	int	m_rgAmmoLast[MAX_AMMO_TYPES]{};

	Vector m_vecAutoAim;
	bool m_fOnTarget = false;
	int m_iDeaths = 0;
	float m_iRespawnFrames = 0;	//!< used in PlayerDeathThink() to make sure players can always respawn

	int m_lastx = 0, m_lasty = 0;  //!< These are the previous update's crosshair angles, DON"T SAVE/RESTORE

	int m_nCustomSprayFrames = 0; //!< Custom clan logo frames for this player
	float m_flNextDecalTime = 0; //!< next time this player can spray a decal

	char m_szTeamName[TEAM_NAME_LENGTH]{};

	void Spawn() override;
	void Pain();

	//	void Think() override;
	virtual void Jump();
	virtual void Duck();
	virtual void PreThink();
	virtual void PostThink();
	Vector GetGunPosition() override;
	bool GiveHealth(float flHealth, int bitsDamageType) override;
	void TraceAttack(const TraceAttackInfo& info) override;

	/**
	*	@brief Take some damage.
	*	NOTE: each call to TakeDamage with bitsDamageType set to a time-based damage will cause the damage time countdown to be reset.
	*	Thus the ongoing effects of poison, radiation etc are implemented with subsequent calls to TakeDamage using DMG_GENERIC.
	*/
	bool TakeDamage(const TakeDamageInfo& info) override;
	void Killed(const KilledInfo& info) override;
	Vector BodyTarget(const Vector& posSrc) override { return Center() + pev->view_ofs * RANDOM_FLOAT(0.5, 1.1); }
	bool IsAlive() override { return (pev->deadflag == DeadFlag::No) && pev->health > 0; }
	bool ShouldFadeOnDeath() override { return false; }
	bool IsPlayer() override { return true; }	//!< Spectators should return false for this, they aren't "players" as far as game logic is concerned

	/**
	*	@details Bots should return false for this, they can't receive NET messages
	*	Spectators should return true for this
	*/
	bool IsNetClient() override { return true; }
	const char* TeamID() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	/**
	*	@brief call this when a player dies to pack up the appropriate weapons and ammo items,
	*	and to destroy anything that shouldn't be packed.
	*	This is pretty brute force :(
	*/
	void PackDeadPlayerItems();
	void RemoveAllItems(bool removeSuit);
	bool SwitchWeapon(CBasePlayerItem* pWeapon);

	/**
	*	@brief JOHN:  sends custom messages if player HUD data has changed  (eg health, ammo)
	*	@details resends any changed player HUD info to the client.
	*	Called every frame by PlayerPreThink
	*	Also called at start of demo recording and playback by ForceClientDllUpdate
	*	to ensure the demo gets messages reflecting all of the HUD state info.
	*/
	virtual void UpdateClientData();

	static TYPEDESCRIPTION m_playerSaveData[];

	/**
	*	@brief Player is moved across the transition by other means
	*/
	int ObjectCaps() override { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void Precache() override;
	bool IsOnLadder();
	bool FlashlightIsOn();
	void FlashlightTurnOn();
	void FlashlightTurnOff();

	/**
	*	@brief updates the position of the player's reserved sound slot in the sound list.
	*/
	void UpdatePlayerSound();
	void DeathSound() override;

	/**
	*	@brief ID's player as such.
	*/
	int Classify() override;

	/**
	*	@brief Set the activity based on an event or current state
	*/
	void SetAnimation(PlayerAnim playerAnim);
	char m_szAnimExtension[32];

	// custom player functions
	virtual void ImpulseCommands();
	void CheatImpulseCommands(int iImpulse);

	/**
	*	@brief find an intermission spot and send the player off into observer mode
	*/
	void StartDeathCam();
	void StartObserver(Vector vecPosition, Vector vecViewAngle);

	void AddPoints(int score, bool bAllowNegativeScore);
	void AddPointsToTeam(int score, bool bAllowNegativeScore);

	/**
	*	@brief Add a weapon to the player (Item == Weapon == Selectable Object)
	*/
	bool AddPlayerItem(CBasePlayerItem* pItem);
	bool RemovePlayerItem(CBasePlayerItem* pItem);

	/**
	*	@brief drop the named item, or if no name, the active item.
	*/
	void DropPlayerItem(const char* pszItemName);
	bool HasPlayerItem(CBasePlayerItem* pCheckItem);
	bool HasNamedPlayerItem(const char* pszItemName);
	bool HasWeapons();//!< do I have ANY weapons?
	void SelectLastItem();

	/**
	*	@brief Switch weapons
	*/
	void SelectItem(const char* pstr);

	/**
	*	@brief Called every frame by the player PreThink
	*/
	void ItemPreFrame();

	/**
	*	@brief Called every frame by the player PostThink
	*/
	void ItemPostFrame();
	void GiveNamedItem(const char* szName);
	void EnableControl(bool fControl);

	/**
	*	@brief Returns the unique ID for the ammo, or -1 if error
	*/
	int GiveAmmo(int iAmount, const char* szName, int iMax);

	int GetAmmoCount(const char* name) const
	{
		auto index = GetAmmoIndex(name);

		if (index != -1)
		{
			return m_rgAmmo[index];
		}

		return -1;
	}

	void SetAmmoCount(const char* name, int amount)
	{
		auto index = GetAmmoIndex(name);

		if (index != -1)
		{
			m_rgAmmo[index] = amount;
		}
	}

	/**
	*	@brief Called from UpdateClientData
	*	makes sure the client has all the necessary ammo info, if values have changed
	*/
	void SendAmmoUpdate();

	void WaterMove();
	void EXPORT PlayerDeathThink();

	/**
	*	@brief handles USE keypress
	*/
	void PlayerUse();

	void CheckSuitUpdate();

	/**
	*	@brief add sentence to suit playlist queue.
	*	@details if type is SuitSoundType::Group, then name is a sentence group (HEV_AA),
	*	otherwise name is a specific sentence name ie: !HEV_AA0.
	*	If iNoRepeat is specified in seconds,
	*	then we won't repeat playback of this word or sentence for at least that number of seconds.
	*/
	void SetSuitUpdate(const char* name, SuitSoundType type, int iNoRepeat);

	/**
	*	@brief if in range of radiation source, ping geiger counter
	*	@details
	*	THE HEV SUIT
	*
	*	The Suit provides 3 main functions: Protection, Notification and Augmentation.
	*	Some functions are automatic, some require power.
	*	The player gets the suit shortly after getting off the train in C1A0 and it stays
	*	with him for the entire game.
	*
	*	Protection
	*
	*		Heat/Cold
	*			When the player enters a hot/cold area, the heating/cooling indicator on the suit
	*			will come on and the battery will drain while the player stays in the area.
	*			After the battery is dead, the player starts to take damage.
	*			This feature is built into the suit and is automatically engaged.
	*		Anti-Toxin Syringe
	*			This will cure the player from being poisoned. Single use item.
	*		Health
	*			Small (1st aid kits, food, etc.)
	*			Large (boxes on walls)
	*		Armor
	*			The armor works using energy to create a protective field that deflects a
	*			percentage of damage projectile and explosive attacks.
	*
	*	Notification (via the HUD)
	*
	*	x	Health
	*	x	Ammo
	*	x	Automatic Health Care
	*			Notifies the player when automatic healing has been engaged.
	*	x	Geiger counter
	*			Classic Geiger counter sound and status bar at top of HUD
	*			alerts player to dangerous levels of radiation. This is not visible when radiation levels are normal.
	*	x	Poison
	*		Armor
	*			Displays the current level of armor.
	*
	*	Augmentation
	*		Long Jump
	*			Used by ducking and then jumping. Causes the player to further than normal.
	*
	*	Things powered by the battery
	*
	*		Armor
	*			Uses N power for every M units of damage.
	*		Heat/Cool
	*			Uses N power for every second in hot/cold area.
	*/
	void UpdateGeigerCounter();

	/*
	*	@details Time based Damage works as follows:
	*	1) There are several types of timebased damage:
	*
	*		::DMG_PARALYZE
	*		::DMG_NERVEGAS
	*		::DMG_POISON
	*		::DMG_RADIATION
	*		::DMG_DROWNRECOVER
	*		::DMG_ACID
	*		::DMG_SLOWBURN
	*		::DMG_SLOWFREEZE
	*
	*	2) A new hit inflicting tbd restarts the tbd counter - each monster has an 8bit counter, per damage type.
	*		The counter is decremented every second, so the maximum time an effect will last is 255/60 = 4.25 minutes.
	*		Of course, staying within the radius of a damaging effect like fire, nervegas, radiation will continually reset the counter to max.
	*
	*	3) Every second that a tbd counter is running, the player takes damage.
	*		The damage is determined by the type of tdb.
	*			Paralyze		- 1/2 movement rate, 30 second duration.
	*			Nervegas		- 5 points per second, 16 second duration = 80 points max dose.
	*			Poison			- 2 points per second, 25 second duration = 50 points max dose.
	*			Radiation		- 1 point per second, 50 second duration = 50 points max dose.
	*			Drown			- 5 points per second, 2 second duration.
	*			Acid/Chemical	- 5 points per second, 10 second duration = 50 points max.
	*			Burn			- 10 points per second, 2 second duration.
	*			Freeze			- 3 points per second, 10 second duration = 30 points max.
	*
	*	4) Certain actions or countermeasures counteract the damaging effects of tbds:
	*
	*		Armor/Heater/Cooler - Chemical(acid),burn, freeze all do damage to armor power, then to body
	*							- recharged by suit recharger
	*		Air In Lungs		- drowning damage is done to air in lungs first, then to body
	*							- recharged by poking head out of water
	*							- 10 seconds if swiming fast
	*		Air In SCUBA		- drowning damage is done to air in tanks first, then to body
	*							- 2 minutes in tanks. Need new tank once empty.
	*		Radiation Syringe	- Each syringe full provides protection vs one radiation dosage
	*		Antitoxin Syringe	- Each syringe full provides protection vs one poisoning (nervegas or poison).
	*		Health kit			- Immediate stop to acid/chemical, fire or freeze damage.
	*		Radiation Shower	- Immediate stop to radiation damage, acid/chemical or fire damage.
	*
	*
	*	If player is taking time based damage, continue doing damage to player -
	*	this simulates the effect of being poisoned, gassed, dosed with radiation etc -
	*	anything that continues to do damage even after the initial contact stops.
	*	Update all time based damage counters, and shut off any that are done.
	*
	*	The m_bitsDamageType bit MUST be set if any damage is to be taken.
	*	This routine will detect the initial on value of the m_bitsDamageType and init the appropriate counter.
	*	Only processes damage every second.
	*
	*	::PARALYZE_DURATION
	*	::PARALYZE_DAMAGE
	*
	*	::NERVEGAS_DURATION
	*	::NERVEGAS_DAMAGE
	*
	*	::POISON_DURATION
	*	::POISON_DAMAGE
	*
	*	::RADIATION_DURATION
	*	::RADIATION_DAMAGE
	*
	*	::ACID_DURATION
	*	::ACID_DAMAGE
	*
	*	::SLOWBURN_DURATION
	*	::SLOWBURN_DAMAGE
	*
	*	::SLOWFREEZE_DURATION
	*	::SLOWFREEZE_DAMAGE
	*/
	void CheckTimeBasedDamage();

	bool BecomeProne() override;
	void BarnacleVictimBitten(CBaseEntity* pBarnacle) override;
	void BarnacleVictimReleased() override;
	static int GetAmmoIndex(const char* psz);
	int AmmoInventory(int iAmmoIndex);

	/**
	*	@brief return player light level plus virtual muzzle flash
	*/
	int Illumination() override;

	void ResetAutoaim();

	/**
	*	@brief set crosshair position to point to enemy
	*/
	Vector GetAutoaimVector(float flDelta);
	Vector AutoaimDeflection(Vector& vecSrc, float flDist, float flDelta);

	/**
	*	@brief Forces all client .dll specific data to be resent to client.
	*	@details When recording a demo,
	*	we need to have the server tell us the entire client state so that the client side .dll can behave correctly.
	*	Reset stuff so that the state is transmitted.
	*/
	void ForceClientDllUpdate();

	/**
	*	@brief UNDONE:  Determine real frame limit, 8 is a placeholder.
	*	Note:  -1 means no custom frames present.
	*/
	void SetCustomDecalFrames(int nFrames);

	/**
	*	@brief Returns the # of custom frames this player's custom clan logo contains.
	*/
	int GetCustomDecalFrames();

	/**
	*	@brief Player ID
	*/
	void InitStatusBar();
	void UpdateStatusBar();
	int m_izSBarState[SBAR_END]{};
	float m_flNextSBarUpdateTime = 0;
	float m_flStatusBarDisappearDelay = 0;
	char m_SbarString0[SBAR_STRING_SIZE]{};
	char m_SbarString1[SBAR_STRING_SIZE]{};

	float m_flNextChatTime = 0;

	void SetPrefsFromUserinfo(char* infobuffer);

	int m_iAutoWepSwitch = 0;

	bool m_bRestored = false;
};

extern bool gInitHUD;
