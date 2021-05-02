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

#include <limits>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "animation.h"
#include "studio.h"

TYPEDESCRIPTION	CBaseAnimating::m_SaveData[] =
{
	DEFINE_FIELD(CBaseMonster, m_flFrameRate, FIELD_FLOAT),
	DEFINE_FIELD(CBaseMonster, m_flGroundSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CBaseMonster, m_flLastEventCheck, FIELD_TIME),
	DEFINE_FIELD(CBaseMonster, m_fSequenceFinished, FIELD_BOOLEAN),
	DEFINE_FIELD(CBaseMonster, m_fSequenceLoops, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CBaseAnimating, CBaseDelay);

float CBaseAnimating::StudioFrameAdvance(float flInterval)
{
	if (flInterval == 0.0)
	{
		flInterval = (gpGlobals->time - pev->animtime);
		if (flInterval <= 0.001)
		{
			pev->animtime = gpGlobals->time;
			return 0.0;
		}
	}
	if (!pev->animtime)
		flInterval = 0.0;

	pev->frame += flInterval * m_flFrameRate * pev->framerate;
	pev->animtime = gpGlobals->time;

	if (pev->frame < 0.0 || pev->frame >= 256.0)
	{
		if (m_fSequenceLoops)
			pev->frame -= (int)(pev->frame / 256.0) * 256.0;
		else
			pev->frame = (pev->frame < 0.0) ? 0 : 255;
		m_fSequenceFinished = true;	// just in case it wasn't caught in GetEvents
	}

	return flInterval;
}

int CBaseAnimating::LookupActivity(int activity)
{
	ASSERT(activity != 0);
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	return ::LookupActivity(pmodel, activity);
}

int CBaseAnimating::LookupActivityHeaviest(int activity)
{
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	return ::LookupActivityHeaviest(pmodel, activity);
}

int CBaseAnimating::LookupSequence(const char* label)
{
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	return ::LookupSequence(pmodel, label);
}

void CBaseAnimating::ResetSequenceInfo()
{
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	GetSequenceInfo(pmodel, this, m_flFrameRate, m_flGroundSpeed);
	m_fSequenceLoops = ((GetSequenceFlags() & STUDIO_LOOPING) != 0);
	pev->animtime = gpGlobals->time;
	pev->framerate = 1.0;
	m_fSequenceFinished = false;
	m_flLastEventCheck = gpGlobals->time;
}

int CBaseAnimating::GetSequenceFlags()
{
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	return ::GetSequenceFlags(pmodel, this);
}

void CBaseAnimating::DispatchAnimEvents(float flInterval)
{
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	if (!pmodel)
	{
		ALERT(at_aiconsole, "Gibbed monster is thinking!\n");
		return;
	}

	// FIXME: I have to do this or some events get missed, and this is probably causing the problem below
	flInterval = 0.1;

	// FIX: this still sometimes hits events twice
	const float flStart = pev->frame + (m_flLastEventCheck - pev->animtime) * m_flFrameRate * pev->framerate;
	const float flEnd = pev->frame + flInterval * m_flFrameRate * pev->framerate;
	m_flLastEventCheck = pev->animtime + flInterval;

	m_fSequenceFinished = flEnd >= 256 || flEnd <= 0.0;

	int index = 0;

	AnimationEvent event;
	while ((index = GetAnimationEvent(pmodel, this, event, flStart, flEnd, index)) != 0)
	{
		HandleAnimEvent(event);
	}
}

float CBaseAnimating::SetBoneController(int iController, float flValue)
{
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	return SetController(pmodel, this, iController, flValue);
}

void CBaseAnimating::InitBoneControllers()
{
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	SetController(pmodel, this, 0, 0.0);
	SetController(pmodel, this, 1, 0.0);
	SetController(pmodel, this, 2, 0.0);
	SetController(pmodel, this, 3, 0.0);
}

float CBaseAnimating::SetBlending(int iBlender, float flValue)
{
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	return ::SetBlending(pmodel, this, iBlender, flValue);
}

void CBaseAnimating::GetBonePosition(int iBone, Vector& origin, Vector& angles)
{
	GET_BONE_POSITION(ENT(pev), iBone, origin, angles);
}

void CBaseAnimating::GetAttachment(int iAttachment, Vector& origin, Vector& angles)
{
	GET_ATTACHMENT(ENT(pev), iAttachment, origin, angles);
}

int CBaseAnimating::FindTransition(int iEndingSequence, int iGoalSequence, int& iDir)
{
	void* pmodel = GET_MODEL_PTR(ENT(pev));
	return ::FindTransition(pmodel, iEndingSequence, iGoalSequence, iDir);
}

int CBaseAnimating::FindTransition(int iEndingSequence, int iGoalSequence)
{
	int iDir;
	int sequence = FindTransition(iEndingSequence, iGoalSequence, iDir);
	if (iDir != 1)
		return -1;
	else
		return sequence;
}

void CBaseAnimating::SetBodygroup(int iGroup, int iValue)
{
	::SetBodygroup(GET_MODEL_PTR(ENT(pev)), this, iGroup, iValue);
}

int CBaseAnimating::GetBodygroup(int iGroup)
{
	return ::GetBodygroup(GET_MODEL_PTR(ENT(pev)), this, iGroup);
}


bool CBaseAnimating::ExtractBbox(int sequence, Vector& mins, Vector& maxs)
{
	return ::ExtractBbox(GET_MODEL_PTR(ENT(pev)), sequence, mins, maxs);
}

void CBaseAnimating::SetSequenceBox()
{
	Vector mins, maxs;

	// Get sequence bbox
	if (ExtractBbox(pev->sequence, mins, maxs))
	{
		// expand box for rotation
		// find min / max for rotations
		const float yaw = pev->angles.y * (M_PI / 180.0);

		const Vector xvector{cos(yaw), sin(yaw), 0};
		const Vector yvector{-sin(yaw), cos(yaw), 0};
		const Vector bounds[2] = {mins, maxs};

		Vector rmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		Vector rmax(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
		Vector base, transformed;

		for (int i = 0; i <= 1; i++)
		{
			base.x = bounds[i].x;
			for (int j = 0; j <= 1; j++)
			{
				base.y = bounds[j].y;
				for (int k = 0; k <= 1; k++)
				{
					base.z = bounds[k].z;

					// transform the point
					transformed.x = xvector.x * base.x + yvector.x * base.y;
					transformed.y = xvector.y * base.x + yvector.y * base.y;
					transformed.z = base.z;

					if (transformed.x < rmin.x)
						rmin.x = transformed.x;
					if (transformed.x > rmax.x)
						rmax.x = transformed.x;
					if (transformed.y < rmin.y)
						rmin.y = transformed.y;
					if (transformed.y > rmax.y)
						rmax.y = transformed.y;
					if (transformed.z < rmin.z)
						rmin.z = transformed.z;
					if (transformed.z > rmax.z)
						rmax.z = transformed.z;
				}
			}
		}
		rmin.z = 0;
		rmax.z = rmin.z + 1;
		UTIL_SetSize(pev, rmin, rmax);
	}
}
