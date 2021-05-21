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

#include "CBullsquid.hpp"
#include "CSquidSpit.hpp"

LINK_ENTITY_TO_CLASS(squidspit, CSquidSpit);

void CSquidSpit::Spawn()
{
	SetMovetype(Movetype::Fly);
	SetClassname("squidspit");

	SetSolidType(Solid::BBox);
	SetRenderMode(RenderMode::TransAlpha);
	SetRenderAmount(255);

	SetModel("sprites/bigspit.spr");
	pev->frame = 0;
	pev->scale = 0.5;

	SetSize(vec3_origin, vec3_origin);

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
}

void CSquidSpit::Animate()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->frame++)
	{
		if (pev->frame > m_maxFrame)
		{
			pev->frame = 0;
		}
	}
}

void CSquidSpit::Shoot(CBaseEntity* pOwner, const Vector& vecStart, const Vector& vecVelocity)
{
	CSquidSpit* pSpit = GetClassPtr((CSquidSpit*)nullptr);
	pSpit->Spawn();

	pSpit->SetAbsOrigin(vecStart);
	pSpit->SetAbsVelocity(vecVelocity);
	pSpit->SetOwner(pOwner);

	pSpit->SetThink(&CSquidSpit::Animate);
	pSpit->pev->nextthink = gpGlobals->time + 0.1;
}

void CSquidSpit::Touch(CBaseEntity* pOther)
{
	// splat sound
	const int iPitch = RANDOM_FLOAT(90, 110);

	EmitSound(SoundChannel::Voice, "bullchicken/bc_acid1.wav", VOL_NORM, ATTN_NORM, iPitch);

	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EmitSound(SoundChannel::Weapon, "bullchicken/bc_spithit1.wav", VOL_NORM, ATTN_NORM, iPitch);
		break;
	case 1:
		EmitSound(SoundChannel::Weapon, "bullchicken/bc_spithit2.wav", VOL_NORM, ATTN_NORM, iPitch);
		break;
	}

	if (!pOther->pev->takedamage)
	{
		TraceResult tr;
		// make a splat on the wall
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 10, IgnoreMonsters::No, this, &tr);
		UTIL_DecalTrace(&tr, DECAL_SPIT1 + RANDOM_LONG(0, 1));

		// make some flecks
		MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, tr.vecEndPos);
		WRITE_BYTE(TE_SPRITE_SPRAY);
		WRITE_COORD(tr.vecEndPos.x);	// pos
		WRITE_COORD(tr.vecEndPos.y);
		WRITE_COORD(tr.vecEndPos.z);
		WRITE_COORD(tr.vecPlaneNormal.x);	// dir
		WRITE_COORD(tr.vecPlaneNormal.y);
		WRITE_COORD(tr.vecPlaneNormal.z);
		WRITE_SHORT(iSquidSpitSprite);	// model
		WRITE_BYTE(5);			// count
		WRITE_BYTE(30);			// speed
		WRITE_BYTE(80);			// noise ( client will divide by 100 )
		MESSAGE_END();
	}
	else
	{
		pOther->TakeDamage({this, this, gSkillData.bullsquidDmgSpit, DMG_GENERIC});
	}

	SetThink(&CSquidSpit::SUB_Remove);
	pev->nextthink = gpGlobals->time;
}
