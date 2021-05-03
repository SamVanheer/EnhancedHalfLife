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

#include "corpse.hpp"

LINK_ENTITY_TO_CLASS(bodyque, CCorpse);

void InitBodyQue()
{
	string_t istrClassname = MAKE_STRING("bodyque");

	g_pBodyQueueHead = UTIL_CreateNamedEntity(istrClassname);
	auto pEntity = g_pBodyQueueHead;

	// Reserve 3 more slots for dead bodies
	for (int i = 0; i < 3; i++)
	{
		pEntity->SetOwner(UTIL_CreateNamedEntity(istrClassname));
		pEntity = pEntity->GetOwner();
	}

	pEntity->SetOwner(g_pBodyQueueHead);
}

void CopyToBodyQue(CBaseEntity* pEntity)
{
	if (pEntity->pev->effects & EF_NODRAW)
		return;

	g_pBodyQueueHead->pev->angles = pEntity->pev->angles;
	g_pBodyQueueHead->pev->model = pEntity->pev->model;
	g_pBodyQueueHead->pev->modelindex = pEntity->pev->modelindex;
	g_pBodyQueueHead->pev->frame = pEntity->pev->frame;
	g_pBodyQueueHead->pev->colormap = pEntity->pev->colormap;
	g_pBodyQueueHead->SetMovetype(Movetype::Toss);
	g_pBodyQueueHead->pev->velocity = pEntity->pev->velocity;
	g_pBodyQueueHead->pev->flags = 0;
	g_pBodyQueueHead->pev->deadflag = pEntity->pev->deadflag;
	g_pBodyQueueHead->pev->renderfx = RenderFX::DeadPlayer;
	g_pBodyQueueHead->pev->renderamt = pEntity->entindex();

	g_pBodyQueueHead->pev->effects = pEntity->pev->effects | EF_NOINTERP;
	//g_pBodyQueueHead->pev->goalstarttime = pEntity->pev->goalstarttime;
	//g_pBodyQueueHead->pev->goalframe	= pEntity->pev->goalframe;
	//g_pBodyQueueHead->pev->goalendtime = pEntity->pev->goalendtime;

	g_pBodyQueueHead->pev->sequence = pEntity->pev->sequence;
	g_pBodyQueueHead->pev->animtime = pEntity->pev->animtime;

	g_pBodyQueueHead->SetAbsOrigin(pEntity->GetAbsOrigin());
	g_pBodyQueueHead->SetSize(pEntity->pev->mins, pEntity->pev->maxs);
	g_pBodyQueueHead = g_pBodyQueueHead->GetOwner();
}

constexpr int DEADHEV_BODYGROUP_HEAD = 1;
constexpr int DEADHEV_HEAD_HELMETED = 1;

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
	if (AreStringsEqual(pkvd->szKeyName, "pose"))
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
	SetModel("models/player.mdl");

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	SetBodygroup(DEADHEV_BODYGROUP_HEAD, DEADHEV_HEAD_HELMETED);
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
