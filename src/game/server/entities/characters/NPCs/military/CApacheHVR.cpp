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

#include "CApacheHVR.hpp"

LINK_ENTITY_TO_CLASS(hvr_rocket, CApacheHVR);

TYPEDESCRIPTION	CApacheHVR::m_SaveData[] =
{
	//	DEFINE_FIELD( CApacheHVR, m_iTrail, FIELD_INTEGER ),	// Dont' save, precache
		DEFINE_FIELD(CApacheHVR, m_vecForward, FIELD_VECTOR),
};

IMPLEMENT_SAVERESTORE(CApacheHVR, CGrenade);

void CApacheHVR::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetModel("models/HVR.mdl");
	SetSize(vec3_origin, vec3_origin);
	SetAbsOrigin(GetAbsOrigin());

	SetThink(&CApacheHVR::IgniteThink);
	SetTouch(&CApacheHVR::ExplodeTouch);

	UTIL_MakeAimVectors(GetAbsAngles());
	m_vecForward = gpGlobals->v_forward;
	pev->gravity = 0.5;

	pev->nextthink = gpGlobals->time + 0.1;

	pev->dmg = 150;
}

void CApacheHVR::Precache()
{
	PRECACHE_MODEL("models/HVR.mdl");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_SOUND("weapons/rocket1.wav");
}

void CApacheHVR::IgniteThink()
{
	// SetMovetype(Movetype::Toss);

	// SetMovetype(Movetype::Fly);
	pev->effects |= EF_LIGHT;

	// make rocket sound
	EmitSound(SoundChannel::Voice, "weapons/rocket1.wav", VOL_NORM, 0.5);

	// rocket trail
	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex());	// entity
	WRITE_SHORT(m_iTrail);	// model
	WRITE_BYTE(15); // life
	WRITE_BYTE(5);  // width
	WRITE_BYTE(224);   // r, g, b
	WRITE_BYTE(224);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);	// brightness

	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	// set to accelerate
	SetThink(&CApacheHVR::AccelerateThink);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CApacheHVR::AccelerateThink()
{
	// check world boundaries
	if (!UTIL_IsInWorld(GetAbsOrigin()))
	{
		UTIL_Remove(this);
		return;
	}

	// accelerate
	const float flSpeed = GetAbsVelocity().Length();
	if (flSpeed < 1800)
	{
		SetAbsVelocity(GetAbsVelocity() + m_vecForward * 200);
	}

	// re-aim
	SetAbsAngles(VectorAngles(GetAbsVelocity()));

	pev->nextthink = gpGlobals->time + 0.1;
}
