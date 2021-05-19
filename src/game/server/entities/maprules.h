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

#include "CBaseEntity.hpp"

/**
*	@file
*
*	This module contains entities for implementing/changing game rules dynamically within each map (.BSP)
*/

class CRuleEntity : public CBaseEntity
{
public:
	void	Spawn() override;
	void	KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	void	SetMaster(string_t iszMaster) { m_iszMaster = iszMaster; }

protected:
	bool	CanFireForActivator(CBaseEntity* pActivator);

private:
	string_t m_iszMaster = iStringNull;
};

/**
*	@brief base class for all rule "point" entities (not brushes)
*/
class CRulePointEntity : public CRuleEntity
{
public:
	void		Spawn() override;
};

/**
*	@brief base class for all rule "brush" entities (not brushes)
*	@details Default behavior is to set up like a trigger, invisible, but keep the model for volume testing
*/
class CRuleBrushEntity : public CRuleEntity
{
public:
	void		Spawn() override;
};

constexpr int SF_SCORE_NEGATIVE = 0x0001;
constexpr int SF_SCORE_TEAM = 0x0002;

/**
*	@brief award points to player / team
*	@details Points +/- total
*	Flag: Allow negative scores					SF_SCORE_NEGATIVE
*	Flag: Award points to team in teamplay		SF_SCORE_TEAM
*/
class CGameScore : public CRulePointEntity
{
public:
	void	Spawn() override;
	void	Use(const UseInfo& info) override;
	void	KeyValue(KeyValueData* pkvd) override;

	inline	int		Points() { return pev->frags; }
	inline	bool	AllowNegativeScore() { return (pev->spawnflags & SF_SCORE_NEGATIVE) != 0; }
	inline	bool	AwardToTeam() { return (pev->spawnflags & SF_SCORE_TEAM) != 0; }

	inline	void	SetPoints(int points) { pev->frags = points; }

private:
};

/**
*	@brief Ends the game in MP
*/
class CGameEnd : public CRulePointEntity
{
public:
	void	Use(const UseInfo& info) override;
};

constexpr int SF_ENVTEXT_ALLPLAYERS = 0x0001;

/**
*	@brief NON-Localized HUD Message (use env_message to display a titles.txt message)
*	@details Flag: All players SF_ENVTEXT_ALLPLAYERS
*/
class CGameText : public CRulePointEntity
{
public:
	void	Use(const UseInfo& info) override;
	void	KeyValue(KeyValueData* pkvd) override;
	void Precache() override;
	void Spawn() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	inline	bool	MessageToAll() { return (pev->spawnflags & SF_ENVTEXT_ALLPLAYERS) != 0; }
	inline	void	MessageSet(const char* pMessage) { pev->message = ALLOC_STRING(pMessage); }
	inline	const char* MessageGet() { return STRING(pev->message); }

private:

	hudtextparms_t m_textParms;
};

constexpr int SF_TEAMMASTER_FIREONCE = 0x0001;
constexpr int SF_TEAMMASTER_ANYTEAM = 0x0002;

/**
*	@brief "Masters" like multisource, but based on the team of the activator
*	Only allows mastered entity to fire if the team matches my team
*	@details team index (pulled from server team list "mp_teamlist"
*	Flag: Remove on Fire
*	Flag: Any team until set? -- Any team can use this until the team is set (otherwise no teams can use it)
*/
class CGameTeamMaster : public CRulePointEntity
{
public:
	void		KeyValue(KeyValueData* pkvd) override;
	void		Use(const UseInfo& info) override;
	int			ObjectCaps() override { return CRulePointEntity::ObjectCaps() | FCAP_MASTER; }

	bool		IsTriggered(CBaseEntity* pActivator) override;
	const char* TeamID() override;
	inline bool RemoveOnFire() { return (pev->spawnflags & SF_TEAMMASTER_FIREONCE) != 0; }
	inline bool AnyTeam() { return (pev->spawnflags & SF_TEAMMASTER_ANYTEAM) != 0; }

private:
	bool		TeamMatch(CBaseEntity* pActivator);

	int m_teamIndex = 0;
	UseType	triggerType = UseType::Off;
};

constexpr int SF_TEAMSET_FIREONCE = 0x0001;
constexpr int SF_TEAMSET_CLEARTEAM = 0x0002;

/**
*	@brief Changes the team of the entity it targets to the activator's team
*	@details Flag: Fire once
*	Flag: Clear team -- Sets the team to "NONE" instead of activator
*/
class CGameTeamSet : public CRulePointEntity
{
public:
	void		Use(const UseInfo& info) override;
	inline bool RemoveOnFire() { return (pev->spawnflags & SF_TEAMSET_FIREONCE) != 0; }
	inline bool ShouldClearTeam() { return (pev->spawnflags & SF_TEAMSET_CLEARTEAM) != 0; }
};

/**
*	@brief players in the zone fire my target when I'm fired
*	@details TODO: Needs master?
*/
class CGamePlayerZone : public CRuleBrushEntity
{
public:
	void		KeyValue(KeyValueData* pkvd) override;
	void		Use(const UseInfo& info) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

private:
	string_t m_iszInTarget = iStringNull;
	string_t m_iszOutTarget = iStringNull;
	string_t m_iszInCount = iStringNull;
	string_t m_iszOutCount = iStringNull;
};

constexpr int SF_PKILL_FIREONCE = 0x0001;

/**
*	@brief Damages the player who fires it
*	@details Flag: Fire once
*/
class CGamePlayerHurt : public CRulePointEntity
{
public:
	void		Use(const UseInfo& info) override;
	inline bool RemoveOnFire() { return (pev->spawnflags & SF_PKILL_FIREONCE) != 0; }
};

constexpr int SF_GAMECOUNT_FIREONCE = 0x0001;
constexpr int SF_GAMECOUNT_RESET = 0x0002;

/**
*	@brief Counts events and fires target
*	@details Flag: Fire once
*	Flag: Reset on Fire
*/
class CGameCounter : public CRulePointEntity
{
public:
	void		Spawn() override;
	void		Use(const UseInfo& info) override;
	inline bool RemoveOnFire() { return (pev->spawnflags & SF_GAMECOUNT_FIREONCE) != 0; }
	inline bool ResetOnFire() { return (pev->spawnflags & SF_GAMECOUNT_RESET) != 0; }

	inline void CountUp() { pev->frags++; }
	inline void CountDown() { pev->frags--; }
	inline void ResetCount() { pev->frags = pev->dmg; }
	inline int  CountValue() { return pev->frags; }
	inline int	LimitValue() { return pev->health; }

	inline bool HitLimit() { return CountValue() == LimitValue(); }

private:

	inline void SetCountValue(int value) { pev->frags = value; }
	inline void SetInitialValue(int value) { pev->dmg = value; }
};

constexpr int SF_GAMECOUNTSET_FIREONCE = 0x0001;

/**
*	@brief Sets the counter's value
*	@details Flag: Fire once
*/
class CGameCounterSet : public CRulePointEntity
{
public:
	void		Use(const UseInfo& info) override;
	inline bool RemoveOnFire() { return (pev->spawnflags & SF_GAMECOUNTSET_FIREONCE) != 0; }
};

constexpr int SF_PLAYEREQUIP_USEONLY = 0x0001;
constexpr int MAX_EQUIP = 32;

/**
*	@brief Sets the default player equipment
*	@details Flag: USE Only
*/
class CGamePlayerEquip : public CRulePointEntity
{
public:
	void		KeyValue(KeyValueData* pkvd) override;
	void		Touch(CBaseEntity* pOther) override;
	void		Use(const UseInfo& info) override;

	inline bool	UseOnly() { return (pev->spawnflags & SF_PLAYEREQUIP_USEONLY) != 0; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

private:

	void		EquipPlayer(CBaseEntity* pPlayer);

	string_t m_weaponNames[MAX_EQUIP]{};
	int m_weaponCount[MAX_EQUIP]{};
};

constexpr int SF_PTEAM_FIREONCE = 0x0001;
constexpr int SF_PTEAM_KILL = 0x0002;
constexpr int SF_PTEAM_GIB = 0x0004;

/**
*	@brief Changes the team of the player who fired it
*	@details Flag: Fire once
*	Flag: Kill Player
*	Flag: Gib Player
*/
class CGamePlayerTeam : public CRulePointEntity
{
public:
	void		Use(const UseInfo& info) override;

private:

	inline bool RemoveOnFire() { return (pev->spawnflags & SF_PTEAM_FIREONCE) != 0; }
	inline bool ShouldKillPlayer() { return (pev->spawnflags & SF_PTEAM_KILL) != 0; }
	inline bool ShouldGibPlayer() { return (pev->spawnflags & SF_PTEAM_GIB) != 0; }

	const char* TargetTeamName(const char* pszTargetName);
};
