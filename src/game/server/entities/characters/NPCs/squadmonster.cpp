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

#include "animation.h"
#include "plane.h"

TYPEDESCRIPTION	CSquadMonster::m_SaveData[] =
{
	DEFINE_FIELD(CSquadMonster, m_hSquadLeader, FIELD_EHANDLE),
	DEFINE_ARRAY(CSquadMonster, m_hSquadMember, FIELD_EHANDLE, MAX_SQUAD_MEMBERS - 1),

	// DEFINE_FIELD( CSquadMonster, m_afSquadSlots, FIELD_INTEGER ), // these need to be reset after transitions!
	DEFINE_FIELD(CSquadMonster, m_fEnemyEluded, FIELD_BOOLEAN),
	DEFINE_FIELD(CSquadMonster, m_flLastEnemySightTime, FIELD_TIME),

	DEFINE_FIELD(CSquadMonster, m_iMySlot, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CSquadMonster, CBaseMonster);

bool CSquadMonster::OccupySlot(int iDesiredSlots)
{
	if (!InSquad())
	{
		return true;
	}

	if (SquadEnemySplit())
	{
		// if the squad members aren't all fighting the same enemy, slots are disabled
		// so that a squad member doesn't get stranded unable to engage his enemy because
		// all of the attack slots are taken by squad members fighting other enemies.
		m_iMySlot = bits_SLOT_SQUAD_SPLIT;
		return true;
	}

	CSquadMonster* pSquadLeader = MySquadLeader();

	if (!(iDesiredSlots ^ pSquadLeader->m_afSquadSlots))
	{
		// none of the desired slots are available. 
		return false;
	}

	const int iSquadSlots = pSquadLeader->m_afSquadSlots;

	for (int i = 0; i < NUM_SLOTS; i++)
	{
		const int iMask = 1 << i;
		if (iDesiredSlots & iMask) // am I looking for this bit?
		{
			if (!(iSquadSlots & iMask))	// Is it already taken?
			{
				// No, use this bit
				pSquadLeader->m_afSquadSlots |= iMask;
				m_iMySlot = iMask;
				//				ALERT ( at_aiconsole, "Took slot %d - %d\n", i, m_hSquadLeader->m_afSquadSlots );
				return true;
			}
		}
	}

	return false;
}

void CSquadMonster::VacateSlot()
{
	if (m_iMySlot != bits_NO_SLOT && InSquad())
	{
		//		ALERT ( at_aiconsole, "Vacated Slot %d - %d\n", m_iMySlot, m_hSquadLeader->m_afSquadSlots );
		MySquadLeader()->m_afSquadSlots &= ~m_iMySlot;
		m_iMySlot = bits_NO_SLOT;
	}
}

void CSquadMonster::ScheduleChange()
{
	VacateSlot();
}

void CSquadMonster::Killed(const KilledInfo& info)
{
	VacateSlot();

	if (InSquad())
	{
		MySquadLeader()->SquadRemove(this);
	}

	CBaseMonster::Killed(info);
}

void CSquadMonster::SquadRemove(CSquadMonster* pRemove)
{
	ASSERT(pRemove != nullptr);
	ASSERT(this->IsLeader());
	ASSERT(pRemove->m_hSquadLeader == this);

	// If I'm the leader, get rid of my squad
	if (pRemove == MySquadLeader())
	{
		for (int i = 0; i < MAX_SQUAD_MEMBERS - 1; i++)
		{
			CSquadMonster* pMember = MySquadMember(i);
			if (pMember)
			{
				pMember->m_hSquadLeader = nullptr;
				m_hSquadMember[i] = nullptr;
			}
		}
	}
	else
	{
		if (CSquadMonster* pSquadLeader = MySquadLeader(); pSquadLeader)
		{
			for (int i = 0; i < MAX_SQUAD_MEMBERS - 1; i++)
			{
				if (pSquadLeader->m_hSquadMember[i] == this)
				{
					pSquadLeader->m_hSquadMember[i] = nullptr;
					break;
				}
			}
		}
	}

	pRemove->m_hSquadLeader = nullptr;
}

bool CSquadMonster::SquadAdd(CSquadMonster* pAdd)
{
	ASSERT(pAdd != nullptr);
	ASSERT(!pAdd->InSquad());
	ASSERT(this->IsLeader());

	for (int i = 0; i < MAX_SQUAD_MEMBERS - 1; i++)
	{
		if (m_hSquadMember[i] == nullptr)
		{
			m_hSquadMember[i] = pAdd;
			pAdd->m_hSquadLeader = this;
			return true;
		}
	}
	return false;
	// should complain here
}

void CSquadMonster::SquadPasteEnemyInfo()
{
	if (CSquadMonster* pSquadLeader = MySquadLeader(); pSquadLeader)
		pSquadLeader->m_vecEnemyLKP = m_vecEnemyLKP;
}

void CSquadMonster::SquadCopyEnemyInfo()
{
	if (CSquadMonster* pSquadLeader = MySquadLeader(); pSquadLeader)
		m_vecEnemyLKP = pSquadLeader->m_vecEnemyLKP;
}

void CSquadMonster::SquadMakeEnemy(CBaseEntity* pEnemy)
{
	if (!InSquad())
		return;

	if (!pEnemy)
	{
		ALERT(at_console, "ERROR: SquadMakeEnemy() - pEnemy is NULL!\n");
		return;
	}

	CSquadMonster* pSquadLeader = MySquadLeader();
	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		if (CSquadMonster* pMember = pSquadLeader->MySquadMember(i); pMember)
		{
			// reset members who aren't activly engaged in fighting
			if (pMember->m_hEnemy != pEnemy && !pMember->HasConditions(bits_COND_SEE_ENEMY))
			{
				if (pMember->m_hEnemy != nullptr)
				{
					// remember their current enemy
					pMember->PushEnemy(pMember->m_hEnemy, pMember->m_vecEnemyLKP);
				}
				// give them a new enemy
				pMember->m_hEnemy = pEnemy;
				pMember->m_vecEnemyLKP = pEnemy->pev->origin;
				pMember->SetConditions(bits_COND_NEW_ENEMY);
			}
		}
	}
}

int CSquadMonster::SquadCount()
{
	if (!InSquad())
		return 0;

	CSquadMonster* pSquadLeader = MySquadLeader();
	int squadCount = 0;
	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		if (pSquadLeader->MySquadMember(i) != nullptr)
			squadCount++;
	}

	return squadCount;
}

int CSquadMonster::SquadRecruit(int searchRadius, int maxMembers)
{
	// Don't recruit if I'm already in a group
	if (InSquad())
		return 0;

	if (maxMembers < 2)
		return 0;

	const int iMyClass = Classify();// cache this monster's class

	// I am my own leader
	m_hSquadLeader = this;
	int squadCount = 1;

	CBaseEntity* pEntity = nullptr;

	if (!IsStringNull(pev->netname))
	{
		// I have a netname, so unconditionally recruit everyone else with that name.
		pEntity = UTIL_FindEntityByString(pEntity, "netname", STRING(pev->netname));
		while (pEntity)
		{
			CSquadMonster* pRecruit = pEntity->MySquadMonsterPointer();

			if (pRecruit)
			{
				if (!pRecruit->InSquad() && pRecruit->Classify() == iMyClass && pRecruit != this)
				{
					// minimum protection here against user error.in worldcraft. 
					if (!SquadAdd(pRecruit))
						break;
					squadCount++;
				}
			}

			pEntity = UTIL_FindEntityByString(pEntity, "netname", STRING(pev->netname));
		}
	}
	else
	{
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, searchRadius)) != nullptr)
		{
			if (CSquadMonster* pRecruit = pEntity->MySquadMonsterPointer();
				pRecruit && pRecruit != this && pRecruit->IsAlive() && !pRecruit->m_hCine)
			{
				// Can we recruit this guy?
				if (!pRecruit->InSquad() && pRecruit->Classify() == iMyClass &&
					((iMyClass != CLASS_ALIEN_MONSTER) || AreStringsEqual(STRING(pev->classname), STRING(pRecruit->pev->classname))) &&
					IsStringNull(pRecruit->pev->netname))
				{
					TraceResult tr;
					UTIL_TraceLine(pev->origin + pev->view_ofs, pRecruit->pev->origin + pev->view_ofs, IgnoreMonsters::Yes, pRecruit, &tr);// try to hit recruit with a traceline.
					if (tr.flFraction == 1.0)
					{
						if (!SquadAdd(pRecruit))
							break;

						squadCount++;
					}
				}
			}
		}
	}

	// no single member squads
	if (squadCount == 1)
	{
		m_hSquadLeader = nullptr;
	}

	return squadCount;
}

bool CSquadMonster::CheckEnemy(CBaseEntity* pEnemy)
{
	const bool updatedLKP = CBaseMonster::CheckEnemy(m_hEnemy);

	// communicate with squad members about the enemy IF this individual has the same enemy as the squad leader.
	if (InSquad() && m_hEnemy.Get() == MySquadLeader()->m_hEnemy)
	{
		if (updatedLKP)
		{
			// have new enemy information, so paste to the squad.
			SquadPasteEnemyInfo();
		}
		else
		{
			// enemy unseen, copy from the squad knowledge.
			SquadCopyEnemyInfo();
		}
	}

	return updatedLKP;
}

void CSquadMonster::StartMonster()
{
	CBaseMonster::StartMonster();

	if ((m_afCapability & bits_CAP_SQUAD) && !InSquad())
	{
		if (!IsStringNull(pev->netname))
		{
			// if I have a groupname, I can only recruit if I'm flagged as leader
			if (!(pev->spawnflags & SF_SQUADMONSTER_LEADER))
			{
				return;
			}
		}

		// try to form squads now.
		const int iSquadSize = SquadRecruit(1024, 4);

		if (iSquadSize)
		{
			ALERT(at_aiconsole, "Squad of %d %s formed\n", iSquadSize, STRING(pev->classname));
		}

		if (IsLeader() && ClassnameIs("monster_human_grunt"))
		{
			SetBodygroup(1, 1); // UNDONE: truly ugly hack
			pev->skin = 0;
		}

	}
}

bool CSquadMonster::NoFriendlyFire()
{
	if (!InSquad())
	{
		return true;
	}

	//!!!BUGBUG - to fix this, the planes must be aligned to where the monster will be firing its gun, not the direction it is facing!!!

	if (m_hEnemy == nullptr)
	{
		// if there's no enemy, pretend there's a friendly in the way, so the grunt won't shoot.
		return false;
	}

	UTIL_MakeVectors(VectorAngles(m_hEnemy->Center() - pev->origin));

	//UTIL_MakeVectors ( pev->angles );

	const Vector vecLeftSide = pev->origin - (gpGlobals->v_right * (pev->size.x * 1.5));
	const Vector vecRightSide = pev->origin + (gpGlobals->v_right * (pev->size.x * 1.5));
	const Vector v_left = gpGlobals->v_right * -1;

	CPlane leftPlane(gpGlobals->v_right, vecLeftSide);
	CPlane rightPlane(v_left, vecRightSide);
	CPlane backPlane(gpGlobals->v_forward, pev->origin);

	/*
		ALERT ( at_console, "LeftPlane: %f %f %f : %f\n", leftPlane.m_vecNormal.x, leftPlane.m_vecNormal.y, leftPlane.m_vecNormal.z, leftPlane.m_flDist );
		ALERT ( at_console, "RightPlane: %f %f %f : %f\n", rightPlane.m_vecNormal.x, rightPlane.m_vecNormal.y, rightPlane.m_vecNormal.z, rightPlane.m_flDist );
		ALERT ( at_console, "BackPlane: %f %f %f : %f\n", backPlane.m_vecNormal.x, backPlane.m_vecNormal.y, backPlane.m_vecNormal.z, backPlane.m_flDist );
	*/

	CSquadMonster* pSquadLeader = MySquadLeader();
	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		if (CSquadMonster* pMember = pSquadLeader->MySquadMember(i);
			pMember && pMember != this)
		{

			if (backPlane.PointInFront(pMember->pev->origin) &&
				leftPlane.PointInFront(pMember->pev->origin) &&
				rightPlane.PointInFront(pMember->pev->origin))
			{
				// this guy is in the check volume! Don't shoot!
				return false;
			}
		}
	}

	return true;
}

NPCState CSquadMonster::GetIdealState()
{
	const int iConditions = ScheduleFlags();

	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch (m_MonsterState)
	{
	case NPCState::Idle:
	case NPCState::Alert:
		if (HasConditions(bits_COND_NEW_ENEMY) && InSquad())
		{
			SquadMakeEnemy(m_hEnemy);
		}
		break;
	}

	return CBaseMonster::GetIdealState();
}

bool CSquadMonster::ValidateCover(const Vector& vecCoverLocation)
{
	if (!InSquad())
	{
		return true;
	}

	if (SquadMemberInRange(vecCoverLocation, 128))
	{
		// another squad member is too close to this piece of cover.
		return false;
	}

	return true;
}

bool CSquadMonster::SquadEnemySplit()
{
	if (!InSquad())
		return false;

	CSquadMonster* pSquadLeader = MySquadLeader();
	CBaseEntity* pEnemy = pSquadLeader->m_hEnemy;

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		if (CSquadMonster* pMember = pSquadLeader->MySquadMember(i);
			pMember != nullptr && pMember->m_hEnemy != nullptr && pMember->m_hEnemy != pEnemy)
		{
			return true;
		}
	}
	return false;
}

bool CSquadMonster::SquadMemberInRange(const Vector& vecLocation, float flDist)
{
	if (!InSquad())
		return false;

	CSquadMonster* pSquadLeader = MySquadLeader();

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		if (CSquadMonster* pSquadMember = pSquadLeader->MySquadMember(i);
			pSquadMember && (vecLocation - pSquadMember->pev->origin).Length2D() <= flDist)
			return true;
	}
	return false;
}

extern Schedule_t	slChaseEnemyFailed[];

Schedule_t* CSquadMonster::GetScheduleOfType(int iType)
{
	switch (iType)
	{
	case SCHED_CHASE_ENEMY_FAILED:
	{
		return &slChaseEnemyFailed[0];
	}

	default:
		return CBaseMonster::GetScheduleOfType(iType);
	}
}
