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
#include "CGargantua.generated.hpp"

class CBeam;
class CSprite;

constexpr float GARG_ATTACKDIST = 80.0;

// Garg animation events
constexpr int GARG_AE_SLASH_LEFT = 1;
//constexpr int GARG_AE_BEAM_ATTACK_RIGHT = 2;	// No longer used
constexpr int GARG_AE_LEFT_FOOT = 3;
constexpr int GARG_AE_RIGHT_FOOT = 4;
constexpr int GARG_AE_STOMP = 5;
constexpr int GARG_AE_BREATHE = 6;

// Gargantua is immune to any damage but this
constexpr int GARG_DAMAGE = DMG_ENERGYBEAM | DMG_CRUSH | DMG_MORTAR | DMG_BLAST;
constexpr std::string_view GARG_EYE_SPRITE_NAME{"sprites/gargeye1.spr"};
constexpr std::string_view GARG_BEAM_SPRITE_NAME{"sprites/xbeam3.spr"};
constexpr std::string_view GARG_BEAM_SPRITE2{"sprites/xbeam3.spr"};
constexpr int GARG_FLAME_LENGTH = 330;
constexpr std::string_view GARG_GIB_MODEL{"models/metalplategibs.mdl"};

constexpr float ATTN_GARG = ATTN_NORM;

constexpr int STOMP_SPRITE_COUNT = 10;

inline int gStompSprite = 0, gGargGibModel = 0;

class EHL_CLASS(EntityName=monster_gargantua) CGargantua : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void OnRemove() override;
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify() override;
	bool TakeDamage(const TakeDamageInfo& info) override;
	void TraceAttack(const TraceAttackInfo& info) override;
	void HandleAnimEvent(AnimationEvent& event) override;

	bool CheckMeleeAttack1(float flDot, float flDist) override;		//!< Swipe
	bool CheckMeleeAttack2(float flDot, float flDist) override;		//!< Flame thrower madness!
	bool CheckRangeAttack1(float flDot, float flDist) override;		//!< Stomp attack
	void SetObjectCollisionBox() override
	{
		pev->absmin = GetAbsOrigin() + Vector(-80, -80, 0);
		pev->absmax = GetAbsOrigin() + Vector(80, 80, 214);
	}

	Schedule_t* GetScheduleOfType(int Type) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;

	void PrescheduleThink() override;

	void Killed(const KilledInfo& info) override;
	void DeathEffect();

	void EyeOff();
	void EyeOn(int level);
	void EyeUpdate();
	void StompAttack();
	void FlameCreate();
	void FlameUpdate();
	void FlameControls(float angleX, float angleY);
	void FlameDestroy();
	inline bool FlameIsOn() { return m_hFlame[0] != nullptr; }

	void FlameDamage(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pInflictor, CBaseEntity* pAttacker, float flDamage, int iClassIgnore, int bitsDamageType);

	CUSTOM_SCHEDULES;

private:
	static const char* pAttackHitSounds[];
	static const char* pBeamAttackSounds[];
	static const char* pAttackMissSounds[];
	static const char* pRicSounds[];
	static const char* pFootSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pAttackSounds[];
	static const char* pStompSounds[];
	static const char* pBreatheSounds[];

	/**
	*	@brief expects a length to trace, amount of damage to do, and damage type.
	*	Used for many contact-range melee attacks. Bites, claws, etc.
	*	@return a pointer to the damaged entity in case the monster wishes to do other stuff to the victim (punchangle, etc)
	*	@details Overridden for Gargantua because his swing starts lower as a percentage of his height
	*	(otherwise he swings over the players head)
	*/
	CBaseEntity* GargantuaCheckTraceHullAttack(float flDist, int iDamage, int iDmgType);

	EHL_FIELD(Persisted)
	EHandle<CSprite> m_hEyeGlow;		// Glow around the eyes

	EHL_FIELD(Persisted)
	EHandle<CBeam> m_hFlame[4];		// Flame beams

	EHL_FIELD(Persisted)
	int m_eyeBrightness = 0;	// Brightness target

	EHL_FIELD(Persisted, Type=Time)
	float m_seeTime = 0;			// Time to attack (when I see the enemy, I set this)

	EHL_FIELD(Persisted, Type=Time)
	float m_flameTime = 0;		// Time of next flame attack

	EHL_FIELD(Persisted, Type=Time)
	float m_painSoundTime = 0;	// Time of next pain sound

	EHL_FIELD(Persisted, Type=Time)
	float m_streakTime = 0;		// streak timer (don't send too many)

	EHL_FIELD(Persisted)
	float m_flameX = 0;			// Flame thrower aim

	EHL_FIELD(Persisted)
	float m_flameY = 0;
};
