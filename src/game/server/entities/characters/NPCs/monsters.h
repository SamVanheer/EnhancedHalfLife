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

#pragma once

/**
*	@file
*
*	monster-related utility code
*/

#include "skill.h"

// CHECKLOCALMOVE result types 
constexpr int LOCALMOVE_INVALID = 0; // move is not possible
constexpr int LOCALMOVE_INVALID_DONT_TRIANGULATE = 1; // move is not possible, don't try to triangulate
constexpr int LOCALMOVE_VALID = 2; // move is possible

// Hit Group standards
constexpr int HITGROUP_GENERIC = 0;
constexpr int HITGROUP_HEAD = 1;
constexpr int HITGROUP_CHEST = 2;
constexpr int HITGROUP_STOMACH = 3;
constexpr int HITGROUP_LEFTARM = 4;
constexpr int HITGROUP_RIGHTARM = 5;
constexpr int HITGROUP_LEFTLEG = 6;
constexpr int HITGROUP_RIGHTLEG = 7;


// Monster Spawnflags
constexpr int SF_MONSTER_WAIT_TILL_SEEN = 1;	//!< spawnflag that makes monsters wait until player can see them before attacking.
constexpr int SF_MONSTER_GAG = 2;				//!< no idle noises from this monster
constexpr int SF_MONSTER_HITMONSTERCLIP = 4;
//										8
constexpr int SF_MONSTER_PRISONER = 16;			//!< monster won't attack anyone, no one will attacke him.
//										32
//										64
constexpr int SF_MONSTER_WAIT_FOR_SCRIPT = 128;	//!< spawnflag that makes monsters wait to check for attacking until the script is done or they've been attacked
constexpr int SF_MONSTER_PREDISASTER = 256;		//!< this is a predisaster scientist or barney. Influences how they speak.
constexpr int SF_MONSTER_FADECORPSE = 512;		//!< Fade out corpse after death
constexpr int SF_MONSTER_FALL_TO_GROUND = 0x80000000;

// specialty spawnflags
constexpr int SF_MONSTER_TURRET_AUTOACTIVATE = 32;
constexpr int SF_MONSTER_TURRET_STARTINACTIVE = 64;
constexpr int SF_MONSTER_WAIT_UNTIL_PROVOKED = 64; //!< don't attack the player unless provoked



// MoveToOrigin stuff
constexpr int MOVE_START_TURN_DIST = 64;	//!< when this far away from moveGoal, start turning to face next goal
constexpr int MOVE_STUCK_DIST = 32;			//!< if a monster can't step this far, it is stuck.


// MoveToOrigin stuff
constexpr int MOVE_NORMAL = 0;				//!< normal move in the direction monster is facing
constexpr int MOVE_STRAFE = 1;				//!< moves in direction specified, no matter which way monster is facing

// spawn flags 256 and above are already taken by the engine
void UTIL_MoveToOrigin( edict_t* pent, const Vector &vecGoal, float flDist, int iMoveType ); 

Vector VecCheckToss ( entvars_t *pev, const Vector &vecSpot1, Vector vecSpot2, float flGravityAdj = 1.0 );
Vector VecCheckThrow ( entvars_t *pev, const Vector &vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj = 1.0 );
extern DLL_GLOBAL Vector		g_vecAttackDir;
void EjectBrass (const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype );
void ExplodeModel( const Vector &vecOrigin, float speed, int model, int count );

bool FBoxVisible ( entvars_t *pevLooker, entvars_t *pevTarget );
bool FBoxVisible ( entvars_t *pevLooker, entvars_t *pevTarget, Vector &vecTargetOrigin, float flSize = 0.0 );

// monster to monster relationship types
constexpr int R_AL = -2;	// (ALLY) pals. Good alternative to R_NO when applicable.
constexpr int R_FR = -1;	// (FEAR)will run
constexpr int R_NO = 0;		// (NO RELATIONSHIP) disregard
constexpr int R_DL = 1;		// (DISLIKE) will attack
constexpr int R_HT = 2;		// (HATE)will attack this character instead of any visible DISLIKEd characters
constexpr int R_NM = 3;		// (NEMESIS)  A monster Will ALWAYS attack its nemsis, no matter what


// these bits represent the monster's memory
constexpr int MEMORY_CLEAR = 0;
constexpr int bits_MEMORY_PROVOKED = 1 << 0;		// right now only used for houndeyes.
constexpr int bits_MEMORY_INCOVER = 1 << 1;			// monster knows it is in a covered position.
constexpr int bits_MEMORY_SUSPICIOUS = 1 << 2;		// Ally is suspicious of the player, and will move to provoked more easily
constexpr int bits_MEMORY_PATH_FINISHED = 1 << 3;	// Finished monster path (just used by big momma for now)
constexpr int bits_MEMORY_ON_PATH = 1 << 4;			// Moving on a path
constexpr int bits_MEMORY_MOVE_FAILED = 1 << 5;		// Movement has already failed
constexpr int bits_MEMORY_FLINCHED = 1 << 6;		// Has already flinched
constexpr int bits_MEMORY_KILLED = 1 << 7;			// HACKHACK -- remember that I've already called my Killed()
constexpr int bits_MEMORY_CUSTOM4 = 1 << 28; 		// Monster-specific memory
constexpr int bits_MEMORY_CUSTOM3 = 1 << 29; 		// Monster-specific memory
constexpr int bits_MEMORY_CUSTOM2 = 1 << 30; 		// Monster-specific memory
constexpr int bits_MEMORY_CUSTOM1 = 1 << 31; 		// Monster-specific memory

// trigger conditions for scripted AI
// these MUST match the CHOICES interface in halflife.fgd for the base monster
enum 
{
	AITRIGGER_NONE = 0,
	AITRIGGER_SEEPLAYER_ANGRY_AT_PLAYER,
	AITRIGGER_TAKEDAMAGE,
	AITRIGGER_HALFHEALTH,
	AITRIGGER_DEATH,
	AITRIGGER_SQUADMEMBERDIE,
	AITRIGGER_SQUADLEADERDIE,
	AITRIGGER_HEARWORLD,
	AITRIGGER_HEARPLAYER,
	AITRIGGER_HEARCOMBAT,
	AITRIGGER_SEEPLAYER_UNCONDITIONAL,
	AITRIGGER_SEEPLAYER_NOT_IN_COMBAT,
};
/*
		0 : "No Trigger"
		1 : "See Player"
		2 : "Take Damage"
		3 : "50% Health Remaining"
		4 : "Death"
		5 : "Squad Member Dead"
		6 : "Squad Leader Dead"
		7 : "Hear World"
		8 : "Hear Player"
		9 : "Hear Combat"
*/

//
// A gib is a chunk of a body, or a piece of wood/metal/rocks/etc.
//
class CGib : public CBaseEntity
{
public:
	void Spawn( const char *szGibModel );
	void EXPORT BounceGibTouch ( CBaseEntity *pOther );
	void EXPORT StickyGibTouch ( CBaseEntity *pOther );
	void EXPORT WaitTillLand();
	void		LimitVelocity();

	int	ObjectCaps() override { return (CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	static	void SpawnHeadGib( entvars_t *pevVictim );
	static	void SpawnRandomGibs( entvars_t *pevVictim, int cGibs, int human );
	static  void SpawnStickyGibs( entvars_t *pevVictim, Vector vecOrigin, int cGibs );

	int		m_bloodColor;
	int		m_cBloodDecals;
	int		m_material;
	float	m_lifeTime;
};


#define CUSTOM_SCHEDULES\
		virtual Schedule_t *ScheduleFromName( const char *pName );\
		static Schedule_t *m_scheduleList[];

#define DEFINE_CUSTOM_SCHEDULES(derivedClass)\
	Schedule_t *derivedClass::m_scheduleList[] =

#define IMPLEMENT_CUSTOM_SCHEDULES(derivedClass, baseClass)\
		Schedule_t *derivedClass::ScheduleFromName( const char *pName )\
		{\
			Schedule_t *pSchedule = ScheduleInList( pName, m_scheduleList, ArraySize(m_scheduleList) );\
			if ( !pSchedule )\
				return baseClass::ScheduleFromName(pName);\
			return pSchedule;\
		}
