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

#include "CFurniture.hpp"

void CFurniture::Spawn()
{
	PRECACHE_MODEL(STRING(pev->model));
	SetModel(STRING(pev->model));

	SetMovetype(Movetype::None);
	SetSolidType(Solid::BBox);
	pev->health = 80000;
	SetDamageMode(DamageMode::Aim);
	pev->effects = 0;
	pev->yaw_speed = 0;
	pev->sequence = 0;
	pev->frame = 0;

	//	pev->nextthink += 1.0;
	//	SetThink (WalkMonsterDelay);

	ResetSequenceInfo();
	pev->frame = 0;
	MonsterInit();
}

int CFurniture::Classify()
{
	return	CLASS_NONE;
}
