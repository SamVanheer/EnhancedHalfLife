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

#include "CFuncRotating.hpp"
#include "CPendulum.hpp"
#include "doors/CBaseDoor.hpp"

void CPendulum::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "distance"))
	{
		m_distance = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "damp"))
	{
		m_damp = atof(pkvd->szValue) * 0.001;
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CPendulum::Spawn()
{
	// set the axis of rotation
	CBaseToggle::AxisDir(this);

	if (IsBitSet(pev->spawnflags, SF_DOOR_PASSABLE))
		SetSolidType(Solid::Not);
	else
		SetSolidType(Solid::BSP);
	SetMovetype(Movetype::Push);
	SetAbsOrigin(GetAbsOrigin());
	SetModel(STRING(pev->model));

	if (m_distance == 0)
		return;

	if (pev->speed == 0)
		pev->speed = 100;

	m_accel = (pev->speed * pev->speed) / (2 * fabs(m_distance));	// Calculate constant acceleration from speed and distance
	m_maxSpeed = pev->speed;
	m_start = GetAbsAngles();
	m_center = GetAbsAngles() + (m_distance * 0.5) * pev->movedir;

	if (IsBitSet(pev->spawnflags, SF_BRUSH_ROTATE_INSTANT))
	{
		SetThink(&CPendulum::SUB_CallUseToggle);
		pev->nextthink = gpGlobals->time + 0.1;
	}
	pev->speed = 0;
	SetUse(&CPendulum::PendulumUse);

	if (IsBitSet(pev->spawnflags, SF_PENDULUM_SWING))
	{
		//TODO Doesn't actually work because this class overrides Touch()
		//Probably doesn't work anyway
		SetTouch(&CPendulum::RopeTouch);
	}
}

void CPendulum::PendulumUse(const UseInfo& info)
{
	if (pev->speed)		// Pendulum is moving, stop it and auto-return if necessary
	{
		if (IsBitSet(pev->spawnflags, SF_PENDULUM_AUTO_RETURN))
		{
			const float delta = CBaseToggle::AxisDelta(pev->spawnflags, GetAbsAngles(), m_start);

			pev->avelocity = m_maxSpeed * pev->movedir;
			pev->nextthink = pev->ltime + (delta / m_maxSpeed);
			SetThink(&CPendulum::Stop);
		}
		else
		{
			pev->speed = 0;		// Dead stop
			SetThink(nullptr);
			pev->avelocity = vec3_origin;
		}
	}
	else
	{
		pev->nextthink = pev->ltime + 0.1;		// Start the pendulum moving
		m_time = gpGlobals->time;		// Save time to calculate dt
		SetThink(&CPendulum::Swing);
		m_dampSpeed = m_maxSpeed;
	}
}

void CPendulum::Stop()
{
	SetAbsAngles(m_start);
	pev->speed = 0;
	SetThink(nullptr);
	pev->avelocity = vec3_origin;
}

void CPendulum::Blocked(CBaseEntity* pOther)
{
	m_time = gpGlobals->time;
}

void CPendulum::Swing()
{
	const float delta = CBaseToggle::AxisDelta(pev->spawnflags, GetAbsAngles(), m_center);
	const float dt = gpGlobals->time - m_time;	// How much time has passed?
	m_time = gpGlobals->time;		// Remember the last time called

	if (delta > 0 && m_accel > 0)
		pev->speed -= m_accel * dt;	// Integrate velocity
	else
		pev->speed += m_accel * dt;

	if (pev->speed > m_maxSpeed)
		pev->speed = m_maxSpeed;
	else if (pev->speed < -m_maxSpeed)
		pev->speed = -m_maxSpeed;
	// scale the destdelta vector by the time spent traveling to get velocity
	pev->avelocity = pev->speed * pev->movedir;

	// Call this again
	pev->nextthink = pev->ltime + 0.1;

	if (m_damp)
	{
		m_dampSpeed -= m_damp * m_dampSpeed * dt;
		if (m_dampSpeed < 30.0)
		{
			SetAbsAngles(m_center);
			pev->speed = 0;
			SetThink(nullptr);
			pev->avelocity = vec3_origin;
		}
		else if (pev->speed > m_dampSpeed)
			pev->speed = m_dampSpeed;
		else if (pev->speed < -m_dampSpeed)
			pev->speed = -m_dampSpeed;
	}
}

void CPendulum::Touch(CBaseEntity* pOther)
{
	if (pev->dmg <= 0)
		return;

	// we can't hurt this thing, so we're not concerned with it
	if (!pOther->pev->takedamage)
		return;

	// calculate damage based on rotation speed
	float damage = pev->dmg * pev->speed * 0.01;

	if (damage < 0)
		damage = -damage;

	pOther->TakeDamage({this, this, damage, DMG_CRUSH});

	pOther->SetAbsVelocity((pOther->GetAbsOrigin() - GetBrushModelOrigin(this)).Normalize() * damage);
}

void CPendulum::RopeTouch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{// not a player!
		ALERT(at_console, "Not a client\n");
		return;
	}

	if (pOther == m_hRopeUser)
	{// this player already on the rope.
		return;
	}

	m_hRopeUser = pOther;
	pOther->SetAbsVelocity(vec3_origin);
	pOther->SetMovetype(Movetype::None);
}
