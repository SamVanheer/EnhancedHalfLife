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

#include "CFlyingMonster.hpp"

constexpr int FLYING_AE_FLAP = 8;
constexpr int FLYING_AE_FLAPSOUND = 9;

LocalMoveResult CFlyingMonster::CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, float* pflDist)
{
	// UNDONE: need to check more than the endpoint
	if (IsBitSet(pev->flags, FL_SWIM) && (UTIL_PointContents(vecEnd) != Contents::Water))
	{
		// ALERT(at_aiconsole, "can't swim out of water\n");
		return LocalMoveResult::Invalid;
	}

	TraceResult tr;
	UTIL_TraceHull(vecStart + Vector(0, 0, 32), vecEnd + Vector(0, 0, 32), IgnoreMonsters::No, Hull::Large, this, &tr);

	// ALERT( at_console, "%.0f %.0f %.0f : ", vecStart.x, vecStart.y, vecStart.z );
	// ALERT( at_console, "%.0f %.0f %.0f\n", vecEnd.x, vecEnd.y, vecEnd.z );

	if (pflDist)
	{
		*pflDist = ((tr.vecEndPos - Vector(0, 0, 32)) - vecStart).Length();// get the distance.
	}

	// ALERT( at_console, "check %d %d %f\n", tr.fStartSolid, tr.fAllSolid, tr.flFraction );
	if (tr.fStartSolid || tr.flFraction < 1.0)
	{
		if (pTarget && pTarget == InstanceOrNull(gpGlobals->trace_ent))
			return LocalMoveResult::Valid;
		return LocalMoveResult::Invalid;
	}

	return LocalMoveResult::Valid;
}

bool CFlyingMonster::Triangulate(const Vector& vecStart, const Vector& vecEnd, float flDist, CBaseEntity* pTargetEnt, Vector* pApex)
{
	return CBaseMonster::Triangulate(vecStart, vecEnd, flDist, pTargetEnt, pApex);
}

Activity CFlyingMonster::GetStoppedActivity()
{
	if (GetMovetype() != Movetype::Fly)		// UNDONE: Ground idle here, IDLE may be something else
		return ACT_IDLE;

	return ACT_HOVER;
}

void CFlyingMonster::Stop()
{
	const Activity stopped = GetStoppedActivity();
	if (m_IdealActivity != stopped)
	{
		m_flightSpeed = 0;
		m_IdealActivity = stopped;
	}
	SetAbsAngles({0, GetAbsAngles().y, 0});
	m_vecTravel = vec3_origin;
}

float CFlyingMonster::ChangeYaw(int speed)
{
	if (GetMovetype() == Movetype::Fly)
	{
		const float diff = YawDiff();
		float target = 0;

		if (m_IdealActivity != GetStoppedActivity())
		{
			if (diff < -20)
				target = 90;
			else if (diff > 20)
				target = -90;
		}

		if (m_flLastZYawTime == 0)
		{
			m_flLastZYawTime = gpGlobals->time - gpGlobals->frametime;
		}

		const float delta = std::min(0.25f, gpGlobals->time - m_flLastZYawTime);

		m_flLastZYawTime = gpGlobals->time;

		Vector angles = GetAbsAngles();
		angles.z = UTIL_Approach(target, angles.z, 220.0 * delta);
		SetAbsAngles(angles);
	}
	return CBaseMonster::ChangeYaw(speed);
}

void CFlyingMonster::Killed(const KilledInfo& info)
{
	SetMovetype(Movetype::Step);
	ClearBits(pev->flags, FL_ONGROUND);
	SetAbsAngles({0, GetAbsAngles().y, 0});
	CBaseMonster::Killed(info);
}

void CFlyingMonster::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case FLYING_AE_FLAP:
		m_flightSpeed = 400;
		break;

	case FLYING_AE_FLAPSOUND:
		if (m_pFlapSound)
			EmitSound(SoundChannel::Body, m_pFlapSound);
		break;

	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

void CFlyingMonster::Move(float flInterval)
{
	if (GetMovetype() == Movetype::Fly)
		m_flGroundSpeed = m_flightSpeed;
	CBaseMonster::Move(flInterval);
}

bool CFlyingMonster::ShouldAdvanceRoute(float flWaypointDist)
{
	// Get true 3D distance to the goal so we actually reach the correct height
	if (m_Route[m_iRouteIndex].iType & bits_MF_IS_GOAL)
		flWaypointDist = (m_Route[m_iRouteIndex].vecLocation - GetAbsOrigin()).Length();

	if (flWaypointDist <= 64 + (m_flGroundSpeed * gpGlobals->frametime))
		return true;

	return false;
}

void CFlyingMonster::MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval)
{
	if (GetMovetype() == Movetype::Fly)
	{
		if (gpGlobals->time - m_stopTime > 1.0f)
		{
			if (m_IdealActivity != m_movementActivity)
			{
				m_IdealActivity = m_movementActivity;
				m_flGroundSpeed = m_flightSpeed = 200;
			}
		}
		const Vector vecMove = GetAbsOrigin() + ((vecDir + (m_vecTravel * m_momentum)).Normalize() * (m_flGroundSpeed * flInterval));

		if (m_IdealActivity != m_movementActivity)
		{
			m_flightSpeed = UTIL_Approach(100, m_flightSpeed, 75 * gpGlobals->frametime);
			if (m_flightSpeed < 100)
				m_stopTime = gpGlobals->time;
		}
		else
			m_flightSpeed = UTIL_Approach(20, m_flightSpeed, 300 * gpGlobals->frametime);

		if (CheckLocalMove(GetAbsOrigin(), vecMove, pTargetEnt, nullptr) != LocalMoveResult::Invalid)
		{
			m_vecTravel = (vecMove - GetAbsOrigin()).Normalize();
			UTIL_MoveToOrigin(this, vecMove, (m_flGroundSpeed * flInterval), MoveToOriginType::Strafe);
		}
		else
		{
			m_IdealActivity = GetStoppedActivity();
			m_stopTime = gpGlobals->time;
			m_vecTravel = vec3_origin;
		}
	}
	else
		CBaseMonster::MoveExecute(pTargetEnt, vecDir, flInterval);
}

float CFlyingMonster::CeilingZ(const Vector& position)
{
	const Vector minUp = position;
	Vector maxUp = position;
	maxUp.z += WORLD_BOUNDARY;

	TraceResult tr;
	UTIL_TraceLine(position, maxUp, IgnoreMonsters::Yes, nullptr, &tr);
	if (tr.flFraction != 1.0)
		maxUp.z = tr.vecEndPos.z;

	if ((pev->flags) & FL_SWIM)
	{
		return UTIL_WaterLevel(position, minUp.z, maxUp.z);
	}
	return maxUp.z;
}

bool CFlyingMonster::ProbeZ(const Vector& position, const Vector& probe, float& fraction)
{
	const Contents conPosition = UTIL_PointContents(position);
	if ((((pev->flags) & FL_SWIM) == FL_SWIM) ^ (conPosition == Contents::Water))
	{
		//    SWIMING & !WATER
		// or FLYING  & WATER
		//
		fraction = 0.0;
		return true; // We hit a water boundary because we are where we don't belong.
	}
	const Contents conProbe = UTIL_PointContents(probe);
	if (conProbe == conPosition)
	{
		// The probe is either entirely inside the water (for fish) or entirely
		// outside the water (for birds).
		//
		fraction = 1.0;
		return false;
	}

	const Vector ProbeUnit = (probe - position).Normalize();
	const float ProbeLength = (probe - position).Length();
	float maxProbeLength = ProbeLength;
	float minProbeLength = 0;

	float diff = maxProbeLength - minProbeLength;
	while (diff > 1.0)
	{
		const float midProbeLength = minProbeLength + diff / 2.0;
		const Vector midProbeVec = midProbeLength * ProbeUnit;
		if (UTIL_PointContents(position + midProbeVec) == conPosition)
		{
			minProbeLength = midProbeLength;
		}
		else
		{
			maxProbeLength = midProbeLength;
		}
		diff = maxProbeLength - minProbeLength;
	}
	fraction = minProbeLength / ProbeLength;

	return true;
}

float CFlyingMonster::FloorZ(const Vector& position)
{
	Vector down = position;
	down.z -= 2048;

	TraceResult tr;
	UTIL_TraceLine(position, down, IgnoreMonsters::Yes, nullptr, &tr);

	if (tr.flFraction != 1.0)
		return tr.vecEndPos.z;

	return down.z;
}