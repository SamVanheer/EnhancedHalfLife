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
#include "CBaseMonster.monsters.hpp"

#include "CInfoBM.hpp"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
constexpr int BIG_AE_STEP1 = 1;			//!< Footstep left
constexpr int BIG_AE_STEP2 = 2;			//!< Footstep right
constexpr int BIG_AE_STEP3 = 3;			//!< Footstep back left
constexpr int BIG_AE_STEP4 = 4;			//!< Footstep back right
constexpr int BIG_AE_SACK = 5;			//!< Sack slosh
constexpr int BIG_AE_DEATHSOUND = 6;	//!< Death sound

constexpr int BIG_AE_MELEE_ATTACKBR = 8;	//!< Leg attack
constexpr int BIG_AE_MELEE_ATTACKBL = 9;	//!< Leg attack
constexpr int BIG_AE_MELEE_ATTACK1 = 10;	//!< Leg attack
constexpr int BIG_AE_MORTAR_ATTACK1 = 11;	//!< Launch a mortar
constexpr int BIG_AE_LAY_CRAB = 12;			//!< Lay a headcrab
constexpr int BIG_AE_JUMP_FORWARD = 13;		//!< Jump up and forward
constexpr int BIG_AE_SCREAM = 14;			//!< alert sound
constexpr int BIG_AE_PAIN_SOUND = 15;		//!< pain sound
constexpr int BIG_AE_ATTACK_SOUND = 16;		//!< attack sound
constexpr int BIG_AE_BIRTH_SOUND = 17;		//!< birth sound
constexpr int BIG_AE_EARLY_TARGET = 50;		//!< Fire target early

// User defined conditions
constexpr int bits_COND_NODE_SEQUENCE = bits_COND_SPECIAL1; 		// pev->netname contains the name of a sequence to play

// Attack distance constants
constexpr int BIG_ATTACKDIST = 170;
constexpr int BIG_MORTARDIST = 800;
constexpr int BIG_MAXCHILDREN = 20;			// Max # of live headcrab children


constexpr int bits_MEMORY_CHILDPAIR = bits_MEMORY_CUSTOM1;
constexpr int bits_MEMORY_ADVANCE_NODE = bits_MEMORY_CUSTOM2;
constexpr int bits_MEMORY_COMPLETED_NODE = bits_MEMORY_CUSTOM3;
constexpr int bits_MEMORY_FIRED_NODE = bits_MEMORY_CUSTOM4;

inline int gSpitSprite, gSpitDebrisSprite;

Vector CheckSplatToss(CBaseEntity* pEntity, const Vector& vecSpot1, const Vector& vecSpot2, float maxHeight);
void MortarSpray(const Vector& position, const Vector& direction, int spriteModel, int count);

// UNDONE:	
//
constexpr std::string_view BIG_CHILDCLASS{"monster_babycrab"};

class CBigMomma : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Activate() override;
	bool TakeDamage(const TakeDamageInfo& info) override;

	void		RunTask(Task_t* pTask) override;
	void		StartTask(Task_t* pTask) override;
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void		TraceAttack(const TraceAttackInfo& info) override;

	void NodeStart(string_t iszNextNode);
	void NodeReach();
	bool ShouldGoToNode();

	void SetYawSpeed() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;
	void LayHeadcrab();

	string_t GetNodeSequence()
	{
		CBaseEntity* pTarget = m_hTargetEnt;
		if (pTarget)
		{
			return pTarget->pev->netname;	// netname holds node sequence
		}
		return iStringNull;
	}

	string_t GetNodePresequence()
	{
		CInfoBM* pTarget = (CInfoBM*)m_hTargetEnt.Get();
		if (pTarget)
		{
			return pTarget->m_preSequence;
		}
		return iStringNull;
	}

	float GetNodeDelay()
	{
		CBaseEntity* pTarget = m_hTargetEnt;
		if (pTarget)
		{
			return pTarget->pev->speed;	// Speed holds node delay
		}
		return 0;
	}

	float GetNodeRange()
	{
		CBaseEntity* pTarget = m_hTargetEnt;
		if (pTarget)
		{
			return pTarget->pev->scale;	// Scale holds node delay
		}
		return 1e6;
	}

	float GetNodeYaw()
	{
		CBaseEntity* pTarget = m_hTargetEnt;
		if (pTarget)
		{
			if (pTarget->GetAbsAngles().y != 0)
				return pTarget->GetAbsAngles().y;
		}
		return GetAbsAngles().y;
	}

	// Restart the crab count on each new level
	void OverrideReset() override
	{
		m_crabCount = 0;
	}

	void DeathNotice(CBaseEntity* pChild) override;

	bool CanLayCrab()
	{
		if (m_crabTime < gpGlobals->time && m_crabCount < BIG_MAXCHILDREN)
		{
			// Don't spawn crabs inside each other
			Vector mins = GetAbsOrigin() - Vector(32, 32, 0);
			Vector maxs = GetAbsOrigin() + Vector(32, 32, 0);

			CBaseEntity* pList[2];
			int count = UTIL_EntitiesInBox(pList, ArraySize(pList), mins, maxs, FL_MONSTER);
			for (int i = 0; i < count; i++)
			{
				if (pList[i] != this)	// Don't hurt yourself!
					return false;
			}
			return true;
		}

		return false;
	}

	void LaunchMortar();

	void SetObjectCollisionBox() override
	{
		pev->absmin = GetAbsOrigin() + Vector(-95, -95, 0);
		pev->absmax = GetAbsOrigin() + Vector(95, 95, 190);
	}

	bool CheckMeleeAttack1(float flDot, float flDist) override;	//!< Slash
	bool CheckMeleeAttack2(float flDot, float flDist) override;	//!< Lay a crab
	bool CheckRangeAttack1(float flDot, float flDist) override;	//!< Mortar launch

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	static const char* pChildDieSounds[];
	static const char* pSackSounds[];
	static const char* pDeathSounds[];
	static const char* pAttackSounds[];
	static const char* pAttackHitSounds[];
	static const char* pBirthSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pFootSounds[];

	CUSTOM_SCHEDULES;

private:
	float m_nodeTime = 0;
	float m_crabTime = 0;
	float m_mortarTime = 0;
	float m_painSoundTime = 0;
	int m_crabCount = 0;
};
