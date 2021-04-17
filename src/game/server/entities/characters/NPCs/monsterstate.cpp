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

/**
*	@file
*
*	base class monster functions for controlling core AI.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "monsters.h"
#include "animation.h"
#include "soundent.h"

//=========================================================
// SetState
//=========================================================
void CBaseMonster::SetState(NPCState State)
{
	/*
		if ( State != m_MonsterState )
		{
			ALERT ( at_aiconsole, "State Changed to %d\n", State );
		}
	*/

	switch (State)
	{

		// Drop enemy pointers when going to idle
	case NPCState::Idle:

		if (m_hEnemy != nullptr)
		{
			m_hEnemy = nullptr;// not allowed to have an enemy anymore.
			ALERT(at_aiconsole, "Stripped\n");
		}
		break;
	}

	m_MonsterState = State;
	m_IdealMonsterState = State;
}

//=========================================================
// RunAI
//=========================================================
void CBaseMonster::RunAI()
{
	// to test model's eye height
	//UTIL_ParticleEffect ( pev->origin + pev->view_ofs, vec3_origin, 255, 10 );

	// IDLE sound permitted in ALERT state is because monsters were silent in ALERT state. Only play IDLE sound in IDLE state
	// once we have sounds for that state.
	if ((m_MonsterState == NPCState::Idle || m_MonsterState == NPCState::Alert) && RANDOM_LONG(0, 99) == 0 && !(pev->spawnflags & SF_MONSTER_GAG))
	{
		IdleSound();
	}

	if (m_MonsterState != NPCState::None &&
		m_MonsterState != NPCState::Prone &&
		m_MonsterState != NPCState::Dead)// don't bother with this crap if monster is prone. 
	{
		// collect some sensory Condition information.
		// don't let monsters outside of the player's PVS act up, or most of the interesting
		// things will happen before the player gets there!
		// UPDATE: We now let COMBAT state monsters think and act fully outside of player PVS. This allows the player to leave 
		// an area where monsters are fighting, and the fight will continue.
		if (!FNullEnt(FIND_CLIENT_IN_PVS(edict())) || (m_MonsterState == NPCState::Combat))
		{
			Look(m_flDistLook);
			Listen();// check for audible sounds. 

			// now filter conditions.
			ClearConditions(IgnoreConditions());

			GetEnemy();
		}

		// do these calculations if monster has an enemy.
		if (m_hEnemy != nullptr)
		{
			CheckEnemy(m_hEnemy);
		}

		CheckAmmo();
	}

	FCheckAITrigger();

	PrescheduleThink();

	MaintainSchedule();

	// if the monster didn't use these conditions during the above call to MaintainSchedule() or CheckAITrigger()
	// we throw them out cause we don't want them sitting around through the lifespan of a schedule
	// that doesn't use them. 
	m_afConditions &= ~(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
}

//=========================================================
// GetIdealState - surveys the Conditions information available
// and finds the best new state for a monster.
//=========================================================
NPCState CBaseMonster::GetIdealState()
{
	int	iConditions;

	iConditions = IScheduleFlags();

	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch (m_MonsterState)
	{
	case NPCState::Idle:

		/*
		IDLE goes to ALERT upon hearing a sound
		-IDLE goes to ALERT upon being injured
		IDLE goes to ALERT upon seeing food
		-IDLE goes to COMBAT upon sighting an enemy
		IDLE goes to HUNT upon smelling food
		*/
	{
		if (iConditions & bits_COND_NEW_ENEMY)
		{
			// new enemy! This means an idle monster has seen someone it dislikes, or 
			// that a monster in combat has found a more suitable target to attack
			m_IdealMonsterState = NPCState::Combat;
		}
		else if (iConditions & bits_COND_LIGHT_DAMAGE)
		{
			MakeIdealYaw(m_vecEnemyLKP);
			m_IdealMonsterState = NPCState::Alert;
		}
		else if (iConditions & bits_COND_HEAVY_DAMAGE)
		{
			MakeIdealYaw(m_vecEnemyLKP);
			m_IdealMonsterState = NPCState::Alert;
		}
		else if (iConditions & bits_COND_HEAR_SOUND)
		{
			CSound* pSound;

			pSound = PBestSound();
			ASSERT(pSound != nullptr);
			if (pSound)
			{
				MakeIdealYaw(pSound->m_vecOrigin);
				if (pSound->m_iType & (bits_SOUND_COMBAT | bits_SOUND_DANGER))
					m_IdealMonsterState = NPCState::Alert;
			}
		}
		else if (iConditions & (bits_COND_SMELL | bits_COND_SMELL_FOOD))
		{
			m_IdealMonsterState = NPCState::Alert;
		}

		break;
	}
	case NPCState::Alert:
		/*
		ALERT goes to IDLE upon becoming bored
		-ALERT goes to COMBAT upon sighting an enemy
		ALERT goes to HUNT upon hearing a noise
		*/
	{
		if (iConditions & (bits_COND_NEW_ENEMY | bits_COND_SEE_ENEMY))
		{
			// see an enemy we MUST attack
			m_IdealMonsterState = NPCState::Combat;
		}
		else if (iConditions & bits_COND_HEAR_SOUND)
		{
			m_IdealMonsterState = NPCState::Alert;
			CSound* pSound = PBestSound();
			ASSERT(pSound != nullptr);
			if (pSound)
				MakeIdealYaw(pSound->m_vecOrigin);
		}
		break;
	}
	case NPCState::Combat:
		/*
		COMBAT goes to HUNT upon losing sight of enemy
		COMBAT goes to ALERT upon death of enemy
		*/
	{
		if (m_hEnemy == nullptr)
		{
			m_IdealMonsterState = NPCState::Alert;
			// pev->effects = EF_BRIGHTFIELD;
			ALERT(at_aiconsole, "***Combat state with no enemy!\n");
		}
		break;
	}
	case NPCState::Hunt:
		/*
		HUNT goes to ALERT upon seeing food
		HUNT goes to ALERT upon being injured
		HUNT goes to IDLE if goal touched
		HUNT goes to COMBAT upon seeing enemy
		*/
	{
		break;
	}
	case NPCState::Script:
		if (iConditions & (bits_COND_TASK_FAILED | bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			ExitScriptedSequence();	// This will set the ideal state
		}
		break;

	case NPCState::Dead:
		m_IdealMonsterState = NPCState::Dead;
		break;
	}

	return m_IdealMonsterState;
}

