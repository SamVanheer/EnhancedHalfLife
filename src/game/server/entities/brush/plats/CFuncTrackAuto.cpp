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

#include "CFuncTrackAuto.hpp"
#include "CFuncTrackTrain.hpp"
#include "CPathTrack.hpp"

void CFuncTrackAuto::UpdateAutoTargets(ToggleState toggleState)
{
	if (!m_hTrackTop || !m_hTrackBottom)
		return;

	CPathTrack* pTarget, * pNextTarget;
	if (m_targetState == ToggleState::AtTop)
	{
		pTarget = m_hTrackTop->GetNext();
		pNextTarget = m_hTrackBottom->GetNext();
	}
	else
	{
		pTarget = m_hTrackBottom->GetNext();
		pNextTarget = m_hTrackTop->GetNext();
	}
	if (pTarget)
	{
		ClearBits(pTarget->pev->spawnflags, SF_PATH_DISABLED);
		if (auto train = m_hTrain.Get(); m_code == TrainCode::Following && train && train->pev->speed == 0)
			train->Use({this, this, UseType::On});
	}

	if (pNextTarget)
		SetBits(pNextTarget->pev->spawnflags, SF_PATH_DISABLED);
}

void CFuncTrackAuto::Use(const UseInfo& info)
{
	if (!UseEnabled())
		return;

	CPathTrack* pTarget;
	if (m_toggle_state == ToggleState::AtTop)
		pTarget = m_hTrackTop;
	else if (m_toggle_state == ToggleState::AtBottom)
		pTarget = m_hTrackBottom;
	else
		pTarget = nullptr;

	if (info.GetActivator()->ClassnameIs("func_tracktrain"))
	{
		m_code = EvaluateTrain(pTarget);
		// Safe to fire?
		if (m_code == TrainCode::Following && m_toggle_state != m_targetState)
		{
			DisableUse();
			if (m_toggle_state == ToggleState::AtTop)
				GoDown();
			else
				GoUp();
		}
	}
	else
	{
		if (pTarget)
			pTarget = pTarget->GetNext();
		if (pTarget && m_hTrain->m_hPath != pTarget && ShouldToggle(info.GetUseType(), m_targetState != ToggleState::AtTop))
		{
			if (m_targetState == ToggleState::AtTop)
				m_targetState = ToggleState::AtBottom;
			else
				m_targetState = ToggleState::AtTop;
		}

		UpdateAutoTargets(m_targetState);
	}
}
