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
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"client.h"
#include	"decals.hpp"
#include	"gamerules.h"
#include	"game.h"
#include "dll_functions.hpp"

extern DLL_GLOBAL Vector		g_vecAttackDir;

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
		vecTemp = info.GetInflictor()->pev->origin - (GetBrushModelOrigin(pev));
	}
	else
		// an actual missile was involved.
	{
		vecTemp = info.GetInflictor()->pev->origin - (GetBrushModelOrigin(pev));
	}

	// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();

	// save damage based on the target's armor level

	// figure momentum add (don't let hurt brushes or other triggers move player)
	if ((!IsNullEnt(info.GetInflictor())) && (pev->movetype == Movetype::Walk || pev->movetype == Movetype::Step) && (info.GetAttacker()->pev->solid != Solid::Trigger))
	{
		Vector vecDir = pev->origin - (info.GetInflictor()->pev->absmin + info.GetInflictor()->pev->absmax) * 0.5;
		vecDir = vecDir.Normalize();

		float flForce = info.GetDamage() * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

		if (flForce > 1000.0)
			flForce = 1000.0;
		pev->velocity = pev->velocity + vecDir * flForce;
	}

	// do the damage
	pev->health -= info.GetDamage();
	if (pev->health <= 0)
	{
		Killed({info.GetAttacker(), GibType::Normal});
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

// Global Savedata for Delay
TYPEDESCRIPTION	CBaseEntity::m_SaveData[] =
{
	DEFINE_FIELD(CBaseEntity, m_hGoalEnt, FIELD_EHANDLE),

	DEFINE_FIELD(CBaseEntity, m_pfnThink, FIELD_FUNCTION),		// UNDONE: Build table of these!!!
	DEFINE_FIELD(CBaseEntity, m_pfnTouch, FIELD_FUNCTION),
	DEFINE_FIELD(CBaseEntity, m_pfnUse, FIELD_FUNCTION),
	DEFINE_FIELD(CBaseEntity, m_pfnBlocked, FIELD_FUNCTION),
};

bool CBaseEntity::Save(CSave& save)
{
	if (save.WriteEntVars("ENTVARS", pev))
		return save.WriteFields("BASE", this, m_SaveData, ArraySize(m_SaveData));

	return false;
}

bool CBaseEntity::Restore(CRestore& restore)
{
	bool status = restore.ReadEntVars("ENTVARS", pev);
	if (status)
		status = restore.ReadFields("BASE", this, m_SaveData, ArraySize(m_SaveData));

	if (pev->modelindex != 0 && !IsStringNull(pev->model))
	{
		Vector mins, maxs;
		mins = pev->mins;	// Set model is about to destroy these
		maxs = pev->maxs;


		PRECACHE_MODEL(STRING(pev->model));
		SET_MODEL(ENT(pev), STRING(pev->model));
		UTIL_SetSize(pev, mins, maxs);	// Reset them
	}

	return status;
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
	pev->solid = Solid::Not;
	// Don't move
	pev->movetype = Movetype::None;
	// Don't draw
	SetBits(pev->effects, EF_NODRAW);
	// Don't think
	pev->nextthink = 0;
	// Relink
	SetAbsOrigin(pev->origin);
}

bool CBaseEntity::IsDormant()
{
	return IsBitSet(pev->flags, FL_DORMANT);
}

bool CBaseEntity::IsInWorld()
{
	// position 
	if (!UTIL_IsInWorld(pev->origin))
	{
		return false;
	}

	// speed
	if (pev->velocity.x >= 2000) return false;
	if (pev->velocity.y >= 2000) return false;
	if (pev->velocity.z >= 2000) return false;
	if (pev->velocity.x <= -2000) return false;
	if (pev->velocity.y <= -2000) return false;
	if (pev->velocity.z <= -2000) return false;

	return true;
}

bool CBaseEntity::ShouldToggle(USE_TYPE useType, bool currentState)
{
	if (useType != USE_TOGGLE && useType != USE_SET)
	{
		if ((currentState && useType == USE_ON) || (!currentState && useType == USE_OFF))
			return false;
	}
	return true;
}

int	CBaseEntity::DamageDecal(int bitsDamageType)
{
	if (pev->rendermode == RenderMode::TransAlpha)
		return -1;

	if (pev->rendermode != RenderMode::Normal)
		return DECAL_BPROOF1;

	return DECAL_GUNSHOT1 + RANDOM_LONG(0, 4);
}

void CBaseEntity::EmitSound(SoundChannel channel, const char* fileName, float volume, float attenuation, int pitch, int flags)
{
	EMIT_SOUND_DYN(edict(), channel, fileName, volume, attenuation, flags, pitch);
}

void CBaseEntity::StopSound(SoundChannel channel, const char* fileName)
{
	EMIT_SOUND_DYN(edict(), channel, fileName, 0, 0, SND_STOP, PITCH_NORM);
}

CBaseEntity* CBaseEntity::Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner)
{
	//TODO: wrap createnamedentity
	auto pEntity = InstanceOrNull(CREATE_NAMED_ENTITY(MAKE_STRING(szName)));
	if (IsNullEnt(pEntity))
	{
		ALERT(at_console, "NULL Ent in Create!\n");
		return nullptr;
	}

	pEntity->pev->owner = EdictOrNull(pOwner);
	pEntity->pev->origin = vecOrigin;
	pEntity->pev->angles = vecAngles;

	DispatchSpawn(pEntity->edict());

	return pEntity;
}

void CBaseEntity::SetAbsOrigin(const Vector& origin)
{
	if (auto ent = edict(); ent)
	{
		SET_ORIGIN(ent, origin);
	}
}
