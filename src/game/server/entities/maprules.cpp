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

#include "maprules.h"

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
	string_t	m_iszMaster;
};

TYPEDESCRIPTION	CRuleEntity::m_SaveData[] =
{
	DEFINE_FIELD(CRuleEntity, m_iszMaster, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CRuleEntity, CBaseEntity);

void CRuleEntity::Spawn()
{
	SetSolidType(Solid::Not);
	pev->movetype = Movetype::None;
	pev->effects = EF_NODRAW;
}

void CRuleEntity::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "master"))
	{
		SetMaster(ALLOC_STRING(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

bool CRuleEntity::CanFireForActivator(CBaseEntity* pActivator)
{
	if (!IsStringNull(m_iszMaster))
	{
		return UTIL_IsMasterTriggered(m_iszMaster, pActivator);
	}

	return true;
}

/**
*	@brief base class for all rule "point" entities (not brushes)
*/
class CRulePointEntity : public CRuleEntity
{
public:
	void		Spawn() override;
};

void CRulePointEntity::Spawn()
{
	CRuleEntity::Spawn();
	pev->frame = 0;
	pev->model = iStringNull;
}

/**
*	@brief base class for all rule "brush" entities (not brushes)
*	@details Default behavior is to set up like a trigger, invisible, but keep the model for volume testing
*/
class CRuleBrushEntity : public CRuleEntity
{
public:
	void		Spawn() override;
};

void CRuleBrushEntity::Spawn()
{
	SetModel(STRING(pev->model));
	CRuleEntity::Spawn();
}

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

LINK_ENTITY_TO_CLASS(game_score, CGameScore);

void CGameScore::Spawn()
{
	CRulePointEntity::Spawn();
}

void CGameScore::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "points"))
	{
		SetPoints(atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else
		CRulePointEntity::KeyValue(pkvd);
}

void CGameScore::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	// Only players can use this
	if (info.GetActivator()->IsPlayer())
	{
		auto player = static_cast<CBasePlayer*>(info.GetActivator());
		if (AwardToTeam())
		{
			player->AddPointsToTeam(Points(), AllowNegativeScore());
		}
		else
		{
			player->AddPoints(Points(), AllowNegativeScore());
		}
	}
}

/**
*	@brief Ends the game in MP
*/
class CGameEnd : public CRulePointEntity
{
public:
	void	Use(const UseInfo& info) override;
};

LINK_ENTITY_TO_CLASS(game_end, CGameEnd);

void CGameEnd::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	g_pGameRules->EndMultiplayerGame();
}

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

	hudtextparms_t	m_textParms;
};

LINK_ENTITY_TO_CLASS(game_text, CGameText);

// Save parms as a block.  Will break save/restore if the structure changes, but this entity didn't ship with Half-Life, so
// it can't impact saved Half-Life games.
TYPEDESCRIPTION	CGameText::m_SaveData[] =
{
	DEFINE_ARRAY(CGameText, m_textParms, FIELD_CHARACTER, sizeof(hudtextparms_t)),
};

IMPLEMENT_SAVERESTORE(CGameText, CRulePointEntity);


void CGameText::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "channel"))
	{
		m_textParms.channel = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "x"))
	{
		m_textParms.x = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "y"))
	{
		m_textParms.y = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "effect"))
	{
		m_textParms.effect = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "color"))
	{
		const auto color = UTIL_StringToIntArray<4>(pkvd->szValue);
		m_textParms.r1 = color[0];
		m_textParms.g1 = color[1];
		m_textParms.b1 = color[2];
		m_textParms.a1 = color[3];
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "color2"))
	{
		const auto color = UTIL_StringToIntArray<4>(pkvd->szValue);
		m_textParms.r2 = color[0];
		m_textParms.g2 = color[1];
		m_textParms.b2 = color[2];
		m_textParms.a2 = color[3];
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "fadein"))
	{
		m_textParms.fadeinTime = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "fadeout"))
	{
		m_textParms.fadeoutTime = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "holdtime"))
	{
		m_textParms.holdTime = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "fxtime"))
	{
		m_textParms.fxTime = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CRulePointEntity::KeyValue(pkvd);
}

void CGameText::Precache()
{
	CRulePointEntity::Precache();

	//Re-allocate the message to handle escape characters
	if (!IsStringNull(pev->message))
	{
		pev->message = ALLOC_ESCAPED_STRING(STRING(pev->message));
	}
}

void CGameText::Spawn()
{
	Precache();

	CRulePointEntity::Spawn();
}

void CGameText::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	if (MessageToAll())
	{
		UTIL_HudMessageAll(m_textParms, MessageGet());
	}
	else
	{
		if (info.GetActivator()->IsNetClient())
		{
			UTIL_HudMessage(static_cast<CBasePlayer*>(info.GetActivator()), m_textParms, MessageGet());
		}
	}
}

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

	int			m_teamIndex;
	UseType	triggerType;
};

LINK_ENTITY_TO_CLASS(game_team_master, CGameTeamMaster);

void CGameTeamMaster::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "teamindex"))
	{
		m_teamIndex = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "triggerstate"))
	{
		triggerType = UTIL_TriggerStateToTriggerType(static_cast<TriggerState>(atoi(pkvd->szValue)));
		pkvd->fHandled = true;
	}
	else
		CRulePointEntity::KeyValue(pkvd);
}

void CGameTeamMaster::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	if (info.GetUseType() == UseType::Set)
	{
		if (info.GetValue() < 0)
		{
			m_teamIndex = -1;
		}
		else
		{
			m_teamIndex = g_pGameRules->GetTeamIndex(info.GetActivator()->TeamID());
		}
		return;
	}

	if (TeamMatch(info.GetActivator()))
	{
		SUB_UseTargets(info.GetActivator(), triggerType, info.GetValue());
		if (RemoveOnFire())
			UTIL_Remove(this);
	}
}

bool CGameTeamMaster::IsTriggered(CBaseEntity* pActivator)
{
	return TeamMatch(pActivator);
}

const char* CGameTeamMaster::TeamID()
{
	if (m_teamIndex < 0)		// Currently set to "no team"
		return "";

	return g_pGameRules->GetIndexedTeamName(m_teamIndex);		// UNDONE: Fill this in with the team from the "teamlist"
}

bool CGameTeamMaster::TeamMatch(CBaseEntity* pActivator)
{
	if (m_teamIndex < 0 && AnyTeam())
		return true;

	if (!pActivator)
		return false;

	return UTIL_TeamsMatch(pActivator->TeamID(), TeamID());
}

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

LINK_ENTITY_TO_CLASS(game_team_set, CGameTeamSet);


void CGameTeamSet::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	if (ShouldClearTeam())
	{
		SUB_UseTargets(info.GetActivator(), UseType::Set, -1);
	}
	else
	{
		SUB_UseTargets(info.GetActivator(), UseType::Set, 0);
	}

	if (RemoveOnFire())
	{
		UTIL_Remove(this);
	}
}

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
	string_t	m_iszInTarget;
	string_t	m_iszOutTarget;
	string_t	m_iszInCount;
	string_t	m_iszOutCount;
};

LINK_ENTITY_TO_CLASS(game_zone_player, CGamePlayerZone);
TYPEDESCRIPTION	CGamePlayerZone::m_SaveData[] =
{
	DEFINE_FIELD(CGamePlayerZone, m_iszInTarget, FIELD_STRING),
	DEFINE_FIELD(CGamePlayerZone, m_iszOutTarget, FIELD_STRING),
	DEFINE_FIELD(CGamePlayerZone, m_iszInCount, FIELD_STRING),
	DEFINE_FIELD(CGamePlayerZone, m_iszOutCount, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CGamePlayerZone, CRuleBrushEntity);

void CGamePlayerZone::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "intarget"))
	{
		m_iszInTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "outtarget"))
	{
		m_iszOutTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "incount"))
	{
		m_iszInCount = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "outcount"))
	{
		m_iszOutCount = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CRuleBrushEntity::KeyValue(pkvd);
}

void CGamePlayerZone::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	int playersInCount = 0;
	int playersOutCount = 0;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		auto pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			TraceResult trace;
			Hull hullNumber;

			hullNumber = Hull::Human;
			if (pPlayer->pev->flags & FL_DUCKING)
			{
				hullNumber = Hull::Head;
			}

			UTIL_TraceModel(pPlayer->pev->origin, pPlayer->pev->origin, hullNumber, this, &trace);

			if (trace.fStartSolid)
			{
				playersInCount++;
				if (!IsStringNull(m_iszInTarget))
				{
					FireTargets(STRING(m_iszInTarget), pPlayer, info.GetActivator(), info.GetUseType(), info.GetValue());
				}
			}
			else
			{
				playersOutCount++;
				if (!IsStringNull(m_iszOutTarget))
				{
					FireTargets(STRING(m_iszOutTarget), pPlayer, info.GetActivator(), info.GetUseType(), info.GetValue());
				}
			}
		}
	}

	if (!IsStringNull(m_iszInCount))
	{
		FireTargets(STRING(m_iszInCount), info.GetActivator(), this, UseType::Set, playersInCount);
	}

	if (!IsStringNull(m_iszOutCount))
	{
		FireTargets(STRING(m_iszOutCount), info.GetActivator(), this, UseType::Set, playersOutCount);
	}
}

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

LINK_ENTITY_TO_CLASS(game_player_hurt, CGamePlayerHurt);


void CGamePlayerHurt::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	if (info.GetActivator()->IsPlayer())
	{
		if (pev->dmg < 0)
			info.GetActivator()->GiveHealth(-pev->dmg, DMG_GENERIC);
		else
			info.GetActivator()->TakeDamage({this, this, pev->dmg, DMG_GENERIC});
	}

	SUB_UseTargets(info.GetActivator(), info.GetUseType(), info.GetValue());

	if (RemoveOnFire())
	{
		UTIL_Remove(this);
	}
}

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

LINK_ENTITY_TO_CLASS(game_counter, CGameCounter);

void CGameCounter::Spawn()
{
	// Save off the initial count
	SetInitialValue(CountValue());
	CRulePointEntity::Spawn();
}

void CGameCounter::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	switch (info.GetUseType())
	{
	case UseType::On:
	case UseType::Toggle:
		CountUp();
		break;

	case UseType::Off:
		CountDown();
		break;

	case UseType::Set:
		SetCountValue((int)info.GetValue());
		break;
	}

	if (HitLimit())
	{
		SUB_UseTargets(info.GetActivator(), UseType::Toggle, 0);
		if (RemoveOnFire())
		{
			UTIL_Remove(this);
		}

		if (ResetOnFire())
		{
			ResetCount();
		}
	}
}

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

LINK_ENTITY_TO_CLASS(game_counter_set, CGameCounterSet);

void CGameCounterSet::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	SUB_UseTargets(info.GetActivator(), UseType::Set, pev->frags);

	if (RemoveOnFire())
	{
		UTIL_Remove(this);
	}
}

constexpr int SF_PLAYEREQUIP_USEONLY = 0x0001;
constexpr int MAX_EQUIP = 32;

/**
*	@brief Sets the default player equipment
*	@details Flag: USE Only
*	TODO: make save game compatible
*/
class CGamePlayerEquip : public CRulePointEntity
{
public:
	void		KeyValue(KeyValueData* pkvd) override;
	void		Touch(CBaseEntity* pOther) override;
	void		Use(const UseInfo& info) override;

	inline bool	UseOnly() { return (pev->spawnflags & SF_PLAYEREQUIP_USEONLY) != 0; }

private:

	void		EquipPlayer(CBaseEntity* pPlayer);

	string_t	m_weaponNames[MAX_EQUIP];
	int			m_weaponCount[MAX_EQUIP];
};

LINK_ENTITY_TO_CLASS(game_player_equip, CGamePlayerEquip);

void CGamePlayerEquip::KeyValue(KeyValueData* pkvd)
{
	CRulePointEntity::KeyValue(pkvd);

	if (!pkvd->fHandled)
	{
		for (int i = 0; i < MAX_EQUIP; i++)
		{
			if (IsStringNull(m_weaponNames[i]))
			{
				char tmp[128];

				UTIL_StripToken(pkvd->szKeyName, tmp);

				m_weaponNames[i] = ALLOC_STRING(tmp);
				m_weaponCount[i] = atoi(pkvd->szValue);
				m_weaponCount[i] = std::max(1, m_weaponCount[i]);
				pkvd->fHandled = true;
				break;
			}
		}
	}
}

void CGamePlayerEquip::Touch(CBaseEntity* pOther)
{
	if (!CanFireForActivator(pOther))
		return;

	if (UseOnly())
		return;

	EquipPlayer(pOther);
}

void CGamePlayerEquip::EquipPlayer(CBaseEntity* pEntity)
{
	CBasePlayer* pPlayer = nullptr;

	if (pEntity->IsPlayer())
	{
		pPlayer = (CBasePlayer*)pEntity;
	}

	if (!pPlayer)
		return;

	for (int i = 0; i < MAX_EQUIP; i++)
	{
		if (IsStringNull(m_weaponNames[i]))
			break;
		for (int j = 0; j < m_weaponCount[i]; j++)
		{
			pPlayer->GiveNamedItem(STRING(m_weaponNames[i]));
		}
	}
}

void CGamePlayerEquip::Use(const UseInfo& info)
{
	EquipPlayer(info.GetActivator());
}

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

LINK_ENTITY_TO_CLASS(game_player_team, CGamePlayerTeam);

const char* CGamePlayerTeam::TargetTeamName(const char* pszTargetName)
{
	CBaseEntity* pTeamEntity = nullptr;

	while ((pTeamEntity = UTIL_FindEntityByTargetname(pTeamEntity, pszTargetName)) != nullptr)
	{
		if (pTeamEntity->ClassnameIs("game_team_master"))
			return pTeamEntity->TeamID();
	}

	return nullptr;
}

void CGamePlayerTeam::Use(const UseInfo& info)
{
	if (!CanFireForActivator(info.GetActivator()))
		return;

	if (info.GetActivator()->IsPlayer())
	{
		const char* pszTargetTeam = TargetTeamName(STRING(pev->target));
		if (pszTargetTeam)
		{
			CBasePlayer* pPlayer = (CBasePlayer*)info.GetActivator();
			g_pGameRules->ChangePlayerTeam(pPlayer, pszTargetTeam, ShouldKillPlayer(), ShouldGibPlayer());
		}
	}

	if (RemoveOnFire())
	{
		UTIL_Remove(this);
	}
}
