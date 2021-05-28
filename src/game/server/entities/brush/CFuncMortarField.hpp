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
#include "CFuncMortarField.generated.hpp"

enum class MortarControlType
{
	Random,
	Activator,
	Table
};

/**
*	@brief the "LaBuznik" mortar device
*	Drop bombs from above
*/
class EHL_CLASS() CFuncMortarField : public CBaseToggle
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;

	// Bmodels don't go across transitions
	int	ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	/**
	*	@brief If connected to a table, then use the table controllers, else hit where the trigger is.
	*/
	void EXPORT FieldUse(const UseInfo& info);

	EHL_FIELD(Persisted)
	string_t m_iszXController = iStringNull;

	EHL_FIELD(Persisted)
	string_t m_iszYController = iStringNull;

	EHL_FIELD(Persisted)
	float m_flSpread = 0;

	EHL_FIELD(Persisted)
	int m_iCount = 0;

	EHL_FIELD(Persisted)
	MortarControlType m_fControl = MortarControlType::Random;
};
