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

#include "CBaseToggle.hpp"
#include "doors/CBaseDoor.hpp"

void CBaseToggle::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "lip"))
	{
		m_flLip = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "distance"))
	{
		m_flMoveDistance = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		BaseClass::KeyValue(pkvd);
}

void CBaseToggle::LinearMove(const Vector& vecDest, float flSpeed)
{
	ASSERTSZ(flSpeed != 0, "LinearMove:  no speed is defined!");
	//	ASSERTSZ(m_pfnCallWhenMoveDone != nullptr, "LinearMove: no post-move function defined");

	m_vecFinalDest = vecDest;

	// Already there?
	if (vecDest == GetAbsOrigin())
	{
		LinearMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	const Vector vecDestDelta = vecDest - GetAbsOrigin();

	// divide vector length by speed to get time to reach dest
	const float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to LinearMoveDone when dest is reached
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink(&CBaseToggle::LinearMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	SetAbsVelocity(vecDestDelta / flTravelTime);
}

void CBaseToggle::LinearMoveDone()
{
	const Vector delta = m_vecFinalDest - GetAbsOrigin();
	const float error = delta.Length();
	//If we're more than 1/32th of a unit away from the target position, move us through linear movement.
	//Otherwise snap us to the position immediately.
	if (error > 0.03125)
	{
		LinearMove(m_vecFinalDest, 100);
		return;
	}

	SetAbsOrigin(m_vecFinalDest);
	SetAbsVelocity(vec3_origin);
	pev->nextthink = -1;
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

void CBaseToggle::AngularMove(const Vector& vecDestAngle, float flSpeed)
{
	ASSERTSZ(flSpeed != 0, "AngularMove:  no speed is defined!");
	//	ASSERTSZ(m_pfnCallWhenMoveDone != nullptr, "AngularMove: no post-move function defined");

	m_vecFinalAngle = vecDestAngle;

	// Already there?
	if (vecDestAngle == GetAbsAngles())
	{
		AngularMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	const Vector vecDestDelta = vecDestAngle - GetAbsAngles();

	// divide by speed to get time to reach dest
	const float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to AngularMoveDone when dest is reached
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink(&CBaseToggle::AngularMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	pev->avelocity = vecDestDelta / flTravelTime;
}

void CBaseToggle::AngularMoveDone()
{
	SetAbsAngles(m_vecFinalAngle);
	pev->avelocity = vec3_origin;
	pev->nextthink = -1;
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

float CBaseToggle::AxisValue(int flags, const Vector& angles)
{
	if (IsBitSet(flags, SF_DOOR_ROTATE_Z))
		return angles.z;
	if (IsBitSet(flags, SF_DOOR_ROTATE_X))
		return angles.x;

	return angles.y;
}

void CBaseToggle::AxisDir(CBaseEntity* pEntity)
{
	if (IsBitSet(pEntity->pev->spawnflags, SF_DOOR_ROTATE_Z))
		pEntity->pev->movedir = vec3_up;	// around z-axis
	else if (IsBitSet(pEntity->pev->spawnflags, SF_DOOR_ROTATE_X))
		pEntity->pev->movedir = vec3_forward;	// around x-axis
	else
		pEntity->pev->movedir = vec3_right;		// around y-axis
}

float CBaseToggle::AxisDelta(int flags, const Vector& angle1, const Vector& angle2)
{
	if (IsBitSet(flags, SF_DOOR_ROTATE_Z))
		return angle1.z - angle2.z;

	if (IsBitSet(flags, SF_DOOR_ROTATE_X))
		return angle1.x - angle2.x;

	return angle1.y - angle2.y;
}
