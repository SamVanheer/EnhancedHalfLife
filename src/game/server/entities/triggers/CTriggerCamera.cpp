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

#include "CTriggerCamera.hpp"
#include "plats/CPathCorner.hpp"			// trigger_camera has train functionality

LINK_ENTITY_TO_CLASS(trigger_camera, CTriggerCamera);

void CTriggerCamera::Spawn()
{
	SetMovetype(Movetype::Noclip);
	SetSolidType(Solid::Not);							// Remove model & collisions
	SetRenderAmount(0);								// The engine won't draw this model if this is set to 0 and blending is on
	SetRenderMode(RenderMode::TransTexture);

	m_initialSpeed = pev->speed;
	if (m_acceleration == 0)
		m_acceleration = 500;
	if (m_deceleration == 0)
		m_deceleration = 500;
}

void CTriggerCamera::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "moveto"))
	{
		m_sPath = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "acceleration"))
	{
		m_acceleration = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "deceleration"))
	{
		m_deceleration = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CTriggerCamera::Use(const UseInfo& info)
{
	if (!ShouldToggle(info.GetUseType(), m_state))
		return;

	auto pActivator = info.GetActivator();

	if ((!pActivator || !pActivator->IsPlayer()) && !g_pGameRules->IsMultiplayer())
	{
		pActivator = UTIL_GetLocalPlayer();
	}

	if (!pActivator)
	{
		return;
	}

	auto player = static_cast<CBasePlayer*>(pActivator);

	// Toggle state
	m_state = !m_state;
	if (!m_state)
	{
		m_flReturnTime = gpGlobals->time;
		return;
	}

	m_hPlayer = player;

	m_flReturnTime = gpGlobals->time + m_flWait;
	pev->speed = m_initialSpeed;
	m_targetSpeed = m_initialSpeed;

	if (IsBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TARGET))
	{
		m_hTarget = m_hPlayer;
	}
	else
	{
		m_hTarget = GetNextTarget();
	}

	// Nothing to look at!
	if (m_hTarget == nullptr)
	{
		return;
	}

	if (IsBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL))
	{
		player->EnableControl(false);
	}

	if (!IsStringNull(m_sPath))
	{
		m_hEntPath = UTIL_FindEntityByTargetname(nullptr, STRING(m_sPath));
	}
	else
	{
		m_hEntPath = nullptr;
	}

	m_flStopTime = gpGlobals->time;
	if (auto path = m_hEntPath.Get(); path)
	{
		if (path->pev->speed != 0)
			m_targetSpeed = path->pev->speed;

		m_flStopTime += path->GetDelay();
	}

	// copy over player information
	if (IsBitSet(pev->spawnflags, SF_CAMERA_PLAYER_POSITION))
	{
		SetAbsOrigin(pActivator->GetAbsOrigin() + pActivator->pev->view_ofs);
		Vector angles = pActivator->GetAbsAngles();
		angles.x = -angles.x;
		angles.z = 0;
		SetAbsAngles(angles);
		SetAbsVelocity(pActivator->GetAbsVelocity());
	}
	else
	{
		SetAbsVelocity(vec3_origin);
	}

	SET_VIEW(pActivator->edict(), edict());

	player->m_hViewEntity = this;

	SetModel(STRING(pActivator->pev->model));

	// follow the player down
	SetThink(&CTriggerCamera::FollowTarget);
	pev->nextthink = gpGlobals->time;

	m_moveDistance = 0;
	Move();
}

void CTriggerCamera::FollowTarget()
{
	auto player = m_hPlayer.Get();

	if (!player)
		return;

	if (m_hTarget == nullptr || m_flReturnTime < gpGlobals->time)
	{
		if (player->IsAlive())
		{
			SET_VIEW(player->edict(), player->edict());
			player->EnableControl(true);
		}

		player->m_hViewEntity = nullptr;
		player->m_bResetViewEntity = false;

		SUB_UseTargets(this, UseType::Toggle, 0);
		pev->avelocity = vec3_origin;
		m_state = false;
		return;
	}

	Vector vecGoal = VectorAngles(m_hTarget->GetAbsOrigin() - GetAbsOrigin());
	vecGoal.x = -vecGoal.x;

	Vector myAngles = GetAbsAngles();

	if (myAngles.y > 360)
		myAngles.y -= 360;

	if (myAngles.y < 0)
		myAngles.y += 360;

	SetAbsAngles(myAngles);

	float dx = vecGoal.x - myAngles.x;
	float dy = vecGoal.y - myAngles.y;

	if (dx < -180)
		dx += 360;
	if (dx > 180)
		dx = dx - 360;

	if (dy < -180)
		dy += 360;
	if (dy > 180)
		dy = dy - 360;

	pev->avelocity.x = dx * 40 * 0.01;
	pev->avelocity.y = dy * 40 * 0.01;

	if (!(IsBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL)))
	{
		SetAbsVelocity(GetAbsVelocity() * 0.8);
		if (GetAbsVelocity().Length() < 10.0)
			SetAbsVelocity(vec3_origin);
	}

	pev->nextthink = gpGlobals->time;

	Move();
}

void CTriggerCamera::Move()
{
	auto path = m_hEntPath.Get();
	// Not moving on a path, return
	if (!path)
		return;

	// Subtract movement from the previous frame
	m_moveDistance -= pev->speed * gpGlobals->frametime;

	// Have we moved enough to reach the target?
	if (m_moveDistance <= 0)
	{
		// Fire the passtarget if there is one
		if (!IsStringNull(path->pev->message))
		{
			FireTargets(STRING(path->pev->message), this, this, UseType::Toggle, 0);
			if (IsBitSet(path->pev->spawnflags, SF_CORNER_FIREONCE))
				path->pev->message = iStringNull;
		}
		// Time to go to the next target
		path = m_hEntPath = path->GetNextTarget();

		// Set up next corner
		if (!path)
		{
			SetAbsVelocity(vec3_origin);
		}
		else
		{
			if (path->pev->speed != 0)
				m_targetSpeed = path->pev->speed;

			const Vector delta = path->GetAbsOrigin() - GetAbsOrigin();
			m_moveDistance = delta.Length();
			pev->movedir = delta.Normalize();
			m_flStopTime = gpGlobals->time + path->GetDelay();
		}
	}

	if (m_flStopTime > gpGlobals->time)
		pev->speed = UTIL_Approach(0, pev->speed, m_deceleration * gpGlobals->frametime);
	else
		pev->speed = UTIL_Approach(m_targetSpeed, pev->speed, m_acceleration * gpGlobals->frametime);

	const float fraction = 2 * gpGlobals->frametime;
	SetAbsVelocity(((pev->movedir * pev->speed) * fraction) + (GetAbsVelocity() * (1 - fraction)));
}
