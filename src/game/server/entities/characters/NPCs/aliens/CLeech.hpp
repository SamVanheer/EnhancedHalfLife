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
#include "CLeech.generated.hpp"

// Animation events
constexpr int LEECH_AE_ATTACK = 1;
constexpr int LEECH_AE_FLOP = 2;

// Movement constants

constexpr int LEECH_ACCELERATE = 10;
constexpr int LEECH_CHECK_DIST = 45;
constexpr int LEECH_SWIM_SPEED = 50;
constexpr int LEECH_SWIM_ACCEL = 80;
constexpr int LEECH_SWIM_DECEL = 10;
constexpr int LEECH_TURN_RATE = 90;
constexpr int LEECH_SIZEX = 10;
constexpr float LEECH_FRAMETIME = 0.1;

#define DEBUG_BEAMS		0

/**
*	@brief basic little swimming monster
*/
class EHL_CLASS() CLeech : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;

	void EXPORT SwimThink();
	void EXPORT DeadThink();
	void Touch(CBaseEntity* pOther) override
	{
		if (pOther->IsPlayer())
		{
			// If the client is pushing me, give me some base velocity
			if (gpGlobals->trace_ent && InstanceOrNull(gpGlobals->trace_ent) == this)
			{
				pev->basevelocity = pOther->GetAbsVelocity();
				pev->flags |= FL_BASEVELOCITY;
			}
		}
	}

	void SetObjectCollisionBox() override
	{
		pev->absmin = GetAbsOrigin() + Vector(-8, -8, 0);
		pev->absmax = GetAbsOrigin() + Vector(8, 8, 2);
	}

	void AttackSound();
	void AlertSound() override;
	void UpdateMotion();

	/**
	*	@brief returns normalized distance to obstacle
	*/
	float ObstacleDistance(CBaseEntity* pTarget);
	void MakeVectors();
	void RecalculateWaterlevel();
	void SwitchLeechState();

	// Base entity functions
	void HandleAnimEvent(AnimationEvent& event) override;
	int	BloodColor() override { return DONT_BLEED; }
	void Killed(const KilledInfo& info) override;
	void Activate() override;
	bool TakeDamage(const TakeDamageInfo& info) override;
	int	Classify() override { return CLASS_INSECT; }
	Relationship GetRelationship(CBaseEntity* pTarget) override;

	static const char* pAttackSounds[];
	static const char* pAlertSounds[];

private:
	// UNDONE: Remove unused boid vars, do group behavior
	EHL_FIELD(Persisted)
	float m_flTurning = 0;// is this boid turning?

	EHL_FIELD(Persisted)
	bool m_fPathBlocked = false;// true if there is an obstacle ahead

	EHL_FIELD(Persisted)
	float m_flAccelerate = 0;

	EHL_FIELD(Persisted)
	float m_obstacle = 0;

	EHL_FIELD(Persisted)
	float m_top = 0;

	EHL_FIELD(Persisted)
	float m_bottom = 0;

	EHL_FIELD(Persisted)
	float m_height = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_waterTime = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_sideTime = 0;		// Timer to randomly check clearance on sides

	EHL_FIELD(Persisted, Type=Time)
	float m_zTime = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_stateTime = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_attackSoundTime = 0;

#if DEBUG_BEAMS
	EHandle<CBeam> m_hb;
	EHandle<CBeam> m_ht;
#endif
};
