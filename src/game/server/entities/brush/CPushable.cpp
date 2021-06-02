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

#include "CPushable.hpp"

const char* CPushable::m_soundNames[3] = {"debris/pushbox1.wav", "debris/pushbox2.wav", "debris/pushbox3.wav"};

void CPushable::Spawn()
{
	if (pev->spawnflags & SF_PUSH_BREAKABLE)
		CBreakable::Spawn();
	else
		Precache();

	SetMovetype(Movetype::PushStep);
	SetSolidType(Solid::BBox);
	SetModel(STRING(pev->model));

	if (pev->friction > 399)
		pev->friction = 399;

	m_maxSpeed = 400 - pev->friction;
	SetBits(pev->flags, FL_FLOAT);
	pev->friction = 0;

	// Pick up off of the floor
	SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, 1));

	// Multiply by area of the box's cross-section (assume 1000 units^3 standard volume)
	pev->skin = (pev->skin * (pev->maxs.x - pev->mins.x) * (pev->maxs.y - pev->mins.y)) * 0.0005;
	m_soundTime = 0;
}

void CPushable::Precache()
{
	for (int i = 0; i < 3; i++)
		PRECACHE_SOUND(m_soundNames[i]);

	if (pev->spawnflags & SF_PUSH_BREAKABLE)
		CBreakable::Precache();
}

void CPushable::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "size"))
	{
		const Hull bbox = static_cast<Hull>(atoi(pkvd->szValue));
		pkvd->fHandled = true;

		switch (bbox)
		{
		case Hull::Point:
			SetSize(Vector(-8, -8, -8), Vector(8, 8, 8));
			break;

		case Hull::Large:
			SetSize(VEC_DUCK_HULL_MIN * 2, VEC_DUCK_HULL_MAX * 2);
			break;

		case Hull::Head:
			SetSize(VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
			break;

		default:
		case Hull::Human:
			SetSize(VEC_HULL_MIN, VEC_HULL_MAX);
			break;
		}
	}
	else if (AreStringsEqual(pkvd->szKeyName, "buoyancy"))
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBreakable::KeyValue(pkvd);
}

void CPushable::Use(const UseInfo& info)
{
	if (!info.GetActivator() || !info.GetActivator()->IsPlayer())
	{
		if (pev->spawnflags & SF_PUSH_BREAKABLE)
			this->CBreakable::Use(info);
		return;
	}

	if (info.GetActivator()->GetAbsVelocity() != vec3_origin)
		Move(info.GetActivator(), false);
}

void CPushable::Touch(CBaseEntity* pOther)
{
	if (pOther->ClassnameIs("worldspawn"))
		return;

	Move(pOther, true);
}

void CPushable::Move(CBaseEntity* pOther, bool push)
{

	// Is entity standing on this pushable ?
	if (IsBitSet(pOther->pev->flags, FL_ONGROUND) && InstanceOrNull(pOther->pev->groundentity) == this)
	{
		// Only push if floating
		if (pev->waterlevel > WaterLevel::Dry)
			SetAbsVelocity(GetAbsVelocity() + Vector(0, 0, pOther->GetAbsVelocity().z * 0.1));

		return;
	}

	bool playerTouch = false;
	if (pOther->IsPlayer())
	{
		if (push && !(pOther->pev->button & (IN_FORWARD | IN_USE)))	// Don't push unless the player is pushing forward and NOT use (pull)
			return;
		playerTouch = true;
	}

	float factor;

	if (playerTouch)
	{
		if (!(pOther->pev->flags & FL_ONGROUND))	// Don't push away from jumping/falling players unless in water
		{
			if (pev->waterlevel < WaterLevel::Feet)
				return;
			else
				factor = 0.1;
		}
		else
			factor = 1;
	}
	else
		factor = 0.25;

	Vector newVelocity = GetAbsVelocity();

	newVelocity.x += pOther->GetAbsVelocity().x * factor;
	newVelocity.y += pOther->GetAbsVelocity().y * factor;

	const float length = newVelocity.Length2D();
	if (push && (length > MaxSpeed()))
	{
		newVelocity.x = newVelocity.x * MaxSpeed() / length;
		newVelocity.y = newVelocity.y * MaxSpeed() / length;
	}

	SetAbsVelocity(newVelocity);

	if (playerTouch)
	{
		pOther->SetAbsVelocity({newVelocity.x, newVelocity.y, pOther->GetAbsVelocity().z});
		if ((gpGlobals->time - m_soundTime) > 0.7f)
		{
			m_soundTime = gpGlobals->time;
			if (length > 0 && IsBitSet(pev->flags, FL_ONGROUND))
			{
				m_lastSound = RANDOM_LONG(0, 2);
				EmitSound(SoundChannel::Weapon, m_soundNames[m_lastSound], 0.5);
				//			SetThink( &CPushable::StopMovementSound );
				//			pev->nextthink = pev->ltime + 0.1;
			}
			else
				StopSound(SoundChannel::Weapon, m_soundNames[m_lastSound]);
		}
	}
}

#if 0
void CPushable::StopMovementSound()
{
	Vector dist = pev->oldorigin - GetAbsOrigin();
	if (dist.Length() <= 0)
		StopSound(SoundChannel::Weapon, m_soundNames[m_lastSound]);
}
#endif

bool CPushable::TakeDamage(const TakeDamageInfo& info)
{
	if (pev->spawnflags & SF_PUSH_BREAKABLE)
		return CBreakable::TakeDamage(info);

	return true;
}
