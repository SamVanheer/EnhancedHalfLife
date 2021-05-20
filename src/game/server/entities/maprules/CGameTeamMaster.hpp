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

#include "CRulePointEntity.hpp"

constexpr int SF_TEAMMASTER_FIREONCE = 0x0001;
constexpr int SF_TEAMMASTER_ANYTEAM = 0x0002;

/**
*	@brief "Masters" like multisource, but based on the team of the activator
*	Only allows mastered entity to fire if the team matches my team
*	@details team index (pulled from server team list "mp_teamlist"
*	Flag: Remove on Fire
*	Flag: Any team until set? -- Any team can use this until the team is set (otherwise no teams can use it)
*/
class EHL_CLASS() CGameTeamMaster : public CRulePointEntity
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
