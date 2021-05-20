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

#include "CCineAI.hpp"

LINK_ENTITY_TO_CLASS(aiscripted_sequence, CCineAI);

bool CCineAI::CanOverrideState()
{
	return true;
}

void CCineAI::PossessEntity()
{
	CBaseMonster* pTarget = nullptr;
	if (CBaseEntity* pEntity = m_hTargetEnt; pEntity)
		pTarget = pEntity->MyMonsterPointer();

	if (pTarget)
	{
		if (!pTarget->CanPlaySequence(CanOverrideState(), SS_INTERRUPT_AI))
		{
			ALERT(at_aiconsole, "(AI)Can't possess entity %s\n", pTarget->GetClassname());
			return;
		}

		pTarget->m_hGoalEnt = this;
		pTarget->m_hCine = this;
		pTarget->m_hTargetEnt = this;

		m_saved_movetype = pTarget->GetMovetype();
		m_saved_solid = pTarget->GetSolidType();
		m_saved_effects = pTarget->pev->effects;
		pTarget->pev->effects |= pev->effects;

		switch (m_fMoveTo)
		{
		case ScriptedMoveTo::No:
		case ScriptedMoveTo::TurnToFace:
			pTarget->m_scriptState = ScriptState::Wait;
			break;

		case ScriptedMoveTo::Walk:
			pTarget->m_scriptState = ScriptState::WalkToMark;
			break;

		case ScriptedMoveTo::Run:
			pTarget->m_scriptState = ScriptState::RunToMark;
			break;

		case ScriptedMoveTo::Instantaneous:
			// zap the monster instantly to the site of the script entity.
			pTarget->SetAbsOrigin(GetAbsOrigin());
			pTarget->pev->ideal_yaw = GetAbsAngles().y;
			pTarget->pev->avelocity = vec3_origin;
			pTarget->SetAbsVelocity(vec3_origin);
			pTarget->pev->effects |= EF_NOINTERP;
			pTarget->SetAbsAngles({0, GetAbsAngles().y, 0});
			pTarget->m_scriptState = ScriptState::Wait;
			m_startTime = gpGlobals->time + 1E6;
			// UNDONE: Add a flag to do this so people can fixup physics after teleporting monsters
			pTarget->pev->flags &= ~FL_ONGROUND;
			break;
		default:
			ALERT(at_aiconsole, "aiscript:  invalid Move To Position value!");
			break;
		}

		ALERT(at_aiconsole, "\"%s\" found and used\n", STRING(pTarget->pev->targetname));

		pTarget->m_IdealMonsterState = NPCState::Script;

		/*
				if (m_iszIdle)
				{
					StartSequence( pTarget, m_iszIdle, false);
					if (AreStringsEqual( STRING(m_iszIdle), STRING(m_iszPlay)))
					{
						pTarget->pev->framerate = 0;
					}
				}
		*/
		// Already in a scripted state?
		if (pTarget->m_MonsterState == NPCState::Script)
		{
			Schedule_t* pNewSchedule = pTarget->GetScheduleOfType(SCHED_AISCRIPT);
			pTarget->ChangeSchedule(pNewSchedule);
		}
	}
}

bool CCineAI::StartSequence(CBaseMonster* pTarget, string_t iszSeq, bool completeOnEmpty)
{
	if (IsStringNull(iszSeq) && completeOnEmpty)
	{
		// no sequence was provided. Just let the monster proceed, however, we still have to fire any Sequence target
		// and remove any non-repeatable CineAI entities here ( because there is code elsewhere that handles those tasks, but
		// not until the animation sequence is finished. We have to manually take care of these things where there is no sequence.

		SequenceDone(pTarget);

		return true;
	}

	pTarget->pev->sequence = pTarget->LookupSequence(STRING(iszSeq));

	if (pTarget->pev->sequence == -1)
	{
		ALERT(at_error, "%s: unknown aiscripted sequence \"%s\"\n", STRING(pTarget->pev->targetname), STRING(iszSeq));
		pTarget->pev->sequence = 0;
		// return false;
	}

	pTarget->pev->frame = 0;
	pTarget->ResetSequenceInfo();
	return true;
}

void CCineAI::FixScriptMonsterSchedule(CBaseMonster* pMonster)
{
	switch (m_iFinishSchedule)
	{
	case SCRIPT_FINISHSCHED_DEFAULT:
		pMonster->ClearSchedule();
		break;
	case SCRIPT_FINISHSCHED_AMBUSH:
		pMonster->ChangeSchedule(pMonster->GetScheduleOfType(SCHED_AMBUSH));
		break;
	default:
		ALERT(at_aiconsole, "FixScriptMonsterSchedule - no case!\n");
		pMonster->ClearSchedule();
		break;
	}
}
