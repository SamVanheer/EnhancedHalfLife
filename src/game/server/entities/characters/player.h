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

#include "materials.hpp"

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
	void	Observer_FindNextPlayer(bool bReverse);

	/**
	*	@brief Handle buttons in observer mode
	*/
	void	Observer_HandleButtons();

	/**
	*	@brief Attempt to change the observer mode
	*/
	void	Observer_SetMode(int iMode);
	void	Observer_CheckTarget();
	void	Observer_CheckProperties();
	EHANDLE	m_hObserverTarget;
	float	m_flNextObserverInput;
	int		m_iObserverWeapon;	//!< weapon of current tracked target
	int		m_iObserverLastMode;//!< last used observer mode
	bool IsObserver() { return pev->iuser1 != 0; }

	//TODO: typo
	int					random_seed;    //!< See that is shared between client & server for shared weapons code

	int					m_iPlayerSound;//!< the index of the sound list slot reserved for this player
	int					m_iTargetVolume;//!< ideal sound volume. 
	int					m_iWeaponVolume;//!< how loud the player's weapon is right now.
	int					m_iExtraSoundTypes;//!< additional classification for this weapon's sound
	int					m_iWeaponFlash;//!< brightness of the weapon flash
	float				m_flStopExtraSoundTime;

	float				m_flFlashLightTime;	//!< Time until next battery draw/Recharge
	int					m_iFlashBattery;		//!< Flashlight Battery Draw

	int					m_afButtonLast;
	int					m_afButtonPressed;
	int					m_afButtonReleased;

	edict_t* m_pentSndLast;			//!< last sound entity to modify player room type
	float				m_flSndRoomtype;		//!< last roomtype set by sound entity
	float				m_flSndRange;			//!< dist from player to sound entity

	float				m_flFallVelocity;

	int					m_rgItems[MAX_ITEMS];
	bool				m_fKnownItem;		//!< True when a new item needs to be added

	unsigned int		m_afPhysicsFlags;	//!< physics flags - set when 'normal' physics should be revisited or overriden
	float				m_fNextSuicideTime; //!< the time after which the player can next use the suicide command


// these are time-sensitive things that we keep track of
	float				m_flTimeStepSound;	//!< when the last stepping sound was made
	float				m_flTimeWeaponIdle; //!< when to play another weapon idle animation.
	float				m_flSwimTime;		//!< how long player has been underwater
	float				m_flDuckTime;		//!< how long we've been ducking
	float				m_flWallJumpTime;	//!< how long until next walljump

	float				m_flSuitUpdate;					//!< when to play next suit update
	int					m_rgSuitPlayList[CSUITPLAYLIST];//!< next sentencenum to play for suit update
	int					m_iSuitPlayNext;				//!< next sentence slot for queue storage;
	int					m_rgiSuitNoRepeat[CSUITNOREPEAT];		//!< suit sentence no repeat list
	float				m_rgflSuitNoRepeatTime[CSUITNOREPEAT];	//!< how long to wait before allowing repeat
	int					m_lastDamageAmount;		//!< Last damage taken
	float				m_tbdPrev;				//!< Time-based damage timer

	float				m_flgeigerRange;		//!< range to nearest radiation source
	float				m_flgeigerDelay;		//!< delay per update of range msg to client
	int					m_igeigerRangePrev;
	int					m_iStepLeft;			//!< alternate left/right foot stepping sound
	char				m_szTextureName[CBTEXTURENAMEMAX];	//!< current texture name we're standing on
	char				m_chTextureType;		//!< current texture type

	int					m_idrowndmg;			//!< track drowning damage taken
	int					m_idrownrestored;		//!< track drowning damage restored

	int					m_bitsHUDDamage;		//!< Damage bits for the current fame. These get sent to 
												//!< the hude via the DAMAGE message
	bool				m_fInitHUD;				//!< True when deferred HUD restart msg needs to be sent
	bool				m_fGameHUDInitialized;
	int					m_iTrain;				//!< Train control position
	bool				m_fWeapon;				//!< Set this to false to force a reset of the current weapon HUD info

	EHANDLE				m_pTank;				//!< the tank which the player is currently controlling,  nullptr if no tank
	EHANDLE				m_hViewEntity;			//!< The view entity being used, or null if the player is using itself as the view entity
	bool m_bResetViewEntity;					//!<True if the player's view needs to be set back to the view entity
	float				m_fDeadTime;			//!< the time at which the player died  (used in PlayerDeathThink())

	bool			m_fNoPlayerSound;	//!< a debugging feature. Player makes no sound if this is true. 
	bool			m_fLongJump; //!< does this player have the longjump module?

	float       m_tSneaking; //TODO: never used, remove
	int			m_iClientHealth;	//!< the health currently known by the client.  If this changes, send a new
	int			m_iClientBattery;	//!< the Battery currently known by the client.  If this changes, send a new
	int			m_iHideHUD;		//!< the players hud weapon info is to be hidden
	int			m_iClientHideHUD;
	int			m_iFOV;			//!< field of view
	int			m_iClientFOV;	//!< client's known FOV
	// usable player items 
	CBasePlayerItem* m_rgpPlayerItems[MAX_ITEM_TYPES];
	CBasePlayerItem* m_pActiveItem;
	CBasePlayerItem* m_pClientActiveItem;  //!< client version of the active item
	CBasePlayerItem* m_pLastItem;
	// shared ammo slots
	int	m_rgAmmo[MAX_AMMO_TYPES];
	int	m_rgAmmoLast[MAX_AMMO_TYPES];

	Vector				m_vecAutoAim;
	bool				m_fOnTarget;
	int					m_iDeaths;
	float				m_iRespawnFrames;	//!< used in PlayerDeathThink() to make sure players can always respawn

	int m_lastx, m_lasty;  //!< These are the previous update's crosshair angles, DON"T SAVE/RESTORE

	int m_nCustomSprayFrames;//!< Custom clan logo frames for this player
	float	m_flNextDecalTime;//!< next time this player can spray a decal

	char m_szTeamName[TEAM_NAME_LENGTH];

	void Spawn() override;
	void Pain();

	//	void Think() override;
	virtual void Jump();
	virtual void Duck();
	virtual void PreThink();
	virtual void PostThink();
	Vector GetGunPosition() override;
	bool TakeHealth(float flHealth, int bitsDamageType) override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;

	/**
	*	@brief Take some damage.
	*	NOTE: each call to TakeDamage with bitsDamageType set to a time-based damage will cause the damage time countdown to be reset.
	*	Thus the ongoing effects of poison, radiation etc are implemented with subsequent calls to TakeDamage using DMG_GENERIC.
	*	TODO: verify that this is actually the case
	*/
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	void	Killed(entvars_t* pevAttacker, int iGib) override;
	Vector BodyTarget(const Vector& posSrc) override { return Center() + pev->view_ofs * RANDOM_FLOAT(0.5, 1.1); }
	void StartSneaking() override { m_tSneaking = gpGlobals->time - 1; }
	void StopSneaking() override { m_tSneaking = gpGlobals->time + 30; }
	bool IsSneaking() override { return m_tSneaking <= gpGlobals->time; }
	bool IsAlive() override { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
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
	*	@brief Marks everything as new so the player will resend this to the hud.
	*/
	void RenewItems();

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

	static	TYPEDESCRIPTION m_playerSaveData[];

	/**
	*	@brief Player is moved across the transition by other means
	*/
	int		ObjectCaps() override { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void	Precache() override;
	bool			IsOnLadder();
	bool			FlashlightIsOn();
	void			FlashlightTurnOn();
	void			FlashlightTurnOff();

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
	void SelectPrevItem(int iItem);
	void SelectNextItem(int iItem);
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

	//TODO: this API is misused by treating the result as a bool when it can return a non-zero value for invalid input
	/**
	*	@brief Returns the unique ID for the ammo, or -1 if error
	*/
	int  GiveAmmo(int iAmount, const char* szName, int iMax);

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
	*/
	void UpdateGeigerCounter();
	void CheckTimeBasedDamage();

	bool FBecomeProne() override;
	void BarnacleVictimBitten(entvars_t* pevBarnacle) override;
	void BarnacleVictimReleased() override;
	static int GetAmmoIndex(const char* psz);
	int AmmoInventory(int iAmmoIndex);

	/**
	*	@brief return player light level plus virtual muzzle flash
	*/
	int Illumination() override;

	void ResetAutoaim();

	//TODO: typo
	/**
	*	@brief set crosshair position to point to enemey
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

	//TODO: move these Gauss-specific variables to the CGauss class
	float m_flStartCharge;
	float m_flAmmoStartCharge;
	float m_flPlayAftershock;
	float m_flNextAmmoBurn;//!< while charging, when to absorb another unit of player's ammo?

	/**
	*	@brief Player ID
	*/
	void InitStatusBar();
	void UpdateStatusBar();
	int m_izSBarState[SBAR_END];
	float m_flNextSBarUpdateTime;
	float m_flStatusBarDisappearDelay;
	char m_SbarString0[SBAR_STRING_SIZE];
	char m_SbarString1[SBAR_STRING_SIZE];

	float m_flNextChatTime;

	void SetPrefsFromUserinfo(char* infobuffer);

	int m_iAutoWepSwitch;

	bool m_bRestored;
};

extern bool gInitHUD;
