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

#include "CBaseEntity.hpp"
#include "CBreakable.hpp"
#include "CGibShooter.generated.hpp"

class CGib;

constexpr int SF_GIBSHOOTER_REPEATABLE = 1; //!< allows a gibshooter to be refired

class EHL_CLASS() CGibShooter : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	Precache() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void EXPORT ShootThink();
	void Use(const UseInfo& info) override;

	virtual CGib* CreateGib();

	EHL_FIELD(Persisted)
	int	m_iGibs = 0;

	EHL_FIELD(Persisted)
	int m_iGibCapacity = 0;

	EHL_FIELD(Persisted)
	Materials m_iGibMaterial = Materials::Glass;

	EHL_FIELD(Persisted)
	int m_iGibModelIndex = 0;

	EHL_FIELD(Persisted)
	float m_flGibVelocity = 0;

	EHL_FIELD(Persisted)
	float m_flVariance = 0;

	EHL_FIELD(Persisted)
	float m_flGibLife = 0;
};
