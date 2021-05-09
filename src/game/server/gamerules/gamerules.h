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

#include <string_view>

class CBaseItem;
class CBaseWeapon;
class CBasePlayer;

/**
*	@brief weapon respawning return codes
*/
enum
{
	GR_NONE = 0,

	GR_PLR_DROP_GUN_ALL,
	GR_PLR_DROP_GUN_ACTIVE,
	GR_PLR_DROP_GUN_NO,

	GR_PLR_DROP_AMMO_ALL,
	GR_PLR_DROP_AMMO_ACTIVE,
	GR_PLR_DROP_AMMO_NO,
};

/**
*	@brief Player relationship return codes
*/
enum
{
	GR_NOTTEAMMATE = 0,
	GR_TEAMMATE,
	GR_ENEMY,
	GR_ALLY,
	GR_NEUTRAL,
};

class CGameRules
{
public:
	/**
	*	@brief fill skill data struct with proper values
	*/
	virtual void RefreshSkillData();

	/**
	*	@brief runs every server frame, should handle any timer tasks, periodic events, etc.
	*/
	virtual void Think() = 0;

	/**
	*	@brief Can this item spawn (eg monsters don't spawn in deathmatch).
	*/
	virtual bool IsAllowedToSpawn(CBaseEntity* pEntity) = 0;

	/**
	*	@brief Are players allowed to switch on their flashlight?
	*/
	virtual bool AllowFlashlight() = 0;

	/**
	*	@brief should the player switch to this weapon?
	*/
	virtual bool ShouldSwitchWeapon(CBasePlayer* pPlayer, CBaseWeapon* pWeapon) = 0;

	/**
	*	@brief I can't use this weapon anymore, get me the next best one.
	*/
	virtual bool GetNextBestWeapon(CBasePlayer* pPlayer, CBaseWeapon* pCurrentWeapon) = 0;

	// Functions to verify the single/multiplayer status of a game
	virtual bool IsMultiplayer() = 0;			//!< is this a multiplayer game? (either coop or deathmatch)
	virtual bool IsDeathmatch() = 0;			//!< is this a deathmatch game?
	virtual bool IsTeamplay() { return false; }	//!< is this deathmatch game being played with team rules?
	virtual bool IsCoOp() = 0;					//!< is this a coop game?

	/**
	*	@brief Are save games supported in this game mode?
	*/
	virtual bool AreSaveGamesSupported() const = 0;

	/**
	*	@brief Can mapper-placed changelevels change the map?
	*/
	virtual bool AreChangeLevelsAllowed() const = 0;

	/**
	*	@brief this is the game name that gets seen in the server browser
	*/
	virtual const char* GetGameDescription() { return GAME_NAME.data(); }

	// Client connection/disconnection
		/**
		*	@brief a client just connected to the server (player hasn't spawned yet)
		*	@details If ClientConnected returns false, the connection is rejected and the user is provided the reason specified in szRejectReason
		*	Only the client's name and remote address are provided to the dll for verification.
		*/
	virtual bool ClientConnected(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128]) = 0;

	/**
	*	@brief the client dll is ready for updating
	*/
	virtual void InitHUD(CBasePlayer* pl) = 0;

	/**
	*	@brief a client just disconnected from the server
	*/
	virtual void ClientDisconnected(edict_t* pClient) = 0;

	/**
	*	@brief the client needs to be informed of the current game mode
	*/
	virtual void UpdateGameMode(CBasePlayer* pPlayer) {}

	// Client damage rules
		/**
		*	@brief this client just hit the ground after a fall. How much damage?
		*/
	virtual float PlayerFallDamage(CBasePlayer* pPlayer) = 0;

	/**
	*	@brief can this player take damage from this attacker?
	*/
	virtual bool  PlayerCanTakeDamage(CBasePlayer* pPlayer, CBaseEntity* pAttacker) { return true; }
	virtual bool ShouldAutoAim(CBasePlayer* pPlayer, CBaseEntity* target) { return true; }

	// Client spawn/respawn control
	/**
	*	@brief called by CBasePlayer::Spawn just before releasing player into the game
	*/
	virtual void PlayerSpawn(CBasePlayer* pPlayer) = 0;

	/**
	*	@brief called by CBasePlayer::PreThink every frame, before physics are run and after keys are accepted
	*/
	virtual void PlayerThink(CBasePlayer* pPlayer) = 0;

	/**
	*	@brief is this player allowed to respawn now?
	*/
	virtual bool PlayerCanRespawn(CBasePlayer* pPlayer) = 0;

	/**
	*	@brief When in the future will this player be able to spawn?
	*/
	virtual float PlayerSpawnTime(CBasePlayer* pPlayer) = 0;

	/**
	*	@brief Place this player on their spawnspot and face them the proper direction.
	*/
	virtual CBaseEntity* GetPlayerSpawnSpot(CBasePlayer* pPlayer);

	virtual bool AllowAutoTargetCrosshair() { return true; }

	/**
	*	@brief the user has typed a command which is unrecognized by everything else; this check to see if the gamerules knows anything about the command
	*	@return true if command handled properly
	*/
	virtual bool ClientCommand(CBasePlayer* pPlayer, const char* pcmd) { return false; }

	/**
	*	@brief the player has changed userinfo;  can change it now
	*/
	virtual void ClientUserInfoChanged(CBasePlayer* pPlayer, char* infobuffer) {}

	// Client kills/scoring
		/**
		*	@brief how many points do I award whoever kills this player?
		*/
	virtual int PointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled) = 0;

	/**
	*	@brief Called each time a player dies
	*/
	virtual void PlayerKilled(CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* pInflictor) = 0;

	/**
	*	@brief Call this from within a GameRules class to report an obituary.
	*/
	virtual void DeathNotice(CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* pInflictor) = 0;

	//Item spawn/respawn control
	/**
	*	@brief is this player allowed to take this item?
	*/
	virtual bool CanHaveItem(CBasePlayer& player, CBaseItem& item);

	/**
	*	@brief call each time a player picks up an item (battery, ammo, weapon)
	*/
	virtual void PlayerGotItem(CBasePlayer& player, CBaseItem& item) = 0;

	/**
	*	@brief Should this item respawn?
	*/
	virtual bool ItemShouldRespawn(CBaseItem& item) = 0;

	/**
	*	@brief when may this item respawn?
	*/
	virtual float ItemRespawnTime(CBaseItem& item) = 0;

	/**
	*	@brief where in the world should this item respawn?
	*	@details Some game variations may choose to randomize spawn locations
	*/
	virtual Vector ItemRespawnSpot(CBaseItem& item);

	/**
	*	@brief can i respawn now, and if not, when should i try again?
	*	Returns 0 if the item can respawn now
	*/
	virtual float ItemTryRespawn(CBaseItem& item) = 0;

	// Ammo retrieval
		/**
		*	@brief
		*/
	virtual bool CanHaveAmmo(CBasePlayer* pPlayer, const char* pszAmmoName, int iMaxCarry);// can this player take more of this ammo?

	/**
	*	@brief
	*/
	virtual void PlayerGotAmmo(CBasePlayer* pPlayer, char* szName, int iCount) = 0;// called each time a player picks up some ammo in the world

	// Healthcharger respawn control
		/**
		*	@brief how long until a depleted HealthCharger recharges itself?
		*/
	virtual float HealthChargerRechargeTime() = 0;

	/**
	*	@brief how long until a depleted HealthCharger recharges itself?
	*/
	virtual float HEVChargerRechargeTime() { return -1; }

	/**
	*	@brief What happens to a dead player's weapons. what do I do with a player's weapons when he's killed?
	*/
	virtual int DeadPlayerWeapons(CBasePlayer* pPlayer) = 0;

	/**
	*	@brief What happens to a dead player's ammo. Do I drop ammo when the player dies? How much?
	*/
	virtual int DeadPlayerAmmo(CBasePlayer* pPlayer) = 0;

	// Teamplay stuff
		/**
		*	@brief what team is this entity on?
		*/
	virtual const char* GetTeamID(CBaseEntity* pEntity) = 0;

	/**
	*	@brief What is the player's relationship with this entity?
	*/
	virtual int PlayerRelationship(CBaseEntity* pPlayer, CBaseEntity* pTarget) = 0;
	virtual int GetTeamIndex(std::string_view teamName) { return -1; }
	virtual const char* GetIndexedTeamName(int teamIndex) { return ""; }
	virtual bool IsValidTeam(const char* pTeamName) { return true; }
	virtual void ChangePlayerTeam(CBasePlayer* pPlayer, const char* pTeamName, bool bKill, bool bGib) {}
	virtual const char* SetDefaultPlayerTeam(CBasePlayer* pPlayer) { return ""; }

	// Sounds
	virtual bool PlayTextureSounds() { return true; }
	virtual bool PlayFootstepSounds(CBasePlayer* pl, float fvol) { return true; }

	// Monsters
	/**
	*	@brief are monsters allowed
	*/
	virtual bool AllowMonsters() = 0;

	/**
	*	@brief Immediately end a multiplayer game
	*/
	virtual void EndMultiplayerGame() {}

	/**
	*	@brief Output to the server log file
	*/
	virtual void LogPrintf(CBaseEntity* player, const char* format, ...);
};

/**
*	@brief instantiate the proper game rules object
*/
CGameRules* InstallGameRules();

/**
*	@brief rules for the single player Half-Life game.
*/
class CHalfLifeRules : public CGameRules
{
public:
	CHalfLifeRules();

	void Think() override;
	bool IsAllowedToSpawn(CBaseEntity* pEntity) override;
	bool AllowFlashlight() override { return true; }

	bool ShouldSwitchWeapon(CBasePlayer* pPlayer, CBaseWeapon* pWeapon) override;
	bool GetNextBestWeapon(CBasePlayer* pPlayer, CBaseWeapon* pCurrentWeapon) override;

	bool IsMultiplayer() override;
	bool IsDeathmatch() override;
	bool IsCoOp() override;

	bool AreSaveGamesSupported() const override { return true; }
	bool AreChangeLevelsAllowed() const override { return true; }

	bool ClientConnected(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128]) override;
	void InitHUD(CBasePlayer* pl) override;
	void ClientDisconnected(edict_t* pClient) override;

	float PlayerFallDamage(CBasePlayer* pPlayer) override;

	void PlayerSpawn(CBasePlayer* pPlayer) override;
	void PlayerThink(CBasePlayer* pPlayer) override;
	bool PlayerCanRespawn(CBasePlayer* pPlayer) override;
	float PlayerSpawnTime(CBasePlayer* pPlayer) override;

	bool AllowAutoTargetCrosshair() override;

	int PointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled) override;
	void PlayerKilled(CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* pInflictor) override;
	void DeathNotice(CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* pInflictor) override;

	void PlayerGotItem(CBasePlayer& player, CBaseItem& item) override;
	bool ItemShouldRespawn(CBaseItem& item) override;
	float ItemRespawnTime(CBaseItem& item) override;
	float ItemTryRespawn(CBaseItem& item) override;

	void PlayerGotAmmo(CBasePlayer* pPlayer, char* szName, int iCount) override;

	float HealthChargerRechargeTime() override;

	int DeadPlayerWeapons(CBasePlayer* pPlayer) override;

	int DeadPlayerAmmo(CBasePlayer* pPlayer) override;

	bool AllowMonsters() override;

	const char* GetTeamID(CBaseEntity* pEntity) override { return ""; }
	int PlayerRelationship(CBaseEntity* pPlayer, CBaseEntity* pTarget) override;
};

/**
*	@brief rules for the basic half life multiplayer competition
*/
class CHalfLifeMultiplay : public CGameRules
{
public:
	CHalfLifeMultiplay();

	void Think() override;
	void RefreshSkillData() override;
	bool IsAllowedToSpawn(CBaseEntity* pEntity) override;
	bool AllowFlashlight() override;

	bool ShouldSwitchWeapon(CBasePlayer* pPlayer, CBaseWeapon* pWeapon) override;
	bool GetNextBestWeapon(CBasePlayer* pPlayer, CBaseWeapon* pCurrentWeapon) override;

	bool IsMultiplayer() override;
	bool IsDeathmatch() override;
	bool IsCoOp() override;

	bool AreSaveGamesSupported() const override { return false; }
	bool AreChangeLevelsAllowed() const override { return false; }

	bool ClientConnected(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128]) override;
	void InitHUD(CBasePlayer* pl) override;
	void ClientDisconnected(edict_t* pClient) override;
	void UpdateGameMode(CBasePlayer* pPlayer) override;

	float PlayerFallDamage(CBasePlayer* pPlayer) override;
	bool  PlayerCanTakeDamage(CBasePlayer* pPlayer, CBaseEntity* pAttacker) override;

	void PlayerSpawn(CBasePlayer* pPlayer) override;
	void PlayerThink(CBasePlayer* pPlayer) override;
	bool PlayerCanRespawn(CBasePlayer* pPlayer) override;
	float PlayerSpawnTime(CBasePlayer* pPlayer) override;
	CBaseEntity* GetPlayerSpawnSpot(CBasePlayer* pPlayer) override;

	bool AllowAutoTargetCrosshair() override;
	bool ClientCommand(CBasePlayer* pPlayer, const char* pcmd) override;
	void ClientUserInfoChanged(CBasePlayer* pPlayer, char* infobuffer) override;

	int PointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled) override;
	void PlayerKilled(CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* pInflictor) override;
	void DeathNotice(CBasePlayer* pVictim, CBaseEntity* pKiller, CBaseEntity* pInflictor) override;

	bool CanHaveItem(CBasePlayer& player, CBaseItem& item) override;
	void PlayerGotItem(CBasePlayer& player, CBaseItem& item) override;
	bool ItemShouldRespawn(CBaseItem& item) override;
	float ItemRespawnTime(CBaseItem& item) override;
	float ItemTryRespawn(CBaseItem& item) override;

	void PlayerGotAmmo(CBasePlayer* pPlayer, char* szName, int iCount) override;

	float HealthChargerRechargeTime() override;
	float HEVChargerRechargeTime() override;

	int DeadPlayerWeapons(CBasePlayer* pPlayer) override;

	int DeadPlayerAmmo(CBasePlayer* pPlayer) override;

	const char* GetTeamID(CBaseEntity* pEntity) override { return ""; }
	int PlayerRelationship(CBaseEntity* pPlayer, CBaseEntity* pTarget) override;

	bool PlayTextureSounds() override { return false; }
	bool PlayFootstepSounds(CBasePlayer* pl, float fvol) override;

	bool AllowMonsters() override;

	void EndMultiplayerGame() override { GoToIntermission(); }

protected:
	/**
	*	@brief Server is changing to a new level, check mapcycle.txt for map name and setup info
	*/
	virtual void ChangeLevel();
	virtual void GoToIntermission();
	float m_flIntermissionEndTime;
	bool m_iEndIntermissionButtonHit;
	void SendMOTDToClient(CBasePlayer* player);
};

inline CGameRules* g_pGameRules = nullptr;
