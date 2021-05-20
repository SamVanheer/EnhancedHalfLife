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

#include "CBaseToggle.hpp"
#include "CBaseMonster.schedule.hpp"

#include "CBaseMonster.generated.hpp"

class CCineMonster;
class CSound;

constexpr int MAX_PATH_SIZE = 10; //!< max number of nodes available for a path.
constexpr int ROUTE_SIZE = 8;				//!< how many waypoints a monster can store at one time
constexpr int MAX_OLD_ENEMIES = 4;			//!< how many old enemies to remember

constexpr int bits_CAP_DUCK = 1 << 0;		//!< crouch
constexpr int bits_CAP_JUMP = 1 << 1;		//!< jump/leap
constexpr int bits_CAP_STRAFE = 1 << 2;		//!< strafe ( walk/run sideways)
constexpr int bits_CAP_SQUAD = 1 << 3;		//!< can form squads
constexpr int bits_CAP_SWIM = 1 << 4;		//!< proficiently navigate in water
constexpr int bits_CAP_CLIMB = 1 << 5;		//!< climb ladders/ropes
constexpr int bits_CAP_USE = 1 << 6;		//!< open doors/push buttons/pull levers
constexpr int bits_CAP_HEAR = 1 << 7;		//!< can hear forced sounds
constexpr int bits_CAP_AUTO_DOORS = 1 << 8; //!< can trigger auto doors
constexpr int bits_CAP_OPEN_DOORS = 1 << 9; //!< can open manual doors
constexpr int bits_CAP_TURN_HEAD = 1 << 10;	//!< can turn head, always bone controller 0

constexpr int bits_CAP_RANGE_ATTACK1 = 1 << 11;	//!< can do a range attack 1
constexpr int bits_CAP_RANGE_ATTACK2 = 1 << 12;	//!< can do a range attack 2
constexpr int bits_CAP_MELEE_ATTACK1 = 1 << 13;	//!< can do a melee attack 1
constexpr int bits_CAP_MELEE_ATTACK2 = 1 << 14;	//!< can do a melee attack 2

constexpr int bits_CAP_FLY = 1 << 15;			//!< can fly, move all around

constexpr int bits_CAP_DOORS_GROUP = bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS;

/**
*	@brief monster to monster relationship types
*/
enum class Relationship
{
	Ally = -2,		//!< pals. Good alternative to R_NO when applicable.
	Fear = -1,		//!< will run
	None = 0,		//!< disregard
	Dislike = 1,	//!< will attack
	Hate = 2,		//!< will attack this character instead of any visible DISLIKEd characters
	Nemesis = 3		//!< A monster Will ALWAYS attack its nemsis, no matter what
};


enum class LocalMoveResult
{
	Invalid = 0,
	InvalidDontTriangulate,
	Valid
};

/**
*	@brief trigger conditions for scripted AI
*	@details these MUST match the CHOICES interface in halflife.fgd for the base monster
*	0 : "No Trigger"
*	1 : "See Player"
*	2 : "Take Damage"
*	3 : "50% Health Remaining"
*	4 : "Death"
*	5 : "Squad Member Dead"
*	6 : "Squad Leader Dead"
*	7 : "Hear World"
*	8 : "Hear Player"
*	9 : "Hear Combat"
*/
enum class AITrigger
{
	None = 0,
	SeePlayerAngryAtPlayer,
	TakeDamage,
	HalfHealth,
	Death,
	SquadMemberDie,
	SquadLeaderDie,
	HearWorld,
	HearPlayer,
	HearCombat,
	SeePlayerUnconditional,
	SeePlayerNotInCombat,
};

/**
*	@brief generic Monster
*/
class EHL_CLASS() CBaseMonster : public CBaseToggle
{
	EHL_GENERATED_BODY()

private:
	EHL_FIELD(Persisted)
	int m_afConditions = 0;

public:
	enum class ScriptState
	{
		Playing = 0,		//!< Playing the sequence
		Wait,				//!< Waiting on everyone in the script to be ready
		Cleanup,			//!< Cancelling the script / cleaning up
		WalkToMark,
		RunToMark,
	};

	// UNDONE: Save schedule data? Can this be done?
	// We may lose our enemy pointer or other data (goal ent, target, etc) that make the current schedule invalid,
	// perhaps it's best to just pick a new one when we start up again.

	// these fields have been added in the process of reworking the state machine. (sjb)
	EHL_FIELD(Persisted)
	EHANDLE m_hEnemy;		 //!< the entity that the monster is fighting.

	EHL_FIELD(Persisted)
	EHANDLE m_hTargetEnt;	 //!< the entity that the monster is trying to reach

	EHL_FIELD(Persisted)
	EHANDLE m_hOldEnemy[MAX_OLD_ENEMIES]{};

	EHL_FIELD(Persisted, Type=Position)
	Vector m_vecOldEnemy[MAX_OLD_ENEMIES]{};

	EHL_FIELD(Persisted)
	float m_flFieldOfView = 0; //!< width of monster's field of view ( dot product )

	EHL_FIELD(Persisted, Type=Time)
	float m_flWaitFinished = 0; //!< if we're told to wait, this is the time that the wait will be over.

	EHL_FIELD(Persisted, Type=Time)
	float m_flMoveWaitFinished = 0;

	EHL_FIELD(Persisted)
	Activity m_Activity = ACT_RESET; //!< what the monster is doing (animation)

	EHL_FIELD(Persisted)
	Activity m_IdealActivity = ACT_RESET; //!< monster should switch to this activity

	int m_LastHitGroup = 0; //!< the last body region that took damage

	EHL_FIELD(Persisted)
	NPCState m_MonsterState = NPCState::None; //!< monster's current state

	EHL_FIELD(Persisted)
	NPCState m_IdealMonsterState = NPCState::None; //!< monster should change to this state

	EHL_FIELD(Persisted)
	TaskStatus m_iTaskStatus = TaskStatus::New;

	Schedule_t* m_pSchedule = nullptr;

	EHL_FIELD(Persisted)
	int m_iScheduleIndex = 0;

	WayPoint_t m_Route[ROUTE_SIZE]{};	//!< Positions of movement
	int m_movementGoal = 0;					//!< Goal that defines route
	int m_iRouteIndex = 0;					//!< index into m_Route[]
	float m_moveWaitTime = 0;				//!< How long I should wait for something to move

	EHL_FIELD(Persisted, Type=Position)
	Vector m_vecMoveGoal;			//!< kept around for node graph moves, so we know our ultimate goal

	EHL_FIELD(Persisted)
	Activity m_movementActivity = ACT_RESET;	//!< When moving, set this activity

	int m_iAudibleList = 0; //!< first index of a linked list of sounds that the monster can hear.
	int m_afSoundTypes = 0;

	EHL_FIELD(Persisted, Type=Position)
	Vector m_vecLastPosition;//!< monster sometimes wants to return to where it started after an operation.

	EHL_FIELD(Persisted)
	int m_iHintNode = 0; //!< this is the hint node that the monster is moving towards or performing active idle on.

	EHL_FIELD(Persisted)
	int m_afMemory = 0;

	EHL_FIELD(Persisted)
	int m_iMaxHealth = 0; //!< keeps track of monster's maximum health value (for re-healing, etc)

	EHL_FIELD(Persisted, Type=Position)
	Vector m_vecEnemyLKP; //!< last known position of enemy. (enemy's origin)

	EHL_FIELD(Persisted)
	int m_cAmmoLoaded = 0; //!< how much ammo is in the weapon (used to trigger reload anim sequences)

	EHL_FIELD(Persisted)
	int m_afCapability = 0;//!< tells us what a monster can/can't do.

	EHL_FIELD(Persisted, Type=Time)
	float m_flNextAttack = 0; //!< cannot attack again until this time

	EHL_FIELD(Persisted)
	int m_bitsDamageType = 0; //!< what types of damage has monster (player) taken

	EHL_FIELD(Persisted)
	byte m_rgbTimeBasedDamage[CDMG_TIMEBASED]{};

	int m_lastDamageAmount = 0;//!< how much damage did monster (player) last take
							//!< time based damage counters, decr. 1 per 2 seconds

	EHL_FIELD(Persisted)
	int m_bloodColor = 0; //!< color of blood particless

	EHL_FIELD(Persisted)
	int m_failSchedule = 0; //!< Schedule type to choose if current schedule fails

	EHL_FIELD(Persisted, Type=Time)
	float m_flHungryTime = 0; //!< set this is a future time to stop the monster from eating for a while. 

	EHL_FIELD(Persisted)
	float m_flDistTooFar = 0; //!< if enemy farther away than this, bits_COND_ENEMY_TOOFAR set in CheckEnemy

	EHL_FIELD(Persisted)
	float m_flDistLook = 0;	//!< distance monster sees (Default 2048)

	EHL_FIELD(Persisted)
	AITrigger m_iTriggerCondition = AITrigger::None; //!< for scripted AI, this is the condition that will cause the activation of the monster's TriggerTarget

	EHL_FIELD(Persisted)
	string_t m_iszTriggerTarget = iStringNull; //!< name of target that should be fired. 

	EHL_FIELD(Persisted)
	Vector m_HackedGunPos; //!< HACK until we can query end of gun

// Scripted sequence Info
	EHL_FIELD(Persisted)
	ScriptState m_scriptState = ScriptState::Playing; //!< internal cinematic state

	EHL_FIELD(Persisted)
	EHandle<CCineMonster> m_hCine;

	float m_flLastYawTime = 0;

	bool PostRestore() override;

	/**
	*	@details !!! netname entvar field is used in squadmonster for groupname!!!
	*/
	void KeyValue(KeyValueData* pkvd) override;

	/**
	*	@brief monster use function
	*	will make a monster angry at whomever activated it. (Actually does nothing)
	*/
	void EXPORT MonsterUse(const UseInfo& info);

	// overrideable Monster member functions

	int	 BloodColor() override { return m_bloodColor; }

	CBaseMonster* MyMonsterPointer() override { return this; }

	/**
	*	@brief Base class monster function to find enemies or food by sight.
	*	@param iDistance distance ( in units ) that the monster can see.
	*
	*	@details Sets the sight bits of the m_afConditions mask to indicate which types of entities were sighted.
	*	Function also sets the Looker's m_pLink to the head of a link list that contains all visible ents.
	*	(linked via each ent's m_pLink field)
	*/
	virtual void Look(int iDistance);

	/**
	*	@brief core ai function!
	*/
	virtual void RunAI();

	/**
	*	@brief monsters dig through the active sound list for any sounds that may interest them. (smells, too!)
	*/
	void Listen();

	bool IsAlive() override { return (pev->deadflag != DeadFlag::Dead); }
	virtual bool ShouldFadeOnDeath();

	// Basic Monster AI functions
	/**
	*	@brief turns a monster towards its ideal_yaw
	*/
	virtual float ChangeYaw(int speed);

	/**
	*	@brief turns a directional vector into a yaw value that points down that vector.
	*/
	float VecToYaw(const Vector& vecDir);

	/**
	*	@brief returns the difference ( in degrees ) between monster's current yaw and ideal_yaw
	*	Positive result is left turn, negative is right turn
	*/
	float YawDiff();

	float DamageForce(float damage);

	// stuff written for new state machine
	/**
	*	@brief calls out to core AI functions and handles this monster's specific animation events
	*/
	virtual void MonsterThink();
	void EXPORT	CallMonsterThink() { this->MonsterThink(); }

	/**
	*	@brief returns an integer that describes the relationship between two types of monster.
	*/
	virtual Relationship GetRelationship(CBaseEntity* pTarget);

	/**
	*	@brief after a monster is spawned, it needs to be dropped into the world,
	*	checked for mobility problems, and put on the proper path, if any.
	*	This function does all of those things after the monster spawns.
	*	Any initialization that should take place for all monsters goes here.
	*/
	virtual void MonsterInit();

	/**
	*	@brief Call after animation/pose is set up
	*/
	virtual void MonsterInitDead();
	virtual void BecomeDead();
	void EXPORT CorpseFallThink();

	/**
	*	@brief Calls StartMonster. Startmonster is virtual, but this function cannot be
	*/
	void EXPORT MonsterInitThink();

	/**
	*	@brief final bit of initization before a monster is turned over to the AI.
	*/
	virtual void StartMonster();

	/**
	*	@brief finds best visible enemy for attack
	*	@details this functions searches the link list whose head is the caller's m_pLink field,
	*	and returns a pointer to the enemy entity in that list that is nearest the caller.
	*
	*	!!!UNDONE - currently, this only returns the closest enemy.
	*	we'll want to consider distance, relationship, attack types, back turned, etc.
	*/
	virtual CBaseEntity* BestVisibleEnemy();

	/**
	*	@brief returns true is the passed ent is in the caller's forward view cone.
	*	The dot product is performed in 2d, making the view cone infinitely tall.
	*/
	virtual bool IsInViewCone(CBaseEntity* pEntity);

	/**
	*	@brief returns true is the passed vector is in the caller's forward view cone.
	*	The dot product is performed in 2d, making the view cone infinitely tall.
	*/
	virtual bool IsInViewCone(const Vector& origin);
	void HandleAnimEvent(AnimationEvent& event) override;

	/**
	*	@brief returns true if the caller can walk a straight line from its current origin to the given location.
	*	If so, don't use the node graph!
	*
	*	@details if a valid pointer to a int is passed,
	*	the function will fill that int with the distance that the check reached before hitting something.
	*	THIS ONLY HAPPENS IF THE LOCAL MOVE CHECK FAILS!
	*
	*	!!!PERFORMANCE - should we try to load balance this?
	*	DON"T USE SETORIGIN!
	*/
	virtual LocalMoveResult CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, float* pflDist);

	/**
	*	@brief take a single step towards the next ROUTE location
	*/
	virtual void Move(float flInterval = 0.1);
	virtual void MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval);
	virtual bool ShouldAdvanceRoute(float flWaypointDist);

	virtual Activity GetStoppedActivity() { return ACT_IDLE; }
	virtual void Stop() { m_IdealActivity = GetStoppedActivity(); }

	/**
	*	@brief This will stop animation until you call ResetSequenceInfo() at some point in the future
	*/
	inline void StopAnimation() { pev->framerate = 0; }

	/**
	*	@brief will survey conditions and set appropriate conditions bits for attack types.
	*	flDot is the cos of the angle of the cone within which the attack can occur.
	*/
	virtual bool CheckRangeAttack1(float flDot, float flDist);
	virtual bool CheckRangeAttack2(float flDot, float flDist); //!< @copydoc CheckRangeAttack1(float, float)
	virtual bool CheckMeleeAttack1(float flDot, float flDist); //!< @copydoc CheckRangeAttack1(float, float)
	virtual bool CheckMeleeAttack2(float flDot, float flDist); //!< @copydoc CheckRangeAttack1(float, float)

	/**
	*	@brief Returns true if monster's m_pSchedule is anything other than nullptr.
	*/
	bool HasSchedule();

	/**
	*	@brief returns true as long as the current schedule is still the proper schedule to be executing, taking into account all conditions
	*/
	bool IsScheduleValid();

	/**
	*	@brief blanks out the caller's schedule pointer and index.
	*/
	void ClearSchedule();

	/**
	*	@brief Returns true if the caller is on the last task in the schedule
	*/
	bool IsScheduleDone();

	/**
	*	@brief replaces the monster's schedule pointer with the passed pointer, and sets the ScheduleIndex back to 0
	*/
	void ChangeSchedule(Schedule_t* pNewSchedule);

	/**
	*	@brief increments the ScheduleIndex
	*/
	void NextScheduledTask();
	Schedule_t* ScheduleInList(const char* pName, Schedule_t** pList, int listCount);

	virtual Schedule_t* ScheduleFromName(const char* pName);
	static Schedule_t* m_scheduleList[];

	/**
	*	@brief does all the per-think schedule maintenance.
	*	ensures that the monster leaves this function with a valid schedule!
	*/
	void MaintainSchedule();

	/**
	*	@brief selects the correct activity and performs any necessary calculations
	*	to start the next task on the schedule.
	*/
	virtual void StartTask(Task_t* pTask);
	virtual void RunTask(Task_t* pTask);

	/**
	*	@brief returns a pointer to one of the monster's available schedules of the indicated type.
	*/
	virtual Schedule_t* GetScheduleOfType(int Type);

	/**
	*	@brief Decides which type of schedule best suits the monster's current state and conditions.
	*	Then calls monster's member function to get a pointer to a schedule of the proper type.
	*/
	virtual Schedule_t* GetSchedule();
	virtual void ScheduleChange() {}

	/*
	virtual bool CanPlaySequence()
	{
		return ((m_pCine == nullptr) && (m_MonsterState == NPCState::None || m_MonsterState == NPCState::Idle || m_IdealMonsterState == NPCState::Idle));
	}
	*/

	/**
	*	@brief determines whether or not the monster can play the scripted sequence or AI sequence that is trying to possess it.
	*	@param fDisregardState if set, the monster will be sucked into the script no matter what state it is in.
	*							ONLY Scripted AI ents should allow this.
	*/
	virtual bool CanPlaySequence(bool fDisregardState, int interruptLevel);
	virtual bool CanPlaySentence(bool fDisregardState) { return IsAlive(); }
	virtual void PlaySentence(const char* pszSentence, float duration, float volume, float attenuation);
	virtual void PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, bool bConcurrent, CBaseEntity* pListener);

	virtual void SentenceStop();

	/**
	*	@brief returns a pointer to the current scheduled task. nullptr if there's a problem.
	*/
	Task_t* GetTask();

	/**
	*	@brief surveys the Conditions information available and finds the best new state for a monster.
	*/
	virtual NPCState GetIdealState();
	virtual void SetActivity(Activity NewActivity);
	void SetSequenceByName(const char* szSequence);
	void SetState(NPCState State);
	virtual void ReportAIState();

	/**
	*	@brief sets all of the bits for attacks that the monster is capable of carrying out on the passed entity.
	*/
	void CheckAttacks(CBaseEntity* pTarget, float flDist);

	/**
	*	@brief part of the Condition collection process,
	*	gets and stores data and conditions pertaining to a monster's enemy.
	*	@return true if Enemy LKP was updated.
	*/
	virtual bool CheckEnemy(CBaseEntity* pEnemy);

	/**
	*	@brief remember the last few enemies, always remember the player
	*/
	void PushEnemy(CBaseEntity* pEnemy, const Vector& vecLastKnownPos);

	/**
	*	@brief try remembering the last few enemies
	*/
	bool PopEnemy();

	/**
	*	@brief tries to build an entire node path from the callers origin to the passed vector.
	*	If this is possible, ROUTE_SIZE waypoints will be copied into the callers m_Route.
	*	@return true if the operation succeeds (path is valid) or false if failed (no path  exists )
	*/
	bool GetNodeRoute(Vector vecDest);

	inline void TaskComplete() { if (!HasConditions(bits_COND_TASK_FAILED)) m_iTaskStatus = TaskStatus::Complete; }
	void MovementComplete();
	inline void TaskFail() { SetConditions(bits_COND_TASK_FAILED); }
	inline void TaskBegin() { m_iTaskStatus = TaskStatus::Running; }
	bool TaskIsRunning();
	inline bool TaskIsComplete() { return m_iTaskStatus == TaskStatus::Complete; }
	inline bool MovementIsComplete() { return (m_movementGoal == MOVEGOAL_NONE); }

	/**
	*	@brief returns an integer with all Conditions bits that are currently set and also set in the current schedule's Interrupt mask.
	*/
	int ScheduleFlags();

	/**
	*	@brief after calculating a path to the monster's target,
	*	this function copies as many waypoints as possible from that path to the monster's Route array
	*/
	bool RefreshRoute();

	/**
	*	@brief returns true is the Route is cleared out ( invalid )
	*/
	bool IsRouteClear();

	/**
	*	@brief Attempts to make the route more direct by cutting out unnecessary nodes & cutting corners.
	*/
	void RouteSimplify(CBaseEntity* pTargetEnt);

	/**
	*	@brief poorly named function that advances the m_iRouteIndex.
	*	If it goes beyond ROUTE_SIZE, the route is refreshed.
	*/
	void AdvanceRoute(float distance);

	/**
	*	@brief tries to overcome local obstacles by triangulating a path around them.
	*	@param[out] pApex is how far the obstruction that we are trying to triangulate around is from the monster.
	*/
	virtual bool Triangulate(const Vector& vecStart, const Vector& vecEnd, float flDist, CBaseEntity* pTargetEnt, Vector* pApex);

	/**
	*	@brief gets a yaw value for the caller that would face the supplied vector.
	*	Value is stuffed into the monster's ideal_yaw
	*/
	void MakeIdealYaw(const Vector& vecTarget);

	/**
	*	@brief allows different yaw_speeds for each activity
	*/
	virtual void SetYawSpeed() {}
	bool BuildRoute(const Vector& vecGoal, int iMoveFlag, CBaseEntity* pTarget);

	/**
	*	@brief tries to build a route as close to the target as possible, even if there isn't a path to the final point.
	*
	*	@details If supplied, search will return a node at least as far away as MinDist from vecThreat,
	*	but no farther than MaxDist.
	*	if MaxDist isn't supplied, it defaults to a reasonable value
	*/
	virtual bool BuildNearestRoute(Vector vecThreat, const Vector& vecViewOffset, float flMinDist, float flMaxDist);
	int RouteClassify(int iMoveFlag);

	/**
	*	@brief Rebuilds the existing route so that the supplied vector and moveflags are the first waypoint in the route,
	*	and fills the rest of the route with as much of the pre-existing route as possible
	*/
	void InsertWaypoint(Vector vecLocation, int afMoveFlags);

	/**
	*	@brief attempts to locate a spot in the world directly to the left or right of the caller that will conceal them from view of pSightEnt
	*/
	bool FindLateralCover(const Vector& vecThreat, const Vector& vecViewOffset);

	/**
	*	@brief tries to find a nearby node that will hide the caller from its enemy.
	*
	*	@details If supplied, search will return a node at least as far away as flMinDist, but no farther than flMaxDist.
	*	if flMaxDist isn't supplied, it defaults to a reasonable value
	*	UNDONE: Should this find the nearest node?
	*/
	virtual bool FindCover(Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist);

	/**
	*	@brief determines whether or not the chosen cover location is a good one to move to.
	*/
	virtual bool ValidateCover(const Vector& vecCoverLocation) { return true; }
	virtual float CoverRadius() { return 784; } // Default cover radius

	/**
	*	@brief prequalifies a monster to do more fine checking of potential attacks.
	*/
	virtual bool CanCheckAttacks();
	virtual void CheckAmmo() {}

	/**
	*	@brief before a set of conditions is allowed to interrupt a monster's schedule,
	*	this function removes conditions that we have flagged to interrupt the current schedule,
	*	but may not want to interrupt the schedule every time. (Pain, for instance)
	*/
	virtual int IgnoreConditions();

	inline void	SetConditions(int iConditions) { m_afConditions |= iConditions; }
	inline void	ClearConditions(int iConditions) { m_afConditions &= ~iConditions; }
	inline bool HasConditions(int iConditions) { if (m_afConditions & iConditions) return true; return false; }
	inline bool HasAllConditions(int iConditions) { if ((m_afConditions & iConditions) == iConditions) return true; return false; }

	/**
	*	@brief tells use whether or not the monster cares about the type of Hint Node given
	*/
	virtual bool ValidateHintType(short sHint);
	int FindHintNode();
	virtual bool CanActiveIdle();

	/**
	*	@brief measures the difference between the way the monster is facing
	*	and determines whether or not to select one of the 180 turn animations.
	*/
	void SetTurnActivity();

	/**
	*	@brief subtracts the volume of the given sound from the distance the sound source is from the caller,
	*	and returns that value, which is considered to be the 'local' volume of the sound.
	*/
	float SoundVolume(CSound* pSound);

	bool MoveToNode(Activity movementAct, float waitTime, const Vector& goal);
	bool MoveToTarget(Activity movementAct, float waitTime);
	bool MoveToLocation(Activity movementAct, float waitTime, const Vector& goal);
	bool MoveToEnemy(Activity movementAct, float waitTime);

	/**
	*	@brief Returns the time when the door will be open
	*/
	float OpenDoorAndWait(CBaseEntity* pDoor);

	/**
	*	@brief returns a bit mask indicating which types of sounds this monster regards.
	*	In the base class implementation, monsters care about all sounds, but no scents.
	*/
	virtual int SoundMask();

	/**
	*	@brief returns a pointer to the sound the monster should react to. Right now responds only to nearest sound.
	*/
	virtual CSound* BestSound();

	/**
	*	@brief returns a pointer to the scent the monster should react to. Right now responds only to nearest scent
	*/
	virtual CSound* BestScent();
	virtual float HearingSensitivity() { return 1.0; }

	/**
	*	@brief tries to send a monster into PRONE state.
	*	right now only used when a barnacle snatches someone, so may have some special case stuff for that.
	*/
	bool BecomeProne() override;

	/**
	*	@brief called by Barnacle victims when the barnacle pulls their head into its mouth
	*/
	virtual void BarnacleVictimBitten(CBaseEntity* pBarnacle);

	/**
	*	@brief called by barnacle victims when the host barnacle is killed.
	*/
	virtual void BarnacleVictimReleased();

	/**
	*	@brief queries the monster's model for $eyeposition and copies that vector to the monster's view_ofs
	*/
	void SetEyePosition();

	/**
	*	@brief returns true if a monster is 'hungry'.
	*/
	bool ShouldEat();

	/**
	*	@brief makes a monster 'full' for a little while.
	*/
	void Eat(float flFullDuration);

	/**
	*	@brief expects a length to trace, amount of damage to do, and damage type.
	*	@details Used for many contact-range melee attacks. Bites, claws, etc.
	*	@return a pointer to the damaged entity in case the monster wishes to do other stuff to the victim (punchangle, etc)
	*/
	CBaseEntity* CheckTraceHullAttack(float flDist, int iDamage, int iDmgType);

	/**
	*	@brief tells us if a monster is facing its ideal yaw.
	*	@details Created this function because many spots in the code were checking the yawdiff against this magic number.
	*	Nicer to have it in one place if we're gonna be stuck with it.
	*/
	bool FacingIdeal();

	/**
	*	@brief checks the monster's AI Trigger Conditions, if there is a condition, then checks to see if condition is met.
	*	If yes, the monster's TriggerTarget is fired.
	*	@return true if the target is fired.
	*/
	bool CheckAITrigger();

	/**
	*	@brief check to see if the monster's bounding box is lying flat on a surface (traces from all four corners are same length.)
	*/
	bool BBoxFlat();

	/**
	*	@brief this function runs after conditions are collected and before scheduling code is run.
	*/
	virtual void PrescheduleThink() {}

	/**
	*	@brief tries to find the best suitable enemy for the monster.
	*/
	bool GetEnemy();
	void MakeDamageBloodDecal(int cCount, float flNoise, TraceResult* ptr, const Vector& vecDir);
	void TraceAttack(const TraceAttackInfo& info) override;

	// combat functions
	/**
	*	@brief determines the best type of death anim to play.
	*/
	virtual Activity GetDeathActivity();

	/**
	*	@brief determines the best type of flinch anim to play.
	*/
	Activity GetSmallFlinchActivity();
	void Killed(const KilledInfo& info) override;

	/**
	*	@brief create some gore and get rid of a monster's model.
	*/
	virtual void GibMonster();
	bool ShouldGibMonster(GibType gibType);
	void CallGibMonster();
	virtual bool HasHumanGibs();
	virtual bool HasAlienGibs();

	/**
	*	@brief Called instead of GibMonster() when gibs are disabled
	*/
	virtual void FadeMonster();

	Vector ShootAtEnemy(const Vector& shootOrigin);

	/**
	*	@brief position to shoot at
	*/
	Vector BodyTarget(const Vector& posSrc) override { return Center() * 0.75 + EyePosition() * 0.25; }

	virtual	Vector GetGunPosition();

	bool GiveHealth(float flHealth, int bitsDamageType) override;

	/**
	*	@brief The damage is coming from inflictor, but get mad at attacker
	*	@details This should be the only function that ever reduces health.
	*	bitsDamageType indicates the type of damage sustained, ie: DMG_SHOCK
	*
	*	Time-based damage: only occurs while the monster is within the trigger_hurt.
	*	When a monster is poisoned via an arrow etc it takes all the poison damage at once.
	*/
	bool TakeDamage(const TakeDamageInfo& info) override;

	/**
	*	@brief takedamage function called when a monster's corpse is damaged.
	*/
	bool DeadTakeDamage(const TakeDamageInfo& info);

	void RadiusDamage(CBaseEntity* pInflictor, CBaseEntity* pAttacker, float flDamage, int iClassIgnore, int bitsDamageType);
	void RadiusDamage(const Vector& vecSrc, CBaseEntity* pInflictor, CBaseEntity* pAttacker, float flDamage, int iClassIgnore, int bitsDamageType);
	bool IsMoving() override { return m_movementGoal != MOVEGOAL_NONE; }

	/**
	*	@brief zeroes out the monster's route array and goal
	*/
	void RouteClear();

	/**
	*	@brief clears out a route to be changed, but keeps goal intact.
	*/
	void RouteNew();

	virtual void DeathSound() {}
	virtual void AlertSound() {}
	virtual void IdleSound() {}
	virtual void PainSound() {}

	virtual void StopFollowing(bool clearSchedule) {}

	inline void	Remember(int iMemory) { m_afMemory |= iMemory; }
	inline void	Forget(int iMemory) { m_afMemory &= ~iMemory; }
	inline bool HasMemory(int iMemory) { if (m_afMemory & iMemory) return true; return false; }
	inline bool HasAllMemories(int iMemory) { if ((m_afMemory & iMemory) == iMemory) return true; return false; }

	bool ExitScriptedSequence();
	bool CineCleanup();

	/**
	*	@brief dead monster drops named item
	*/
	CBaseEntity* DropItem(const char* pszItemName, const Vector& vecPos, const Vector& vecAng);// drop an item.
};