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

#include "CBaseMonster.hpp"
#include "CTentacle.generated.hpp"

constexpr int ACT_T_IDLE = 1010;
constexpr int ACT_T_TAP = 1020;
constexpr int ACT_T_STRIKE = 1030;
constexpr int ACT_T_REARIDLE = 1040;

/**
*	@brief silo of death tentacle monster (half life)
*/
class EHL_CLASS() CTentacle : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	CTentacle();

	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;

	// Don't allow the tentacle to go across transitions!!!
	int	ObjectCaps() override { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	void SetObjectCollisionBox() override
	{
		pev->absmin = GetAbsOrigin() + Vector(-400, -400, 0);
		pev->absmax = GetAbsOrigin() + Vector(400, 400, 850);
	}

	void EXPORT Cycle();
	void EXPORT CommandUse(const UseInfo& info);
	void EXPORT Start();
	void EXPORT DieThink();

	void EXPORT Test();

	void EXPORT HitTouch(CBaseEntity* pOther);

	float HearingSensitivity() override { return 2.0; }

	bool TakeDamage(const TakeDamageInfo& info) override;
	void HandleAnimEvent(AnimationEvent& event) override;
	void Killed(const KilledInfo& info) override;

	NPCState GetIdealState() override { return NPCState::Idle; }
	//TODO: should override base, but has different signature
	bool CanPlaySequence(bool fDisregardState) { return true; }

	int Classify() override;

	int Level(float dz);
	int MyLevel();
	float MyHeight();

	EHL_FIELD(Persisted)
	float m_flInitialYaw = 0;

	EHL_FIELD(Persisted)
	int m_iGoalAnim = 0;

	EHL_FIELD(Persisted)
	int m_iLevel = 0;

	EHL_FIELD(Persisted)
	int m_iDir = 0;

	EHL_FIELD(Persisted)
	float m_flFramerateAdj = 0;

	EHL_FIELD(Persisted)
	float m_flSoundYaw = 0;

	EHL_FIELD(Persisted)
	int m_iSoundLevel = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flSoundTime = 0;

	EHL_FIELD(Persisted)
	float m_flSoundRadius = 0;

	EHL_FIELD(Persisted)
	int m_iHitDmg = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flHitTime = 0;

	EHL_FIELD(Persisted)
	float m_flTapRadius = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flNextSong = 0;

	static inline bool g_fFlySound = false;
	static inline bool g_fSquirmSound = false;

	EHL_FIELD(Persisted)
	float m_flMaxYaw = 0;

	EHL_FIELD(Persisted)
	int m_iTapSound = 0;

	EHL_FIELD(Persisted, Type=Position)
	Vector m_vecPrevSound;

	EHL_FIELD(Persisted, Type=Time)
	float m_flPrevSoundTime = 0;

	static const char* pHitSilo[];
	static const char* pHitDirt[];
	static const char* pHitWater[];
};
