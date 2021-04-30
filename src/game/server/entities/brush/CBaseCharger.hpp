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

class CBaseCharger : public CBaseToggle
{
public:
	void Spawn() override;
	void Precache() override;
	void Activate() override;
	void EXPORT Off();
	void EXPORT Recharge();
	void KeyValue(KeyValueData* pkvd) override;
	void Use(const UseInfo& info) override;
	int	ObjectCaps() override { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	void CheckIfOutOfCharge(bool fireTargets);

protected:
	virtual [[nodiscard]] float GetCurrentValue(CBaseEntity* target) = 0;
	virtual [[nodiscard]] bool SetCurrentValue(CBaseEntity* target, float value) = 0;

	/**
	*	@brief Gets the maximum value that the chargeable value can be
	*	@return -1 for no maximum, a positive value otherwise
	*/
	virtual [[nodiscard]] float GetMaximumValue(CBaseEntity* target) = 0;

	virtual [[nodiscard]] float GetDefaultRechargeDelay() = 0;

public:
	static TYPEDESCRIPTION m_SaveData[];

	float m_flNextCharge = 0;
	float m_flSoundTime = 0;

	ChargerState m_State = ChargerState::Off;

	/**
	*	@brief DeathMatch Delay until recharged
	*	-1 == use gamerules provided value, >= 0 == use mapper defined value
	*/
	float m_flRechargeDelay = -1;

	/**
	*	@brief Amount of charge to apply per use
	*/
	int m_iChargePerUse = 1;

	/**
	*	@brief Interval between charges
	*/
	float m_flChargeInterval = 0.1f;

	/**
	*	@brief CHARGER_NOT_INITIALIZED == use total capacity to initialize, >= CHARGER_INFINITE_CAPACITY == use mapper defined value
	*/
	int m_iCurrentCapacity = CHARGER_NOT_INITIALIZED;

	int m_iPitch = PITCH_NORM;

	//These should all be assigned to a sensible default by the derived class constructor
	//Overridable by mapper
	int m_iTotalCapacity = 0;

	string_t m_iszChargeOnSound = iStringNull;
	string_t m_iszChargeLoopSound = iStringNull;
	string_t m_iszRefuseChargeSound = iStringNull;
	string_t m_iszRechargeSound = iStringNull;

	string_t m_iszFireOnRecharge = iStringNull;
	string_t m_iszFireOnEmpty = iStringNull;
};
