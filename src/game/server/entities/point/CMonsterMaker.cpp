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

#include "dll_functions.hpp"
#include "CMonsterMaker.hpp"

void CMonsterMaker::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "monstercount"))
	{
		m_cNumMonsters = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_imaxlivechildren"))
	{
		m_iMaxLiveChildren = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "monstertype"))
	{
		m_iszMonsterClassname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

void CMonsterMaker::Spawn()
{
	SetSolidType(Solid::Not);

	m_cLiveChildren = 0;
	Precache();
	if (HasTargetname())
	{
		if (pev->spawnflags & SF_MONSTERMAKER_CYCLIC)
		{
			SetUse(&CMonsterMaker::CyclicUse);// drop one monster each time we fire
		}
		else
		{
			SetUse(&CMonsterMaker::ToggleUse);// so can be turned on/off
		}

		if (IsBitSet(pev->spawnflags, SF_MONSTERMAKER_START_ON))
		{// start making monsters as soon as monstermaker spawns
			m_fActive = true;
			SetThink(&CMonsterMaker::MakerThink);
		}
		else
		{// wait to be activated.
			m_fActive = false;
			SetThink(&CMonsterMaker::SUB_DoNothing);
		}
	}
	else
	{// no targetname, just start.
		pev->nextthink = gpGlobals->time + m_flDelay;
		m_fActive = true;
		SetThink(&CMonsterMaker::MakerThink);
	}

	m_fFadeChildren = m_cNumMonsters != 1;
	m_flGround = 0;
}

void CMonsterMaker::Precache()
{
	CBaseMonster::Precache();

	UTIL_PrecacheOther(STRING(m_iszMonsterClassname));
}

void CMonsterMaker::MakeMonster()
{
	if (m_iMaxLiveChildren > 0 && m_cLiveChildren >= m_iMaxLiveChildren)
	{// not allowed to make a new one yet. Too many live ones out right now.
		return;
	}

	if (!m_flGround)
	{
		// set altitude. Now that I'm activated, any breakables, etc should be out from under me. 
		TraceResult tr;

		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 2048), IgnoreMonsters::Yes, this, &tr);
		m_flGround = tr.vecEndPos.z;
	}

	Vector mins = GetAbsOrigin() - Vector(34, 34, 0);
	Vector maxs = GetAbsOrigin() + Vector(34, 34, 0);
	maxs.z = GetAbsOrigin().z;
	mins.z = m_flGround;

	CBaseEntity* pList[2];
	int count = UTIL_EntitiesInBox(pList, ArraySize(pList), mins, maxs, FL_CLIENT | FL_MONSTER);
	if (count)
	{
		// don't build a stack of monsters!
		return;
	}

	auto pEntity = UTIL_CreateNamedEntity(m_iszMonsterClassname);

	if (IsNullEnt(pEntity))
	{
		ALERT(at_console, "NULL Ent in MonsterMaker!\n");
		return;
	}

	// If I have a target, fire!
	if (HasTarget())
	{
		// delay already overloaded for this entity, so can't call SUB_UseTargets()
		FireTargets(GetTarget(), this, this, UseType::Toggle, 0);
	}

	pEntity->SetAbsOrigin(GetAbsOrigin());
	pEntity->SetAbsAngles(GetAbsAngles());
	SetBits(pEntity->pev->spawnflags, SF_MONSTER_FALL_TO_GROUND);

	// Children hit monsterclip brushes
	if (pev->spawnflags & SF_MONSTERMAKER_MONSTERCLIP)
		SetBits(pEntity->pev->spawnflags, SF_MONSTER_HITMONSTERCLIP);

	DispatchSpawn(pEntity->edict());
	pEntity->SetOwner(this);

	if (!IsStringNull(pev->netname))
	{
		// if I have a netname (overloaded), give the child monster that name as a targetname
		pEntity->SetTargetname(STRING(pev->netname));
	}

	m_cLiveChildren++;// count this monster
	m_cNumMonsters--;

	if (m_cNumMonsters == 0)
	{
		// Disable this forever.  Don't kill it because it still gets death notices
		SetThink(nullptr);
		SetUse(nullptr);
	}
}

void CMonsterMaker::CyclicUse(const UseInfo& info)
{
	MakeMonster();
}

void CMonsterMaker::ToggleUse(const UseInfo& info)
{
	if (!ShouldToggle(info.GetUseType(), m_fActive))
		return;

	if (m_fActive)
	{
		m_fActive = false;
		SetThink(nullptr);
	}
	else
	{
		m_fActive = true;
		SetThink(&CMonsterMaker::MakerThink);
	}

	pev->nextthink = gpGlobals->time;
}

void CMonsterMaker::MakerThink()
{
	pev->nextthink = gpGlobals->time + m_flDelay;

	MakeMonster();
}

void CMonsterMaker::DeathNotice(CBaseEntity* pChild)
{
	// ok, we've gotten the deathnotice from our child, now clear out its owner if we don't want it to fade.
	m_cLiveChildren--;

	if (!m_fFadeChildren)
	{
		pChild->SetOwner(nullptr);
	}
}
