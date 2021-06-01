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

#include "animation.hpp"
#include "CCineMonster.hpp"
#include "CBaseMonster.defaultai.hpp"

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

LINK_ENTITY_TO_CLASS(scripted_sequence, CCineMonster);

void CCineMonster::Spawn()
{
	// SetSolidType(Solid::Trigger);
	// SetSize( Vector(-8, -8, -8), Vector(8, 8, 8));
	SetSolidType(Solid::Not);

	// REMOVE: The old side-effect
#if 0
	if (m_iszIdle)
		m_fMoveTo = 4;
#endif

	// if no targetname, start now
	if (!HasTargetname() || !IsStringNull(m_iszIdle))
	{
		SetThink(&CCineMonster::CineThink);
		pev->nextthink = gpGlobals->time + 1.0;
		// Wait to be used?
		if (HasTargetname())
			m_startTime = gpGlobals->time + 1E6;
	}

	m_interruptable = !(pev->spawnflags & SF_SCRIPT_NOINTERRUPT);
}

bool CCineMonster::CanOverrideState()
{
	return (pev->spawnflags & SF_SCRIPT_OVERRIDESTATE) != 0;
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


	SetSolidType(Solid::Not);// kill the trigger for now !!!UNDONE
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
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, GetAbsOrigin(), m_flRadius)) != nullptr)
		{
			if (pEntity->ClassnameIs(STRING(m_iszEntity)))
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
	//TOOD: most of this is identical to aiscripted_sequence
	CBaseMonster* pTarget = nullptr;
	if (CBaseEntity* pEntity = m_hTargetEnt; pEntity)
		pTarget = pEntity->MyMonsterPointer();

	if (pTarget)
	{

		// FindEntity() just checked this!
#if 0
		if (!pTarget->CanPlaySequence(CanOverrideState()))
		{
			ALERT(at_aiconsole, "Can't possess entity %s\n", pTarget->GetClassname());
			return;
		}
#endif

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
			pTarget->SetAbsOrigin(GetAbsOrigin());
			pTarget->pev->ideal_yaw = GetAbsAngles().y;
			pTarget->pev->avelocity = vec3_origin;
			pTarget->SetAbsVelocity(vec3_origin);
			pTarget->pev->effects |= EF_NOINTERP;
			pTarget->SetAbsAngles({0, GetAbsAngles().y, 0});
			pTarget->m_scriptState = ScriptState::Wait;
			m_startTime = gpGlobals->time + 1E6;
			// UNDONE: Add a flag to do this so people can fixup physics after teleporting monsters
			//			pTarget->pev->flags &= ~FL_ONGROUND;
			break;
		}
		//		ALERT( at_aiconsole, "\"%s\" found and used (INT: %s)\n", pTarget->GetTargetname(), IsBitSet(pev->spawnflags, SF_SCRIPT_NOINTERRUPT)?"No":"Yes" );

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

void CCineMonster::CineThink()
{
	if (FindEntity())
	{
		PossessEntity();
		ALERT(at_aiconsole, "script \"%s\" using monster \"%s\"\n", GetTargetname(), STRING(m_iszEntity));
	}
	else
	{
		CancelScript();
		ALERT(at_aiconsole, "script \"%s\" can't find monster \"%s\"\n", GetTargetname(), STRING(m_iszEntity));
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
		ALERT(at_error, "%s: unknown scripted sequence \"%s\"\n", pTarget->GetTargetname(), STRING(iszSeq));
		pTarget->pev->sequence = 0;
		// return false;
	}

#if 0
	const char* s = !(pev->spawnflags & SF_SCRIPT_NOINTERRUPT) ? "Yes" : "No";

	ALERT(at_console, "%s (%s): started \"%s\":INT:%s\n", pTarget->GetTargetname(), pTarget->GetClassname(), STRING(iszSeq), s);
#endif

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
	SUB_UseTargets(nullptr, UseType::Toggle, 0);
}

void CCineMonster::FixScriptMonsterSchedule(CBaseMonster* pMonster)
{
	if (pMonster->m_IdealMonsterState != NPCState::Dead)
		pMonster->m_IdealMonsterState = NPCState::Idle;
	pMonster->ClearSchedule();
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

	if (auto cine = m_hCine.Get(); cine)
	{
		cine->CancelScript();
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

void ScriptEntityCancel(CBaseEntity* pCine)
{
	// make sure they are a scripted_sequence
	if (pCine->ClassnameIs("scripted_sequence"))
	{
		auto pCineTarget = static_cast<CCineMonster*>(pCine);
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

	if (!HasTargetname())
	{
		ScriptEntityCancel(this);
		return;
	}

	CBaseEntity* pCineTarget = nullptr;

	while ((pCineTarget = UTIL_FindEntityByTargetname(pCineTarget, GetTargetname())) != nullptr)
	{
		ScriptEntityCancel(pCineTarget);
	}
}

void CCineMonster::DelayStart(int state)
{
	CBaseEntity* pCine = nullptr;

	while ((pCine = UTIL_FindEntityByTargetname(pCine, GetTargetname())) != nullptr)
	{
		if (pCine->ClassnameIs("scripted_sequence"))
		{
			CCineMonster* pTarget = (CCineMonster*)pCine;
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

	{
		CBaseEntity* pCandidate = nullptr;

		while (!pTarget && (pCandidate = UTIL_FindEntityByTargetname(pCandidate, STRING(m_iszEntity))) != nullptr)
		{
			if (IsBitSet(pCandidate->pev->flags, FL_MONSTER))
			{
				pTarget = pCandidate->MyMonsterPointer();
			}
		}
	}

	// If no entity with that targetname, check the classname
	if (!pTarget)
	{
		CBaseEntity* pCandidate = nullptr;

		while (!pTarget && (pCandidate = UTIL_FindEntityByClassname(pCandidate, STRING(m_iszEntity))) != nullptr)
		{
			pTarget = pCandidate->MyMonsterPointer();
		}
	}
	// Found a compatible entity
	if (pTarget)
	{
		if (void* pmodel = pTarget->GetModelPointer(); pmodel)
		{
			// Look through the event list for stuff to precache
			SequencePrecache(pmodel, STRING(m_iszIdle));
			SequencePrecache(pmodel, STRING(m_iszPlay));
		}
	}
}

bool CBaseMonster::CineCleanup()
{
	CCineMonster* pOldCine = m_hCine;

	// am I linked to a cinematic?
	if (auto cine = m_hCine.Get(); cine)
	{
		// okay, reset me to what it thought I was before
		cine->m_hTargetEnt = nullptr;
		SetMovetype(cine->m_saved_movetype);
		SetSolidType(cine->m_saved_solid);
		pev->effects = cine->m_saved_effects;
	}
	else
	{
		// arg, punt
		SetMovetype(Movetype::Step);// this is evil
		SetSolidType(Solid::SlideBox);
	}
	m_hCine = nullptr;
	m_hTargetEnt = nullptr;
	m_hGoalEnt = nullptr;
	if (pev->deadflag == DeadFlag::Dying)
	{
		// last frame of death animation?
		pev->health = 0;
		pev->framerate = 0.0;
		SetSolidType(Solid::Not);
		SetState(NPCState::Dead);
		pev->deadflag = DeadFlag::Dead;
		SetSize(pev->mins, Vector(pev->maxs.x, pev->maxs.y, pev->mins.z + 2));

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
		SetMovetype(Movetype::None);
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
			const Vector oldOrigin = GetAbsOrigin();

			// UNDONE: ugly hack.  Don't move monster if they don't "seem" to move
			// this really needs to be done with the AX,AY,etc. flags, but that aren't consistantly
			// being set, so animations that really do move won't be caught.
			if ((oldOrigin - new_origin).Length2D() < 8.0)
				new_origin = oldOrigin;

			SetAbsOrigin({new_origin.x, new_origin.y, oldOrigin.z + 1});

			pev->flags |= FL_ONGROUND;
			const int drop = DROP_TO_FLOOR(edict());

			// Origin in solid?  Set to org at the end of the sequence
			if (drop < 0)
				SetAbsOrigin(oldOrigin);
			else if (drop == 0) // Hanging in air?
			{
				SetAbsOrigin({GetAbsOrigin().x, GetAbsOrigin().y, new_origin.z});
				pev->flags &= ~FL_ONGROUND;
			}
			// else entity hit floor, leave there

			//SetAbsOrigin({GetAbsOrigin().x, GetAbsOrigin().y, new_origin.z + 5.0f}); // damn, got to fix this

			pev->effects |= EF_NOINTERP;
		}

		// We should have some animation to put these guys in, but for now it's idle.
		// Due to NOINTERP above, there won't be any blending between this anim & the sequence
		m_Activity = ACT_RESET;
	}
	// set them back into a normal state
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
