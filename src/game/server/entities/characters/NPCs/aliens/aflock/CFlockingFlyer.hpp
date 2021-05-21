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

#pragma once

#include "CBaseMonster.hpp"
#include "CFlockingFlyer.generated.hpp"

constexpr int AFLOCK_MAX_RECRUIT_RADIUS = 1024;
constexpr int AFLOCK_FLY_SPEED = 125;
constexpr int AFLOCK_TURN_RATE = 75;
constexpr int AFLOCK_ACCELERATE = 10;
constexpr int AFLOCK_CHECK_DIST = 192;
constexpr int AFLOCK_TOO_CLOSE = 100;
constexpr int AFLOCK_TOO_FAR = 256;

//TODO: should probably make this inherit from CSquadMonster and remove the squad code from this class
class EHL_CLASS() CFlockingFlyer : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void SpawnCommonCode();
	void EXPORT IdleThink();
	void BoidAdvanceFrame();

	/**
	*	@brief Leader boid calls this to form a flock from surrounding boids
	*/
	void EXPORT FormFlock();

	/**
	*	@brief player enters the pvs, so get things going.
	*/
	void EXPORT Start();

	/**
	*	@brief Leader boids use this think every tenth
	*/
	void EXPORT FlockLeaderThink();

	/**
	*	@brief follower boids execute this code when flocking
	*/
	void EXPORT FlockFollowerThink();
	void EXPORT FallHack();
	void MakeSound();

	/**
	*	@brief Searches for boids that are too close and pushes them away
	*/
	void SpreadFlock();

	/**
	*	@brief Alters the caller's course if he's too close to others
	*	This function should **ONLY** be called when Caller's velocity is normalized!!
	*/
	void SpreadFlock2();
	void Killed(const KilledInfo& info) override;

	/**
	*	@brief returns true if there is an obstacle ahead
	*/
	bool PathBlocked();
	//void KeyValue( KeyValueData *pkvd ) override;

	int IsLeader() { return m_hSquadLeader == this; }
	int	InSquad() { return m_hSquadLeader != nullptr; }

	/**
	*	@brief return the number of members of this squad
	*	callable from leaders & followers
	*/
	int	SquadCount();

	/**
	*	@brief remove pRemove from my squad.
	*	If I am pRemove, promote m_pSquadNext to leader
	*/
	void SquadRemove(CFlockingFlyer* pRemove);

	/**
	*	@brief Unlink the squad pointers.
	*/
	void SquadUnlink();

	/**
	*	@brief add pAdd to my squad
	*/
	void SquadAdd(CFlockingFlyer* pAdd);

	/**
	*	@brief Unlink all squad members
	*/
	void SquadDisband();

	EHL_FIELD(Persisted)
	EHandle<CFlockingFlyer> m_hSquadLeader;

	EHL_FIELD(Persisted)
	EHandle<CFlockingFlyer> m_hSquadNext;

	EHL_FIELD(Persisted)
	bool m_fTurning = false;// is this boid turning?

	EHL_FIELD(Persisted)
	bool m_fCourseAdjust = false;// followers set this flag true to override flocking while they avoid something

	EHL_FIELD(Persisted)
	bool m_fPathBlocked = false;// true if there is an obstacle ahead

	EHL_FIELD(Persisted, Type=Position)
	Vector m_vecReferencePoint;// last place we saw leader

	EHL_FIELD(Persisted)
	Vector m_vecAdjustedVelocity;// adjusted velocity (used when fCourseAdjust is true)

	EHL_FIELD(Persisted)
	float m_flGoalSpeed = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flLastBlockedTime = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flFakeBlockedTime = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flAlertTime = 0;

	//Don't need to save
	float m_flFlockNextSoundTime = 0;
};
