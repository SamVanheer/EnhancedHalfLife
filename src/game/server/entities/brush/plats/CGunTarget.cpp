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

#include "CGunTarget.hpp"
#include "CPathCorner.hpp"

LINK_ENTITY_TO_CLASS(func_guntarget, CGunTarget);

TYPEDESCRIPTION	CGunTarget::m_SaveData[] =
{
	DEFINE_FIELD(CGunTarget, m_on, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CGunTarget, CBaseMonster);

void CGunTarget::Spawn()
{
	SetSolidType(Solid::BSP);
	SetMovetype(Movetype::Push);

	SetAbsOrigin(GetAbsOrigin());
	SetModel(STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 100;

	// Don't take damage until "on"
	SetDamageMode(DamageMode::No);
	pev->flags |= FL_MONSTER;

	m_on = false;
	pev->max_health = pev->health;

	if (pev->spawnflags & FGUNTARGET_START_ON)
	{
		SetThink(&CGunTarget::Start);
		pev->nextthink = pev->ltime + 0.3;
	}
}

void CGunTarget::Activate()
{
	// now find our next target
	if (CBaseEntity* pTarg = GetNextTarget(); pTarg)
	{
		m_hTargetEnt = pTarg;
		SetAbsOrigin(pTarg->GetAbsOrigin() - (pev->mins + pev->maxs) * 0.5);
	}
}

void CGunTarget::Start()
{
	Use({this, this, UseType::On});
}

void CGunTarget::Next()
{
	SetThink(nullptr);

	m_hTargetEnt = GetNextTarget();
	CBaseEntity* pTarget = m_hTargetEnt;

	if (!pTarget)
	{
		Stop();
		return;
	}
	SetMoveDone(&CGunTarget::Wait);
	LinearMove(pTarget->GetAbsOrigin() - (pev->mins + pev->maxs) * 0.5, pev->speed);
}

void CGunTarget::Wait()
{
	CBaseEntity* pTarget = m_hTargetEnt;

	if (!pTarget)
	{
		Stop();
		return;
	}

	// Fire the pass target if there is one
	if (!IsStringNull(pTarget->pev->message))
	{
		FireTargets(STRING(pTarget->pev->message), this, this, UseType::Toggle, 0);
		if (IsBitSet(pTarget->pev->spawnflags, SF_CORNER_FIREONCE))
			pTarget->pev->message = iStringNull;
	}

	m_flWait = pTarget->GetDelay();

	pev->target = pTarget->pev->target;
	SetThink(&CGunTarget::Next);
	if (m_flWait != 0)
	{// -1 wait will wait forever!		
		pev->nextthink = pev->ltime + m_flWait;
	}
	else
	{
		Next();// do it RIGHT now!
	}
}

void CGunTarget::Stop()
{
	SetAbsVelocity(vec3_origin);
	pev->nextthink = 0;
	SetDamageMode(DamageMode::No);
}

bool	CGunTarget::TakeDamage(const TakeDamageInfo& info)
{
	if (pev->health > 0)
	{
		pev->health -= info.GetDamage();
		if (pev->health <= 0)
		{
			pev->health = 0;
			Stop();
			if (!IsStringNull(pev->message))
				FireTargets(STRING(pev->message), this, this, UseType::Toggle, 0);
		}
	}
	return false;
}

void CGunTarget::Use(const UseInfo& info)
{
	if (!ShouldToggle(info.GetUseType(), m_on))
		return;

	if (m_on)
	{
		Stop();
	}
	else
	{
		SetDamageMode(DamageMode::Aim);
		m_hTargetEnt = GetNextTarget();
		if (m_hTargetEnt == nullptr)
			return;
		pev->health = pev->max_health;
		Next();
	}
}
