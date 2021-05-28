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

#include "CBaseEntity.hpp"
#include "client.hpp"
#include "game.hpp"
#include "dll_functions.hpp"

extern DLL_GLOBAL Vector		g_vecAttackDir;

void CBaseEntity::KeyValue(KeyValueData* pkvd)
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
		pkvd->fHandled = false;
	}
}

// give health
bool CBaseEntity::GiveHealth(float flHealth, int bitsDamageType)
{
	if (!pev->takedamage)
		return false;

	// heal
	if (pev->health >= pev->max_health)
		return false;

	pev->health += flHealth;

	if (pev->health > pev->max_health)
		pev->health = pev->max_health;

	return true;
}

bool CBaseEntity::TakeDamage(const TakeDamageInfo& info)
{
	if (!pev->takedamage)
		return false;

	// UNDONE: some entity types may be immune or resistant to some bitsDamageType

	// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
	// (that is, no actual entity projectile was involved in the attack so use the shooter's origin). 
	Vector vecTemp;
	if (info.GetAttacker() == info.GetInflictor())
	{
		vecTemp = info.GetInflictor()->GetAbsOrigin() - GetBrushModelOrigin(this);
	}
	else
		// an actual missile was involved.
	{
		vecTemp = info.GetInflictor()->GetAbsOrigin() - GetBrushModelOrigin(this);
	}

	// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();

	// save damage based on the target's armor level

	// figure momentum add (don't let hurt brushes or other triggers move player)
	if ((!IsNullEnt(info.GetInflictor())) && (GetMovetype() == Movetype::Walk || GetMovetype() == Movetype::Step) && (info.GetAttacker()->GetSolidType() != Solid::Trigger))
	{
		Vector vecDir = GetAbsOrigin() - (info.GetInflictor()->pev->absmin + info.GetInflictor()->pev->absmax) * 0.5;
		vecDir = vecDir.Normalize();

		float flForce = info.GetDamage() * ((32.0 * 32.0 * 72.0) / (static_cast<double>(pev->size.x) * static_cast<double>(pev->size.y) * pev->size.z)) * 5.0;

		if (flForce > 1000.0)
			flForce = 1000.0;
		SetAbsVelocity(GetAbsVelocity() + vecDir * flForce);
	}

	// do the damage
	pev->health -= info.GetDamage();
	if (pev->health <= 0)
	{
		Killed({info.GetInflictor(), info.GetAttacker(), GibType::Normal});
		return false;
	}

	return true;
}

void CBaseEntity::Killed(const KilledInfo& info)
{
	SetDamageMode(DamageMode::No);
	pev->deadflag = DeadFlag::Dead;
	UTIL_Remove(this);
}

CBaseEntity* CBaseEntity::GetNextTarget()
{
	if (IsStringNull(pev->target))
		return nullptr;
	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
	if (IsNullEnt(pTarget))
		return nullptr;

	return pTarget;
}

bool CBaseEntity::PostRestore()
{
	if (pev->modelindex != 0 && !IsStringNull(pev->model))
	{
		Vector mins, maxs;
		mins = pev->mins;	// Set model is about to destroy these
		maxs = pev->maxs;


		PRECACHE_MODEL(STRING(pev->model));
		SetModel(STRING(pev->model));
		SetSize(mins, maxs);	// Reset them
	}

	return true;
}

void CBaseEntity::SetObjectCollisionBox()
{
	::SetObjectCollisionBox(pev);
}

bool CBaseEntity::Intersects(CBaseEntity* pOther)
{
	if (pOther->pev->absmin.x > pev->absmax.x ||
		pOther->pev->absmin.y > pev->absmax.y ||
		pOther->pev->absmin.z > pev->absmax.z ||
		pOther->pev->absmax.x < pev->absmin.x ||
		pOther->pev->absmax.y < pev->absmin.y ||
		pOther->pev->absmax.z < pev->absmin.z)
		return false;
	return true;
}

void CBaseEntity::MakeDormant()
{
	SetBits(pev->flags, FL_DORMANT);

	// Don't touch
	SetSolidType(Solid::Not);
	// Don't move
	SetMovetype(Movetype::None);
	// Don't draw
	SetBits(pev->effects, EF_NODRAW);
	// Don't think
	pev->nextthink = 0;
	// Relink
	SetAbsOrigin(GetAbsOrigin());
}

bool CBaseEntity::IsDormant()
{
	return IsBitSet(pev->flags, FL_DORMANT);
}

bool CBaseEntity::IsInWorld()
{
	// position 
	if (!UTIL_IsInWorld(GetAbsOrigin()))
	{
		return false;
	}

	// speed
	if (GetAbsVelocity().x >= 2000) return false;
	if (GetAbsVelocity().y >= 2000) return false;
	if (GetAbsVelocity().z >= 2000) return false;
	if (GetAbsVelocity().x <= -2000) return false;
	if (GetAbsVelocity().y <= -2000) return false;
	if (GetAbsVelocity().z <= -2000) return false;

	return true;
}

bool CBaseEntity::ShouldToggle(UseType useType, bool currentState)
{
	if (useType != UseType::Toggle && useType != UseType::Set)
	{
		if ((currentState && useType == UseType::On) || (!currentState && useType == UseType::Off))
			return false;
	}
	return true;
}

int	CBaseEntity::DamageDecal(int bitsDamageType)
{
	if (GetRenderMode() == RenderMode::TransAlpha)
		return -1;

	if (GetRenderMode() != RenderMode::Normal)
		return DECAL_BPROOF1;

	return DECAL_GUNSHOT1 + RANDOM_LONG(0, 4);
}

void CBaseEntity::EmitSound(SoundChannel channel, const char* fileName, float volume, float attenuation, int pitch, int flags)
{
	EMIT_SOUND_DYN(this, channel, fileName, volume, attenuation, flags, pitch);
}

void CBaseEntity::StopSound(SoundChannel channel, const char* fileName)
{
	EMIT_SOUND_DYN(this, channel, fileName, 0, 0, SND_STOP, PITCH_NORM);
}

CBaseEntity* CBaseEntity::Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner, bool callSpawn)
{
	auto pEntity = UTIL_CreateNamedEntity(MAKE_STRING(szName));
	if (IsNullEnt(pEntity))
	{
		ALERT(at_console, "NULL Ent in Create!\n");
		return nullptr;
	}

	pEntity->SetOwner(pOwner);
	pEntity->SetAbsOrigin(vecOrigin);
	pEntity->SetAbsAngles(vecAngles);

	if (callSpawn)
	{
		DispatchSpawn(pEntity->edict());
	}

	return pEntity;
}

void CBaseEntity::SetAbsOrigin(const Vector& origin)
{
	if (auto ent = edict(); ent)
	{
		g_engfuncs.pfnSetOrigin(ent, origin);
	}
}

void CBaseEntity::SetSize(const Vector& mins, const Vector& maxs)
{
	g_engfuncs.pfnSetSize(edict(), mins, maxs);
}
