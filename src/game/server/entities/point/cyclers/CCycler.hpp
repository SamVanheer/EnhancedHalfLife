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

class EHL_CLASS() CCycler : public CBaseMonster
{
public:
	void GenericCyclerSpawn(const char* szModel, const Vector& vecMin, const Vector& vecMax);
	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE); }

	/**
	*	@brief changes sequences when shot
	*/
	bool TakeDamage(const TakeDamageInfo& info) override;
	void Spawn() override;
	void Think() override;
	//void Pain( float flDamage );

	/**
	*	@brief starts a rotation trend
	*/
	void Use(const UseInfo& info) override;

	// Don't treat as a live target
	bool IsAlive() override { return false; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	bool m_animate = false;
};
