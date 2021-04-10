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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "corpse.hpp"

LINK_ENTITY_TO_CLASS(bodyque, CCorpse);

void InitBodyQue()
{
	string_t istrClassname = MAKE_STRING("bodyque");

	g_pBodyQueueHead = CREATE_NAMED_ENTITY(istrClassname);
	entvars_t* pev = VARS(g_pBodyQueueHead);

	// Reserve 3 more slots for dead bodies
	for (int i = 0; i < 3; i++)
	{
		pev->owner = CREATE_NAMED_ENTITY(istrClassname);
		pev = VARS(pev->owner);
	}

	pev->owner = g_pBodyQueueHead;
}

void CopyToBodyQue(entvars_t* pev)
{
	if (pev->effects & EF_NODRAW)
		return;

	entvars_t* pevHead = VARS(g_pBodyQueueHead);

	pevHead->angles = pev->angles;
	pevHead->model = pev->model;
	pevHead->modelindex = pev->modelindex;
	pevHead->frame = pev->frame;
	pevHead->colormap = pev->colormap;
	pevHead->movetype = MOVETYPE_TOSS;
	pevHead->velocity = pev->velocity;
	pevHead->flags = 0;
	pevHead->deadflag = pev->deadflag;
	pevHead->renderfx = kRenderFxDeadPlayer;
	pevHead->renderamt = ENTINDEX(ENT(pev));

	pevHead->effects = pev->effects | EF_NOINTERP;
	//pevHead->goalstarttime = pev->goalstarttime;
	//pevHead->goalframe	= pev->goalframe;
	//pevHead->goalendtime = pev->goalendtime ;

	pevHead->sequence = pev->sequence;
	pevHead->animtime = pev->animtime;

	UTIL_SetOrigin(pevHead, pev->origin);
	UTIL_SetSize(pevHead, pev->mins, pev->maxs);
	g_pBodyQueueHead = pevHead->owner;
}

/**
*	@brief Dead HEV suit prop
*/
class CDeadHEV : public CBaseMonster
{
public:
	void Spawn() override;
	int	Classify() override { return CLASS_HUMAN_MILITARY; }

	void KeyValue(KeyValueData* pkvd) override;

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	static constexpr const char* m_szPoses[4] = {"deadback", "deadsitting", "deadstomach", "deadtable"};
};

void CDeadHEV::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(monster_hevsuit_dead, CDeadHEV);

void CDeadHEV::Spawn()
{
	PRECACHE_MODEL("models/player.mdl");
	SET_MODEL(ENT(pev), "models/player.mdl");

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	pev->body = 1;
	m_bloodColor = BLOOD_COLOR_RED;

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);

	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead hevsuit with bad pose\n");
		pev->sequence = 0;
		pev->effects = EF_BRIGHTFIELD;
	}

	// Corpses have less health
	pev->health = 8;

	MonsterInitDead();
}
