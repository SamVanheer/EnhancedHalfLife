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

#include "CFuncTrackChange.hpp"
#include "CFuncTrackTrain.hpp"
#include "CPathTrack.hpp"

void CFuncTrackChange::Spawn()
{
	Setup();
	if (IsBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
		m_vecPosition2.z = GetAbsOrigin().z;

	SetupRotation();

	if (IsBitSet(pev->spawnflags, SF_TRACK_STARTBOTTOM))
	{
		SetAbsOrigin(m_vecPosition2);
		m_toggle_state = ToggleState::AtBottom;
		SetAbsAngles(m_start);
		m_targetState = ToggleState::AtTop;
	}
	else
	{
		SetAbsOrigin(m_vecPosition1);
		m_toggle_state = ToggleState::AtTop;
		SetAbsAngles(m_end);
		m_targetState = ToggleState::AtBottom;
	}

	EnableUse();
	pev->nextthink = pev->ltime + 2.0;
	SetThink(&CFuncTrackChange::Find);
	Precache();
}

void CFuncTrackChange::Precache()
{
	// Can't trigger sound
	PRECACHE_SOUND("buttons/button11.wav");

	CFuncPlatRot::Precache();
}

// UNDONE: Filter touches before re-evaluating the train.
void CFuncTrackChange::Touch(CBaseEntity* pOther)
{
#if 0
	TRAIN_CODE code;
#endif
}

void CFuncTrackChange::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "train"))
	{
		m_trainName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "toptrack"))
	{
		m_trackTopName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "bottomtrack"))
	{
		m_trackBottomName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
	{
		CFuncPlatRot::KeyValue(pkvd);		// Pass up to base class
	}
}

void CFuncTrackChange::OverrideReset()
{
	pev->nextthink = pev->ltime + 1.0;
	SetThink(&CFuncTrackChange::Find);
}

void CFuncTrackChange::Find()
{
	// Find track entities
	CBaseEntity* target = UTIL_FindEntityByTargetname(nullptr, STRING(m_trackTopName));
	if (!IsNullEnt(target))
	{
		m_hTrackTop = CPathTrack::Instance(target);
		target = UTIL_FindEntityByTargetname(nullptr, STRING(m_trackBottomName));
		if (!IsNullEnt(target))
		{
			m_hTrackBottom = CPathTrack::Instance(target);
			target = UTIL_FindEntityByTargetname(nullptr, STRING(m_trainName));
			if (!IsNullEnt(target))
			{
				m_hTrain = CFuncTrackTrain::Instance(target);
				if (!m_hTrain)
				{
					ALERT(at_error, "Can't find train for track change! %s\n", STRING(m_trainName));
					return;
				}
				const Vector center = (pev->absmin + pev->absmax) * 0.5;
				m_hTrackBottom = m_hTrackBottom->Nearest(center);
				m_hTrackTop = m_hTrackTop->Nearest(center);
				UpdateAutoTargets(m_toggle_state);
				SetThink(nullptr);
				return;
			}
			else
			{
				ALERT(at_error, "Can't find train for track change! %s\n", STRING(m_trainName));
			}
		}
		else
			ALERT(at_error, "Can't find bottom track for track change! %s\n", STRING(m_trackBottomName));
	}
	else
		ALERT(at_error, "Can't find top track for track change! %s\n", STRING(m_trackTopName));
}

TrainCode CFuncTrackChange::EvaluateTrain(CPathTrack* pcurrent)
{
	auto train = m_hTrain.Get();

	// Go ahead and work, we don't have anything to switch, so just be an elevator
	if (!pcurrent || !train)
		return TrainCode::Safe;

	if (train->m_hPath == pcurrent || (pcurrent->m_hPrevious && train->m_hPath == pcurrent->m_hPrevious) ||
		(pcurrent->m_hNext && train->m_hPath == pcurrent->m_hNext))
	{
		if (train->pev->speed != 0)
			return TrainCode::Blocking;

		const Vector dist = GetAbsOrigin() - train->GetAbsOrigin();
		const float length = dist.Length2D();
		if (length < train->m_length)		// Empirically determined close distance
			return TrainCode::Following;
		else if (length > (150 + train->m_length))
			return TrainCode::Safe;

		return TrainCode::Blocking;
	}

	return TrainCode::Safe;
}

void CFuncTrackChange::UpdateTrain(Vector& dest)
{
	auto train = m_hTrain.Get();

	const float time = pev->nextthink - pev->ltime;

	train->SetAbsVelocity(GetAbsVelocity());
	train->pev->avelocity = pev->avelocity;
	train->NextThink(train->pev->ltime + time, false);

	// Attempt at getting the train to rotate properly around the origin of the trackchange
	if (time <= 0)
		return;

	const Vector offset = train->GetAbsOrigin() - GetAbsOrigin();
	const Vector delta = dest - GetAbsAngles();
	// Transform offset into local coordinates
	UTIL_MakeInvVectors(delta, gpGlobals);
	Vector local
	{
		DotProduct(offset, gpGlobals->v_forward),
		DotProduct(offset, gpGlobals->v_right),
		DotProduct(offset, gpGlobals->v_up)
	};

	local = local - offset;
	train->SetAbsVelocity(GetAbsVelocity() + (local * (1.0 / time)));
}

void CFuncTrackChange::GoDown()
{
	if (m_code == TrainCode::Blocking)
		return;

	// HitBottom may get called during CFuncPlat::GoDown(), so set up for that
	// before you call GoDown()

	UpdateAutoTargets(ToggleState::GoingDown);
	// If ROTMOVE, move & rotate
	if (IsBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
	{
		SetMoveDone(&CFuncTrackChange::CallHitBottom);
		m_toggle_state = ToggleState::GoingDown;
		AngularMove(m_start, pev->speed);
	}
	else
	{
		CFuncPlat::GoDown();
		SetMoveDone(&CFuncTrackChange::CallHitBottom);
		RotMove(m_start, pev->nextthink - pev->ltime);
	}
	// Otherwise, rotate first, move second

	// If the train is moving with the platform, update it
	if (m_code == TrainCode::Following)
	{
		UpdateTrain(m_start);
		m_hTrain->m_hPath = nullptr;
	}
}

void CFuncTrackChange::GoUp()
{
	if (m_code == TrainCode::Blocking)
		return;

	// HitTop may get called during CFuncPlat::GoUp(), so set up for that
	// before you call GoUp();

	UpdateAutoTargets(ToggleState::GoingUp);
	if (IsBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
	{
		m_toggle_state = ToggleState::GoingUp;
		SetMoveDone(&CFuncTrackChange::CallHitTop);
		AngularMove(m_end, pev->speed);
	}
	else
	{
		// If ROTMOVE, move & rotate
		CFuncPlat::GoUp();
		SetMoveDone(&CFuncTrackChange::CallHitTop);
		RotMove(m_end, pev->nextthink - pev->ltime);
	}

	// Otherwise, move first, rotate second

	// If the train is moving with the platform, update it
	if (m_code == TrainCode::Following)
	{
		UpdateTrain(m_end);
		m_hTrain->m_hPath = nullptr;
	}
}

void CFuncTrackChange::UpdateAutoTargets(ToggleState toggleState)
{
	if (!m_hTrackTop || !m_hTrackBottom)
		return;

	if (toggleState == ToggleState::AtTop)
		ClearBits(m_hTrackTop->pev->spawnflags, SF_PATH_DISABLED);
	else
		SetBits(m_hTrackTop->pev->spawnflags, SF_PATH_DISABLED);

	if (toggleState == ToggleState::AtBottom)
		ClearBits(m_hTrackBottom->pev->spawnflags, SF_PATH_DISABLED);
	else
		SetBits(m_hTrackBottom->pev->spawnflags, SF_PATH_DISABLED);
}

void CFuncTrackChange::Use(const UseInfo& info)
{
	if (m_toggle_state != ToggleState::AtTop && m_toggle_state != ToggleState::AtBottom)
		return;

	// If train is in "safe" area, but not on the elevator, play alarm sound
	if (m_toggle_state == ToggleState::AtTop)
		m_code = EvaluateTrain(m_hTrackTop);
	else if (m_toggle_state == ToggleState::AtBottom)
		m_code = EvaluateTrain(m_hTrackBottom);
	else
		m_code = TrainCode::Blocking;
	if (m_code == TrainCode::Blocking)
	{
		// Play alarm and return
		EmitSound(SoundChannel::Voice, "buttons/button11.wav");
		return;
	}

	// Otherwise, it's safe to move
	// If at top, go down
	// at bottom, go up

	DisableUse();
	if (m_toggle_state == ToggleState::AtTop)
		GoDown();
	else
		GoUp();
}

void CFuncTrackChange::HitBottom()
{
	CFuncPlatRot::HitBottom();
	if (m_code == TrainCode::Following)
	{
		//		UpdateTrain();
		m_hTrain->SetTrack(m_hTrackBottom);
	}
	SetThink(nullptr);
	pev->nextthink = -1;

	UpdateAutoTargets(m_toggle_state);

	EnableUse();
}

void CFuncTrackChange::HitTop()
{
	CFuncPlatRot::HitTop();
	if (m_code == TrainCode::Following)
	{
		//		UpdateTrain();
		m_hTrain->SetTrack(m_hTrackTop);
	}

	// Don't let the plat go back down
	SetThink(nullptr);
	pev->nextthink = -1;
	UpdateAutoTargets(m_toggle_state);
	EnableUse();
}
