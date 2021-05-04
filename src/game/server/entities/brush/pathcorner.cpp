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

#include "trains.h"

class CPathCorner : public CPointEntity
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	float GetDelay() override { return m_flWait; }
	//	void Touch( CBaseEntity *pOther ) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

private:
	float	m_flWait;
};

LINK_ENTITY_TO_CLASS(path_corner, CPathCorner);

// Global Savedata for Delay
TYPEDESCRIPTION	CPathCorner::m_SaveData[] =
{
	DEFINE_FIELD(CPathCorner, m_flWait, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CPathCorner, CPointEntity);

void CPathCorner::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CPathCorner::Spawn()
{
	ASSERTSZ(!IsStringNull(pev->targetname), "path_corner without a targetname");
}

#if 0
void CPathCorner::Touch(CBaseEntity* pOther)
{
	if (IsBitSet(pOther->pev->flags, FL_MONSTER))
	{// monsters don't navigate path corners based on touch anymore
		return;
	}

	// If OTHER isn't explicitly looking for this path_corner, bail out
	if (pOther->m_hGoalEnt != this)
	{
		return;
	}

	// If OTHER has an enemy, this touch is incidental, ignore
	if (!IsNullEnt(pOther->pev->enemy))
	{
		return;		// fighting, not following a path
	}

	// UNDONE: support non-zero flWait
	/*
	if (m_flWait != 0)
		ALERT(at_warning, "Non-zero path-cornder waits NYI");
	*/

	// Find the next "stop" on the path, make it the goal of the "toucher".
	if (IsStringNull(pev->target))
	{
		ALERT(at_warning, "PathCornerTouch: no next stop specified");
	}

	pOther->m_hGoalEnt = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));

	// If "next spot" was not found (does not exist - level design error)
	if (!pOther->m_hGoalEnt)
	{
		ALERT(at_console, "PathCornerTouch--%s couldn't find next stop in path: %s", GetClassname(), STRING(pev->target));
		return;
	}

	// Turn towards the next stop in the path.
	pOther->pev->ideal_yaw = UTIL_VecToYaw(pOther->m_hGoalEnt->GetAbsOrigin() - pOther->GetAbsOrigin());
}
#endif

TYPEDESCRIPTION	CPathTrack::m_SaveData[] =
{
	DEFINE_FIELD(CPathTrack, m_length, FIELD_FLOAT),
	DEFINE_FIELD(CPathTrack, m_hNext, FIELD_EHANDLE),
	DEFINE_FIELD(CPathTrack, m_hAltPath, FIELD_EHANDLE),
	DEFINE_FIELD(CPathTrack, m_hPrevious, FIELD_EHANDLE),
	DEFINE_FIELD(CPathTrack, m_altName, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CPathTrack, CBaseEntity);
LINK_ENTITY_TO_CLASS(path_track, CPathTrack);

void CPathTrack::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "altpath"))
	{
		m_altName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CPathTrack::Use(const UseInfo& info)
{
	// Use toggles between two paths
	if (m_hAltPath)
	{
		const bool on = !IsBitSet(pev->spawnflags, SF_PATH_ALTERNATE);
		if (ShouldToggle(info.GetUseType(), on))
		{
			if (on)
				SetBits(pev->spawnflags, SF_PATH_ALTERNATE);
			else
				ClearBits(pev->spawnflags, SF_PATH_ALTERNATE);
		}
	}
	else	// Use toggles between enabled/disabled
	{
		const bool on = !IsBitSet(pev->spawnflags, SF_PATH_DISABLED);
		if (ShouldToggle(info.GetUseType(), on))
		{
			if (on)
				SetBits(pev->spawnflags, SF_PATH_DISABLED);
			else
				ClearBits(pev->spawnflags, SF_PATH_DISABLED);
		}
	}
}

void CPathTrack::Link()
{
	if (!IsStringNull(pev->target))
	{
		CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
		if (!IsNullEnt(pTarget))
		{
			m_hNext = CPathTrack::Instance(pTarget);

			if (m_hNext)		// If no next pointer, this is the end of a path
			{
				m_hNext->SetPrevious(this);
			}
		}
		else
			ALERT(at_console, "Dead end link %s\n", STRING(pev->target));
	}

	// Find "alternate" path
	if (!IsStringNull(m_altName))
	{
		CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(m_altName));
		if (!IsNullEnt(pTarget))
		{
			auto path = m_hAltPath = CPathTrack::Instance(pTarget);

			if (path) // If no next pointer, this is the end of a path
			{
				path->SetPrevious(this);
			}
		}
	}
}

void CPathTrack::Spawn()
{
	SetSolidType(Solid::Trigger);
	SetSize(Vector(-8, -8, -8), Vector(8, 8, 8));

	m_hNext = nullptr;
	m_hPrevious = nullptr;
	// DEBUGGING CODE
#if PATH_SPARKLE_DEBUG
	SetThink(Sparkle);
	pev->nextthink = gpGlobals->time + 0.5;
#endif
}

void CPathTrack::Activate()
{
	if (!IsStringNull(pev->targetname))		// Link to next, and back-link
		Link();
}

CPathTrack* CPathTrack::ValidPath(CPathTrack* ppath, int testFlag)
{
	if (!ppath)
		return nullptr;

	if (testFlag && IsBitSet(ppath->pev->spawnflags, SF_PATH_DISABLED))
		return nullptr;

	return ppath;
}

void CPathTrack::Project(CPathTrack* pstart, CPathTrack* pend, Vector& origin, float dist)
{
	if (pstart && pend)
	{
		const Vector dir = (pend->GetAbsOrigin() - pstart->GetAbsOrigin()).Normalize();
		origin = pend->GetAbsOrigin() + dir * dist;
	}
}

CPathTrack* CPathTrack::GetNext()
{
	if (auto path = m_hAltPath; path && IsBitSet(pev->spawnflags, SF_PATH_ALTERNATE) && !IsBitSet(pev->spawnflags, SF_PATH_ALTREVERSE))
		return path;

	return m_hNext;
}

CPathTrack* CPathTrack::GetPrevious()
{
	if (auto path = m_hAltPath; path && IsBitSet(pev->spawnflags, SF_PATH_ALTERNATE) && IsBitSet(pev->spawnflags, SF_PATH_ALTREVERSE))
		return path;

	return m_hPrevious;
}

void CPathTrack::SetPrevious(CPathTrack* pprev)
{
	// Only set previous if this isn't my alternate path
	if (pprev && !AreStringsEqual(STRING(pprev->pev->targetname), STRING(m_altName)))
		m_hPrevious = pprev;
}

// Assumes this is ALWAYS enabled
CPathTrack* CPathTrack::LookAhead(Vector& origin, float dist, int move)
{
	const float originalDist = dist;

	CPathTrack* pcurrent = this;
	Vector currentPos = origin;

	if (dist < 0)		// Travelling backwards through path
	{
		dist = -dist;
		while (dist > 0)
		{
			const Vector dir = pcurrent->GetAbsOrigin() - currentPos;
			const float length = dir.Length();
			if (!length)
			{
				if (!ValidPath(pcurrent->GetPrevious(), move)) 	// If there is no previous node, or it's disabled, return now.
				{
					if (!move)
						Project(pcurrent->GetNext(), pcurrent, origin, dist);
					return nullptr;
				}
				pcurrent = pcurrent->GetPrevious();
			}
			else if (length > dist)	// enough left in this path to move
			{
				origin = currentPos + (dir * (dist / length));
				return pcurrent;
			}
			else
			{
				dist -= length;
				currentPos = pcurrent->GetAbsOrigin();
				origin = currentPos;
				if (!ValidPath(pcurrent->GetPrevious(), move))	// If there is no previous node, or it's disabled, return now.
					return nullptr;

				pcurrent = pcurrent->GetPrevious();
			}
		}
		origin = currentPos;
		return pcurrent;
	}
	else
	{
		while (dist > 0)
		{
			if (!ValidPath(pcurrent->GetNext(), move))	// If there is no next node, or it's disabled, return now.
			{
				if (!move)
					Project(pcurrent->GetPrevious(), pcurrent, origin, dist);
				return nullptr;
			}
			const Vector dir = pcurrent->GetNext()->GetAbsOrigin() - currentPos;
			const float length = dir.Length();
			if (!length && !ValidPath(pcurrent->GetNext()->GetNext(), move))
			{
				if (dist == originalDist) // HACK -- up against a dead end
					return nullptr;
				return pcurrent;
			}
			if (length > dist)	// enough left in this path to move
			{
				origin = currentPos + (dir * (dist / length));
				return pcurrent;
			}
			else
			{
				dist -= length;
				currentPos = pcurrent->GetNext()->GetAbsOrigin();
				pcurrent = pcurrent->GetNext();
				origin = currentPos;
			}
		}
		origin = currentPos;
	}

	return pcurrent;
}

// Assumes this is ALWAYS enabled
CPathTrack* CPathTrack::Nearest(const Vector& origin)
{
	Vector delta = origin - GetAbsOrigin();
	delta.z = 0;
	float minDist = delta.Length();
	CPathTrack* pnearest = this;
	CPathTrack* ppath = GetNext();

	// Hey, I could use the old 2 racing pointers solution to this, but I'm lazy :)
	int deadCount = 0;
	while (ppath && ppath != this)
	{
		deadCount++;
		if (deadCount > 9999)
		{
			ALERT(at_error, "Bad sequence of path_tracks from %s", STRING(pev->targetname));
			return nullptr;
		}
		delta = origin - ppath->GetAbsOrigin();
		delta.z = 0;
		const float dist = delta.Length();
		if (dist < minDist)
		{
			minDist = dist;
			pnearest = ppath;
		}
		ppath = ppath->GetNext();
	}
	return pnearest;
}

CPathTrack* CPathTrack::Instance(CBaseEntity* pent)
{
	if (pent && pent->ClassnameIs("path_track"))
		return (CPathTrack*)pent;
	return nullptr;
}

// DEBUGGING CODE
#if PATH_SPARKLE_DEBUG
void CPathTrack::Sparkle()
{

	pev->nextthink = gpGlobals->time + 0.2;
	if (IsBitSet(pev->spawnflags, SF_PATH_DISABLED))
		UTIL_ParticleEffect(GetAbsOrigin(), Vector(0, 0, 100), 210, 10);
	else
		UTIL_ParticleEffect(GetAbsOrigin(), Vector(0, 0, 100), 84, 10);
}
#endif
