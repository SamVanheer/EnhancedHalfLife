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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "animation.h"
#include "schedule.h"
#include "scripted.h"
#include "defaultai.h"

void CCineMonster::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "m_iszIdle"))
	{
		m_iszIdle = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_iszPlay"))
	{
		m_iszPlay = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_iszEntity"))
	{
		m_iszEntity = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_fMoveTo"))
	{
		//TODO: validate input
		m_fMoveTo = static_cast<ScriptedMoveTo>(atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_flRepeat"))
	{
		m_flRepeat = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_flRadius"))
	{
		m_flRadius = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "m_iFinishSchedule"))
	{
		m_iFinishSchedule = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
	{
		CBaseMonster::KeyValue(pkvd);
	}
}

TYPEDESCRIPTION	CCineMonster::m_SaveData[] =
{
	DEFINE_FIELD(CCineMonster, m_iszIdle, FIELD_STRING),
	DEFINE_FIELD(CCineMonster, m_iszPlay, FIELD_STRING),
	DEFINE_FIELD(CCineMonster, m_iszEntity, FIELD_STRING),
	DEFINE_FIELD(CCineMonster, m_fMoveTo, FIELD_INTEGER),
	DEFINE_FIELD(CCineMonster, m_flRepeat, FIELD_FLOAT),
	DEFINE_FIELD(CCineMonster, m_flRadius, FIELD_FLOAT),

	DEFINE_FIELD(CCineMonster, m_iDelay, FIELD_INTEGER),
	DEFINE_FIELD(CCineMonster, m_startTime, FIELD_TIME),

	DEFINE_FIELD(CCineMonster,	m_saved_movetype, FIELD_INTEGER),
	DEFINE_FIELD(CCineMonster,	m_saved_solid, FIELD_INTEGER),
	DEFINE_FIELD(CCineMonster, m_saved_effects, FIELD_INTEGER),
	DEFINE_FIELD(CCineMonster, m_iFinishSchedule, FIELD_INTEGER),
	DEFINE_FIELD(CCineMonster, m_interruptable, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CCineMonster, CBaseMonster);

LINK_ENTITY_TO_CLASS(scripted_sequence, CCineMonster);

LINK_ENTITY_TO_CLASS(aiscripted_sequence, CCineAI);

void CCineMonster::Spawn()
{
	// pev->solid = Solid::Trigger;
	// UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
	pev->solid = Solid::Not;

	// REMOVE: The old side-effect
#if 0
	if (m_iszIdle)
		m_fMoveTo = 4;
#endif

	// if no targetname, start now
	if (IsStringNull(pev->targetname) || !IsStringNull(m_iszIdle))
	{
		SetThink(&CCineMonster::CineThink);
		pev->nextthink = gpGlobals->time + 1.0;
		// Wait to be used?
		if (!IsStringNull(pev->targetname))
			m_startTime = gpGlobals->time + 1E6;
	}

	m_interruptable = !(pev->spawnflags & SF_SCRIPT_NOINTERRUPT);
}

bool CCineMonster::CanOverrideState()
{
	return (pev->spawnflags & SF_SCRIPT_OVERRIDESTATE) != 0;
}

bool CCineAI::CanOverrideState()
{
	return true;
}

void CCineMonster::Use(const UseInfo& info)
{
	// do I already know who I should use
	CBaseMonster* pTarget = nullptr;

	if (CBaseEntity* pEntity = m_hTargetEnt; pEntity)
		pTarget = pEntity->MyMonsterPointer();

	if (pTarget)
	{
		// am I already playing the script?
		if (pTarget->m_scriptState == ScriptState::Playing)
			return;

		m_startTime = gpGlobals->time + 0.05;
	}
	else
	{
		// if not, try finding them
		SetThink(&CCineMonster::CineThink);
		pev->nextthink = gpGlobals->time;
	}
}

void CCineMonster::Blocked(CBaseEntity* pOther)
{
}

void CCineMonster::Touch(CBaseEntity* pOther)
{
	/*
		ALERT( at_aiconsole, "Cine Touch\n" );
		if (m_pentTarget && OFFSET(pOther->pev) == OFFSET(m_pentTarget))
		{
			CBaseMonster *pTarget = GetClassPtr((CBaseMonster *)VARS(m_pentTarget));
			pTarget->m_monsterState == NPCState::Script;
		}
	*/
}

/*
	entvars_t *pevOther = VARS( gpGlobals->other );

	if ( !IsBitSet ( pevOther->flags , FL_MONSTER ) )
	{// touched by a non-monster.
		return;
	}

	pevOther->origin.z += 1;

	if ( IsBitSet ( pevOther->flags, FL_ONGROUND ) )
	{// clear the onground so physics don't bitch
		pevOther->flags -= FL_ONGROUND;
	}

	// toss the monster!
	pevOther->velocity = pev->movedir * pev->speed;
	pevOther->velocity.z += m_flHeight;


	pev->solid = Solid::Not;// kill the trigger for now !!!UNDONE
}
*/

bool CCineMonster::FindEntity()
{
	m_hTargetEnt = nullptr;
	CBaseMonster* pTargetMonster = nullptr;

	{
		CBaseEntity* pTarget = nullptr;

		while ((pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(m_iszEntity))) != nullptr)
		{
			if (IsBitSet(pTarget->pev->flags, FL_MONSTER))
			{
				pTargetMonster = pTarget->MyMonsterPointer();
				if (pTargetMonster && pTargetMonster->CanPlaySequence(CanOverrideState(), SS_INTERRUPT_BY_NAME))
				{
					m_hTargetEnt = pTargetMonster;
					return true;
				}
				ALERT(at_console, "Found %s, but can't play!\n", STRING(m_iszEntity));
			}

			pTargetMonster = nullptr;
		}
	}

	//TODO: at this point this will always be null, so it's pointless to check for that
	if (!pTargetMonster)
	{
		CBaseEntity* pEntity = nullptr;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, m_flRadius)) != nullptr)
		{
			if (ClassnameIs(pEntity->pev, STRING(m_iszEntity)))
			{
				if (IsBitSet(pEntity->pev->flags, FL_MONSTER))
				{
					pTargetMonster = pEntity->MyMonsterPointer();
					if (pTargetMonster && pTargetMonster->CanPlaySequence(CanOverrideState(), SS_INTERRUPT_IDLE))
					{
						m_hTargetEnt = pTargetMonster;
						return true;
					}
				}
			}
		}
	}
	//TODO: pointlessly assigning null to both of these; already done above and will always be null if we reach this
	pTargetMonster = nullptr;
	m_hTargetEnt = nullptr;
	return false;
}

void CCineMonster::PossessEntity()
{
	CBaseMonster* pTarget = nullptr;
	if (CBaseEntity* pEntity = m_hTargetEnt; pEntity)
		pTarget = pEntity->MyMonsterPointer();

	if (pTarget)
	{

		// FindEntity() just checked this!
#if 0
		if (!pTarget->CanPlaySequence(CanOverrideState()))
		{
			ALERT(at_aiconsole, "Can't possess entity %s\n", STRING(pTarget->pev->classname));
			return;
		}
#endif

		pTarget->m_pGoalEnt = this;
		pTarget->m_pCine = this;
		pTarget->m_hTargetEnt = this;

		m_saved_movetype = pTarget->pev->movetype;
		m_saved_solid = pTarget->pev->solid;
		m_saved_effects = pTarget->pev->effects;
		pTarget->pev->effects |= pev->effects;

		switch (m_fMoveTo)
		{
		case ScriptedMoveTo::No:
			pTarget->m_scriptState = ScriptState::Wait;
			break;

		case ScriptedMoveTo::Walk:
			pTarget->m_scriptState = ScriptState::WalkToMark;
			DelayStart(1);
			break;

		case ScriptedMoveTo::Run:
			pTarget->m_scriptState = ScriptState::RunToMark;
			DelayStart(1);
			break;

		case ScriptedMoveTo::Instantaneous:
			UTIL_SetOrigin(pTarget->pev, pev->origin);
			pTarget->pev->ideal_yaw = pev->angles.y;
			pTarget->pev->avelocity = vec3_origin;
			pTarget->pev->velocity = vec3_origin;
			pTarget->pev->effects |= EF_NOINTERP;
			pTarget->pev->angles.y = pev->angles.y;
			pTarget->m_scriptState = ScriptState::Wait;
			m_startTime = gpGlobals->time + 1E6;
			// UNDONE: Add a flag to do this so people can fixup physics after teleporting monsters
			//			pTarget->pev->flags &= ~FL_ONGROUND;
			break;
		}
		//		ALERT( at_aiconsole, "\"%s\" found and used (INT: %s)\n", STRING( pTarget->pev->targetname ), IsBitSet(pev->spawnflags, SF_SCRIPT_NOINTERRUPT)?"No":"Yes" );

		pTarget->m_IdealMonsterState = NPCState::Script;
		if (!IsStringNull(m_iszIdle))
		{
			StartSequence(pTarget, m_iszIdle, false);
			if (AreStringsEqual(STRING(m_iszIdle), STRING(m_iszPlay)))
			{
				pTarget->pev->framerate = 0;
			}
		}
	}
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
			ALERT(at_aiconsole, "(AI)Can't possess entity %s\n", STRING(pTarget->pev->classname));
			return;
		}

		pTarget->m_pGoalEnt = this;
		pTarget->m_pCine = this;
		pTarget->m_hTargetEnt = this;

		m_saved_movetype = pTarget->pev->movetype;
		m_saved_solid = pTarget->pev->solid;
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
			UTIL_SetOrigin(pTarget->pev, pev->origin);
			pTarget->pev->ideal_yaw = pev->angles.y;
			pTarget->pev->avelocity = vec3_origin;
			pTarget->pev->velocity = vec3_origin;
			pTarget->pev->effects |= EF_NOINTERP;
			pTarget->pev->angles.y = pev->angles.y;
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

void CCineMonster::CineThink()
{
	if (FindEntity())
	{
		PossessEntity();
		ALERT(at_aiconsole, "script \"%s\" using monster \"%s\"\n", STRING(pev->targetname), STRING(m_iszEntity));
	}
	else
	{
		CancelScript();
		ALERT(at_aiconsole, "script \"%s\" can't find monster \"%s\"\n", STRING(pev->targetname), STRING(m_iszEntity));
		pev->nextthink = gpGlobals->time + 1.0;
	}
}

bool CCineMonster::StartSequence(CBaseMonster* pTarget, string_t iszSeq, bool completeOnEmpty)
{
	if (IsStringNull(iszSeq) && completeOnEmpty)
	{
		SequenceDone(pTarget);
		return false;
	}

	pTarget->pev->sequence = pTarget->LookupSequence(STRING(iszSeq));
	if (pTarget->pev->sequence == -1)
	{
		ALERT(at_error, "%s: unknown scripted sequence \"%s\"\n", STRING(pTarget->pev->targetname), STRING(iszSeq));
		pTarget->pev->sequence = 0;
		// return false;
	}

#if 0
	const char* s = !(pev->spawnflags & SF_SCRIPT_NOINTERRUPT) ? "Yes" : "No";

	ALERT(at_console, "%s (%s): started \"%s\":INT:%s\n", STRING(pTarget->pev->targetname), STRING(pTarget->pev->classname), STRING(iszSeq), s);
#endif

	pTarget->pev->frame = 0;
	pTarget->ResetSequenceInfo();
	return true;
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

void CCineMonster::SequenceDone(CBaseMonster* pMonster)
{
	//ALERT( at_aiconsole, "Sequence %s finished\n", STRING( m_pCine->m_iszPlay ) );

	if (!(pev->spawnflags & SF_SCRIPT_REPEATABLE))
	{
		SetThink(&CCineMonster::SUB_Remove);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	// This is done so that another sequence can take over the monster when triggered by the first

	pMonster->CineCleanup();

	FixScriptMonsterSchedule(pMonster);

	// This may cause a sequence to attempt to grab this guy NOW, so we have to clear him out
	// of the existing sequence
	SUB_UseTargets(nullptr, USE_TOGGLE, 0);
}

void CCineMonster::FixScriptMonsterSchedule(CBaseMonster* pMonster)
{
	if (pMonster->m_IdealMonsterState != NPCState::Dead)
		pMonster->m_IdealMonsterState = NPCState::Idle;
	pMonster->ClearSchedule();
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

bool CBaseMonster::ExitScriptedSequence()
{
	if (pev->deadflag == DeadFlag::Dying)
	{
		// is this legal?
		// BUGBUG -- This doesn't call Killed()
		m_IdealMonsterState = NPCState::Dead;
		return false;
	}

	if (m_pCine)
	{
		m_pCine->CancelScript();
	}

	return true;
}

void CCineMonster::AllowInterrupt(bool fAllow)
{
	if (pev->spawnflags & SF_SCRIPT_NOINTERRUPT)
		return;
	m_interruptable = fAllow;
}

bool CCineMonster::CanInterrupt()
{
	if (!m_interruptable)
		return false;

	CBaseEntity* pTarget = m_hTargetEnt;

	return pTarget != nullptr && pTarget->pev->deadflag == DeadFlag::No;
}

int	CCineMonster::IgnoreConditions()
{
	if (CanInterrupt())
		return 0;
	return SCRIPT_BREAK_CONDITIONS;
}

void ScriptEntityCancel(edict_t* pentCine)
{
	// make sure they are a scripted_sequence
	if (ClassnameIs(pentCine, "scripted_sequence"))
	{
		CCineMonster* pCineTarget = GetClassPtr((CCineMonster*)VARS(pentCine));
		// make sure they have a monster in mind for the script
		CBaseMonster* pTarget = nullptr;
		if (CBaseEntity* pEntity = pCineTarget->m_hTargetEnt; pEntity)
			pTarget = pEntity->MyMonsterPointer();

		if (pTarget)
		{
			// make sure their monster is actually playing a script
			if (pTarget->m_MonsterState == NPCState::Script)
			{
				// tell them do die
				pTarget->m_scriptState = CCineMonster::ScriptState::Cleanup;
				// do it now
				pTarget->CineCleanup();
			}
		}
	}
}

void CCineMonster::CancelScript()
{
	ALERT(at_aiconsole, "Cancelling script: %s\n", STRING(m_iszPlay));

	if (IsStringNull(pev->targetname))
	{
		ScriptEntityCancel(edict());
		return;
	}

	CBaseEntity* pCineTarget = nullptr;

	while ((pCineTarget = UTIL_FindEntityByTargetname(pCineTarget, STRING(pev->targetname))) != nullptr)
	{
		ScriptEntityCancel(pCineTarget->edict());
	}
}

void CCineMonster::DelayStart(int state)
{
	CBaseEntity* pCine = nullptr;

	while ((pCine = UTIL_FindEntityByTargetname(pCine, STRING(pev->targetname))) != nullptr)
	{
		if (ClassnameIs(pCine->pev, "scripted_sequence"))
		{
			CCineMonster* pTarget = (CCineMonster*) pCine;
			if (state)
			{
				pTarget->m_iDelay++;
			}
			else
			{
				pTarget->m_iDelay--;
				if (pTarget->m_iDelay <= 0)
					pTarget->m_startTime = gpGlobals->time + 0.05;
			}
		}
	}
}

void CCineMonster::Activate()
{
	// The entity name could be a target name or a classname
	// Check the targetname
	CBaseMonster* pTarget = nullptr;

	CBaseEntity* pCandidate = nullptr;

	while (!pTarget && (pCandidate = UTIL_FindEntityByTargetname(pCandidate, STRING(m_iszEntity))) != nullptr)
	{
		if (IsBitSet(pCandidate->pev->flags, FL_MONSTER))
		{
			pTarget = pCandidate->MyMonsterPointer();
		}
	}

	// If no entity with that targetname, check the classname
	if (!pTarget)
	{
		edict_t* pentTarget = FIND_ENTITY_BY_CLASSNAME(nullptr, STRING(m_iszEntity));
		while (!pTarget && !IsNullEnt(pentTarget))
		{
			pTarget = GetMonsterPointer(pentTarget);
			//TODO: should be using classname lookup here!
			pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(m_iszEntity));
		}
	}
	// Found a compatible entity
	if (pTarget)
	{
		if (void* pmodel = GET_MODEL_PTR(pTarget->edict()); pmodel)
		{
			// Look through the event list for stuff to precache
			SequencePrecache(pmodel, STRING(m_iszIdle));
			SequencePrecache(pmodel, STRING(m_iszPlay));
		}
	}
}

bool CBaseMonster::CineCleanup()
{
	CCineMonster* pOldCine = m_pCine;

	// am I linked to a cinematic?
	if (m_pCine)
	{
		// okay, reset me to what it thought I was before
		m_pCine->m_hTargetEnt = nullptr;
		pev->movetype = m_pCine->m_saved_movetype;
		pev->solid = m_pCine->m_saved_solid;
		pev->effects = m_pCine->m_saved_effects;
	}
	else
	{
		// arg, punt
		pev->movetype = Movetype::Step;// this is evil
		pev->solid = Solid::SlideBox;
	}
	m_pCine = nullptr;
	m_hTargetEnt = nullptr;
	m_pGoalEnt = nullptr;
	if (pev->deadflag == DeadFlag::Dying)
	{
		// last frame of death animation?
		pev->health = 0;
		pev->framerate = 0.0;
		pev->solid = Solid::Not;
		SetState(NPCState::Dead);
		pev->deadflag = DeadFlag::Dead;
		UTIL_SetSize(pev, pev->mins, Vector(pev->maxs.x, pev->maxs.y, pev->mins.z + 2));

		if (pOldCine && IsBitSet(pOldCine->pev->spawnflags, SF_SCRIPT_LEAVECORPSE))
		{
			SetUse(nullptr);		// BUGBUG -- This doesn't call Killed()
			SetThink(nullptr);	// This will probably break some stuff
			SetTouch(nullptr);
		}
		else
			SUB_StartFadeOut(); // SetThink( SUB_DoNothing );
		// This turns off animation & physics in case their origin ends up stuck in the world or something
		StopAnimation();
		pev->movetype = Movetype::None;
		pev->effects |= EF_NOINTERP;	// Don't interpolate either, assume the corpse is positioned in its final resting place
		return false;
	}

	// If we actually played a sequence
	if (pOldCine && !IsStringNull(pOldCine->m_iszPlay))
	{
		if (!(pOldCine->pev->spawnflags & SF_SCRIPT_NOSCRIPTMOVEMENT))
		{
			// reset position
			Vector new_origin, new_angle;
			GetBonePosition(0, new_origin, new_angle);

			// Figure out how far they have moved
			// We can't really solve this problem because we can't query the movement of the origin relative
			// to the sequence.  We can get the root bone's position as we do here, but there are
			// cases where the root bone is in a different relative position to the entity's origin
			// before/after the sequence plays.  So we are stuck doing this:

			// !!!HACKHACK: Float the origin up and drop to floor because some sequences have
			// irregular motion that can't be properly accounted for.

			// UNDONE: THIS SHOULD ONLY HAPPEN IF WE ACTUALLY PLAYED THE SEQUENCE.
			const Vector oldOrigin = pev->origin;

			// UNDONE: ugly hack.  Don't move monster if they don't "seem" to move
			// this really needs to be done with the AX,AY,etc. flags, but that aren't consistantly
			// being set, so animations that really do move won't be caught.
			if ((oldOrigin - new_origin).Length2D() < 8.0)
				new_origin = oldOrigin;

			pev->origin.x = new_origin.x;
			pev->origin.y = new_origin.y;
			pev->origin.z += 1;

			pev->flags |= FL_ONGROUND;
			const int drop = DROP_TO_FLOOR(ENT(pev));

			// Origin in solid?  Set to org at the end of the sequence
			if (drop < 0)
				pev->origin = oldOrigin;
			else if (drop == 0) // Hanging in air?
			{
				pev->origin.z = new_origin.z;
				pev->flags &= ~FL_ONGROUND;
			}
			// else entity hit floor, leave there

			// pEntity->pev->origin.z = new_origin.z + 5.0; // damn, got to fix this

			UTIL_SetOrigin(pev, pev->origin);
			pev->effects |= EF_NOINTERP;
		}

		// We should have some animation to put these guys in, but for now it's idle.
		// Due to NOINTERP above, there won't be any blending between this anim & the sequence
		m_Activity = ACT_RESET;
	}
	// set them back into a normal state
	pev->enemy = nullptr;
	if (pev->health > 0)
		m_IdealMonsterState = NPCState::Idle; // m_previousState;
	else
	{
		// Dropping out because he got killed
		// Can't call killed() no attacker and weirdness (late gibbing) may result
		m_IdealMonsterState = NPCState::Dead;
		SetConditions(bits_COND_LIGHT_DAMAGE);
		pev->deadflag = DeadFlag::Dying;
		CheckAITrigger();
		pev->deadflag = DeadFlag::No;
	}

	//	SetAnimation( m_MonsterState );
	ClearBits(pev->spawnflags, SF_MONSTER_WAIT_FOR_SCRIPT);

	return true;
}

enum class SentenceAttenuation
{
	SmallRadius,
	MediumRadius,
	LargeRadius,
	PlayEverywhere
};

class CScriptedSentence : public CBaseToggle
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Use(const UseInfo& info) override;
	void EXPORT FindThink();
	void EXPORT DelayThink();
	int	 ObjectCaps() override { return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	CBaseMonster* FindEntity();
	bool AcceptableSpeaker(CBaseMonster* pMonster);
	bool StartSentence(CBaseMonster* pTarget);


private:
	string_t m_iszSentence;		// string index for idle animation
	string_t m_iszEntity;	// entity that is wanted for this sentence
	float	m_flRadius;		// range to search
	float	m_flDuration;	// How long the sentence lasts
	float	m_flRepeat;	// repeat rate
	float	m_flAttenuation;
	float	m_flVolume;
	bool	m_active;
	string_t m_iszListener;	// name of entity to look at while talking
};

constexpr int SF_SENTENCE_ONCE = 0x0001;
constexpr int SF_SENTENCE_FOLLOWERS = 0x0002;	//!< only say if following player
constexpr int SF_SENTENCE_INTERRUPT = 0x0004;	//!< force talking except when dead
constexpr int SF_SENTENCE_CONCURRENT = 0x0008;	//!< allow other people to keep talking

TYPEDESCRIPTION	CScriptedSentence::m_SaveData[] =
{
	DEFINE_FIELD(CScriptedSentence, m_iszSentence, FIELD_STRING),
	DEFINE_FIELD(CScriptedSentence, m_iszEntity, FIELD_STRING),
	DEFINE_FIELD(CScriptedSentence, m_flRadius, FIELD_FLOAT),
	DEFINE_FIELD(CScriptedSentence, m_flDuration, FIELD_FLOAT),
	DEFINE_FIELD(CScriptedSentence, m_flRepeat, FIELD_FLOAT),
	DEFINE_FIELD(CScriptedSentence, m_flAttenuation, FIELD_FLOAT),
	DEFINE_FIELD(CScriptedSentence, m_flVolume, FIELD_FLOAT),
	DEFINE_FIELD(CScriptedSentence, m_active, FIELD_BOOLEAN),
	DEFINE_FIELD(CScriptedSentence, m_iszListener, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CScriptedSentence, CBaseToggle);

LINK_ENTITY_TO_CLASS(scripted_sentence, CScriptedSentence);

void CScriptedSentence::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "sentence"))
	{
		m_iszSentence = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "entity"))
	{
		m_iszEntity = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "duration"))
	{
		m_flDuration = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "refire"))
	{
		m_flRepeat = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "attenuation"))
	{
		pev->impulse = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "volume"))
	{
		m_flVolume = atof(pkvd->szValue) * 0.1;
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "listener"))
	{
		m_iszListener = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CScriptedSentence::Use(const UseInfo& info)
{
	if (!m_active)
		return;
	//	ALERT( at_console, "Firing sentence: %s\n", STRING(m_iszSentence) );
	SetThink(&CScriptedSentence::FindThink);
	pev->nextthink = gpGlobals->time;
}

void CScriptedSentence::Spawn()
{
	pev->solid = Solid::Not;

	m_active = true;
	// if no targetname, start now
	if (IsStringNull(pev->targetname))
	{
		SetThink(&CScriptedSentence::FindThink);
		pev->nextthink = gpGlobals->time + 1.0;
	}

	switch (static_cast<SentenceAttenuation>(pev->impulse))
	{
	case SentenceAttenuation::MediumRadius: // Medium radius
		m_flAttenuation = ATTN_STATIC;
		break;

	case SentenceAttenuation::LargeRadius:	// Large radius
		m_flAttenuation = ATTN_NORM;
		break;

	case SentenceAttenuation::PlayEverywhere:	//EVERYWHERE
		m_flAttenuation = ATTN_NONE;
		break;

	default:
	case SentenceAttenuation::SmallRadius: // Small radius
		m_flAttenuation = ATTN_IDLE;
		break;
	}
	pev->impulse = 0;

	// No volume, use normal
	if (m_flVolume <= 0)
		m_flVolume = 1.0;
}

void CScriptedSentence::FindThink()
{
	if (CBaseMonster* pMonster = FindEntity(); pMonster)
	{
		StartSentence(pMonster);
		if (pev->spawnflags & SF_SENTENCE_ONCE)
			UTIL_Remove(this);
		SetThink(&CScriptedSentence::DelayThink);
		pev->nextthink = gpGlobals->time + m_flDuration + m_flRepeat;
		m_active = false;
		//		ALERT( at_console, "%s: found monster %s\n", STRING(m_iszSentence), STRING(m_iszEntity) );
	}
	else
	{
		//		ALERT( at_console, "%s: can't find monster %s\n", STRING(m_iszSentence), STRING(m_iszEntity) );
		pev->nextthink = gpGlobals->time + m_flRepeat + 0.5;
	}
}

void CScriptedSentence::DelayThink()
{
	m_active = true;
	if (IsStringNull(pev->targetname))
		pev->nextthink = gpGlobals->time + 0.1;
	SetThink(&CScriptedSentence::FindThink);
}

bool CScriptedSentence::AcceptableSpeaker(CBaseMonster* pMonster)
{
	if (pMonster)
	{
		if (pev->spawnflags & SF_SENTENCE_FOLLOWERS)
		{
			if (pMonster->m_hTargetEnt == nullptr || !pMonster->m_hTargetEnt->IsPlayer())
				return false;
		}
		const bool override = (pev->spawnflags & SF_SENTENCE_INTERRUPT) != 0;

		if (pMonster->CanPlaySentence(override))
			return true;
	}
	return false;
}

CBaseMonster* CScriptedSentence::FindEntity()
{
	CBaseEntity* pTarget = nullptr;

	while ((pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(m_iszEntity))) != nullptr)
	{
		if (CBaseMonster* pMonster = pTarget->MyMonsterPointer(); pMonster != nullptr)
		{
			if (AcceptableSpeaker(pMonster))
				return pMonster;
			//			ALERT( at_console, "%s (%s), not acceptable\n", STRING(pMonster->pev->classname), STRING(pMonster->pev->targetname) );
		}
	}

	pTarget = nullptr;
	while ((pTarget = UTIL_FindEntityInSphere(pTarget, pev->origin, m_flRadius)) != nullptr)
	{
		if (ClassnameIs(pTarget->pev, STRING(m_iszEntity)))
		{
			if (IsBitSet(pTarget->pev->flags, FL_MONSTER))
			{
				if (CBaseMonster* pMonster = pTarget->MyMonsterPointer(); AcceptableSpeaker(pMonster))
					return pMonster;
			}
		}
	}

	return nullptr;
}

bool CScriptedSentence::StartSentence(CBaseMonster* pTarget)
{
	if (!pTarget)
	{
		ALERT(at_aiconsole, "Not Playing sentence %s\n", STRING(m_iszSentence));
		return false;
	}

	CBaseEntity* pListener = nullptr;
	if (!IsStringNull(m_iszListener))
	{
		float radius = m_flRadius;

		if (AreStringsEqual(STRING(m_iszListener), "player"))
			radius = WORLD_BOUNDARY;	// Always find the player

		pListener = UTIL_FindEntityGeneric(STRING(m_iszListener), pTarget->pev->origin, radius);
	}

	const bool bConcurrent = !(pev->spawnflags & SF_SENTENCE_CONCURRENT);

	pTarget->PlayScriptedSentence(STRING(m_iszSentence), m_flDuration, m_flVolume, m_flAttenuation, bConcurrent, pListener);
	ALERT(at_aiconsole, "Playing sentence %s (%.1f)\n", STRING(m_iszSentence), m_flDuration);
	SUB_UseTargets(nullptr, USE_TOGGLE, 0);
	return true;
}

/**
*	@brief this is the cool comment I cut-and-pasted
*/
class CFurniture : public CBaseMonster
{
public:
	/**
	*	@brief This used to have something to do with bees flying, but now it only initializes moving furniture in scripted sequences
	*/
	void Spawn() override;

	/**
	*	@brief ID's Furniture as neutral (noone will attack it)
	*/
	int	 Classify() override;
	int	ObjectCaps() override { return (CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
};

LINK_ENTITY_TO_CLASS(monster_furniture, CFurniture);

void CFurniture::Spawn()
{
	PRECACHE_MODEL(STRING(pev->model));
	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->movetype = Movetype::None;
	pev->solid = Solid::BBox;
	pev->health = 80000;
	SetDamageMode(DamageMode::Aim);
	pev->effects = 0;
	pev->yaw_speed = 0;
	pev->sequence = 0;
	pev->frame = 0;

	//	pev->nextthink += 1.0;
	//	SetThink (WalkMonsterDelay);

	ResetSequenceInfo();
	pev->frame = 0;
	MonsterInit();
}

int CFurniture::Classify()
{
	return	CLASS_NONE;
}
