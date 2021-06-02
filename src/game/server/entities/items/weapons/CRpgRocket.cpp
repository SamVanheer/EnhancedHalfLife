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

#include "CRpg.hpp"
#include "CRpgRocket.hpp"

CRpgRocket* CRpgRocket::CreateRpgRocket(const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner, CRpg* pLauncher)
{
	CRpgRocket* pRocket = static_cast<CRpgRocket*>(g_EntityList.Create("rpg_rocket"));

	pRocket->SetAbsOrigin(vecOrigin);
	pRocket->SetAbsAngles(vecAngles);
	pRocket->Spawn();
	pRocket->SetTouch(&CRpgRocket::RocketTouch);
	pRocket->m_hLauncher = pLauncher;// remember what RPG fired me. 
	pLauncher->m_cActiveRockets++;// register this missile as active for the launcher
	pRocket->SetOwner(pOwner);

	return pRocket;
}

void CRpgRocket::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Bounce);
	SetSolidType(Solid::BBox);

	SetModel("models/rpgrocket.mdl");
	SetSize(vec3_origin, vec3_origin);
	SetAbsOrigin(GetAbsOrigin());

	SetThink(&CRpgRocket::IgniteThink);
	SetTouch(&CRpgRocket::ExplodeTouch);

	Vector myAngles = GetAbsAngles();

	myAngles.x -= 30;
	UTIL_MakeVectors(myAngles);
	myAngles.x = -(myAngles.x + 30);
	SetAbsAngles(myAngles);

	SetAbsVelocity(gpGlobals->v_forward * 250);
	pev->gravity = 0.5;

	pev->nextthink = gpGlobals->time + 0.4;

	pev->dmg = gSkillData.plrDmgRPG;
}

void CRpgRocket::RocketTouch(CBaseEntity* pOther)
{
	if (auto launcher = m_hLauncher.Get(); launcher)
	{
		// my launcher is still around, tell it I'm dead.
		launcher->m_cActiveRockets--;
	}

	StopSound(SoundChannel::Voice, "weapons/rocket1.wav");
	ExplodeTouch(pOther);
}

void CRpgRocket::Precache()
{
	PRECACHE_MODEL("models/rpgrocket.mdl");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_SOUND("weapons/rocket1.wav");
}

void CRpgRocket::IgniteThink()
{
	// SetMovetype(Movetype::Toss);

	SetMovetype(Movetype::Fly);
	pev->effects |= EF_LIGHT;

	// make rocket sound
	EmitSound(SoundChannel::Voice, "weapons/rocket1.wav", VOL_NORM, 0.5);

	// rocket trail
	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex());	// entity
	WRITE_SHORT(m_iTrail);	// model
	WRITE_BYTE(40); // life
	WRITE_BYTE(5);  // width
	WRITE_BYTE(224);   // r, g, b
	WRITE_BYTE(224);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);	// brightness

	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	m_flIgniteTime = gpGlobals->time;

	// set to follow laser spot
	SetThink(&CRpgRocket::FollowThink);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CRpgRocket::FollowThink()
{
	UTIL_MakeAimVectors(GetAbsAngles());

	// Examine all entities within a reasonable radius
	CBaseEntity* pOther = nullptr;
	Vector vecTarget = gpGlobals->v_forward;
	float flMax = WORLD_BOUNDARY;
	TraceResult tr;

	while ((pOther = UTIL_FindEntityByClassname(pOther, "laser_spot")) != nullptr)
	{
		UTIL_TraceLine(GetAbsOrigin(), pOther->GetAbsOrigin(), IgnoreMonsters::No, this, &tr);
		// ALERT( at_console, "%f\n", tr.flFraction );
		if (tr.flFraction >= 0.90)
		{
			Vector vecDir = pOther->GetAbsOrigin() - GetAbsOrigin();
			const float flDist = vecDir.Length();
			vecDir = vecDir.Normalize();
			const float flDot = DotProduct(gpGlobals->v_forward, vecDir);
			if ((flDot > 0) && (flDist * (1 - flDot) < flMax))
			{
				flMax = flDist * (1 - flDot);
				vecTarget = vecDir;
			}
		}
	}

	SetAbsAngles(VectorAngles(vecTarget));

	// this acceleration and turning math is totally wrong, but it seems to respond well so don't change it.
	const float flSpeed = GetAbsVelocity().Length();
	if (gpGlobals->time - m_flIgniteTime < 1.0f)
	{
		SetAbsVelocity(GetAbsVelocity() * 0.2 + vecTarget * (flSpeed * 0.8 + 400));
		if (pev->waterlevel == WaterLevel::Head)
		{
			// go slow underwater
			if (GetAbsVelocity().Length() > 300)
			{
				SetAbsVelocity(GetAbsVelocity().Normalize() * 300);
			}
			UTIL_BubbleTrail(GetAbsOrigin() - GetAbsVelocity() * 0.1, GetAbsOrigin(), 4);
		}
		else
		{
			if (GetAbsVelocity().Length() > 2000)
			{
				SetAbsVelocity(GetAbsVelocity().Normalize() * 2000);
			}
		}
	}
	else
	{
		if (pev->effects & EF_LIGHT)
		{
			pev->effects = 0;
			StopSound(SoundChannel::Voice, "weapons/rocket1.wav");
		}
		SetAbsVelocity(GetAbsVelocity() * 0.2 + vecTarget * flSpeed * 0.798);
		if (pev->waterlevel == WaterLevel::Dry && GetAbsVelocity().Length() < 1500)
		{
			Detonate();
		}
	}
	// ALERT( at_console, "%.0f\n", flSpeed );

	pev->nextthink = gpGlobals->time + 0.1;
}
