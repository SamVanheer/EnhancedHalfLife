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
*	Monster-related utility code
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "navigation/nodes.h"
#include "monsters.h"
#include "animation.h"
#include "weapons.h"
#include "scripted.h"
#include "squadmonster.h"
#include "decals.hpp"
#include "soundent.h"
#include "gamerules.h"
#include "dll_functions.hpp"

constexpr int MONSTER_CUT_CORNER_DIST = 8; // 8 means the monster's bounding box is contained without the box of the node in WC

// Global Savedata for monster
// UNDONE: Save schedule data?  Can this be done?  We may
// lose our enemy pointer or other data (goal ent, target, etc)
// that make the current schedule invalid, perhaps it's best
// to just pick a new one when we start up again.
TYPEDESCRIPTION	CBaseMonster::m_SaveData[] =
{
	DEFINE_FIELD(CBaseMonster, m_hEnemy, FIELD_EHANDLE),
	DEFINE_FIELD(CBaseMonster, m_hTargetEnt, FIELD_EHANDLE),
	DEFINE_ARRAY(CBaseMonster, m_hOldEnemy, FIELD_EHANDLE, MAX_OLD_ENEMIES),
	DEFINE_ARRAY(CBaseMonster, m_vecOldEnemy, FIELD_POSITION_VECTOR, MAX_OLD_ENEMIES),
	DEFINE_FIELD(CBaseMonster, m_flFieldOfView, FIELD_FLOAT),
	DEFINE_FIELD(CBaseMonster, m_flWaitFinished, FIELD_TIME),
	DEFINE_FIELD(CBaseMonster, m_flMoveWaitFinished, FIELD_TIME),

	DEFINE_FIELD(CBaseMonster, m_Activity, FIELD_INTEGER),
	DEFINE_FIELD(CBaseMonster, m_IdealActivity, FIELD_INTEGER),
	DEFINE_FIELD(CBaseMonster, m_LastHitGroup, FIELD_INTEGER),
	DEFINE_FIELD(CBaseMonster, m_MonsterState, FIELD_INTEGER),
	DEFINE_FIELD(CBaseMonster, m_IdealMonsterState, FIELD_INTEGER),
	DEFINE_FIELD(CBaseMonster, m_iTaskStatus, FIELD_INTEGER),

	//Schedule_t			*m_pSchedule;

	DEFINE_FIELD(CBaseMonster, m_iScheduleIndex, FIELD_INTEGER),
	DEFINE_FIELD(CBaseMonster, m_afConditions, FIELD_INTEGER),
	//WayPoint_t			m_Route[ ROUTE_SIZE ];
//	DEFINE_FIELD( CBaseMonster, m_movementGoal, FIELD_INTEGER ),
//	DEFINE_FIELD( CBaseMonster, m_iRouteIndex, FIELD_INTEGER ),
//	DEFINE_FIELD( CBaseMonster, m_moveWaitTime, FIELD_FLOAT ),

	DEFINE_FIELD(CBaseMonster, m_vecMoveGoal, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CBaseMonster, m_movementActivity, FIELD_INTEGER),

	//		int					m_iAudibleList; // first index of a linked list of sounds that the monster can hear.
//	DEFINE_FIELD( CBaseMonster, m_afSoundTypes, FIELD_INTEGER ),
	DEFINE_FIELD(CBaseMonster, m_vecLastPosition, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CBaseMonster, m_iHintNode, FIELD_INTEGER),
	DEFINE_FIELD(CBaseMonster, m_afMemory, FIELD_INTEGER),
	DEFINE_FIELD(CBaseMonster, m_iMaxHealth, FIELD_INTEGER),

	DEFINE_FIELD(CBaseMonster, m_vecEnemyLKP, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CBaseMonster, m_cAmmoLoaded, FIELD_INTEGER),
	DEFINE_FIELD(CBaseMonster, m_afCapability, FIELD_INTEGER),

	DEFINE_FIELD(CBaseMonster, m_flNextAttack, FIELD_TIME),
	DEFINE_FIELD(CBaseMonster, m_bitsDamageType, FIELD_INTEGER),
	DEFINE_ARRAY(CBaseMonster, m_rgbTimeBasedDamage, FIELD_CHARACTER, CDMG_TIMEBASED),
	DEFINE_FIELD(CBaseMonster, m_bloodColor, FIELD_INTEGER),
	DEFINE_FIELD(CBaseMonster, m_failSchedule, FIELD_INTEGER),

	DEFINE_FIELD(CBaseMonster, m_flHungryTime, FIELD_TIME),
	DEFINE_FIELD(CBaseMonster, m_flDistTooFar, FIELD_FLOAT),
	DEFINE_FIELD(CBaseMonster, m_flDistLook, FIELD_FLOAT),
	DEFINE_FIELD(CBaseMonster, m_iTriggerCondition, FIELD_INTEGER),
	DEFINE_FIELD(CBaseMonster, m_iszTriggerTarget, FIELD_STRING),

	DEFINE_FIELD(CBaseMonster, m_HackedGunPos, FIELD_VECTOR),

	DEFINE_FIELD(CBaseMonster, m_scriptState, FIELD_INTEGER),
	DEFINE_FIELD(CBaseMonster, m_hCine, FIELD_EHANDLE),
};

//IMPLEMENT_SAVERESTORE( CBaseMonster, CBaseToggle );
bool CBaseMonster::Save(CSave& save)
{
	if (!CBaseToggle::Save(save))
		return false;
	return save.WriteFields("CBaseMonster", this, m_SaveData, ArraySize(m_SaveData));
}

bool CBaseMonster::Restore(CRestore& restore)
{
	if (!CBaseToggle::Restore(restore))
		return false;
	const bool status = restore.ReadFields("CBaseMonster", this, m_SaveData, ArraySize(m_SaveData));

	// We don't save/restore routes yet
	RouteClear();

	// We don't save/restore schedules yet
	m_pSchedule = nullptr;
	m_iTaskStatus = TaskStatus::New;

	// Reset animation
	m_Activity = ACT_RESET;

	// If we don't have an enemy, clear conditions like see enemy, etc.
	if (m_hEnemy == nullptr)
		m_afConditions = 0;

	return status;
}

void CBaseMonster::Eat(float flFullDuration)
{
	m_flHungryTime = gpGlobals->time + flFullDuration;
}

bool CBaseMonster::ShouldEat()
{
	return m_flHungryTime <= gpGlobals->time;
}

void CBaseMonster::BarnacleVictimBitten(CBaseEntity* pBarnacle)
{
	if (Schedule_t* pNewSchedule = GetScheduleOfType(SCHED_BARNACLE_VICTIM_CHOMP); pNewSchedule)
	{
		ChangeSchedule(pNewSchedule);
	}
}

void CBaseMonster::BarnacleVictimReleased()
{
	m_IdealMonsterState = NPCState::Idle;

	pev->velocity = vec3_origin;
	pev->movetype = Movetype::Step;
}

void CBaseMonster::Listen()
{
	m_iAudibleList = SOUNDLIST_EMPTY;
	ClearConditions(bits_COND_HEAR_SOUND | bits_COND_SMELL | bits_COND_SMELL_FOOD);
	m_afSoundTypes = 0;

	int iMySounds = SoundMask();

	if (m_pSchedule)
	{
		//!!!WATCH THIS SPOT IF YOU ARE HAVING SOUND RELATED BUGS!
		// Make sure your schedule AND personal sound masks agree!
		iMySounds &= m_pSchedule->iSoundMask;
	}

	int iSound = CSoundEnt::ActiveList();

	// UNDONE: Clear these here?
	ClearConditions(bits_COND_HEAR_SOUND | bits_COND_SMELL_FOOD | bits_COND_SMELL);
	const float hearingSensitivity = HearingSensitivity();

	while (iSound != SOUNDLIST_EMPTY)
	{
		CSound* pCurrentSound = CSoundEnt::SoundPointerForIndex(iSound);

		if (pCurrentSound &&
			(pCurrentSound->m_iType & iMySounds) &&
			(pCurrentSound->m_vecOrigin - EarPosition()).Length() <= pCurrentSound->m_iVolume * hearingSensitivity)

			//if ( ( g_pSoundEnt->m_SoundPool[ iSound ].m_iType & iMySounds ) && ( g_pSoundEnt->m_SoundPool[ iSound ].m_vecOrigin - EarPosition()).Length () <= g_pSoundEnt->m_SoundPool[ iSound ].m_iVolume * hearingSensitivity ) 
		{
			// the monster cares about this sound, and it's close enough to hear.
			//g_pSoundEnt->m_SoundPool[ iSound ].m_iNextAudible = m_iAudibleList;
			pCurrentSound->m_iNextAudible = m_iAudibleList;

			if (pCurrentSound->IsSound())
			{
				// this is an audible sound.
				SetConditions(bits_COND_HEAR_SOUND);
			}
			else
			{
				// if not a sound, must be a smell - determine if it's just a scent, or if it's a food scent
//				if ( g_pSoundEnt->m_SoundPool[ iSound ].m_iType & ( bits_SOUND_MEAT | bits_SOUND_CARCASS ) )
				if (pCurrentSound->m_iType & (bits_SOUND_MEAT | bits_SOUND_CARCASS))
				{
					// the detected scent is a food item, so set both conditions.
					// !!!BUGBUG - maybe a virtual function to determine whether or not the scent is food?
					SetConditions(bits_COND_SMELL_FOOD);
					SetConditions(bits_COND_SMELL);
				}
				else
				{
					// just a normal scent. 
					SetConditions(bits_COND_SMELL);
				}
			}

			//			m_afSoundTypes |= g_pSoundEnt->m_SoundPool[ iSound ].m_iType;
			m_afSoundTypes |= pCurrentSound->m_iType;

			m_iAudibleList = iSound;
		}

		//		iSound = g_pSoundEnt->m_SoundPool[ iSound ].m_iNext;
		iSound = pCurrentSound->m_iNext;
	}
}

float CBaseMonster::SoundVolume(CSound* pSound)
{
	return (pSound->m_iVolume - ((pSound->m_vecOrigin - pev->origin).Length()));
}

bool CBaseMonster::ValidateHintType(short sHint)
{
	return false;
}

void CBaseMonster::Look(int iDistance)
{
	int	iSighted = 0;

	// DON'T let visibility information from last frame sit around!
	ClearConditions(bits_COND_SEE_HATE | bits_COND_SEE_DISLIKE | bits_COND_SEE_ENEMY | bits_COND_SEE_FEAR | bits_COND_SEE_NEMESIS | bits_COND_SEE_CLIENT);

	m_pLink = nullptr;

	// See no evil if prisoner is set
	if (!IsBitSet(pev->spawnflags, SF_MONSTER_PRISONER))
	{
		CBaseEntity* pList[100];

		const Vector delta = Vector(iDistance, iDistance, iDistance);

		// Find only monsters/clients in box, NOT limited to PVS
		const int count = UTIL_EntitiesInBox(pList, ArraySize(pList), pev->origin - delta, pev->origin + delta, FL_CLIENT | FL_MONSTER);
		for (int i = 0; i < count; i++)
		{
			// the current visible entity that we're dealing with
			CBaseEntity* pSightEnt = pList[i];
			// !!!temporarily only considering other monsters and clients, don't see prisoners
			if (pSightEnt != this &&
				!IsBitSet(pSightEnt->pev->spawnflags, SF_MONSTER_PRISONER) &&
				pSightEnt->pev->health > 0)
			{
				// the looker will want to consider this entity
				// don't check anything else about an entity that can't be seen, or an entity that you don't care about.
				if (GetRelationship(pSightEnt) != Relationship::None && IsInViewCone(pSightEnt) && !IsBitSet(pSightEnt->pev->flags, FL_NOTARGET) && IsVisible(pSightEnt))
				{
					if (pSightEnt->IsPlayer())
					{
						if (pev->spawnflags & SF_MONSTER_WAIT_TILL_SEEN)
						{
							CBaseMonster* pClient = pSightEnt->MyMonsterPointer();
							// don't link this client in the list if the monster is wait till seen and the player isn't facing the monster
							if (pSightEnt && !pClient->IsInViewCone(this))
							{
								// we're not in the player's view cone. 
								continue;
							}
							else
							{
								// player sees us, become normal now.
								pev->spawnflags &= ~SF_MONSTER_WAIT_TILL_SEEN;
							}
						}

						// if we see a client, remember that (mostly for scripted AI)
						iSighted |= bits_COND_SEE_CLIENT;
					}

					pSightEnt->m_pLink = m_pLink;
					m_pLink = pSightEnt;

					if (pSightEnt == m_hEnemy)
					{
						// we know this ent is visible, so if it also happens to be our enemy, store that now.
						iSighted |= bits_COND_SEE_ENEMY;
					}

					// don't add the Enemy's relationship to the conditions. We only want to worry about conditions when
					// we see monsters other than the Enemy.
					switch (GetRelationship(pSightEnt))
					{
					case	Relationship::Nemesis:
						iSighted |= bits_COND_SEE_NEMESIS;
						break;
					case	Relationship::Hate:
						iSighted |= bits_COND_SEE_HATE;
						break;
					case	Relationship::Dislike:
						iSighted |= bits_COND_SEE_DISLIKE;
						break;
					case	Relationship::Fear:
						iSighted |= bits_COND_SEE_FEAR;
						break;
					case    Relationship::Ally:
						break;
					default:
						ALERT(at_aiconsole, "%s can't assess %s\n", STRING(pev->classname), STRING(pSightEnt->pev->classname));
						break;
					}
				}
			}
		}
	}

	SetConditions(iSighted);
}

int CBaseMonster::SoundMask()
{
	return	bits_SOUND_WORLD |
		bits_SOUND_COMBAT |
		bits_SOUND_PLAYER;
}

CSound* CBaseMonster::BestSound()
{
	int	iBestSound = -1;
	float flBestDist = WORLD_SIZE;// so first nearby sound will become best so far.

	int iThisSound = m_iAudibleList;

	if (iThisSound == SOUNDLIST_EMPTY)
	{
		ALERT(at_aiconsole, "ERROR! monster %s has no audible sounds!\n", STRING(pev->classname));
#if _DEBUG
		ALERT(at_error, "NULL Return from BestSound\n");
#endif
		return nullptr;
	}

	while (iThisSound != SOUNDLIST_EMPTY)
	{
		CSound* pSound = CSoundEnt::SoundPointerForIndex(iThisSound);

		if (pSound && pSound->IsSound())
		{
			const float flDist = (pSound->m_vecOrigin - EarPosition()).Length();

			if (flDist < flBestDist)
			{
				iBestSound = iThisSound;
				flBestDist = flDist;
			}
		}

		iThisSound = pSound->m_iNextAudible;
	}
	if (iBestSound >= 0)
	{
		return CSoundEnt::SoundPointerForIndex(iBestSound);
	}
#if _DEBUG
	ALERT(at_error, "NULL Return from BestSound\n");
#endif
	return nullptr;
}

CSound* CBaseMonster::BestScent()
{
	int	iBestScent = -1;
	float flBestDist = WORLD_SIZE;// so first nearby smell will become best so far.

	int iThisScent = m_iAudibleList;// smells are in the sound list.

	if (iThisScent == SOUNDLIST_EMPTY)
	{
		ALERT(at_aiconsole, "ERROR! BestScent() has empty soundlist!\n");
#if _DEBUG
		ALERT(at_error, "NULL Return from BestSound\n");
#endif
		return nullptr;
	}

	while (iThisScent != SOUNDLIST_EMPTY)
	{
		CSound* pSound = CSoundEnt::SoundPointerForIndex(iThisScent);

		if (pSound->IsScent())
		{
			const float flDist = (pSound->m_vecOrigin - pev->origin).Length();

			if (flDist < flBestDist)
			{
				iBestScent = iThisScent;
				flBestDist = flDist;
			}
		}

		iThisScent = pSound->m_iNextAudible;
	}
	if (iBestScent >= 0)
	{
		return CSoundEnt::SoundPointerForIndex(iBestScent);
	}
#if _DEBUG
	ALERT(at_error, "NULL Return from BestScent\n");
#endif
	return nullptr;
}

void CBaseMonster::MonsterThink()
{
	pev->nextthink = gpGlobals->time + 0.1;// keep monster thinking.

	RunAI();

	const float flInterval = StudioFrameAdvance(); // animate
// start or end a fidget
// This needs a better home -- switching animations over time should be encapsulated on a per-activity basis
// perhaps MaintainActivity() or a ShiftAnimationOverTime() or something.
	if (m_MonsterState != NPCState::Script && m_MonsterState != NPCState::Dead && m_Activity == ACT_IDLE && m_fSequenceFinished)
	{
		int iSequence;

		if (m_fSequenceLoops)
		{
			// animation does loop, which means we're playing subtle idle. Might need to 
			// fidget.
			iSequence = LookupActivity(m_Activity);
		}
		else
		{
			// animation that just ended doesn't loop! That means we just finished a fidget
			// and should return to our heaviest weighted idle (the subtle one)
			iSequence = LookupActivityHeaviest(m_Activity);
		}
		if (iSequence != ACTIVITY_NOT_AVAILABLE)
		{
			pev->sequence = iSequence;	// Set to new anim (if it's there)
			ResetSequenceInfo();
		}
	}

	DispatchAnimEvents(flInterval);

	if (!MovementIsComplete())
	{
		Move(flInterval);
	}
#if _DEBUG	
	else
	{
		if (!TaskIsRunning() && !TaskIsComplete())
			ALERT(at_error, "Schedule stalled!!\n");
	}
#endif
}

void CBaseMonster::MonsterUse(const UseInfo& info)
{
	//Don't do this because it can resurrect dying monsters
	//m_IdealMonsterState = NPCState::Alert;
}

int CBaseMonster::IgnoreConditions()
{
	int iIgnoreConditions = 0;

	if (!ShouldEat())
	{
		// not hungry? Ignore food smell.
		iIgnoreConditions |= bits_COND_SMELL_FOOD;
	}

	if (auto cine = m_hCine.Get(); m_MonsterState == NPCState::Script && cine)
		iIgnoreConditions |= cine->IgnoreConditions();

	return iIgnoreConditions;
}

void CBaseMonster::RouteClear()
{
	RouteNew();
	m_movementGoal = MOVEGOAL_NONE;
	m_movementActivity = ACT_IDLE;
	Forget(bits_MEMORY_MOVE_FAILED);
}

void CBaseMonster::RouteNew()
{
	m_Route[0].iType = 0;
	m_iRouteIndex = 0;
}

bool CBaseMonster::IsRouteClear()
{
	if (m_Route[m_iRouteIndex].iType == 0 || m_movementGoal == MOVEGOAL_NONE)
		return true;

	return false;
}

bool CBaseMonster::RefreshRoute()
{
	RouteNew();

	bool returnCode = false;

	switch (m_movementGoal)
	{
	case MOVEGOAL_PATHCORNER:
	{
		// monster is on a path_corner loop
		CBaseEntity* pPathCorner = m_hGoalEnt;

		for (int i = 0; pPathCorner && i < ROUTE_SIZE; ++i)
		{
			m_Route[i].iType = bits_MF_TO_PATHCORNER;
			m_Route[i].vecLocation = pPathCorner->pev->origin;

			pPathCorner = pPathCorner->GetNextTarget();

			// Last path_corner in list?
			if (!pPathCorner)
				m_Route[i].iType |= bits_MF_IS_GOAL;
		}
	}
	returnCode = true;
	break;

	case MOVEGOAL_ENEMY:
		returnCode = BuildRoute(m_vecEnemyLKP, bits_MF_TO_ENEMY, m_hEnemy);
		break;

	case MOVEGOAL_LOCATION:
		returnCode = BuildRoute(m_vecMoveGoal, bits_MF_TO_LOCATION, nullptr);
		break;

	case MOVEGOAL_TARGETENT:
		if (m_hTargetEnt != nullptr)
		{
			returnCode = BuildRoute(m_hTargetEnt->pev->origin, bits_MF_TO_TARGETENT, m_hTargetEnt);
		}
		break;

	case MOVEGOAL_NODE:
		returnCode = GetNodeRoute(m_vecMoveGoal);
		//			if ( returnCode )
		//				RouteSimplify( nullptr );
		break;
	}

	return returnCode;
}

bool CBaseMonster::MoveToEnemy(Activity movementAct, float waitTime)
{
	m_movementActivity = movementAct;
	m_moveWaitTime = waitTime;

	m_movementGoal = MOVEGOAL_ENEMY;
	return RefreshRoute();
}

bool CBaseMonster::MoveToLocation(Activity movementAct, float waitTime, const Vector& goal)
{
	m_movementActivity = movementAct;
	m_moveWaitTime = waitTime;

	m_movementGoal = MOVEGOAL_LOCATION;
	m_vecMoveGoal = goal;
	return RefreshRoute();
}

bool CBaseMonster::MoveToTarget(Activity movementAct, float waitTime)
{
	m_movementActivity = movementAct;
	m_moveWaitTime = waitTime;

	m_movementGoal = MOVEGOAL_TARGETENT;
	return RefreshRoute();
}

bool CBaseMonster::MoveToNode(Activity movementAct, float waitTime, const Vector& goal)
{
	m_movementActivity = movementAct;
	m_moveWaitTime = waitTime;

	m_movementGoal = MOVEGOAL_NODE;
	m_vecMoveGoal = goal;
	return RefreshRoute();
}

void UTIL_MoveToOrigin(CBaseEntity* pent, const Vector& vecGoal, float flDist, MoveToOriginType iMoveType)
{
	MOVE_TO_ORIGIN(pent->edict(), vecGoal, flDist, static_cast<int>(iMoveType));
}

#ifdef _DEBUG
void DrawRoute(CBaseEntity* pEntity, WayPoint_t* m_Route, int m_iRouteIndex, int r, int g, int b)
{
	if (m_Route[m_iRouteIndex].iType == 0)
	{
		ALERT(at_aiconsole, "Can't draw route!\n");
		return;
	}

	//	UTIL_ParticleEffect ( m_Route[ m_iRouteIndex ].vecLocation, vec3_origin, 255, 25 );

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(pEntity->pev->origin.x);
	WRITE_COORD(pEntity->pev->origin.y);
	WRITE_COORD(pEntity->pev->origin.z);
	WRITE_COORD(m_Route[m_iRouteIndex].vecLocation.x);
	WRITE_COORD(m_Route[m_iRouteIndex].vecLocation.y);
	WRITE_COORD(m_Route[m_iRouteIndex].vecLocation.z);

	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); // frame start
	WRITE_BYTE(10); // framerate
	WRITE_BYTE(1); // life
	WRITE_BYTE(16);  // width
	WRITE_BYTE(0);   // noise
	WRITE_BYTE(r);   // r, g, b
	WRITE_BYTE(g);   // r, g, b
	WRITE_BYTE(b);   // r, g, b
	WRITE_BYTE(255);	// brightness
	WRITE_BYTE(10);		// speed
	MESSAGE_END();

	for (int i = m_iRouteIndex; i < ROUTE_SIZE - 1; i++)
	{
		if ((m_Route[i].iType & bits_MF_IS_GOAL) || (m_Route[i + 1].iType == 0))
			break;

		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMPOINTS);
		WRITE_COORD(m_Route[i].vecLocation.x);
		WRITE_COORD(m_Route[i].vecLocation.y);
		WRITE_COORD(m_Route[i].vecLocation.z);
		WRITE_COORD(m_Route[i + 1].vecLocation.x);
		WRITE_COORD(m_Route[i + 1].vecLocation.y);
		WRITE_COORD(m_Route[i + 1].vecLocation.z);
		WRITE_SHORT(g_sModelIndexLaser);
		WRITE_BYTE(0); // frame start
		WRITE_BYTE(10); // framerate
		WRITE_BYTE(1); // life
		WRITE_BYTE(8);  // width
		WRITE_BYTE(0);   // noise
		WRITE_BYTE(r);   // r, g, b
		WRITE_BYTE(g);   // r, g, b
		WRITE_BYTE(b);   // r, g, b
		WRITE_BYTE(255);	// brightness
		WRITE_BYTE(10);		// speed
		MESSAGE_END();

		//		UTIL_ParticleEffect ( m_Route[ i ].vecLocation, vec3_origin, 255, 25 );
	}
}
#endif

bool ShouldSimplify(int routeType)
{
	routeType &= ~bits_MF_IS_GOAL;

	if ((routeType == bits_MF_TO_PATHCORNER) || (routeType & bits_MF_DONT_SIMPLIFY))
		return false;
	return true;
}

void CBaseMonster::RouteSimplify(CBaseEntity* pTargetEnt)
{
	// BUGBUG: this doesn't work 100% yet
	int i;

	int count = 0;

	for (i = m_iRouteIndex; i < ROUTE_SIZE; i++)
	{
		if (!m_Route[i].iType)
			break;
		else
			count++;
		if (m_Route[i].iType & bits_MF_IS_GOAL)
			break;
	}
	// Can't simplify a direct route!
	if (count < 2)
	{
		//		DrawRoute( this, m_Route, m_iRouteIndex, 0, 0, 255 );
		return;
	}

	WayPoint_t	outRoute[ROUTE_SIZE * 2];	// Any points except the ends can turn into 2 points in the simplified route
	int outCount = 0;
	Vector vecStart = pev->origin;
	for (i = 0; i < count - 1; i++)
	{
		// Don't eliminate path_corners
		if (!ShouldSimplify(m_Route[m_iRouteIndex + i].iType))
		{
			outRoute[outCount] = m_Route[m_iRouteIndex + i];
			outCount++;
		}
		else if (CheckLocalMove(vecStart, m_Route[m_iRouteIndex + i + 1].vecLocation, pTargetEnt, nullptr) == LocalMoveResult::Valid)
		{
			// Skip vert
			continue;
		}
		else
		{
			// Halfway between this and next
			const Vector vecTest = (m_Route[m_iRouteIndex + i + 1].vecLocation + m_Route[m_iRouteIndex + i].vecLocation) * 0.5;

			// Halfway between this and previous
			const Vector vecSplit = (m_Route[m_iRouteIndex + i].vecLocation + vecStart) * 0.5;

			const int iType = (m_Route[m_iRouteIndex + i].iType | bits_MF_TO_DETOUR) & ~bits_MF_NOT_TO_MASK;
			if (CheckLocalMove(vecStart, vecTest, pTargetEnt, nullptr) == LocalMoveResult::Valid)
			{
				outRoute[outCount].iType = iType;
				outRoute[outCount].vecLocation = vecTest;
			}
			else if (CheckLocalMove(vecSplit, vecTest, pTargetEnt, nullptr) == LocalMoveResult::Valid)
			{
				outRoute[outCount].iType = iType;
				outRoute[outCount].vecLocation = vecSplit;
				outRoute[outCount + 1].iType = iType;
				outRoute[outCount + 1].vecLocation = vecTest;
				outCount++; // Adding an extra point
			}
			else
			{
				outRoute[outCount] = m_Route[m_iRouteIndex + i];
			}
		}
		// Get last point
		vecStart = outRoute[outCount].vecLocation;
		outCount++;
	}
	ASSERT(i < count);
	outRoute[outCount] = m_Route[m_iRouteIndex + i];
	outCount++;

	// Terminate
	outRoute[outCount].iType = 0;
	ASSERT(outCount < (ROUTE_SIZE * 2));

	// Copy the simplified route, disable for testing
	m_iRouteIndex = 0;
	for (i = 0; i < ROUTE_SIZE && i < outCount; i++)
	{
		m_Route[i] = outRoute[i];
	}

	// Terminate route
	if (i < ROUTE_SIZE)
		m_Route[i].iType = 0;

	// Debug, test movement code
#if 0
//	if ( CVAR_GET_FLOAT( "simplify" ) != 0 )
	DrawRoute(this, outRoute, 0, 255, 0, 0);
	//	else
	DrawRoute(this, m_Route, m_iRouteIndex, 0, 255, 0);
#endif
}

bool CBaseMonster::BecomeProne()
{
	if (IsBitSet(pev->flags, FL_ONGROUND))
	{
		pev->flags -= FL_ONGROUND;
	}

	m_IdealMonsterState = NPCState::Prone;
	return true;
}

bool CBaseMonster::CheckRangeAttack1(float flDot, float flDist)
{
	if (flDist > 64 && flDist <= 784 && flDot >= 0.5)
	{
		return true;
	}
	return false;
}

bool CBaseMonster::CheckRangeAttack2(float flDot, float flDist)
{
	if (flDist > 64 && flDist <= 512 && flDot >= 0.5)
	{
		return true;
	}
	return false;
}

bool CBaseMonster::CheckMeleeAttack1(float flDot, float flDist)
{
	// Decent fix to keep folks from kicking/punching hornets and snarks is to check the onground flag(sjb)
	if (flDist <= 64 && flDot >= 0.7 && m_hEnemy != nullptr && IsBitSet(m_hEnemy->pev->flags, FL_ONGROUND))
	{
		return true;
	}
	return false;
}

bool CBaseMonster::CheckMeleeAttack2(float flDot, float flDist)
{
	if (flDist <= 64 && flDot >= 0.7)
	{
		return true;
	}
	return false;
}

void CBaseMonster::CheckAttacks(CBaseEntity* pTarget, float flDist)
{
	UTIL_MakeVectors(pev->angles);

	const Vector2D vec2LOS = (pTarget->pev->origin - pev->origin).Make2D().Normalize();

	const float flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

	// we know the enemy is in front now. We'll find which attacks the monster is capable of by
	// checking for corresponding Activities in the model file, then do the simple checks to validate
	// those attack types.

	// Clear all attack conditions
	ClearConditions(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_RANGE_ATTACK2 | bits_COND_CAN_MELEE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK2);

	if (m_afCapability & bits_CAP_RANGE_ATTACK1)
	{
		if (CheckRangeAttack1(flDot, flDist))
			SetConditions(bits_COND_CAN_RANGE_ATTACK1);
	}
	if (m_afCapability & bits_CAP_RANGE_ATTACK2)
	{
		if (CheckRangeAttack2(flDot, flDist))
			SetConditions(bits_COND_CAN_RANGE_ATTACK2);
	}
	if (m_afCapability & bits_CAP_MELEE_ATTACK1)
	{
		if (CheckMeleeAttack1(flDot, flDist))
			SetConditions(bits_COND_CAN_MELEE_ATTACK1);
	}
	if (m_afCapability & bits_CAP_MELEE_ATTACK2)
	{
		if (CheckMeleeAttack2(flDot, flDist))
			SetConditions(bits_COND_CAN_MELEE_ATTACK2);
	}
}

bool CBaseMonster::CanCheckAttacks()
{
	if (HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_ENEMY_TOOFAR))
	{
		return true;
	}

	return false;
}

bool CBaseMonster::CheckEnemy(CBaseEntity* pEnemy)
{
	bool updatedLKP = false;// set this to true if you update the EnemyLKP in this function.

	ClearConditions(bits_COND_ENEMY_FACING_ME);

	if (!IsVisible(pEnemy))
	{
		ASSERT(!HasConditions(bits_COND_SEE_ENEMY));
		SetConditions(bits_COND_ENEMY_OCCLUDED);
	}
	else
		ClearConditions(bits_COND_ENEMY_OCCLUDED);

	if (!pEnemy->IsAlive())
	{
		SetConditions(bits_COND_ENEMY_DEAD);
		ClearConditions(bits_COND_SEE_ENEMY | bits_COND_ENEMY_OCCLUDED);
		return false;
	}

	Vector vecEnemyPos = pEnemy->pev->origin;
	// distance to enemy's origin
	float flDistToEnemy = (vecEnemyPos - pev->origin).Length();
	vecEnemyPos.z += pEnemy->pev->size.z * 0.5;
	// distance to enemy's head
	const float flDistToEnemy2 = (vecEnemyPos - pev->origin).Length();
	if (flDistToEnemy2 < flDistToEnemy)
		flDistToEnemy = flDistToEnemy2;
	else
	{
		// distance to enemy's feet
		vecEnemyPos.z -= pEnemy->pev->size.z;
		const float flDistToEnemy3 = (vecEnemyPos - pev->origin).Length();
		if (flDistToEnemy3 < flDistToEnemy)
			flDistToEnemy = flDistToEnemy3;
	}

	if (HasConditions(bits_COND_SEE_ENEMY))
	{
		updatedLKP = true;
		m_vecEnemyLKP = pEnemy->pev->origin;

		if (CBaseMonster* pEnemyMonster = pEnemy->MyMonsterPointer(); pEnemyMonster)
		{
			if (pEnemyMonster->IsInViewCone(this))
			{
				SetConditions(bits_COND_ENEMY_FACING_ME);
			}
			else
				ClearConditions(bits_COND_ENEMY_FACING_ME);
		}

		if (pEnemy->pev->velocity != vec3_origin)
		{
			// trail the enemy a bit
			m_vecEnemyLKP = m_vecEnemyLKP - pEnemy->pev->velocity * RANDOM_FLOAT(-0.05, 0);
		}
		else
		{
			// UNDONE: use pev->oldorigin?
		}
	}
	else if (!HasConditions(bits_COND_ENEMY_OCCLUDED | bits_COND_SEE_ENEMY) && (flDistToEnemy <= 256))
	{
		// if the enemy is not occluded, and unseen, that means it is behind or beside the monster.
		// if the enemy is near enough the monster, we go ahead and let the monster know where the
		// enemy is. 
		updatedLKP = true;
		m_vecEnemyLKP = pEnemy->pev->origin;
	}

	if (flDistToEnemy >= m_flDistTooFar)
	{
		// enemy is very far away from monster
		SetConditions(bits_COND_ENEMY_TOOFAR);
	}
	else
		ClearConditions(bits_COND_ENEMY_TOOFAR);

	if (CanCheckAttacks())
	{
		CheckAttacks(m_hEnemy, flDistToEnemy);
	}

	if (m_movementGoal == MOVEGOAL_ENEMY)
	{
		for (int i = m_iRouteIndex; i < ROUTE_SIZE; i++)
		{
			if (m_Route[i].iType == (bits_MF_IS_GOAL | bits_MF_TO_ENEMY))
			{
				// UNDONE: Should we allow monsters to override this distance (80?)
				if ((m_Route[i].vecLocation - m_vecEnemyLKP).Length() > 80)
				{
					// Refresh
					RefreshRoute();
					return updatedLKP;
				}
			}
		}
	}

	return updatedLKP;
}

void CBaseMonster::PushEnemy(CBaseEntity* pEnemy, Vector& vecLastKnownPos)
{
	if (pEnemy == nullptr)
		return;

	// UNDONE: blah, this is bad, we should use a stack but I'm too lazy to code one.
	int i;

	for (i = 0; i < MAX_OLD_ENEMIES; i++)
	{
		if (m_hOldEnemy[i] == pEnemy)
			return;
		if (m_hOldEnemy[i] == nullptr) // someone died, reuse their slot
			break;
	}
	if (i >= MAX_OLD_ENEMIES)
		return;

	m_hOldEnemy[i] = pEnemy;
	m_vecOldEnemy[i] = vecLastKnownPos;
}

bool CBaseMonster::PopEnemy()
{
	// UNDONE: blah, this is bad, we should use a stack but I'm too lazy to code one.
	for (int i = MAX_OLD_ENEMIES - 1; i >= 0; i--)
	{
		if (m_hOldEnemy[i] != nullptr)
		{
			if (m_hOldEnemy[i]->IsAlive()) // cheat and know when they die
			{
				m_hEnemy = m_hOldEnemy[i];
				m_vecEnemyLKP = m_vecOldEnemy[i];
				// ALERT( at_console, "remembering\n");
				return true;
			}
			else
			{
				m_hOldEnemy[i] = nullptr;
			}
		}
	}
	return false;
}

void CBaseMonster::SetActivity(Activity NewActivity)
{
	const int iSequence = LookupActivity(NewActivity);

	// Set to the desired anim, or default anim if the desired is not present
	if (iSequence > ACTIVITY_NOT_AVAILABLE)
	{
		if (pev->sequence != iSequence || !m_fSequenceLoops)
		{
			// don't reset frame between walk and run
			if (!(m_Activity == ACT_WALK || m_Activity == ACT_RUN) || !(NewActivity == ACT_WALK || NewActivity == ACT_RUN))
				pev->frame = 0;
		}

		pev->sequence = iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo();
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT(at_aiconsole, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity);
		pev->sequence = 0;	// Set to the reset anim (if it's there)
	}

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

	// In case someone calls this with something other than the ideal activity
	m_IdealActivity = m_Activity;
}

void CBaseMonster::SetSequenceByName(const char* szSequence)
{
	const int iSequence = LookupSequence(szSequence);

	// Set to the desired anim, or default anim if the desired is not present
	if (iSequence > ACTIVITY_NOT_AVAILABLE)
	{
		if (pev->sequence != iSequence || !m_fSequenceLoops)
		{
			pev->frame = 0;
		}

		pev->sequence = iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo();
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT(at_aiconsole, "%s has no sequence named:%f\n", STRING(pev->classname), szSequence);
		pev->sequence = 0;	// Set to the reset anim (if it's there)
	}
}

constexpr int	LOCAL_STEP_SIZE = 16;
LocalMoveResult CBaseMonster::CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, float* pflDist)
{
	const Vector vecStartPos = pev->origin; // record monster's position before trying the move

	const float flYaw = UTIL_VecToYaw(vecEnd - vecStart);// build a yaw that points to the goal.
	const float flDist = (vecEnd - vecStart).Length2D();// get the distance.
	LocalMoveResult iReturn = LocalMoveResult::Valid;// assume everything will be ok.

	// move the monster to the start of the local move that's to be checked.
	SetAbsOrigin(vecStart);// !!!BUGBUG - won't this fire triggers? - nope, SetOrigin doesn't fire

	if (!(pev->flags & (FL_FLY | FL_SWIM)))
	{
		DROP_TO_FLOOR(edict());//make sure monster is on the floor!
	}

	//pev->origin.z = vecStartPos.z;//!!!HACKHACK

//	pev->origin = vecStart;

/*
	if ( flDist > 1024 )
	{
		// !!!PERFORMANCE - this operation may be too CPU intensive to try checks this large.
		// We don't lose much here, because a distance this great is very likely
		// to have something in the way.

		// since we've actually moved the monster during the check, undo the move.
		pev->origin = vecStartPos;
		return false;
	}
*/
// this loop takes single steps to the goal.
	for (float flStep = 0; flStep < flDist; flStep += LOCAL_STEP_SIZE)
	{
		float stepSize = LOCAL_STEP_SIZE;

		if ((flStep + LOCAL_STEP_SIZE) >= (flDist - 1))
			stepSize = (flDist - flStep) - 1;

		//		UTIL_ParticleEffect ( pev->origin, vec3_origin, 255, 25 );

		if (!WALK_MOVE(edict(), flYaw, stepSize, WalkMoveMode::CheckOnly))
		{// can't take the next step, fail!

			if (pflDist != nullptr)
			{
				*pflDist = flStep;
			}
			if (pTarget && pTarget->edict() == gpGlobals->trace_ent)
			{
				// if this step hits target ent, the move is legal.
				iReturn = LocalMoveResult::Valid;
				break;
			}
			else
			{
				// If we're going toward an entity, and we're almost getting there, it's OK.
//				if ( pTarget && fabs( flDist - iStep ) < LOCAL_STEP_SIZE )
//					fReturn = true;
//				else
				iReturn = LocalMoveResult::Invalid;
				break;
			}
		}
	}

	if (iReturn == LocalMoveResult::Valid && !(pev->flags & (FL_FLY | FL_SWIM)) && (!pTarget || (pTarget->pev->flags & FL_ONGROUND)))
	{
		// The monster can move to a spot UNDER the target, but not to it. Don't try to triangulate, go directly to the node graph.
		// UNDONE: Magic # 64 -- this used to be pev->size.z but that won't work for small creatures like the headcrab
		if (fabs(vecEnd.z - pev->origin.z) > 64)
		{
			iReturn = LocalMoveResult::InvalidDontTriangulate;
		}
	}
	/*
	// uncommenting this block will draw a line representing the nearest legal move.
	WRITE_BYTE(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(MessageDest::Broadcast, TE_SHOWLINE);
	WRITE_COORD(MessageDest::Broadcast, pev->origin.x);
	WRITE_COORD(MessageDest::Broadcast, pev->origin.y);
	WRITE_COORD(MessageDest::Broadcast, pev->origin.z);
	WRITE_COORD(MessageDest::Broadcast, vecStart.x);
	WRITE_COORD(MessageDest::Broadcast, vecStart.y);
	WRITE_COORD(MessageDest::Broadcast, vecStart.z);
	*/

	// since we've actually moved the monster during the check, undo the move.
	SetAbsOrigin(vecStartPos);

	return iReturn;
}

float CBaseMonster::OpenDoorAndWait(CBaseEntity* pDoor)
{
	float flTravelTime = 0;

	//ALERT(at_aiconsole, "A door. ");
	if (pDoor && !pDoor->IsLockedByMaster())
	{
		//ALERT(at_aiconsole, "unlocked! ");
		pDoor->Use({this, this, UseType::On});
		//ALERT(at_aiconsole, "pevDoor->nextthink = %d ms\n", (int)(1000*pevDoor->nextthink));
		//ALERT(at_aiconsole, "pevDoor->ltime = %d ms\n", (int)(1000*pevDoor->ltime));
		//ALERT(at_aiconsole, "pev-> nextthink = %d ms\n", (int)(1000*pev->nextthink));
		//ALERT(at_aiconsole, "pev->ltime = %d ms\n", (int)(1000*pev->ltime));
		flTravelTime = pDoor->pev->nextthink - pDoor->pev->ltime;
		//ALERT(at_aiconsole, "Waiting %d ms\n", (int)(1000*flTravelTime));
		if (!IsStringNull(pDoor->pev->targetname))
		{
			CBaseEntity* pTarget = nullptr;

			while ((pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pDoor->pev->targetname))) != nullptr)
			{
				if (pTarget->pev != pDoor->pev)
				{
					if (pTarget->ClassnameIs(STRING(pDoor->pev->classname)))
					{
						pTarget->Use({this, this, UseType::On});
					}
				}
			}
		}
	}

	return gpGlobals->time + flTravelTime;
}

void CBaseMonster::AdvanceRoute(float distance)
{

	if (m_iRouteIndex == ROUTE_SIZE - 1)
	{
		// time to refresh the route.
		if (!RefreshRoute())
		{
			ALERT(at_aiconsole, "Can't Refresh Route!!\n");
		}
	}
	else
	{
		if (!(m_Route[m_iRouteIndex].iType & bits_MF_IS_GOAL))
		{
			// If we've just passed a path_corner, advance m_hGoalEnt
			if ((m_Route[m_iRouteIndex].iType & ~bits_MF_NOT_TO_MASK) == bits_MF_TO_PATHCORNER)
				m_hGoalEnt = m_hGoalEnt->GetNextTarget();

			// IF both waypoints are nodes, then check for a link for a door and operate it.
			//
			if ((m_Route[m_iRouteIndex].iType & bits_MF_TO_NODE) == bits_MF_TO_NODE
				&& (m_Route[m_iRouteIndex + 1].iType & bits_MF_TO_NODE) == bits_MF_TO_NODE)
			{
				//ALERT(at_aiconsole, "SVD: Two nodes. ");

				const int iSrcNode = WorldGraph.FindNearestNode(m_Route[m_iRouteIndex].vecLocation, this);
				const int iDestNode = WorldGraph.FindNearestNode(m_Route[m_iRouteIndex + 1].vecLocation, this);

				int iLink;
				WorldGraph.HashSearch(iSrcNode, iDestNode, iLink);

				if (iLink >= 0 && WorldGraph.m_pLinkPool[iLink].m_hLinkEnt != nullptr)
				{
					//ALERT(at_aiconsole, "A link. ");
					if (WorldGraph.HandleLinkEnt(iSrcNode, WorldGraph.m_pLinkPool[iLink].m_hLinkEnt, m_afCapability, CGraph::NodeQuery::Dynamic))
					{
						//ALERT(at_aiconsole, "usable.");
						if (CBaseEntity* pDoor = WorldGraph.m_pLinkPool[iLink].m_hLinkEnt; pDoor)
						{
							m_flMoveWaitFinished = OpenDoorAndWait(pDoor);
							//							ALERT( at_aiconsole, "Wating for door %.2f\n", m_flMoveWaitFinished-gpGlobals->time );
						}
					}
				}
				//ALERT(at_aiconsole, "\n");
			}
			m_iRouteIndex++;
		}
		else	// At goal!!!
		{
			if (distance < m_flGroundSpeed * 0.2 /* FIX */)
			{
				MovementComplete();
			}
		}
	}
}

int CBaseMonster::RouteClassify(int iMoveFlag)
{
	int movementGoal = MOVEGOAL_NONE;

	if (iMoveFlag & bits_MF_TO_TARGETENT)
		movementGoal = MOVEGOAL_TARGETENT;
	else if (iMoveFlag & bits_MF_TO_ENEMY)
		movementGoal = MOVEGOAL_ENEMY;
	else if (iMoveFlag & bits_MF_TO_PATHCORNER)
		movementGoal = MOVEGOAL_PATHCORNER;
	else if (iMoveFlag & bits_MF_TO_NODE)
		movementGoal = MOVEGOAL_NODE;
	else if (iMoveFlag & bits_MF_TO_LOCATION)
		movementGoal = MOVEGOAL_LOCATION;

	return movementGoal;
}

bool CBaseMonster::BuildRoute(const Vector& vecGoal, int iMoveFlag, CBaseEntity* pTarget)
{
	RouteNew();
	m_movementGoal = RouteClassify(iMoveFlag);

	// so we don't end up with no moveflags
	m_Route[0].vecLocation = vecGoal;
	m_Route[0].iType = iMoveFlag | bits_MF_IS_GOAL;

	// check simple local move
	float flDist;
	const LocalMoveResult iLocalMove = CheckLocalMove(pev->origin, vecGoal, pTarget, &flDist);

	Vector vecApex;

	if (iLocalMove == LocalMoveResult::Valid)
	{
		// monster can walk straight there!
		return true;
	}
	// try to triangulate around any obstacles.
	else if (iLocalMove != LocalMoveResult::InvalidDontTriangulate && Triangulate(pev->origin, vecGoal, flDist, pTarget, &vecApex))
	{
		// there is a slightly more complicated path that allows the monster to reach vecGoal
		m_Route[0].vecLocation = vecApex;
		m_Route[0].iType = (iMoveFlag | bits_MF_TO_DETOUR);

		m_Route[1].vecLocation = vecGoal;
		m_Route[1].iType = iMoveFlag | bits_MF_IS_GOAL;

		/*
		WRITE_BYTE(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(MessageDest::Broadcast, TE_SHOWLINE);
		WRITE_COORD(MessageDest::Broadcast, vecApex.x );
		WRITE_COORD(MessageDest::Broadcast, vecApex.y );
		WRITE_COORD(MessageDest::Broadcast, vecApex.z );
		WRITE_COORD(MessageDest::Broadcast, vecApex.x );
		WRITE_COORD(MessageDest::Broadcast, vecApex.y );
		WRITE_COORD(MessageDest::Broadcast, vecApex.z + 128 );
		*/

		RouteSimplify(pTarget);
		return true;
	}

	// last ditch, try nodes
	if (GetNodeRoute(vecGoal))
	{
		//		ALERT ( at_console, "Can get there on nodes\n" );
		m_vecMoveGoal = vecGoal;
		RouteSimplify(pTarget);
		return true;
	}

	// b0rk
	return false;
}

void CBaseMonster::InsertWaypoint(Vector vecLocation, int afMoveFlags)
{
	// we have to save some Index and Type information from the real
	// path_corner or node waypoint that the monster was trying to reach. This makes sure that data necessary 
	// to refresh the original path exists even in the new waypoints that don't correspond directy to a path_corner
	// or node. 
	const int type = afMoveFlags | (m_Route[m_iRouteIndex].iType & ~bits_MF_NOT_TO_MASK);

	for (int i = ROUTE_SIZE - 1; i > 0; i--)
		m_Route[i] = m_Route[i - 1];

	m_Route[m_iRouteIndex].vecLocation = vecLocation;
	m_Route[m_iRouteIndex].iType = type;
}

bool CBaseMonster::Triangulate(const Vector& vecStart, const Vector& vecEnd, float flDist, CBaseEntity* pTargetEnt, Vector* pApex)
{
	// If the hull width is less than 24, use 24 because CheckLocalMove uses a min of
	// 24.
	const float sizeX = std::clamp(pev->size.x, 24.0f, 48.0f);
	const float sizeZ = pev->size.z;
	//if (sizeZ < 24.0)
	//	sizeZ = 24.0;

	const Vector vecForward = (vecEnd - vecStart).Normalize();

	Vector vecDirUp = vec3_up;
	Vector vecDir = CrossProduct(vecForward, vecDirUp);

	// start checking right about where the object is, picking two equidistant starting points, one on
	// the left, one on the right. As we progress through the loop, we'll push these away from the obstacle, 
	// hoping to find a way around on either side. pev->size.x is added to the ApexDist in order to help select
	// an apex point that insures that the monster is sufficiently past the obstacle before trying to turn back
	// onto its original course.

	// the spot we'll try to triangulate to on the left
	Vector vecLeft = pev->origin + (vecForward * (flDist + sizeX)) - vecDir * (sizeX * 3);

	// the spot we'll try to triangulate to on the right
	Vector vecRight = pev->origin + (vecForward * (flDist + sizeX)) + vecDir * (sizeX * 3);

	Vector vecTop;// the spot we'll try to triangulate to on the top
	Vector vecBottom;// the spot we'll try to triangulate to on the bottom

	if (pev->movetype == Movetype::Fly)
	{
		vecTop = pev->origin + (vecForward * flDist) + (vecDirUp * sizeZ * 3);
		vecBottom = pev->origin + (vecForward * flDist) - (vecDirUp * sizeZ * 3);
	}

	// the spot that we'll move to after hitting the triangulated point, before moving on to our normal goal.
	const Vector vecFarSide = m_Route[m_iRouteIndex].vecLocation;

	vecDir = vecDir * sizeX * 2;
	if (pev->movetype == Movetype::Fly)
		vecDirUp = vecDirUp * sizeZ * 2;

	for (int i = 0; i < 8; i++)
	{
		// Debug, Draw the triangulation
#if 0
		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_SHOWLINE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(vecRight.x);
		WRITE_COORD(vecRight.y);
		WRITE_COORD(vecRight.z);
		MESSAGE_END();

		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_SHOWLINE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(vecLeft.x);
		WRITE_COORD(vecLeft.y);
		WRITE_COORD(vecLeft.z);
		MESSAGE_END();
#endif

#if 0
		if (pev->movetype == Movetype::Fly)
		{
			MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
			WRITE_BYTE(TE_SHOWLINE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_COORD(vecTop.x);
			WRITE_COORD(vecTop.y);
			WRITE_COORD(vecTop.z);
			MESSAGE_END();

			MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
			WRITE_BYTE(TE_SHOWLINE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_COORD(vecBottom.x);
			WRITE_COORD(vecBottom.y);
			WRITE_COORD(vecBottom.z);
			MESSAGE_END();
		}
#endif

		if (CheckLocalMove(pev->origin, vecRight, pTargetEnt, nullptr) == LocalMoveResult::Valid)
		{
			if (CheckLocalMove(vecRight, vecFarSide, pTargetEnt, nullptr) == LocalMoveResult::Valid)
			{
				if (pApex)
				{
					*pApex = vecRight;
				}

				return true;
			}
		}
		if (CheckLocalMove(pev->origin, vecLeft, pTargetEnt, nullptr) == LocalMoveResult::Valid)
		{
			if (CheckLocalMove(vecLeft, vecFarSide, pTargetEnt, nullptr) == LocalMoveResult::Valid)
			{
				if (pApex)
				{
					*pApex = vecLeft;
				}

				return true;
			}
		}

		if (pev->movetype == Movetype::Fly)
		{
			if (CheckLocalMove(pev->origin, vecTop, pTargetEnt, nullptr) == LocalMoveResult::Valid)
			{
				if (CheckLocalMove(vecTop, vecFarSide, pTargetEnt, nullptr) == LocalMoveResult::Valid)
				{
					if (pApex)
					{
						*pApex = vecTop;
						//ALERT(at_aiconsole, "triangulate over\n");
					}

					return true;
				}
			}
#if 1
			if (CheckLocalMove(pev->origin, vecBottom, pTargetEnt, nullptr) == LocalMoveResult::Valid)
			{
				if (CheckLocalMove(vecBottom, vecFarSide, pTargetEnt, nullptr) == LocalMoveResult::Valid)
				{
					if (pApex)
					{
						*pApex = vecBottom;
						//ALERT(at_aiconsole, "triangulate under\n");
					}

					return true;
				}
			}
#endif
		}

		vecRight = vecRight + vecDir;
		vecLeft = vecLeft - vecDir;
		if (pev->movetype == Movetype::Fly)
		{
			vecTop = vecTop + vecDirUp;
			vecBottom = vecBottom - vecDirUp;
		}
	}

	return false;
}

constexpr float DIST_TO_CHECK = 200.0f;

void CBaseMonster::Move(float flInterval)
{
	// Don't move if no valid route
	if (IsRouteClear())
	{
		// If we still have a movement goal, then this is probably a route truncated by SimplifyRoute()
		// so refresh it.
		if (m_movementGoal == MOVEGOAL_NONE || !RefreshRoute())
		{
			ALERT(at_aiconsole, "Tried to move with no route!\n");
			TaskFail();
			return;
		}
	}

	if (m_flMoveWaitFinished > gpGlobals->time)
		return;

	// Debug, test movement code
#if 0
//	if ( CVAR_GET_FLOAT("stopmove" ) != 0 )
	{
		if (m_movementGoal == MOVEGOAL_ENEMY)
			RouteSimplify(m_hEnemy);
		else
			RouteSimplify(m_hTargetEnt);
		RefreshRoute();
		return;
	}
#else
// Debug, draw the route
//	DrawRoute( this, m_Route, m_iRouteIndex, 0, 200, 0 );
#endif

	// if the monster is moving directly towards an entity (enemy for instance), we'll set this pointer
	// to that entity for the CheckLocalMove and Triangulate functions.
	CBaseEntity* pTargetEnt = nullptr;

	// local move to waypoint.
	const Vector vecDir = (m_Route[m_iRouteIndex].vecLocation - pev->origin).Normalize();
	const float flWaypointDist = (m_Route[m_iRouteIndex].vecLocation - pev->origin).Length2D();

	MakeIdealYaw(m_Route[m_iRouteIndex].vecLocation);
	ChangeYaw(pev->yaw_speed);

	// if the waypoint is closer than CheckDist, CheckDist is the dist to waypoint
	const float flCheckDist = std::min(DIST_TO_CHECK, flWaypointDist);

	if ((m_Route[m_iRouteIndex].iType & (~bits_MF_NOT_TO_MASK)) == bits_MF_TO_ENEMY)
	{
		// only on a PURE move to enemy ( i.e., ONLY MF_TO_ENEMY set, not MF_TO_ENEMY and DETOUR )
		pTargetEnt = m_hEnemy;
	}
	else if ((m_Route[m_iRouteIndex].iType & ~bits_MF_NOT_TO_MASK) == bits_MF_TO_TARGETENT)
	{
		pTargetEnt = m_hTargetEnt;
	}

	// !!!BUGBUG - CheckDist should be derived from ground speed.
	// If this fails, it should be because of some dynamic entity blocking this guy.
	// We've already checked this path, so we should wait and time out if the entity doesn't move
	float flDist = 0; // how far the lookahead check got before hitting an object.
	Vector vecApex;

	if (CheckLocalMove(pev->origin, pev->origin + vecDir * flCheckDist, pTargetEnt, &flDist) != LocalMoveResult::Valid)
	{
		// Can't move, stop
		Stop();
		// Blocking entity is in global trace_ent
		CBaseEntity* pBlocker = CBaseEntity::Instance(gpGlobals->trace_ent);
		if (pBlocker)
		{
			DispatchBlocked(edict(), pBlocker->edict());
		}

		if (pBlocker && m_moveWaitTime > 0 && pBlocker->IsMoving() && !pBlocker->IsPlayer() && (gpGlobals->time - m_flMoveWaitFinished) > 3.0f)
		{
			// Can we still move toward our target?
			if (flDist < m_flGroundSpeed)
			{
				// No, Wait for a second
				m_flMoveWaitFinished = gpGlobals->time + m_moveWaitTime;
				return;
			}
			// Ok, still enough room to take a step
		}
		else
		{
			// try to triangulate around whatever is in the way.
			if (Triangulate(pev->origin, m_Route[m_iRouteIndex].vecLocation, flDist, pTargetEnt, &vecApex))
			{
				InsertWaypoint(vecApex, bits_MF_TO_DETOUR);
				RouteSimplify(pTargetEnt);
			}
			else
			{
				//				ALERT ( at_aiconsole, "Couldn't Triangulate\n" );
				Stop();
				// Only do this once until your route is cleared
				if (m_moveWaitTime > 0 && !(m_afMemory & bits_MEMORY_MOVE_FAILED))
				{
					RefreshRoute();
					if (IsRouteClear())
					{
						TaskFail();
					}
					else
					{
						// Don't get stuck
						if ((gpGlobals->time - m_flMoveWaitFinished) < 0.2)
							Remember(bits_MEMORY_MOVE_FAILED);

						m_flMoveWaitFinished = gpGlobals->time + 0.1;
					}
				}
				else
				{
					TaskFail();
					ALERT(at_aiconsole, "%s Failed to move (%d)!\n", STRING(pev->classname), HasMemory(bits_MEMORY_MOVE_FAILED));
					//ALERT( at_aiconsole, "%f, %f, %f\n", pev->origin.z, (pev->origin + (vecDir * flCheckDist)).z, m_Route[m_iRouteIndex].vecLocation.z );
				}
				return;
			}
		}
	}

	// close enough to the target, now advance to the next target. This is done before actually reaching
	// the target so that we get a nice natural turn while moving.
	if (ShouldAdvanceRoute(flWaypointDist))///!!!BUGBUG- magic number
	{
		AdvanceRoute(flWaypointDist);
	}

	// Might be waiting for a door
	if (m_flMoveWaitFinished > gpGlobals->time)
	{
		Stop();
		return;
	}

	// UNDONE: this is a hack to quit moving farther than it has looked ahead.
	if (flCheckDist < m_flGroundSpeed * flInterval)
	{
		flInterval = flCheckDist / m_flGroundSpeed;
		// ALERT( at_console, "%.02f\n", flInterval );
	}
	MoveExecute(pTargetEnt, vecDir, flInterval);

	if (MovementIsComplete())
	{
		Stop();
		RouteClear();
	}
}

bool CBaseMonster::ShouldAdvanceRoute(float flWaypointDist)
{
	if (flWaypointDist <= MONSTER_CUT_CORNER_DIST)
	{
		// ALERT( at_console, "cut %f\n", flWaypointDist );
		return true;
	}

	return false;
}

void CBaseMonster::MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval)
{
	//	float flYaw = UTIL_VecToYaw ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin );// build a yaw that points to the goal.
	//	WALK_MOVE( ENT(pev), flYaw, m_flGroundSpeed * flInterval, WALKMOVE_NORMAL );
	if (m_IdealActivity != m_movementActivity)
		m_IdealActivity = m_movementActivity;

	float flTotal = m_flGroundSpeed * pev->framerate * flInterval;
	while (flTotal > 0.001)
	{
		// don't walk more than 16 units or stairs stop working
		const float flStep = std::min(16.0f, flTotal);
		UTIL_MoveToOrigin(this, m_Route[m_iRouteIndex].vecLocation, flStep, MoveToOriginType::Normal);
		flTotal -= flStep;
	}
	// ALERT( at_console, "dist %f\n", m_flGroundSpeed * pev->framerate * flInterval );
}

void CBaseMonster::MonsterInit()
{
	if (!g_pGameRules->AllowMonsters())
	{
		pev->flags |= FL_KILLME;		// Post this because some monster code modifies class data after calling this function
//		REMOVE_ENTITY(ENT(pev));
		return;
	}

	// Set fields common to all monsters
	pev->effects = 0;
	SetDamageMode(DamageMode::Aim);
	pev->ideal_yaw = pev->angles.y;
	pev->max_health = pev->health;
	pev->deadflag = DeadFlag::No;
	m_IdealMonsterState = NPCState::Idle;// Assume monster will be idle, until proven otherwise

	m_IdealActivity = ACT_IDLE;

	SetBits(pev->flags, FL_MONSTER);
	if (pev->spawnflags & SF_MONSTER_HITMONSTERCLIP)
		pev->flags |= FL_MONSTERCLIP;

	ClearSchedule();
	RouteClear();
	InitBoneControllers(); // FIX: should be done in Spawn

	m_iHintNode = NO_NODE;

	m_afMemory = MEMORY_CLEAR;

	m_hEnemy = nullptr;

	m_flDistTooFar = 1024.0;
	m_flDistLook = 2048.0;

	// set eye position
	SetEyePosition();

	SetThink(&CBaseMonster::MonsterInitThink);
	pev->nextthink = gpGlobals->time + 0.1;
	SetUse(&CBaseMonster::MonsterUse);
}

void CBaseMonster::MonsterInitThink()
{
	StartMonster();
}

void CBaseMonster::StartMonster()
{
	// update capabilities
	if (LookupActivity(ACT_RANGE_ATTACK1) != ACTIVITY_NOT_AVAILABLE)
	{
		m_afCapability |= bits_CAP_RANGE_ATTACK1;
	}
	if (LookupActivity(ACT_RANGE_ATTACK2) != ACTIVITY_NOT_AVAILABLE)
	{
		m_afCapability |= bits_CAP_RANGE_ATTACK2;
	}
	if (LookupActivity(ACT_MELEE_ATTACK1) != ACTIVITY_NOT_AVAILABLE)
	{
		m_afCapability |= bits_CAP_MELEE_ATTACK1;
	}
	if (LookupActivity(ACT_MELEE_ATTACK2) != ACTIVITY_NOT_AVAILABLE)
	{
		m_afCapability |= bits_CAP_MELEE_ATTACK2;
	}

	// Raise monster off the floor one unit, then drop to floor
	if (pev->movetype != Movetype::Fly && !IsBitSet(pev->spawnflags, SF_MONSTER_FALL_TO_GROUND))
	{
		pev->origin.z += 1;
		DROP_TO_FLOOR(edict());
		// Try to move the monster to make sure it's not stuck in a brush.
		if (!WALK_MOVE(edict(), 0, 0, WalkMoveMode::Normal))
		{
			ALERT(at_error, "Monster %s stuck in wall--level design error", STRING(pev->classname));
			pev->effects = EF_BRIGHTFIELD;
		}
	}
	else
	{
		pev->flags &= ~FL_ONGROUND;
	}

	if (!IsStringNull(pev->target))// this monster has a target
	{
		// Find the monster's initial target entity, stash it
		m_hGoalEnt = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));

		if (!m_hGoalEnt)
		{
			ALERT(at_error, "ReadyMonster()--%s couldn't find target %s", STRING(pev->classname), STRING(pev->target));
		}
		else
		{
			// Monster will start turning towards his destination
			MakeIdealYaw(m_hGoalEnt->pev->origin);

			// JAY: How important is this error message?  Big Momma doesn't obey this rule, so I took it out.
#if 0
			// At this point, we expect only a path_corner as initial goal
			if (!m_hGoalEnt->ClassnameIs("path_corner"))
			{
				ALERT(at_warning, "ReadyMonster--monster's initial goal '%s' is not a path_corner", STRING(pev->target));
			}
#endif

			// set the monster up to walk a path corner path. 
			// !!!BUGBUG - this is a minor bit of a hack.
			// JAYJAY
			m_movementGoal = MOVEGOAL_PATHCORNER;

			if (pev->movetype == Movetype::Fly)
				m_movementActivity = ACT_FLY;
			else
				m_movementActivity = ACT_WALK;

			if (!RefreshRoute())
			{
				ALERT(at_aiconsole, "Can't Create Route!\n");
			}
			SetState(NPCState::Idle);
			ChangeSchedule(GetScheduleOfType(SCHED_IDLE_WALK));
		}
	}

	//SetState ( m_IdealMonsterState );
	//SetActivity ( m_IdealActivity );

	// Delay drop to floor to make sure each door in the level has had its chance to spawn
	// Spread think times so that they don't all happen at the same time (Carmack)
	SetThink(&CBaseMonster::CallMonsterThink);
	pev->nextthink += RANDOM_FLOAT(0.1, 0.4); // spread think times.

	if (!IsStringNull(pev->targetname))// wait until triggered
	{
		SetState(NPCState::Idle);
		// UNDONE: Some scripted sequence monsters don't have an idle?
		SetActivity(ACT_IDLE);
		ChangeSchedule(GetScheduleOfType(SCHED_WAIT_TRIGGER));
	}
}

void CBaseMonster::MovementComplete()
{
	switch (m_iTaskStatus)
	{
	case TaskStatus::New:
	case TaskStatus::Running:
		m_iTaskStatus = TaskStatus::RunningTask;
		break;

	case TaskStatus::RunningMovement:
		TaskComplete();
		break;

	case TaskStatus::RunningTask:
		ALERT(at_error, "Movement completed twice!\n");
		break;

	case TaskStatus::Complete:
		break;
	}
	m_movementGoal = MOVEGOAL_NONE;
}

bool CBaseMonster::TaskIsRunning()
{
	if (m_iTaskStatus != TaskStatus::Complete &&
		m_iTaskStatus != TaskStatus::RunningMovement)
		return true;

	return false;
}

Relationship CBaseMonster::GetRelationship(CBaseEntity* pTarget)
{
	//Define these as aliases to avoid cluttering this up
	//TODO: rework this
	const Relationship R_AL = Relationship::Ally;
	const Relationship R_FR = Relationship::Fear;
	const Relationship R_NO = Relationship::None;
	const Relationship R_DL = Relationship::Dislike;
	const Relationship R_HT = Relationship::Hate;
	const Relationship R_NM = Relationship::Nemesis;

	static constexpr Relationship iEnemy[CLASS_COUNT][CLASS_COUNT] =
	{			 //   NONE	 MACH	 PLYR	 HPASS	 HMIL	 AMIL	 APASS	 AMONST	APREY	 APRED	 INSECT	PLRALY	PBWPN	ABWPN
		/*NONE*/		{ R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO,	R_NO,	R_NO	},
		/*MACHINE*/		{ R_NO	,R_NO	,R_DL	,R_DL	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_DL,	R_DL,	R_DL	},
		/*PLAYER*/		{ R_NO	,R_DL	,R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO,	R_DL,	R_DL	},
		/*HUMANPASSIVE*/{ R_NO	,R_NO	,R_AL	,R_AL	,R_HT	,R_FR	,R_NO	,R_HT	,R_DL	,R_FR	,R_NO	,R_AL,	R_NO,	R_NO	},
		/*HUMANMILITAR*/{ R_NO	,R_NO	,R_HT	,R_DL	,R_NO	,R_HT	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_HT,	R_NO,	R_NO	},
		/*ALIENMILITAR*/{ R_NO	,R_DL	,R_HT	,R_DL	,R_HT	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_DL,	R_NO,	R_NO	},
		/*ALIENPASSIVE*/{ R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO,	R_NO,	R_NO	},
		/*ALIENMONSTER*/{ R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_NO	,R_DL,	R_NO,	R_NO	},
		/*ALIENPREY   */{ R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_NO	,R_FR	,R_NO	,R_DL,	R_NO,	R_NO	},
		/*ALIENPREDATO*/{ R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO	,R_NO	,R_HT	,R_DL	,R_NO	,R_DL,	R_NO,	R_NO	},
		/*INSECT*/		{ R_FR	,R_FR	,R_FR	,R_FR	,R_FR	,R_NO	,R_FR	,R_FR	,R_FR	,R_FR	,R_NO	,R_FR,	R_NO,	R_NO	},
		/*PLAYERALLY*/	{ R_NO	,R_DL	,R_AL	,R_AL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_NO,	R_NO,	R_NO	},
		/*PBIOWEAPON*/	{ R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_DL	,R_NO	,R_DL,	R_NO,	R_DL	},
		/*ABIOWEAPON*/	{ R_NO	,R_NO	,R_DL	,R_DL	,R_DL	,R_AL	,R_NO	,R_DL	,R_DL	,R_NO	,R_NO	,R_DL,	R_DL,	R_NO	}
	};

	const int myClass = Classify();
	const int targetClass = pTarget->Classify();

	if (myClass < 0 || myClass >= CLASS_COUNT
		|| targetClass < 0 || targetClass >= CLASS_COUNT)
	{
		return R_NO;
	}

	return iEnemy[myClass][targetClass];
}

bool CBaseMonster::FindCover(Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist)
{
	if (!flMaxDist)
	{
		// user didn't supply a MaxDist, so work up a crazy one.
		flMaxDist = 784;
	}

	if (flMinDist > 0.5 * flMaxDist)
	{
#if _DEBUG
		ALERT(at_console, "FindCover MinDist (%.0f) too close to MaxDist (%.0f)\n", flMinDist, flMaxDist);
#endif
		flMinDist = 0.5 * flMaxDist;
	}

	if (!WorldGraph.m_fGraphPresent || !WorldGraph.m_fGraphPointersSet)
	{
		ALERT(at_aiconsole, "Graph not ready for findcover!\n");
		return false;
	}

	const int iMyNode = WorldGraph.FindNearestNode(pev->origin, this);
	int iThreatNode = WorldGraph.FindNearestNode(vecThreat, this);
	const int iMyHullIndex = WorldGraph.HullIndex(this);

	if (iMyNode == NO_NODE)
	{
		ALERT(at_aiconsole, "FindCover() - %s has no nearest node!\n", STRING(pev->classname));
		return false;
	}
	if (iThreatNode == NO_NODE)
	{
		// ALERT ( at_aiconsole, "FindCover() - Threat has no nearest node!\n" );
		iThreatNode = iMyNode;
		// return false;
	}

	const Vector vecLookersOffset = vecThreat + vecViewOffset;// calculate location of enemy's eyes
	TraceResult tr;

	// we'll do a rough sample to find nodes that are relatively nearby
	for (int i = 0; i < WorldGraph.m_cNodes; i++)
	{
		const int nodeNumber = (i + WorldGraph.m_iLastCoverSearch) % WorldGraph.m_cNodes;

		const CNode& node = WorldGraph.Node(nodeNumber);
		WorldGraph.m_iLastCoverSearch = nodeNumber + 1; // next monster that searches for cover node will start where we left off here.

		// could use an optimization here!!
		const float flDist = (pev->origin - node.m_vecOrigin).Length();

		// DON'T do the trace check on a node that is farther away than a node that we've already found to 
		// provide cover! Also make sure the node is within the mins/maxs of the search.
		if (flDist >= flMinDist && flDist < flMaxDist)
		{
			UTIL_TraceLine(node.m_vecOrigin + vecViewOffset, vecLookersOffset, IgnoreMonsters::Yes, IgnoreGlass::Yes, edict(), &tr);

			// if this node will block the threat's line of sight to me...
			if (tr.flFraction != 1.0)
			{
				// ..and is also closer to me than the threat, or the same distance from myself and the threat the node is good.
				if ((iMyNode == iThreatNode) || WorldGraph.PathLength(iMyNode, nodeNumber, iMyHullIndex, m_afCapability) <= WorldGraph.PathLength(iThreatNode, nodeNumber, iMyHullIndex, m_afCapability))
				{
					if (ValidateCover(node.m_vecOrigin) && MoveToLocation(ACT_RUN, 0, node.m_vecOrigin))
					{
						/*
						MESSAGE_BEGIN( MessageDest::Broadcast, SVC_TEMPENTITY );
							WRITE_BYTE( TE_SHOWLINE);

							WRITE_COORD( node.m_vecOrigin.x );
							WRITE_COORD( node.m_vecOrigin.y );
							WRITE_COORD( node.m_vecOrigin.z );

							WRITE_COORD( vecLookersOffset.x );
							WRITE_COORD( vecLookersOffset.y );
							WRITE_COORD( vecLookersOffset.z );
						MESSAGE_END();
						*/

						return true;
					}
				}
			}
		}
	}
	return false;
}

bool CBaseMonster::BuildNearestRoute(Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist)
{
	if (!flMaxDist)
	{
		// user didn't supply a MaxDist, so work up a crazy one.
		flMaxDist = 784;
	}

	if (flMinDist > 0.5 * flMaxDist)
	{
#if _DEBUG
		ALERT(at_console, "FindCover MinDist (%.0f) too close to MaxDist (%.0f)\n", flMinDist, flMaxDist);
#endif
		flMinDist = 0.5 * flMaxDist;
	}

	if (!WorldGraph.m_fGraphPresent || !WorldGraph.m_fGraphPointersSet)
	{
		ALERT(at_aiconsole, "Graph not ready for BuildNearestRoute!\n");
		return false;
	}

	const int iMyNode = WorldGraph.FindNearestNode(pev->origin, this);
	const int iMyHullIndex = WorldGraph.HullIndex(this);

	if (iMyNode == NO_NODE)
	{
		ALERT(at_aiconsole, "BuildNearestRoute() - %s has no nearest node!\n", STRING(pev->classname));
		return false;
	}

	const Vector vecLookersOffset = vecThreat + vecViewOffset;// calculate location of enemy's eyes
	TraceResult tr;

	// we'll do a rough sample to find nodes that are relatively nearby
	for (int i = 0; i < WorldGraph.m_cNodes; i++)
	{
		const int nodeNumber = (i + WorldGraph.m_iLastCoverSearch) % WorldGraph.m_cNodes;

		const CNode& node = WorldGraph.Node(nodeNumber);
		WorldGraph.m_iLastCoverSearch = nodeNumber + 1; // next monster that searches for cover node will start where we left off here.

		// can I get there?
		if (WorldGraph.NextNodeInRoute(iMyNode, nodeNumber, iMyHullIndex, 0) != iMyNode)
		{
			const float flDist = (vecThreat - node.m_vecOrigin).Length();

			// is it close?
			if (flDist > flMinDist && flDist < flMaxDist)
			{
				// can I see where I want to be from there?
				UTIL_TraceLine(node.m_vecOrigin + pev->view_ofs, vecLookersOffset, IgnoreMonsters::Yes, edict(), &tr);

				if (tr.flFraction == 1.0)
				{
					// try to actually get there
					if (BuildRoute(node.m_vecOrigin, bits_MF_TO_LOCATION, nullptr))
					{
						flMaxDist = flDist;
						m_vecMoveGoal = node.m_vecOrigin;
						return true; // UNDONE: keep looking for something closer!
					}
				}
			}
		}
	}

	return false;
}

CBaseEntity* CBaseMonster::BestVisibleEnemy()
{
	int iNearest = WORLD_SIZE;// so first visible entity will become the closest.
	CBaseEntity* pNextEnt = m_pLink;
	CBaseEntity* pReturn = nullptr;
	Relationship iBestRelationship = Relationship::None;

	while (pNextEnt != nullptr)
	{
		if (pNextEnt->IsAlive())
		{
			if (GetRelationship(pNextEnt) > iBestRelationship)
			{
				// this entity is disliked MORE than the entity that we 
				// currently think is the best visible enemy. No need to do 
				// a distance check, just get mad at this one for now.
				iBestRelationship = GetRelationship(pNextEnt);
				iNearest = (pNextEnt->pev->origin - pev->origin).Length();
				pReturn = pNextEnt;
			}
			else if (GetRelationship(pNextEnt) == iBestRelationship)
			{
				// this entity is disliked just as much as the entity that
				// we currently think is the best visible enemy, so we only
				// get mad at it if it is closer.
				const int iDist = (pNextEnt->pev->origin - pev->origin).Length();

				if (iDist <= iNearest)
				{
					iNearest = iDist;
					iBestRelationship = GetRelationship(pNextEnt);
					pReturn = pNextEnt;
				}
			}
		}

		pNextEnt = pNextEnt->m_pLink;
	}

	return pReturn;
}

void CBaseMonster::MakeIdealYaw(Vector vecTarget)
{
	Vector vecProjection;

	// strafing monster needs to face 90 degrees away from its goal
	if (m_movementActivity == ACT_STRAFE_LEFT)
	{
		vecProjection.x = -vecTarget.y;
		vecProjection.y = vecTarget.x;

		pev->ideal_yaw = UTIL_VecToYaw(vecProjection - pev->origin);
	}
	else if (m_movementActivity == ACT_STRAFE_RIGHT)
	{
		vecProjection.x = vecTarget.y;
		vecProjection.y = vecTarget.x;

		pev->ideal_yaw = UTIL_VecToYaw(vecProjection - pev->origin);
	}
	else
	{
		pev->ideal_yaw = UTIL_VecToYaw(vecTarget - pev->origin);
	}
}

float CBaseMonster::YawDiff()
{
	const float flCurrentYaw = UTIL_AngleMod(pev->angles.y);

	if (flCurrentYaw == pev->ideal_yaw)
	{
		return 0;
	}

	return UTIL_AngleDiff(pev->ideal_yaw, flCurrentYaw);
}

float CBaseMonster::ChangeYaw(int yawSpeed)
{
	const float current = UTIL_AngleMod(pev->angles.y);
	const float ideal = pev->ideal_yaw;
	if (current != ideal)
	{
		const float delta = std::min(0.25f, gpGlobals->time - m_flLastYawTime);

		m_flLastYawTime = gpGlobals->time;

		const float speed = yawSpeed * delta * 2;
		float move = ideal - current;

		if (ideal > current)
		{
			if (move >= 180)
				move = move - 360;
		}
		else
		{
			if (move <= -180)
				move = move + 360;
		}

		if (move > 0)
		{// turning to the monster's left
			if (move > speed)
				move = speed;
		}
		else
		{// turning to the monster's right
			if (move < -speed)
				move = -speed;
		}

		pev->angles.y = UTIL_AngleMod(current + move);

		// turn head in desired direction only if they have a turnable head
		if (m_afCapability & bits_CAP_TURN_HEAD)
		{
			float yaw = pev->ideal_yaw - pev->angles.y;
			if (yaw > 180) yaw -= 360;
			if (yaw < -180) yaw += 360;
			// yaw *= 0.8;
			SetBoneController(0, yaw);
		}

		return move;
	}

	return 0;
}

float CBaseMonster::VecToYaw(Vector vecDir)
{
	if (vecDir.x == 0 && vecDir.y == 0 && vecDir.z == 0)
		return pev->angles.y;

	return UTIL_VecToYaw(vecDir);
}

void CBaseMonster::SetEyePosition()
{
	Vector  vecEyePosition;
	void* pmodel = GET_MODEL_PTR(edict());

	GetEyePosition(pmodel, vecEyePosition);

	pev->view_ofs = vecEyePosition;

	if (pev->view_ofs == vec3_origin)
	{
		ALERT(at_aiconsole, "%s has no view_ofs!\n", STRING(pev->classname));
	}
}

void CBaseMonster::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case SCRIPT_EVENT_DEAD:
		if (m_MonsterState == NPCState::Script)
		{
			pev->deadflag = DeadFlag::Dying;
			// Kill me now! (and fade out when CineCleanup() is called)
#if _DEBUG
			ALERT(at_aiconsole, "Death event: %s\n", STRING(pev->classname));
#endif
			pev->health = 0;
		}
#if _DEBUG
		else
			ALERT(at_aiconsole, "INVALID death event:%s\n", STRING(pev->classname));
#endif
		break;
	case SCRIPT_EVENT_NOT_DEAD:
		if (m_MonsterState == NPCState::Script)
		{
			pev->deadflag = DeadFlag::No;
			// This is for life/death sequences where the player can determine whether a character is dead or alive after the script 
			pev->health = pev->max_health;
		}
		break;

	case SCRIPT_EVENT_SOUND:			// Play a named wave file
		EmitSound(SoundChannel::Body, event.options, VOL_NORM, ATTN_IDLE);
		break;

	case SCRIPT_EVENT_SOUND_VOICE:
		EmitSound(SoundChannel::Voice, event.options, VOL_NORM, ATTN_IDLE);
		break;

	case SCRIPT_EVENT_SENTENCE_RND1:		// Play a named sentence group 33% of the time
		if (RANDOM_LONG(0, 2) == 0)
			break;

		[[fallthrough]];

	case SCRIPT_EVENT_SENTENCE:			// Play a named sentence group
		SENTENCEG_PlayRndSz(this, event.options, VOL_NORM, ATTN_IDLE, PITCH_NORM);
		break;

	case SCRIPT_EVENT_FIREEVENT:		// Fire a trigger
		FireTargets(event.options, this, this, UseType::Toggle, 0);
		break;

	case SCRIPT_EVENT_NOINTERRUPT:		// Can't be interrupted from now on
		if (auto cine = m_hCine.Get(); cine)
			cine->AllowInterrupt(false);
		break;

	case SCRIPT_EVENT_CANINTERRUPT:		// OK to interrupt now
		if (auto cine = m_hCine.Get(); cine)
			cine->AllowInterrupt(true);
		break;

#if 0
	case SCRIPT_EVENT_INAIR:			// Don't DROP_TO_FLOOR()
	case SCRIPT_EVENT_ENDANIMATION:		// Set ending animation sequence to
		break;
#endif

	case MONSTER_EVENT_BODYDROP_HEAVY:
		if (pev->flags & FL_ONGROUND)
		{
			if (RANDOM_LONG(0, 1) == 0)
			{
				EmitSound(SoundChannel::Body, "common/bodydrop3.wav", VOL_NORM, ATTN_NORM, 90);
			}
			else
			{
				EmitSound(SoundChannel::Body, "common/bodydrop4.wav", VOL_NORM, ATTN_NORM, 90);
			}
		}
		break;

	case MONSTER_EVENT_BODYDROP_LIGHT:
		if (pev->flags & FL_ONGROUND)
		{
			if (RANDOM_LONG(0, 1) == 0)
			{
				EmitSound(SoundChannel::Body, "common/bodydrop3.wav");
			}
			else
			{
				EmitSound(SoundChannel::Body, "common/bodydrop4.wav");
			}
		}
		break;

	case MONSTER_EVENT_SWISHSOUND:
	{
		// NO MONSTER may use this anim event unless that monster's precache precaches this sound!!!
		EmitSound(SoundChannel::Body, "zombie/claw_miss2.wav");
		break;
	}

	default:
		ALERT(at_aiconsole, "Unhandled animation event %d for %s\n", event.event, STRING(pev->classname));
		break;

	}
}

// Combat

Vector CBaseMonster::GetGunPosition()
{
	UTIL_MakeVectors(pev->angles);

	// Vector vecSrc = pev->origin + gpGlobals->v_forward * 10;
	//vecSrc.z = pevShooter->absmin.z + pevShooter->size.z * 0.7;
	//vecSrc.z = pev->origin.z + (pev->view_ofs.z - 4);
	const Vector vecSrc = pev->origin
		+ gpGlobals->v_forward * m_HackedGunPos.y
		+ gpGlobals->v_right * m_HackedGunPos.x
		+ gpGlobals->v_up * m_HackedGunPos.z;

	return vecSrc;
}

bool CBaseMonster::GetNodeRoute(Vector vecDest)
{
	const int iSrcNode = WorldGraph.FindNearestNode(pev->origin, this);
	const int iDestNode = WorldGraph.FindNearestNode(vecDest, this);

	if (iSrcNode == -1)
	{
		// no node nearest self
//		ALERT ( at_aiconsole, "GetNodeRoute: No valid node near self!\n" );
		return false;
	}
	else if (iDestNode == -1)
	{
		// no node nearest target
//		ALERT ( at_aiconsole, "GetNodeRoute: No valid node near target!\n" );
		return false;
	}

	// valid src and dest nodes were found, so it's safe to proceed with
	// find shortest path
	const int iNodeHull = WorldGraph.HullIndex(this); // make this a monster virtual function

	int iPath[MAX_PATH_SIZE];
	int iResult = WorldGraph.FindShortestPath(iPath, iSrcNode, iDestNode, iNodeHull, m_afCapability);

	if (!iResult)
	{
#if 1
		ALERT(at_aiconsole, "No Path from %d to %d!\n", iSrcNode, iDestNode);
		return false;
#else
		const bool bRoutingSave = WorldGraph.m_fRoutingComplete;
		WorldGraph.m_fRoutingComplete = false;
		iResult = WorldGraph.FindShortestPath(iPath, iSrcNode, iDestNode, iNodeHull, m_afCapability);
		WorldGraph.m_fRoutingComplete = bRoutingSave;
		if (!iResult)
		{
			ALERT(at_aiconsole, "No Path from %d to %d!\n", iSrcNode, iDestNode);
			return false;
		}
		else
		{
			ALERT(at_aiconsole, "Routing is inconsistent!");
		}
#endif
	}

	// there's a valid path within iPath now, so now we will fill the route array
	// up with as many of the waypoints as it will hold.

	// don't copy ROUTE_SIZE entries if the path returned is shorter
	// than ROUTE_SIZE!!!
	const int iNumToCopy = std::min(ROUTE_SIZE, iResult);

	for (int i = 0; i < iNumToCopy; i++)
	{
		m_Route[i].vecLocation = WorldGraph.m_pNodes[iPath[i]].m_vecOrigin;
		m_Route[i].iType = bits_MF_TO_NODE;
	}

	if (iNumToCopy < ROUTE_SIZE)
	{
		m_Route[iNumToCopy].vecLocation = vecDest;
		m_Route[iNumToCopy].iType |= bits_MF_IS_GOAL;
	}

	return true;
}

int CBaseMonster::FindHintNode()
{
	if (!WorldGraph.m_fGraphPresent)
	{
		ALERT(at_aiconsole, "find_hintnode: graph not ready!\n");
		return NO_NODE;
	}

	if (WorldGraph.m_iLastActiveIdleSearch >= WorldGraph.m_cNodes)
	{
		WorldGraph.m_iLastActiveIdleSearch = 0;
	}

	TraceResult tr;

	for (int i = 0; i < WorldGraph.m_cNodes; i++)
	{
		const int nodeNumber = (i + WorldGraph.m_iLastActiveIdleSearch) % WorldGraph.m_cNodes;
		const CNode& node = WorldGraph.Node(nodeNumber);

		if (node.m_sHintType)
		{
			// this node has a hint. Take it if it is visible, the monster likes it, and the monster has an animation to match the hint's activity.
			if (ValidateHintType(node.m_sHintType))
			{
				if (!node.m_sHintActivity || LookupActivity(node.m_sHintActivity) != ACTIVITY_NOT_AVAILABLE)
				{
					UTIL_TraceLine(pev->origin + pev->view_ofs, node.m_vecOrigin + pev->view_ofs, IgnoreMonsters::Yes, edict(), &tr);

					if (tr.flFraction == 1.0)
					{
						WorldGraph.m_iLastActiveIdleSearch = nodeNumber + 1; // next monster that searches for hint nodes will start where we left off.
						return nodeNumber;// take it!
					}
				}
			}
		}
	}

	WorldGraph.m_iLastActiveIdleSearch = 0;// start at the top of the list for the next search.

	return NO_NODE;
}

void CBaseMonster::ReportAIState()
{
	const ALERT_TYPE level = at_console;

	static constexpr const char* pStateNames[] = {"None", "Idle", "Combat", "Alert", "Hunt", "Prone", "Scripted", "PlayDead", "Dead"};

	static_assert(ArraySize(pStateNames) == NPCStatesCount);

	ALERT(level, "%s: ", STRING(pev->classname));
	if ((std::size_t)m_MonsterState < ArraySize(pStateNames))
		ALERT(level, "State: %s, ", pStateNames[static_cast<std::size_t>(m_MonsterState)]);

	for (int i = 0; activity_map[i].type != 0; ++i)
	{
		if (activity_map[i].type == static_cast<int>(m_Activity))
		{
			ALERT(level, "Activity %s, ", activity_map[i].name);
			break;
		}
	}

	if (m_pSchedule)
	{
		const char* pName = m_pSchedule->pName;
		if (!pName)
			pName = "Unknown";
		ALERT(level, "Schedule %s, ", pName);

		if (Task_t* pTask = GetTask(); pTask)
			ALERT(level, "Task %d (#%d), ", pTask->iTask, m_iScheduleIndex);
	}
	else
		ALERT(level, "No Schedule, ");

	if (m_hEnemy != nullptr)
		ALERT(level, "\nEnemy is %s", STRING(m_hEnemy->pev->classname));
	else
		ALERT(level, "No enemy");

	if (IsMoving())
	{
		ALERT(level, " Moving ");
		if (m_flMoveWaitFinished > gpGlobals->time)
			ALERT(level, ": Stopped for %.2f. ", m_flMoveWaitFinished - gpGlobals->time);
		else if (m_IdealActivity == GetStoppedActivity())
			ALERT(level, ": In stopped anim. ");
	}

	if (CSquadMonster* pSquadMonster = MySquadMonsterPointer(); pSquadMonster)
	{
		if (!pSquadMonster->InSquad())
		{
			ALERT(level, "not ");
		}

		ALERT(level, "In Squad, ");

		if (!pSquadMonster->IsLeader())
		{
			ALERT(level, "not ");
		}

		ALERT(level, "Leader.");
	}

	ALERT(level, "\n");
	ALERT(level, "Yaw speed:%3.1f,Health: %3.1f\n", pev->yaw_speed, pev->health);
	if (pev->spawnflags & SF_MONSTER_PRISONER)
		ALERT(level, " PRISONER! ");
	if (pev->spawnflags & SF_MONSTER_PREDISASTER)
		ALERT(level, " Pre-Disaster! ");
	ALERT(level, "\n");
}

void CBaseMonster::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "TriggerTarget"))
	{
		m_iszTriggerTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "TriggerCondition"))
	{
		//TODO: validate input
		m_iTriggerCondition = static_cast<AITrigger>(atoi(pkvd->szValue));
		pkvd->fHandled = true;
	}
	else
	{
		CBaseToggle::KeyValue(pkvd);
	}
}

bool CBaseMonster::CheckAITrigger()
{
	if (m_iTriggerCondition == AITrigger::None)
	{
		// no conditions, so this trigger is never fired.
		return false;
	}

	bool fFireTarget = false;

	switch (m_iTriggerCondition)
	{
	case AITrigger::SeePlayerAngryAtPlayer:
		if (m_hEnemy != nullptr && m_hEnemy->IsPlayer() && HasConditions(bits_COND_SEE_ENEMY))
		{
			fFireTarget = true;
		}
		break;
	case AITrigger::SeePlayerUnconditional:
		if (HasConditions(bits_COND_SEE_CLIENT))
		{
			fFireTarget = true;
		}
		break;
	case AITrigger::SeePlayerNotInCombat:
		if (HasConditions(bits_COND_SEE_CLIENT) &&
			m_MonsterState != NPCState::Combat &&
			m_MonsterState != NPCState::Prone &&
			m_MonsterState != NPCState::Script)
		{
			fFireTarget = true;
		}
		break;
	case AITrigger::TakeDamage:
		if (m_afConditions & (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			fFireTarget = true;
		}
		break;
	case AITrigger::Death:
		if (pev->deadflag != DeadFlag::No)
		{
			fFireTarget = true;
		}
		break;
	case AITrigger::HalfHealth:
		if (IsAlive() && pev->health <= (pev->max_health / 2))
		{
			fFireTarget = true;
		}
		break;
		/*

		  // !!!UNDONE - no persistant game state that allows us to track these two.

			case AITRIGGER_SQUADMEMBERDIE:
				break;
			case AITRIGGER_SQUADLEADERDIE:
				break;
		*/
	case AITrigger::HearWorld:
		if (m_afConditions & bits_COND_HEAR_SOUND && m_afSoundTypes & bits_SOUND_WORLD)
		{
			fFireTarget = true;
		}
		break;
	case AITrigger::HearPlayer:
		if (m_afConditions & bits_COND_HEAR_SOUND && m_afSoundTypes & bits_SOUND_PLAYER)
		{
			fFireTarget = true;
		}
		break;
	case AITrigger::HearCombat:
		if (m_afConditions & bits_COND_HEAR_SOUND && m_afSoundTypes & bits_SOUND_COMBAT)
		{
			fFireTarget = true;
		}
		break;
	}

	if (fFireTarget)
	{
		// fire the target, then set the trigger conditions to NONE so we don't fire again
		ALERT(at_aiconsole, "AI Trigger Fire Target\n");
		FireTargets(STRING(m_iszTriggerTarget), this, this, UseType::Toggle, 0);
		m_iTriggerCondition = AITrigger::None;
		return true;
	}

	return false;
}

bool CBaseMonster::CanPlaySequence(bool fDisregardMonsterState, int interruptLevel)
{
	if (m_hCine || !IsAlive() || m_MonsterState == NPCState::Prone)
	{
		// monster is already running a scripted sequence or dead!
		return false;
	}

	if (fDisregardMonsterState)
	{
		// ok to go, no matter what the monster state. (scripted AI)
		return true;
	}

	if (m_MonsterState == NPCState::None || m_MonsterState == NPCState::Idle || m_IdealMonsterState == NPCState::Idle)
	{
		// ok to go, but only in these states
		return true;
	}

	if (m_MonsterState == NPCState::Alert && interruptLevel >= SS_INTERRUPT_BY_NAME)
		return true;

	// unknown situation
	return false;
}

constexpr int COVER_CHECKS = 5;	// how many checks are made
constexpr int COVER_DELTA = 48;	// distance between checks

bool CBaseMonster::FindLateralCover(const Vector& vecThreat, const Vector& vecViewOffset)
{
	UTIL_MakeVectors(pev->angles);
	Vector vecStepRight = gpGlobals->v_right * COVER_DELTA;
	vecStepRight.z = 0;

	Vector vecLeftTest = pev->origin;
	Vector vecRightTest = pev->origin;

	TraceResult	tr;

	for (int i = 0; i < COVER_CHECKS; i++)
	{
		vecLeftTest = vecLeftTest - vecStepRight;
		vecRightTest = vecRightTest + vecStepRight;

		// it's faster to check the SightEnt's visibility to the potential spot than to check the local move, so we do that first.
		UTIL_TraceLine(vecThreat + vecViewOffset, vecLeftTest + pev->view_ofs, IgnoreMonsters::Yes, IgnoreGlass::Yes, edict(), &tr);

		if (tr.flFraction != 1.0)
		{
			if (ValidateCover(vecLeftTest) && CheckLocalMove(pev->origin, vecLeftTest, nullptr, nullptr) == LocalMoveResult::Valid)
			{
				if (MoveToLocation(ACT_RUN, 0, vecLeftTest))
				{
					return true;
				}
			}
		}

		// it's faster to check the SightEnt's visibility to the potential spot than to check the local move, so we do that first.
		UTIL_TraceLine(vecThreat + vecViewOffset, vecRightTest + pev->view_ofs, IgnoreMonsters::Yes, IgnoreGlass::Yes, edict(), &tr);

		if (tr.flFraction != 1.0)
		{
			if (ValidateCover(vecRightTest) && CheckLocalMove(pev->origin, vecRightTest, nullptr, nullptr) == LocalMoveResult::Valid)
			{
				if (MoveToLocation(ACT_RUN, 0, vecRightTest))
				{
					return true;
				}
			}
		}
	}

	return false;
}

Vector CBaseMonster::ShootAtEnemy(const Vector& shootOrigin)
{
	if (CBaseEntity* pEnemy = m_hEnemy; pEnemy)
	{
		return ((pEnemy->BodyTarget(shootOrigin) - pEnemy->pev->origin) + m_vecEnemyLKP - shootOrigin).Normalize();
	}
	else
		return gpGlobals->v_forward;
}

bool CBaseMonster::FacingIdeal()
{
	if (fabs(YawDiff()) <= 0.006)//!!!BUGBUG - no magic numbers!!!
	{
		return true;
	}

	return false;
}

bool CBaseMonster::CanActiveIdle()
{
	/*
	if ( m_MonsterState == NPCState::Idle && m_IdealMonsterState == NPCState::Idle && !IsMoving() )
	{
		return true;
	}
	*/
	return false;
}

void CBaseMonster::PlaySentence(const char* pszSentence, float duration, float volume, float attenuation)
{
	if (pszSentence && IsAlive())
	{
		if (pszSentence[0] == '!')
			EmitSound(SoundChannel::Voice, pszSentence, volume, attenuation);
		else
			SENTENCEG_PlayRndSz(this, pszSentence, volume, attenuation, PITCH_NORM);
	}
}

void CBaseMonster::PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, bool bConcurrent, CBaseEntity* pListener)
{
	PlaySentence(pszSentence, duration, volume, attenuation);
}

void CBaseMonster::SentenceStop()
{
	EmitSound(SoundChannel::Voice, "common/null.wav", VOL_NORM, ATTN_IDLE);
}

void CBaseMonster::CorpseFallThink()
{
	if (pev->flags & FL_ONGROUND)
	{
		SetThink(nullptr);

		SetSequenceBox();
		SetAbsOrigin(pev->origin);// link into world.
	}
	else
		pev->nextthink = gpGlobals->time + 0.1;
}

void CBaseMonster::MonsterInitDead()
{
	InitBoneControllers();

	pev->solid = Solid::BBox;
	pev->movetype = Movetype::Toss;// so he'll fall to ground

	pev->frame = 0;
	ResetSequenceInfo();
	pev->framerate = 0;

	// Copy health
	pev->max_health = pev->health;
	pev->deadflag = DeadFlag::Dead;

	SetSize(vec3_origin, vec3_origin);
	SetAbsOrigin(pev->origin);

	// Setup health counters, etc.
	BecomeDead();
	SetThink(&CBaseMonster::CorpseFallThink);
	pev->nextthink = gpGlobals->time + 0.5;
}

bool CBaseMonster::BBoxFlat()
{
	const float flXSize = pev->size.x / 2;
	const float flYSize = pev->size.y / 2;

	Vector vecPoint
	{
		pev->origin.x + flXSize,
		pev->origin.y + flYSize,
		pev->origin.z
	};

	TraceResult	tr;
	UTIL_TraceLine(vecPoint, vecPoint - Vector(0, 0, 100), IgnoreMonsters::Yes, edict(), &tr);
	float flLength = (vecPoint - tr.vecEndPos).Length();

	vecPoint.x = pev->origin.x - flXSize;
	vecPoint.y = pev->origin.y - flYSize;

	UTIL_TraceLine(vecPoint, vecPoint - Vector(0, 0, 100), IgnoreMonsters::Yes, edict(), &tr);
	float flLength2 = (vecPoint - tr.vecEndPos).Length();
	if (flLength2 > flLength)
	{
		return false;
	}
	flLength = flLength2;

	vecPoint.x = pev->origin.x - flXSize;
	vecPoint.y = pev->origin.y + flYSize;
	UTIL_TraceLine(vecPoint, vecPoint - Vector(0, 0, 100), IgnoreMonsters::Yes, edict(), &tr);
	flLength2 = (vecPoint - tr.vecEndPos).Length();
	if (flLength2 > flLength)
	{
		return false;
	}
	flLength = flLength2;

	vecPoint.x = pev->origin.x + flXSize;
	vecPoint.y = pev->origin.y - flYSize;
	UTIL_TraceLine(vecPoint, vecPoint - Vector(0, 0, 100), IgnoreMonsters::Yes, edict(), &tr);
	flLength2 = (vecPoint - tr.vecEndPos).Length();
	if (flLength2 > flLength)
	{
		return false;
	}
	flLength = flLength2;

	return true;
}

bool CBaseMonster::GetEnemy()
{
	if (HasConditions(bits_COND_SEE_HATE | bits_COND_SEE_DISLIKE | bits_COND_SEE_NEMESIS))
	{
		if (CBaseEntity* pNewEnemy = BestVisibleEnemy(); pNewEnemy != m_hEnemy && pNewEnemy != nullptr)
		{
			// DO NOT mess with the monster's m_hEnemy pointer unless the schedule the monster is currently running will be interrupted
			// by COND_NEW_ENEMY. This will eliminate the problem of monsters getting a new enemy while they are in a schedule that doesn't care,
			// and then not realizing it by the time they get to a schedule that does. I don't feel this is a good permanent fix. 

			if (m_pSchedule)
			{
				if (m_pSchedule->iInterruptMask & bits_COND_NEW_ENEMY)
				{
					PushEnemy(m_hEnemy, m_vecEnemyLKP);
					SetConditions(bits_COND_NEW_ENEMY);
					m_hEnemy = pNewEnemy;
					m_vecEnemyLKP = m_hEnemy->pev->origin;
				}
				// if the new enemy has an owner, take that one as well
				if (pNewEnemy->pev->owner != nullptr)
				{
					if (auto pOwner = Instance(pNewEnemy->pev->owner); pOwner)
					{
						if (CBaseEntity* pMonsterOwner = pOwner->MyMonsterPointer();
							pMonsterOwner && (pMonsterOwner->pev->flags & FL_MONSTER) && GetRelationship(pMonsterOwner) != Relationship::None)
						{
							PushEnemy(pMonsterOwner, m_vecEnemyLKP);
						}
					}
				}
			}
		}
	}

	// remember old enemies
	if (m_hEnemy == nullptr && PopEnemy())
	{
		if (m_pSchedule)
		{
			if (m_pSchedule->iInterruptMask & bits_COND_NEW_ENEMY)
			{
				SetConditions(bits_COND_NEW_ENEMY);
			}
		}
	}

	if (m_hEnemy != nullptr)
	{
		// monster has an enemy.
		return true;
	}

	return false;// monster has no enemy
}

CBaseEntity* CBaseMonster::DropItem(const char* pszItemName, const Vector& vecPos, const Vector& vecAng)
{
	if (!pszItemName)
	{
		ALERT(at_console, "DropItem() - No item name!\n");
		return nullptr;
	}

	if (CBaseEntity* pItem = CBaseEntity::Create(pszItemName, vecPos, vecAng, this); pItem)
	{
		// do we want this behavior to be default?! (sjb)
		pItem->pev->velocity = pev->velocity;
		pItem->pev->avelocity = Vector(0, RANDOM_FLOAT(0, 100), 0);
		return pItem;
	}
	else
	{
		ALERT(at_console, "DropItem() - Didn't create!\n");
		return nullptr;
	}
}

bool CBaseMonster::ShouldFadeOnDeath()
{
	// if flagged to fade out or I have an owner (I came from a monster spawner)
	return (pev->spawnflags & SF_MONSTER_FADECORPSE) || !IsNullEnt(pev->owner);
}
