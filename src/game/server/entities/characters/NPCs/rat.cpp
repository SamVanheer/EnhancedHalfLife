/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================

/**
*	@brief environmental monster
*/
class CRat : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify() override;
};

LINK_ENTITY_TO_CLASS(monster_rat, CRat);

int	CRat::Classify()
{
	return	CLASS_INSECT;
}

void CRat::SetYawSpeed()
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
	default:
		ys = 45;
		break;
	}

	pev->yaw_speed = ys;
}

void CRat::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/bigrat.mdl");
	UTIL_SetSize(pev, vec3_origin, vec3_origin);

	pev->solid = Solid::SlideBox;
	pev->movetype = Movetype::Step;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->health = 8;
	pev->view_ofs = Vector(0, 0, 6);// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = NPCState::None;

	MonsterInit();
}

void CRat::Precache()
{
	PRECACHE_MODEL("models/bigrat.mdl");
}
