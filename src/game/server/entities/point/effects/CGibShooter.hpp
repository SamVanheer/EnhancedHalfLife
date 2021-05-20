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

#include "CBaseDelay.hpp"
#include "CBreakable.hpp"

class CGib;

constexpr int SF_GIBSHOOTER_REPEATABLE = 1; //!< allows a gibshooter to be refired

class EHL_CLASS() CGibShooter : public CBaseDelay
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void EXPORT ShootThink();
	void Use(const UseInfo& info) override;

	virtual CGib* CreateGib();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	int	m_iGibs = 0;
	int m_iGibCapacity = 0;
	Materials m_iGibMaterial = Materials::Glass;
	int m_iGibModelIndex = 0;
	float m_flGibVelocity = 0;
	float m_flVariance = 0;
	float m_flGibLife = 0;
};
