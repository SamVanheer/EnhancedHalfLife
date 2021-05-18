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

#include "basemonster.h"

#include "CSquadMonster.generated.hpp"

constexpr int	SF_SQUADMONSTER_LEADER = 32;


constexpr int bits_NO_SLOT = 0;

// HUMAN GRUNT SLOTS
constexpr int bits_SLOT_HGRUNT_ENGAGE1 = 1 << 0;
constexpr int bits_SLOT_HGRUNT_ENGAGE2 = 1 << 1;
constexpr int bits_SLOTS_HGRUNT_ENGAGE = bits_SLOT_HGRUNT_ENGAGE1 | bits_SLOT_HGRUNT_ENGAGE2;

constexpr int bits_SLOT_HGRUNT_GRENADE1 = 1 << 2;
constexpr int bits_SLOT_HGRUNT_GRENADE2 = 1 << 3;
constexpr int bits_SLOTS_HGRUNT_GRENADE = bits_SLOT_HGRUNT_GRENADE1 | bits_SLOT_HGRUNT_GRENADE2;

// ALIEN GRUNT SLOTS
constexpr int bits_SLOT_AGRUNT_HORNET1 = 1 << 4;
constexpr int bits_SLOT_AGRUNT_HORNET2 = 1 << 5;
constexpr int bits_SLOT_AGRUNT_CHASE = 1 << 6;
constexpr int bits_SLOTS_AGRUNT_HORNET = bits_SLOT_AGRUNT_HORNET1 | bits_SLOT_AGRUNT_HORNET2;

// HOUNDEYE SLOTS
constexpr int bits_SLOT_HOUND_ATTACK1 = 1 << 7;
constexpr int bits_SLOT_HOUND_ATTACK2 = 1 << 8;
constexpr int bits_SLOT_HOUND_ATTACK3 = 1 << 9;
constexpr int bits_SLOTS_HOUND_ATTACK = bits_SLOT_HOUND_ATTACK1 | bits_SLOT_HOUND_ATTACK2 | bits_SLOT_HOUND_ATTACK3;

// global slots
constexpr int bits_SLOT_SQUAD_SPLIT = 1 << 10;// squad members don't all have the same enemy

constexpr int NUM_SLOTS = 11;// update this every time you add/remove a slot.

constexpr int MAX_SQUAD_MEMBERS = 5;

/**
*	@brief for any monster that forms squads.
*/
class EHL_CLASS() CSquadMonster : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	// squad leader info
	EHL_FIELD(Persisted)
	EHandle<CSquadMonster> m_hSquadLeader;		//!< who is my leader

	EHL_FIELD(Persisted)
	EHandle<CSquadMonster> m_hSquadMember[MAX_SQUAD_MEMBERS - 1]{};	//!< valid only for leader

	// these need to be reset after transitions!
	int m_afSquadSlots = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flLastEnemySightTime = 0; //!< last time anyone in the squad saw the enemy

	EHL_FIELD(Persisted)
	bool m_fEnemyEluded = false;

	// squad member info
	EHL_FIELD(Persisted)
	int m_iMySlot = 0; //!< this is the behaviour slot that the monster currently holds in the squad. 

	bool CheckEnemy(CBaseEntity* pEnemy) override;
	void StartMonster() override;
	void VacateSlot();
	void ScheduleChange() override;
	void Killed(const KilledInfo& info) override;

	/**
	*	@brief if any slots of the passed slots are available, the monster will be assigned to one.
	*/
	bool OccupySlot(int iDesiredSlot);

	/**
	*	@brief checks for possibility of friendly fire
	*	@details Builds a large box in front of the grunt and checks to see if any squad members are in that box.
	*/
	bool NoFriendlyFire();

	// squad functions still left in base class
	CSquadMonster* MySquadLeader()
	{
		if (CSquadMonster* pSquadLeader = m_hSquadLeader; pSquadLeader != nullptr)
			return pSquadLeader;
		return this;
	}
	CSquadMonster* MySquadMember(int i)
	{
		if (i >= MAX_SQUAD_MEMBERS - 1)
			return this;
		else
			return m_hSquadMember[i];
	}
	bool InSquad() { return m_hSquadLeader != nullptr; }
	bool IsLeader() { return m_hSquadLeader == this; }

	/**
	*	@brief get some monsters of my classification and link them as a group.
	*	@return the group size
	*/
	int SquadRecruit(int searchRadius, int maxMembers);

	/**
	*	@brief return the number of members of this squad callable from leaders & followers
	*/
	int	SquadCount();

	/**
	*	@brief remove pRemove from my squad.
	*	If I am pRemove, promote m_pSquadNext to leader
	*/
	void SquadRemove(CSquadMonster* pRemove);

	/**
	*	@brief add pAdd to my squad
	*/
	bool SquadAdd(CSquadMonster* pAdd);

	/**
	*	@brief makes everyone in the squad angry at the same entity.
	*/
	void SquadMakeEnemy(CBaseEntity* pEnemy);

	/**
	*	@brief called by squad members that have current info on the enemy
	*	so that it can be stored for members who don't have current info.
	*/
	void SquadPasteEnemyInfo();

	/**
	*	@brief called by squad members who don't have current info on the enemy.
	*	Reads from the same fields in the leader's data that other squad members write to,
	*	so the most recent data is always available here.
	*/
	void SquadCopyEnemyInfo();

	/**
	*	@brief returns true if not all squad members are fighting the same enemy.
	*/
	bool SquadEnemySplit();
	bool SquadMemberInRange(const Vector& vecLocation, float flDist);

	CSquadMonster* MySquadMonsterPointer() override { return this; }

	/**
	*	@brief determines whether or not the chosen cover location is a good one to move to.
	*	(currently based on proximity to others in the squad)
	*/
	bool ValidateCover(const Vector& vecCoverLocation) override;

	NPCState GetIdealState() override;
	Schedule_t* GetScheduleOfType(int iType) override;
};

