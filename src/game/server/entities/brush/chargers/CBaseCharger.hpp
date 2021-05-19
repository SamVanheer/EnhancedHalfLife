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

#include "CBaseToggle.hpp"

#include "CBaseCharger.generated.hpp"

enum class ChargerState
{
	Off,
	Starting,
	Charging
};

constexpr int CHARGER_INFINITE_CAPACITY = -1;
constexpr int CHARGER_NOT_INITIALIZED = -2;

/**
*	@brief If set, fire_on_recharge or fire_on_empty will be triggered on map spawn depending on initial capacity
*/
constexpr int SF_CHARGER_FIRE_ON_SPAWN = 1 << 0;

class EHL_CLASS() CBaseCharger : public CBaseToggle
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void Activate() override;
	void EXPORT Off();
	void EXPORT Recharge();
	void KeyValue(KeyValueData* pkvd) override;
	void Use(const UseInfo& info) override;
	int	ObjectCaps() override { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }

	void CheckIfOutOfCharge(bool fireTargets);

protected:
	[[nodiscard]] virtual float GetCurrentValue(CBaseEntity* target) = 0;
	[[nodiscard]] virtual bool SetCurrentValue(CBaseEntity* target, float value) = 0;

	/**
	*	@brief Gets the maximum value that the chargeable value can be
	*	@return -1 for no maximum, a positive value otherwise
	*/
	[[nodiscard]] virtual float GetMaximumValue(CBaseEntity* target) = 0;

	[[nodiscard]] virtual float GetDefaultRechargeDelay() = 0;

public:
	EHL_FIELD(Persisted, Type=Time)
	float m_flNextCharge = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flSoundTime = 0;

	EHL_FIELD(Persisted)
	ChargerState m_State = ChargerState::Off;

	/**
	*	@brief DeathMatch Delay until recharged
	*	-1 == use gamerules provided value, >= 0 == use mapper defined value
	*/
	EHL_FIELD(Persisted)
	float m_flRechargeDelay = -1;

	/**
	*	@brief Amount of charge to apply per use
	*/
	EHL_FIELD(Persisted)
	int m_iChargePerUse = 1;

	/**
	*	@brief Interval between charges
	*/
	EHL_FIELD(Persisted)
	float m_flChargeInterval = 0.1f;

	/**
	*	@brief CHARGER_NOT_INITIALIZED == use total capacity to initialize, >= CHARGER_INFINITE_CAPACITY == use mapper defined value
	*/
	EHL_FIELD(Persisted)
	int m_iCurrentCapacity = CHARGER_NOT_INITIALIZED;

	EHL_FIELD(Persisted)
	int m_iPitch = PITCH_NORM;

	//These should all be assigned to a sensible default by the derived class constructor
	//Overridable by mapper
	EHL_FIELD(Persisted)
	int m_iTotalCapacity = 0;

	EHL_FIELD(Persisted, Type = SoundName)
	string_t m_iszChargeOnSound = iStringNull;

	EHL_FIELD(Persisted, Type = SoundName)
	string_t m_iszChargeLoopSound = iStringNull;

	EHL_FIELD(Persisted, Type = SoundName)
	string_t m_iszRefuseChargeSound = iStringNull;

	EHL_FIELD(Persisted, Type=SoundName)
	string_t m_iszRechargeSound = iStringNull;

	EHL_FIELD(Persisted)
	string_t m_iszFireOnRecharge = iStringNull;

	EHL_FIELD(Persisted)
	string_t m_iszFireOnEmpty = iStringNull;
};
