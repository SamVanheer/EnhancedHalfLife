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

#include "CControllerHeadBall.hpp"

LINK_ENTITY_TO_CLASS(controller_head_ball, CControllerHeadBall);

void CControllerHeadBall::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetModel("sprites/xspark4.spr");
	SetRenderMode(RenderMode::TransAdd);
	SetRenderColor({255, 255, 255});
	SetRenderAmount(255);
	pev->scale = 2.0;

	SetSize(vec3_origin, vec3_origin);
	SetAbsOrigin(GetAbsOrigin());

	SetThink(&CControllerHeadBall::HuntThink);
	SetTouch(&CControllerHeadBall::BounceTouch);

	m_vecIdeal = vec3_origin;

	pev->nextthink = gpGlobals->time + 0.1;

	m_hOwner = InstanceOrWorld(pev->owner);
	pev->dmgtime = gpGlobals->time;
}

void CControllerHeadBall::Precache()
{
	PRECACHE_MODEL("sprites/xspark1.spr");
	PRECACHE_SOUND("debris/zap4.wav");
	PRECACHE_SOUND("weapons/electro4.wav");
}

void CControllerHeadBall::HuntThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	SetRenderAmount(GetRenderAmount() - 5);

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(entindex());		// entity, attachment
	WRITE_COORD(GetAbsOrigin().x);		// origin
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	WRITE_COORD(GetRenderAmount() / 16);	// radius
	WRITE_BYTE(255);	// R
	WRITE_BYTE(255);	// G
	WRITE_BYTE(255);	// B
	WRITE_BYTE(2);	// life * 10
	WRITE_COORD(0); // decay
	MESSAGE_END();

	// check world boundaries
	if (gpGlobals->time - pev->dmgtime > 5 || GetRenderAmount() < 64 || m_hEnemy == nullptr || m_hOwner == nullptr || !UTIL_IsInWorld(GetAbsOrigin()))
	{
		SetTouch(nullptr);
		UTIL_Remove(this);
		return;
	}

	MovetoTarget(m_hEnemy->Center());

	if ((m_hEnemy->Center() - GetAbsOrigin()).Length() < 64)
	{
		TraceResult tr;
		UTIL_TraceLine(GetAbsOrigin(), m_hEnemy->Center(), IgnoreMonsters::No, this, &tr);

		if (CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit); pEntity != nullptr && pEntity->pev->takedamage)
		{
			ClearMultiDamage();
			pEntity->TraceAttack({m_hOwner, gSkillData.controllerDmgZap, GetAbsVelocity(), tr, DMG_SHOCK});
			ApplyMultiDamage(this, m_hOwner);
		}

		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMENTPOINT);
		WRITE_SHORT(entindex());
		WRITE_COORD(tr.vecEndPos.x);
		WRITE_COORD(tr.vecEndPos.y);
		WRITE_COORD(tr.vecEndPos.z);
		WRITE_SHORT(g_sModelIndexLaser);
		WRITE_BYTE(0); // frame start
		WRITE_BYTE(10); // framerate
		WRITE_BYTE(3); // life
		WRITE_BYTE(20);  // width
		WRITE_BYTE(0);   // noise
		WRITE_BYTE(255);   // r, g, b
		WRITE_BYTE(255);   // r, g, b
		WRITE_BYTE(255);   // r, g, b
		WRITE_BYTE(255);	// brightness
		WRITE_BYTE(10);		// speed
		MESSAGE_END();

		UTIL_EmitAmbientSound(this, tr.vecEndPos, "weapons/electro4.wav", 0.5, ATTN_NORM, 0, RANDOM_LONG(140, 160));

		m_flNextAttack = gpGlobals->time + 3.0;

		SetThink(&CControllerHeadBall::DieThink);
		pev->nextthink = gpGlobals->time + 0.3;
	}

	// Crawl( );
}

void CControllerHeadBall::DieThink()
{
	UTIL_Remove(this);
}

void CControllerHeadBall::MovetoTarget(Vector vecTarget)
{
	// accelerate
	float flSpeed = m_vecIdeal.Length();
	if (flSpeed == 0)
	{
		m_vecIdeal = GetAbsVelocity();
		flSpeed = m_vecIdeal.Length();
	}

	if (flSpeed > 400)
	{
		m_vecIdeal = m_vecIdeal.Normalize() * 400;
	}
	m_vecIdeal = m_vecIdeal + (vecTarget - GetAbsOrigin()).Normalize() * 100;
	SetAbsVelocity(m_vecIdeal);
}

void CControllerHeadBall::Crawl()
{
	const Vector vecAim = Vector(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1)).Normalize();
	const Vector vecPnt = GetAbsOrigin() + GetAbsVelocity() * 0.3 + vecAim * 64;

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMENTPOINT);
	WRITE_SHORT(entindex());
	WRITE_COORD(vecPnt.x);
	WRITE_COORD(vecPnt.y);
	WRITE_COORD(vecPnt.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); // frame start
	WRITE_BYTE(10); // framerate
	WRITE_BYTE(3); // life
	WRITE_BYTE(20);  // width
	WRITE_BYTE(0);   // noise
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);	// brightness
	WRITE_BYTE(10);		// speed
	MESSAGE_END();
}

void CControllerHeadBall::BounceTouch(CBaseEntity* pOther)
{
	Vector vecDir = m_vecIdeal.Normalize();

	const TraceResult tr = UTIL_GetGlobalTrace();

	const float n = -DotProduct(tr.vecPlaneNormal, vecDir);

	vecDir = 2.0 * tr.vecPlaneNormal * n + vecDir;

	m_vecIdeal = vecDir * m_vecIdeal.Length();
}
