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

/**
*	@file
*
*	frequently used global functions
*/

#include "navigation/nodes.h"
#include "doors.h"

// Landmark class
void CPointEntity::Spawn()
{
	SetSolidType(Solid::Not);
	//	SetSize( vec3_origin, vec3_origin);
}

/**
*	@brief Null Entity, remove on startup
*/
class CNullEntity : public CBaseEntity
{
public:
	void Spawn() override;
};

void CNullEntity::Spawn()
{
	UTIL_RemoveNow(this);
}
LINK_ENTITY_TO_CLASS(info_null, CNullEntity);

void CBaseEntity::UpdateOnRemove()
{
	if (IsBitSet(pev->flags, FL_GRAPHED))
	{
		// this entity was a LinkEnt in the world node graph, so we must remove it from
		// the graph since we are removing it from the world.
		//TODO: still needed? Using EHANDLE should solve this problem
		for (int i = 0; i < WorldGraph.m_cLinks; i++)
		{
			if (WorldGraph.m_pLinkPool[i].m_hLinkEnt == this)
			{
				// if this link has a link ent which is the same ent that is removing itself, remove it!
				WorldGraph.m_pLinkPool[i].m_hLinkEnt = nullptr;
			}
		}
	}
	if (!IsStringNull(pev->globalname))
		gGlobalState.EntitySetState(pev->globalname, GlobalEntState::Dead);
}

void CBaseEntity::SUB_Remove()
{
	UpdateOnRemove();
	if (pev->health > 0)
	{
		// this situation can screw up monsters who can't tell their entity pointers are invalid.
		pev->health = 0;
		ALERT(at_aiconsole, "SUB_Remove called on entity with health > 0\n");
	}

	UTIL_RemoveNow(this);
}

void CBaseEntity::SUB_DoNothing()
{
}

// Global Savedata for Delay
TYPEDESCRIPTION	CBaseDelay::m_SaveData[] =
{
	DEFINE_FIELD(CBaseDelay, m_flDelay, FIELD_FLOAT),
	DEFINE_FIELD(CBaseDelay, m_iszKillTarget, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CBaseDelay, CBaseEntity);

void CBaseDelay::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "delay"))
	{
		m_flDelay = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "killtarget"))
	{
		m_iszKillTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
	{
		CBaseEntity::KeyValue(pkvd);
	}
}

void CBaseEntity::SUB_UseTargets(CBaseEntity* pActivator, UseType useType, float value)
{
	if (!IsStringNull(pev->target))
	{
		FireTargets(STRING(pev->target), pActivator, this, useType, value);
	}
}

void FireTargets(const char* targetName, CBaseEntity* pActivator, CBaseEntity* pCaller, UseType useType, float value)
{
	if (!targetName)
		return;

	ALERT(at_aiconsole, "Firing: (%s)\n", targetName);

	CBaseEntity* pTarget = nullptr;

	while ((pTarget = UTIL_FindEntityByTargetname(pTarget, targetName)) != nullptr)
	{
		if (!(pTarget->pev->flags & FL_KILLME)) // Don't use dying ents
		{
			ALERT(at_aiconsole, "Found: %s, firing (%s)\n", pTarget->GetClassname(), targetName);
			pTarget->Use({pActivator, pCaller, useType, value});
		}
	}
}

LINK_ENTITY_TO_CLASS(DelayedUse, CBaseDelay);

void CBaseDelay::SUB_UseTargets(CBaseEntity* pActivator, UseType useType, float value)
{
	//
	// exit immediately if we don't have a target or kill target
	//
	if (IsStringNull(pev->target) && IsStringNull(m_iszKillTarget))
		return;

	//
	// check for a delay
	//
	if (m_flDelay != 0)
	{
		// create a temp object to fire at a later time
		CBaseDelay* pTemp = GetClassPtr((CBaseDelay*)nullptr);
		pTemp->SetClassname("DelayedUse");

		pTemp->pev->nextthink = gpGlobals->time + m_flDelay;

		pTemp->SetThink(&CBaseDelay::DelayThink);

		// Save the useType
		pTemp->pev->button = (int)useType;
		pTemp->m_iszKillTarget = m_iszKillTarget;
		pTemp->m_flDelay = 0; // prevent "recursion"
		pTemp->pev->target = pev->target;
		//TODO: do this properly
		// HACKHACK
		// This wasn't in the release build of Half-Life.  We should have moved m_hActivator into this class
		// but changing member variable hierarchy would break save/restore without some ugly code.
		// This code is not as ugly as that code
		if (pActivator && pActivator->IsPlayer())		// If a player activates, then save it
		{
			pTemp->SetOwner(pActivator);
		}
		else
		{
			pTemp->SetOwner(nullptr);
		}

		return;
	}

	//
	// kill the killtargets
	//

	if (!IsStringNull(m_iszKillTarget))
	{
		ALERT(at_aiconsole, "KillTarget: %s\n", STRING(m_iszKillTarget));

		CBaseEntity* pKillTarget = nullptr;

		while ((pKillTarget = UTIL_FindEntityByTargetname(pKillTarget, STRING(m_iszKillTarget))) != nullptr)
		{
			UTIL_Remove(pKillTarget);

			//TODO: do this before removing it to be sure it's still a valid pointer
			ALERT(at_aiconsole, "killing %s\n", pKillTarget->GetClassname());
		}
	}

	//
	// fire targets
	//
	if (!IsStringNull(pev->target))
	{
		FireTargets(STRING(pev->target), pActivator, this, useType, value);
	}
}

void CBaseDelay::DelayThink()
{
	CBaseEntity* pActivator = nullptr;

	if (auto owner = GetOwner(); owner)		// A player activated this on delay
	{
		pActivator = owner;
	}
	// The use type is cached (and stashed) in pev->button
	SUB_UseTargets(pActivator, (UseType)pev->button, 0);
	UTIL_RemoveNow(this);
}

// Global Savedata for Toggle
TYPEDESCRIPTION	CBaseToggle::m_SaveData[] =
{
	DEFINE_FIELD(CBaseToggle, m_toggle_state, FIELD_INTEGER),
	DEFINE_FIELD(CBaseToggle, m_flActivateFinished, FIELD_TIME),
	DEFINE_FIELD(CBaseToggle, m_flMoveDistance, FIELD_FLOAT),
	DEFINE_FIELD(CBaseToggle, m_flWait, FIELD_FLOAT),
	DEFINE_FIELD(CBaseToggle, m_flLip, FIELD_FLOAT),
	DEFINE_FIELD(CBaseToggle, m_flTWidth, FIELD_FLOAT),
	DEFINE_FIELD(CBaseToggle, m_flTLength, FIELD_FLOAT),
	DEFINE_FIELD(CBaseToggle, m_vecPosition1, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CBaseToggle, m_vecPosition2, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CBaseToggle, m_vecAngle1, FIELD_VECTOR),		// UNDONE: Position could go through transition, but also angle?
	DEFINE_FIELD(CBaseToggle, m_vecAngle2, FIELD_VECTOR),		// UNDONE: Position could go through transition, but also angle?
	DEFINE_FIELD(CBaseToggle, m_flHeight, FIELD_FLOAT),
	DEFINE_FIELD(CBaseToggle, m_hActivator, FIELD_EHANDLE),
	DEFINE_FIELD(CBaseToggle, m_pfnCallWhenMoveDone, FIELD_FUNCTION),
	DEFINE_FIELD(CBaseToggle, m_vecFinalDest, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CBaseToggle, m_vecFinalAngle, FIELD_VECTOR),
	DEFINE_FIELD(CBaseToggle, m_sMaster, FIELD_STRING),
	DEFINE_FIELD(CBaseToggle, m_bitsDamageInflict, FIELD_INTEGER),	// damage type inflicted
};

IMPLEMENT_SAVERESTORE(CBaseToggle, CBaseAnimating);

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
	else if (AreStringsEqual(pkvd->szKeyName, "master"))
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "distance"))
	{
		m_flMoveDistance = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseDelay::KeyValue(pkvd);
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

bool CBaseToggle::IsLockedByMaster()
{
	return !IsStringNull(m_sMaster) && !UTIL_IsMasterTriggered(m_sMaster, m_hActivator);
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
