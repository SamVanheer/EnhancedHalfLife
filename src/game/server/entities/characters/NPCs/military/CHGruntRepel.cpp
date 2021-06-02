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

#include "CHGruntRepel.hpp"
#include "customentity.hpp"

void CHGruntRepel::Spawn()
{
	Precache();
	SetSolidType(Solid::Not);

	SetUse(&CHGruntRepel::RepelUse);
}

void CHGruntRepel::Precache()
{
	UTIL_PrecacheOther("monster_human_grunt");
	m_iSpriteTexture = PRECACHE_MODEL("sprites/rope.spr");
}

void CHGruntRepel::RepelUse(const UseInfo& info)
{
	TraceResult tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -WORLD_BOUNDARY), IgnoreMonsters::No, this, &tr);
	/*
	if ( tr.pHit && Instance( tr.pHit )->GetSolidType() != Solid::BSP)
		return;
	*/

	CBaseEntity* pEntity = Create("monster_human_grunt", GetAbsOrigin(), GetAbsAngles());
	CBaseMonster* pGrunt = pEntity->MyMonsterPointer();
	pGrunt->SetMovetype(Movetype::Fly);
	pGrunt->SetAbsVelocity(Vector(0, 0, RANDOM_FLOAT(-196, -128)));
	pGrunt->SetActivity(ACT_GLIDE);
	// UNDONE: position?
	pGrunt->m_vecLastPosition = tr.vecEndPos;

	CBeam* pBeam = CBeam::BeamCreate("sprites/rope.spr", 10);
	pBeam->PointEntInit(GetAbsOrigin() + Vector(0, 0, 112), pGrunt->entindex());
	pBeam->SetFlags(BEAM_FSOLID);
	pBeam->SetColor(255, 255, 255);
	pBeam->SetThink(&CBeam::SUB_Remove);
	pBeam->pev->nextthink = gpGlobals->time + -WORLD_BOUNDARY * tr.flFraction / pGrunt->GetAbsVelocity().z + 0.5;

	UTIL_Remove(this);
}
