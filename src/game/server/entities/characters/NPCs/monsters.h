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

#include "brush/func_break.h"
#include "skill.h"

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

enum class MoveToOriginType
{
	Normal = 0,	//!< normal move in the direction monster is facing
	Strafe = 1,	//!< moves in direction specified, no matter which way monster is facing
};

// spawn flags 256 and above are already taken by the engine
void UTIL_MoveToOrigin(CBaseEntity* pent, const Vector& vecGoal, float flDist, MoveToOriginType iMoveType);

/**
*	@brief returns the velocity at which an object should be lobbed from vecspot1 to land near vecspot2.
*	@return vec3_origin if toss is not feasible.
*/
Vector CheckToss(CBaseEntity* pEntity, const Vector& vecSpot1, Vector vecSpot2, float flGravityAdj = 1.0);

/**
*	@brief returns the velocity vector at which an object should be thrown from vecspot1 to hit vecspot2.
*	@return vec3_origin if throw is not feasible.
*/
Vector CheckThrow(CBaseEntity* pEntity, const Vector& vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj = 1.0);
extern DLL_GLOBAL Vector		g_vecAttackDir;

/**
*	@brief tosses a brass shell from passed origin at passed velocity
*/
void EjectBrass(const Vector& vecOrigin, const Vector& vecVelocity, float rotation, int model, int soundtype);
void ExplodeModel(const Vector& vecOrigin, float speed, int model, int count);

/**
*	@brief a more accurate ( and slower ) version of IsVisible.
*	!!!UNDONE - make this CBaseMonster?
*/
bool IsBoxVisible(CBaseEntity* pLooker, CBaseEntity* pTarget, Vector& vecTargetOrigin, float flSize = 0.0);

void DrawRoute(CBaseEntity* pEntity, WayPoint_t* m_Route, int m_iRouteIndex, int r, int g, int b);

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

//
// A gib is a chunk of a body, or a piece of wood/metal/rocks/etc.
//
class CGib : public CBaseEntity
{
public:
	/**
	*	@brief Throw a chunk
	*/
	void Spawn(const char* szGibModel);

	/**
	*	@brief Gib bounces on the ground or wall, sponges some blood down, too!
	*/
	void EXPORT BounceGibTouch(CBaseEntity* pOther);

	/**
	*	@brief Sticky gib puts blood on the wall and stays put.
	*/
	void EXPORT StickyGibTouch(CBaseEntity* pOther);

	/**
	*	@brief in order to emit their meaty scent from the proper location,
	*	gibs should wait until they stop bouncing to emit their scent. That's what this function does.
	*/
	void EXPORT WaitTillLand();
	void		LimitVelocity();

	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	static	void SpawnHeadGib(CBaseEntity* pVictim);
	static	void SpawnRandomGibs(CBaseEntity* pVictim, int cGibs, bool human);
	static  void SpawnStickyGibs(CBaseEntity* pVictim, Vector vecOrigin, int cGibs);

	int		m_bloodColor;
	int		m_cBloodDecals;
	Materials m_material;
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
