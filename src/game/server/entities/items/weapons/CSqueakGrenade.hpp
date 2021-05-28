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
#include "CSqueakGrenade.generated.hpp"

enum w_squeak_e
{
	WSQUEAK_IDLE1 = 0,
	WSQUEAK_FIDGET,
	WSQUEAK_JUMP,
	WSQUEAK_RUN,
};

class EHL_CLASS() CSqueakGrenade : public CBaseMonster
{
	EHL_GENERATED_BODY()

	void Spawn() override;
	void Precache() override;
	int  Classify() override;
	void EXPORT SuperBounceTouch(CBaseEntity* pOther);
	void EXPORT HuntThink();
	int  BloodColor() override { return BLOOD_COLOR_YELLOW; }
	void Killed(const KilledInfo& info) override;
	void GibMonster() override;

	static inline float m_flNextBounceSoundTime = 0;

	// CBaseEntity *m_pTarget;
	EHL_FIELD(Persisted, Type=Time)
	float m_flDie = 0;

	EHL_FIELD(Persisted)
	Vector m_vecTarget;

	EHL_FIELD(Persisted, Type=Time)
	float m_flNextHunt = 0;

	EHL_FIELD(Persisted, Type=Time)
	float m_flNextHit = 0;

	EHL_FIELD(Persisted, Type=Position)
	Vector m_posPrev;

	EHL_FIELD(Persisted)
	EHANDLE m_hOwner;

	//Not saved, recalculated on demand
	int  m_iMyClass = 0;
};
